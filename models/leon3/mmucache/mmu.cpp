// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file mmu.cpp
/// Implementation of a memory management unit. The mmu can be configured to
/// have split or combined TLBs for instructions and data. The TLB size can be
/// configured as well. The memory page size is currently currently fixed to
/// 4kB.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include "core/models/leon3/mmucache/mmu.h"
#include "core/common/verbose.h"
#include "core/models/extern/LEON3/simulatorSources/leon3.funclt/instructions.hpp"

mmu::mmu(ModuleName name, // sysc module name,
         mmu_cache_if * _mmu_cache,    // pointer to memory interface
         unsigned int itlbnum,         // number of instruction tlbs
         unsigned int dtlbnum,         // number of data tlbs
         unsigned int tlb_type,        // tlb type
         unsigned int tlb_rep,         // tlb replacement strategy
         unsigned int mmupgsz,         // mmu page size
	 bool pow_mon) :               // power monitoring on/off
            sc_module(name),
            m_mmu_cache(_mmu_cache),
	    m_itlbnum(itlbnum),
	    m_dtlbnum(dtlbnum),
	    m_itlblog2((unsigned int)log2((double)m_itlbnum)),
	    m_dtlblog2((unsigned int)log2((double)m_dtlbnum)),
	    m_tlb_type(tlb_type),
	    m_tlb_rep(tlb_rep),
            m_mmupgsz(mmupgsz),
	    m_pseudo_rand(0),
	    m_pow_mon(pow_mon),
            m_performance_counters("performance_counters"),
            tihits("instruction_tlb_hits", 8, m_performance_counters),
            tdhits("data_tlb_hits", 8, m_performance_counters),
            timisses("instruction_tlb_misses", 0ull, m_performance_counters),
            tdmisses("data_tlb_misses", 0ull, m_performance_counters),
            sta_power_norm("power.mmu_cache.mmu.sta_power_norm", 7.19e+7, true), // Normalized static power of controller
            int_power_norm("power.mmu_cache.mmu.int_power_norm", 3.74e-8, true), // Normalized static power of controller
            sta_tlb_power_norm("power.mmu_cache.mmu.tlb_power_norm", 6543750, true), // Normalized static power of tlb
            int_tlb_power_norm("power.mmu_cache.mmu.tlb_power_norm", 2.7225e-9, true), // Normalized internal power of tlb
            dyn_tlb_read_energy_norm("power.mmu_cache.mmu.dyn_tlb_read_energy_norm", 1.08125e-11, true), // Normalized read energy of tlb
            dyn_tlb_write_energy_norm("power.mmu_cache.mmu.dyn_tlb_write_energy_norm", 1.08125e-11, true), // Normalized write energy of tlb
            power("power"),
            sta_power("sta_power", 0.0, power), // Static power of mmu
            int_power("int_power", 0.0, power), // Internal power of mmu
            swi_power("swi_power", 0.0, power), // Switching power of mmu
            power_frame_starting_time("power_frame_starting_time", SC_ZERO_TIME, power),
            itlbram("itlb", power), // Parameter array for itlb power out parameters
            dyn_itlb_read_energy("dyn_itlb_read_energy", 0.0, itlbram), // itlb read energy
            dyn_itlb_write_energy("dyn_itlb_write_energy", 0.0, itlbram), // itlb write energy
            dyn_itlb_reads("dyn_itlb_reads", 0ull, itlbram), // number of itlb reads
            dyn_itlb_writes("dyn_itlb_writes", 0ull, itlbram), // number of itlb writes
            dtlbram("dtlb", power),
            dyn_dtlb_read_energy("dyn_dtlb_read_energy", 0.0, dtlbram), // dtlb read energy
            dyn_dtlb_write_energy("dyn_dtlb_write_energy", 0.0, dtlbram), // dtlb write energy
            dyn_dtlb_reads("dyn_dtlb_reads", 0ull, dtlbram), // number of dtlb reads
            dyn_dtlb_writes("dyn_dtlb_writes", 0ull, dtlbram), // number of dtlb writes
	    clockcycle(10, sc_core::SC_NS) {

    // The number of instruction and data tlbs must be in the range of 2-32
    assert((m_itlbnum>=2)&&(m_itlbnum<=32));
    assert((m_dtlbnum>=2)&&(m_dtlbnum<=32));

    m_api = gs::cnf::GCnf_Api::getApiInstance(this);

    // initialize MMU control register (ITLB, DTLB)
    MMU_CONTROL_REG = 0;
    MMU_CONTROL_REG = ((1 << 24) | (m_itlblog2 << 21) | (m_dtlblog2 << 18));

    MMU_CONTEXT_TABLE_POINTER_REG = 0;
    MMU_CONTEXT_REG = 0;
    MMU_FAULT_STATUS_REG = 0;
    MMU_FAULT_ADDRESS_REG = 0;

    // generate associative memory (map) for instruction tlb
    itlb = new std::map<t_VAT, t_PTE_context>;
    itlb_adaptor = new tlb_adaptor("itlb_adaptor", _mmu_cache, this, itlb,
            m_itlbnum);

    // are we in split tlb mode?
    if (m_tlb_type == 0x0) {

        // generate another associative memory (map) for data tlb
        dtlb = new std::map<t_VAT, t_PTE_context>;
        dtlb_adaptor = new tlb_adaptor("dtlb_adaptor", _mmu_cache, this, dtlb,
                m_dtlbnum);

        // update MMU control register (ST)
        MMU_CONTROL_REG |= (1 << 14);

        v::debug << this->name() << "Created split instruction and data TLBs."
                << v::endl;

    } else {

        // combined tlb mode -> share instruction tlb
        dtlb = itlb;
        dtlb_adaptor = itlb_adaptor;

        v::debug << this->name()
                << "Created combined instruction and data TLBs." << v::endl;

    }

    // The page size can be 4k, 8k, 16k or 32k.
    // Depending on the configuration the indices for the address table
    // lookup have different range.

    switch (mmupgsz) {

        case 0: // 4 kB
            m_idx1 = 8;
            m_idx2 = 6;
            m_idx3 = 6;
            m_vtag_width = 20;
            break;
        case 1: // 8 kB
            m_idx1 = 7;
            m_idx2 = 6;
            m_idx3 = 6;
            m_vtag_width = 19;
            // Update MMU control register (PSZ)
            MMU_CONTROL_REG |= (1 << 16);
            break;
        case 2: // 16 kB
            m_idx1 = 6;
            m_idx2 = 6;
            m_idx3 = 6;
            m_vtag_width = 18;
            // Update MMU control register (PSZ)
            MMU_CONTROL_REG |= (2 << 16);
            break;
        case 3: // 32 kB
            m_idx1 = 4;
            m_idx2 = 7;
            m_idx3 = 6;
            m_vtag_width = 17;
            // Update MMU control register (PSZ)
            MMU_CONTROL_REG |= (3 << 16);
            break;
        default: // not supported
            v::error << this->name() << "Selected mmupgsz not supported!"
                    << v::endl;
            assert(false);
    }

    // Register for power monitoring
    //PM::registerIP(this,"mmu",m_pow_mon);
    //PM::send_idle(this,"idle",sc_time_stamp(),m_pow_mon);

    // Init execution statistic
    for (uint32_t i=0; i<8; i++) {
      tihits[i] = 0;
      tdhits[i] = 0;
    }

    timisses = 0;
    tdmisses = 0;

    // Register power callback functions
    if (m_pow_mon) {

      GC_REGISTER_TYPED_PARAM_CALLBACK(&sta_power, gs::cnf::pre_read, mmu, sta_power_cb);
      GC_REGISTER_TYPED_PARAM_CALLBACK(&int_power, gs::cnf::pre_read, mmu, int_power_cb);
      GC_REGISTER_TYPED_PARAM_CALLBACK(&swi_power, gs::cnf::pre_read, mmu, swi_power_cb);

    }

    unsigned tmp_access_table[8][8] = {
      { 0, 0, 0, 0, 2, 0, 3, 3 },
      { 0, 0, 0, 0, 2, 0, 0, 0 },
      { 2, 2, 0, 0, 0, 2, 3, 3 },
      { 2, 2, 0, 0, 0, 2, 0, 0 },
      { 2, 0, 2, 0, 2, 2, 3, 3 },
      { 2, 0, 2, 0, 2, 0, 2, 0 },
      { 2, 2, 2, 0, 2, 2, 3, 3 },
      { 2, 2, 2, 0, 2, 2, 2, 0 } };
    memcpy( this->access_table, tmp_access_table, sizeof(unsigned)*8*8 ); // ugly

    // Configuration report
    v::info << this->name() << " ******************************************************************************* " << v::endl;
    v::info << this->name() << " * Created mmu with following parameters: " << v::endl;
    v::info << this->name() << " * -------------------------------------- " << v::endl;
    v::info << this->name() << " * itlbnum: " << m_itlbnum << v::endl;
    v::info << this->name() << " * dtlbnum: " << m_dtlbnum << v::endl;
    v::info << this->name() << " * tlb_type (0 - split, 1 - shared): " << m_tlb_type << v::endl;
    v::info << this->name() << " * tlb_rep: " << tlb_rep << v::endl;
    v::info << this->name() << " * mmupgsz (0, 2 - 4kb, 3 - 8kb, 4 - 16kb, 5 - 32kb): " << mmupgsz << v::endl;
    v::info << this->name() << " * pow_mon: " << m_pow_mon << v::endl;
    v::info << this->name() << " * ***************************************************************************** " << v::endl;
}

// Destructor
mmu::~mmu() {

  GC_UNREGISTER_CALLBACKS();

}


signed mmu::get_physical_address( uint64_t * paddr, signed * prot, unsigned * access_index,
                                  uint64_t vaddr, int asi, uint64_t * page_size,
                                  unsigned * debug, bool is_dbg, sc_core::sc_time * t, unsigned is_write, unsigned * pde_REMOVE ) {

    signed access_perms = 0, error_code = 0, is_dirty, is_user;
    unsigned pde_ptr, pde;
    bool cacheable_mem;

    // According to the SparcV8 Manual: Pages of the Reference MMU are always aligned on 4K-byte boundaries; hence, the lower-order
    // 12 bits of a physical address are always the same as the low-order 12 bits of
    // the virtual address. The Gaisler MMU additionally supports 8k, 16k and 32k alignment,
//    unsigned int offset = ((addr << m_vtag_width) >> m_vtag_width);
    t_VAT vpn = (vaddr >> (32 - m_vtag_width));

    // The Virtual Address Tag consists of three indices, which are used to look
    // up the three level page table in main memory. The indices are extracted from
    // the tag and translated to byte addresses (shift left 2 bit)
    //unsigned int idx1 = (vpn >> (m_idx2 + m_idx3) << 2);
    //unsigned int idx2 = (vpn << (32 - m_idx2 - m_idx3)) >> (30 - m_idx3);
    //unsigned int idx3 = (vpn << (32 - m_idx3)) >> (30 - m_idx3);
    //unsigned int idx1 = (vpn >> 12);
    //unsigned int idx2 = (vpn << 6) & 0x3e;
    //unsigned int idx3 = vpn & 0x3e;
    v::debug << this->name() << "VPN: " << hex << vpn << " vtag_width " << hex << m_vtag_width<< endl;
    unsigned int idx1 = (vpn >> 12) & 0xff;
    unsigned int idx2 = (vpn >> 6) & 0x3f;
    unsigned int idx3 = vpn & 0x3f;


    // User access:       ASI = 0x8 or 0xA
    // Supervisor access: ASI = 0x9 or 0xB
    is_user = ! (asi & 0x1);
    // Instruction:       ASI = 0x8 or 0x9
    // Data:              ASI = 0xA or 0xB
    unsigned is_instruction_access = ! (asi & 0x2);

/*  // --> handled within tlb_adaptor.h
    if( ! (MMU_CONTROL_REG & 0x1)) {
        *paddr = vaddr;
        return 0;
    }
*/
    /*
       AT
        0  - ld/st? -> data/inst? -> user/priv? -> Load from User Data Space
        1           |             |             \> Load from Supervisor Data Space
        2           |             -> user/priv? -> Load/Execute from User Instruction Space
        3           |                           \> Load/Execute from Supervisor Instruction Space
        4           -> data/inst? -> user/priv? -> Store to User Data Space
        5                         |             \> Store to Supervisor Data Space
        6                         -> user/priv? -> Store to User Instruction Space
        7                                       \> Store to Supervisor Instruction Space
    */
    *access_index = (is_write << 2) | (is_instruction_access << 1) | (! is_user) ; // AT (Access Type)
    *paddr = 0xffffffffffff0000ULL; // has size of 36bits!

    // COMPOSITION OF A PAGE TABLE DESCRIPTOR (PTD)
    // [31-2] PTP - Page Table Pointer. Physical address of the base of a next-level
    // page table. The PTP appears on bits 35 (32) through 6 of the physical address bus
    // during miss processing. The page table pointed to by a PTP must be aligned on a
    // boundary equal to the size of the page table. The sizes of the three levels of
    // page tables are: level 1 - 1024 bytes, level 2 - 256 bytes, level 3 - 256 bytes.
    // [1-0] ET - Entry type. (0 - reserved, 1 - PTD, 2 - PTE, 3 - reserved)

    // COMPOSITION OF A PAGE TABLE ENTRY (PTE)
    // [31-8] PPN - Physical Page Number. The PPN appears on bits 35 through 12 of the
    // physical address bus when a translation completes (for 4kb default configuration)
    // [7] C - Cacheable. If this bit is one, the page is cacheable by an instruction
    // and/or data cache.
    // [6] M - Modified. This bit is set to one by the MMU when the page is accessed
    // for writing (except when accessed via passthrough ASI).
    // [5] R - Referenced. this bit is set to one by the MMU when the page is accessed
    // (except when accessed via passthrough ASI)
    // [4-2] Access Permissions (see page 248 of Sparc Reference Manual)
    // [1-0] ET - Entry type. (0 - reserved, 1 - PTD, 2 - PTE, 3 - reserved)

    // tlb miss processing
    v::debug << this->name()
            << "START TLB MISS PROCESSING FOR VIRTUAL ADDRESS: " << std::hex
            << vaddr << " with indices 1/2/3: " << idx1 << "/" << idx2 << "/" << idx3 << " for context: " << MMU_CONTEXT_REG << v::endl;

    // **************************************
    // Context-Table lookup
    // **************************************

    pde_ptr = (MMU_CONTEXT_TABLE_POINTER_REG << 4) + (MMU_CONTEXT_REG << 2);
    m_mmu_cache->mem_read(pde_ptr, 0x8, (unsigned char *)&pde, 4, t, debug, is_dbg, cacheable_mem,  false);
    #ifdef LITTLE_ENDIAN_BO
    swap_Endianess(pde);
    #endif
    *pde_REMOVE = pde;

    *page_size = 0x0; // -1 -> 0xFFFFFFFFFFFFFFFF (uint64_t) :-)
    switch( pde & 0x3 ) {
    default:
    case 0: // Invalid
      return 1 << 2;                                        // L: Level 0, FT: Invalid address error
    case 2: // L0 PTE -> shouldn't possibly not happen
    case 3: // Reserved
      return 4 << 2;                                        // L: Level 0, FT: Translation error
    
    case 1: // 1. load from 1st-level page table
        pde_ptr = ((pde & ~3) << 4)+(idx1 << 2);
        m_mmu_cache->mem_read(pde_ptr, 0x8, (unsigned char *)&pde, 4, t, debug, is_dbg, cacheable_mem, false);
        #ifdef LITTLE_ENDIAN_BO
        swap_Endianess(pde);
        #endif
        *pde_REMOVE = pde;

        switch( pde & 0x3 ) {
        case 0: // Invalid
            return (1 << 8) | (1 << 2);                     // L: Level 1, FT: Invalid address error
        case 3: // Reserved
            return (1 << 8) | (4 << 2);                     // L: Level 1, FT: Translation error

        default:
        case 1: // 2. load from 2nd-level page table
            pde_ptr = (((pde & ~0x3) << 4) + (idx2 << 2));
            m_mmu_cache->mem_read( pde_ptr, 0x8, (unsigned char *)&pde, 4, t, debug, is_dbg, cacheable_mem, false);
            #ifdef LITTLE_ENDIAN_BO
            swap_Endianess(pde);
            #endif
            *pde_REMOVE = pde;

            switch( pde & 0x3 ) {
            default:
            case 0: // Invalid
                return (2 << 8) | (1 << 2);                 // L: Level 2, FT: Invalid address error
            case 3: // Reserved
                return (2 << 8) | (4 << 2);                 // L: Level 2, FT: Translation error

            case 1: // 3. load from 3rd-level page table
                pde_ptr = (((pde & ~0x3) << 4) + (idx3<<2));
                m_mmu_cache->mem_read( pde_ptr, 0x8, (unsigned char *)&pde, 4, t, debug, is_dbg, cacheable_mem, false);
                #ifdef LITTLE_ENDIAN_BO
                swap_Endianess(pde);
                #endif
                *pde_REMOVE = pde;

                switch( pde & 0x3 ) {
                case 0: // Invalid
                    return (3 << 8) | (1 << 2);             // L: Level 3, FT: Invalid address error
                case 1: // PDE -> should not happen
                case 3: // Reserved
                    return (3 << 8) | (4 << 2);             // L: Level 3, FT: Translation error
                case 2:
                    break;
                }
                *page_size = 0x1000;
                break;
            case 2:
                *page_size = 0x40000;
            }
            break;
        case 2:
            *page_size = 0x1000000;
        }
    }

    // Page 257: FT -> PTE.V should be "PTE valid" --> check access (FT)
    access_perms = (pde >> 0x2) & 0x7;
    error_code = this->access_table[ *access_index ][ access_perms ] << 2; // FT -> depends on table (see page 257: FT second table)

    signed h;
    if( ! error_code ) {
/*    is_dirty = (rw & 1) && !(pde & PG_MODIFIED_MASK); // 0x6
    if (!(pde & PG_ACCESSED_MASK) || is_dirty) { // 0x5
        pde |= PG_ACCESSED_MASK;
        if (is_dirty) {
            pde |= PG_MODIFIED_MASK;
        }
        stl_phys_notdirty(cs->as, pde_ptr, pde);
    }
*/
        /* update page modified and dirty bits */
        is_dirty = ( is_write );
        if( is_dirty )    
            pde |= (0x1 << 6); // Set Modified Bit
        
        if( is_dirty || !( pde & (0x1 << 5) ) ) // if it got dirty, or has not been accessed yet -> referenced!
            pde |= (0x1 << 5); // Set Referenced Bit


        // READ = 0x1, WRITE = 0x2, EXECUTE = 0x4
        signed access_perms2[2][8] = {
            { 0x1,
              0x1 | 0x2,
              0x1 | 0x4,
              0x1 | 0x2 | 0x4,
              0x4,
              0x1,
              0x0,
              0x0
            },
            { 0x1,
              0x1 | 0x2,
              0x1 | 0x4,
              0x1 | 0x2 | 0x4,
              0x4,
              0x1 | 0x2,
              0x1 | 0x4,
              0x1 | 0x2 | 0x4
            }
        };
/*
    READ          = ! is_write
    WRITE         = is_write
    EXECUTE       = is_instruction ( && ! is_write ? )
    Supervisor    = ! is_user
    User          = is_user
*/
        h = access_perms2[ ( is_user ) ? 0x0 : 0x1 ][ (pde >> 0x2) & 0x7 ];
    }

    *paddr = ((pde & ~0xFF) << 4 | (vaddr & ((*page_size) - 1)));
    *paddr &= (((uint64_t)1 << 36) - 1);

    if( ! error_code ) {
        if( (! (h & 0x1)) && ! is_write )
            std::cout << "--------------> no READ allowed!  " << std::hex << *paddr << std::endl;

        if( (!(h & 0x2)) && is_write )
            std::cout << "--------------> no WRITE allowed!" << std::hex << *paddr << " " << error_code << std::endl;

        if( (!(h & 0x4)) && is_instruction_access )
            std::cout << "no EXECUTE allowed!" << std::endl;
    }
/*
instruction_access_exception tt=0x01
    A blocking error exception occurred on an instruction access (for example,
    an MMU indicated that the page was invalid or read-protected).
privileged_instruction tt=0x03
    An attempt was made to execute a privileged instruction while S = 0
data_access_exception
    A blocking error exception occurred on a load/store data access. (for exam-
    ple, an MMU indicated that the page was invalid or write-protected).
*/

    return error_code;
}


// look up a tlb (page descriptor cache)
// and return physical address
signed mmu::tlb_lookup(unsigned int addr, unsigned asi,
                             std::map<t_VAT, t_PTE_context> * tlb,
                             unsigned int tlb_size, sc_core::sc_time * t,
                             unsigned int * debug, bool is_dbg, bool &cacheable,
                             unsigned is_write /* LOAD / STORE? */, uint64_t * paddr ) {
/*
    Appendix I - Table I-1
    The MMU will only receive the ASIs as follows: 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0x20-0x2F
    Out of those, only the ASIs 0x8, 0x9, 0xA, 0xB will enter this routine!
*/

    // According to the SparcV8 Manual: Pages of the Reference MMU are always aligned on 4K-byte boundaries; hence, the lower-order
    // 12 bits of a physical address are always the same as the low-order 12 bits of
    // the virtual address. The Gaisler MMU additionally supports 8k, 16k and 32k alignment,
//    unsigned int offset = ((addr << m_vtag_width) >> m_vtag_width);
    t_VAT vpn = (addr >> (32 - m_vtag_width));

    
    // Locals for intermediate results
    t_PTE_context tmp;
    *paddr = 0xffffffffffff0000ULL; // has size of 36bits!
    unsigned int pde;

/*
    // Search virtual address tag in ipdc (associative)
    v::debug << this->name() << "Access with ADDR: " << std::hex << addr << " VPN: " << std::hex << vpn
            << " and OFFSET: " << std::hex << offset << v::endl;
    pdciter = tlb->find(vpn);

    // Log tlb reads for power monitoring
    if (m_pow_mon) {
      // All tlbs are read in parallel !
      if (tlb == itlb) {
        dyn_itlb_reads += tlb_size;
      } else {
        dyn_dtlb_reads += tlb_size;
      }
    }

    // TLB hit
    if (pdciter != tlb->end()) {

      // Read the PDC entry
      tmp = pdciter->second;

      v::debug << this->name() << "Virtual Address Tag Hit in TLB " << tmp.tlb_no << " for address: "
                << hex << addr << v::endl;

        // Check the context tag
        if (tmp.context == MMU_CONTEXT_REG) {

            v::debug << this->name() << "CONTEXT hit " << hex << MMU_CONTEXT_REG << v::endl;

            // Build physical address from PTE and offset, and return
            //paddr = (((tmp.pte >> 8) << (32 - m_vtag_width)) | (addr & (tmp.page_size -1)));
            *paddr = ((tmp.pte & ~0xff) << 4 | (addr & (tmp.page_size - 1)));
            *paddr &= ((0x1ull << 36) - 1);
            if ((tmp.pte & (1<<7)) == 0) {
              v::debug << this->name() << "data not cacheable!" << v::endl;
              cacheable = false;
            } else {
              cacheable = true;
            }

            // Update debug information
            TLBHIT_SET(*debug);

            if (tlb == itlb) {
                tihits[tmp.tlb_no]++;
            } else {
                tdhits[tmp.tlb_no]++;
            }

            // Update LRU history
            if (m_tlb_rep==0) {
                lru_update(vpn, tlb, tlb_size);
            }
            return 0;
        } else {

            v::debug << this->name() << "CONTEXT miss" << v::endl;

            // Update debug information
            TLBMISS_SET(*debug);

            if (tlb == itlb) {
                timisses++;
            } else {
	        tdmisses++;
            }
        }
    } else {

        v::debug << this->name() << "Virtual Address Tag miss" << v::endl;

        // Update debug information
        TLBMISS_SET(*debug);

	      if (tlb == itlb) {
	        timisses++;
	      } else {
	        tdmisses++;
	      }
    }

*/



    
    uint64_t page_size;
    unsigned access_index;
    signed error_code = get_physical_address( paddr, NULL, &access_index,
                                  addr, asi, &page_size,
                                  debug, is_dbg, t, is_write, &pde );

    if( error_code ) {
        v::error << this->name()
                 << "In " << ((error_code >> 8) & 0x3) << "-Level Page Table / Entry type not valid: "
                 << " VA: " << v::uint32 << addr << ", is " << ( is_write ? "write" : "load" )
                 << " and " << ((! (asi & 0x2)) ? "instruction" : "data" ) << v::endl;

       sleep(1);

        // Set fault status and fault address
        // (will be set 0, if read, otherwise set OW bit)
        if( ! MMU_FAULT_STATUS_REG )
            MMU_FAULT_STATUS_REG = 1;               // OW - Overwrite bit
        // ToDo: Fault Priorities due to overwritting!
        MMU_FAULT_STATUS_REG |= access_index << 5;  // AT - Access Type
        MMU_FAULT_STATUS_REG |= error_code;         // provides L and FT


        // Instruction:       ASI = 0x8 or 0x9
        // Data:              ASI = 0xA or 0xB
        unsigned is_instruction_access = ! (asi & 0x2);
        // The Fault Address Valid bit is set to one if the contents of the Fault
        // Address Register are valid. The Fault Address Register need not be valid
        // for instruction faults. The Fault Address Register must be valid for data
        // faults and translation errors.
        if( ! is_instruction_access )
            MMU_FAULT_STATUS_REG |= 1 << 1;             // FAV - Fault Address Register valid
        
        MMU_FAULT_ADDRESS_REG = addr;


        // NF is the “No Fault” bit. When NF= 0, any fault detected by the MMU
        // causes FSR and FAR to be updated and causes a fault to be generated to
        // the processor. When NF= 1, a fault on an access to ASI 9 is handled as
        // when NF= 0; a fault on an access to any other ASI causes FSR and FAR
        // to be updated but no fault is generated to the processor.

        // If a fault on access to an ASI other than 9 occurs while NF= 1, subsequently
        // resetting NF from 1 to 0 does not cause a fault to the processor
        // (even though FSR.FT ≠ 0 at that time).
        // A change in value of the NF bit takes effect as soon as the bit is written;
        // a subsequent access to ASI 9 will be evaluated according to the new
        // value of the NF bit.
        if( ! ( MMU_CONTROL_REG & 0x2 ) || ((MMU_CONTROL_REG & 0x2) && asi == 0x9 ) ) {
/*          according to instructions.hpp ... not sure why ... doesn't make sense according to sparc manual: Chapter 7 Table 7-1

            #define INSTR_ACCESS_MMU_MISS 2
            #define INSTR_ACCESS_ERROR 3
            #define INSTR_ACCESS_EXC 5

            #define DATA_ACCESS_ERROR 17
            #define DATA_ACCESS_MMU_MISS 18
            #define DATA_ACCESS_EXC 19
*/
            if( is_instruction_access ) {
                v::error << this->name() << "Trap encountered (instruction_access_exception) tt = 0x01" << v::endl;
                m_mmu_cache->trigger_exception( INSTR_ACCESS_EXC );
            }
            else {
                v::error << this->name() << "Trap encountered (data_access_exception) tt = 0x09" << v::endl;
                m_mmu_cache->trigger_exception( DATA_ACCESS_EXC );
            }
            return -1;
        }
    }

    switch (pde & 0x3) {
    case 0x2:
        v::debug << this->name() << ((error_code >> 8) & 0x3) << "-Level Page Table returned PTE: "
                << std::hex << pde << v::endl;

        // In case of a virtual address tag miss a new PDC entry is created.
        // For context miss the existing entry will be replaced.
        if (tlb->size() == tlb_size) {
            v::debug << this->name() << "TLB full" << std::hex << pde  << v::endl;

            // Remove a TLB entry, with respect to replacement strategy
            tmp.tlb_no = tlb_remove(tlb, tlb_size);
        } else {
            v::debug << this->name() << "Create new entry PDC entry - TLB number: " << tlb->size() << v::endl;
            tmp.tlb_no = tlb->size();
        }

        // add to PDC
        tmp.context = MMU_CONTEXT_REG;
        tmp.pte = pde;
        tmp.lru = 0xffffffffffffffff - 1;
        tmp.page_size = page_size;

        (*tlb)[vpn] = tmp;

        // Log TLB writes for power monitoring
        if (m_pow_mon) {
          if (tlb == itlb) {
            dyn_itlb_writes++;
          } else {
            dyn_dtlb_writes++;
          }
        }

        // build physical address from PTE and offset
        *paddr = ((pde & ~0xFF) << 4 | (addr & (page_size - 1)));
        *paddr &= (((uint64_t)1 << 36) - 1);

        if ((pde & (1<<7)) == 0) {
          v::debug << this->name() << "data not cacheable!" << v::endl;
          cacheable = false;
        } else {
          cacheable = true;
        }

        v::debug << this->name() << "Mapping complete - Virtual Addr: "
                << std::hex << addr << " Physical Addr: " << std::hex << paddr
                << v::endl;
        return 0;

    case 0x1:
        if( ((error_code >> 8) & 0x3) == 3 ) // if level 3 goto DEFAULT
            break;
        // can otherwise only happen in level 1 or 2
        v::debug << this->name() << ((error_code >> 8) & 0x3) << "-Level Page Table returned PTD: "
                << std::hex << pde << v::endl;
        break;
    }

    return -1;
}

// Read MMU Control Register
unsigned int mmu::read_mcr() {

  unsigned int tmp = MMU_CONTROL_REG;
  v::debug << this->name() << "MMU_CONTROL_REG was read: " << hex << tmp << v::endl;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif

  return (tmp);
}

// Write MMU Control Register
void mmu::write_mcr(unsigned int * data) {

  unsigned int tmp = *data;
  unsigned int tmp2 = MMU_CONTROL_REG;

  tmp2 = tmp2 & ~0x00008003;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif

  // Only TD [15], NF [1] and E [0] are writable
  MMU_CONTROL_REG = tmp2 | (tmp & 0x00008003);

  v::debug << name() << "Write " << tmp << " (" << *data << ") to MMU_CONTROL_REG: " << hex << v::setw(8) << MMU_CONTROL_REG << v::endl;
}

// Read MMU Context Table Pointer Register
unsigned int mmu::read_mctpr() {

  unsigned int tmp = MMU_CONTEXT_TABLE_POINTER_REG;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif
  
  v::debug << name() << "Read from MMU_CONTEXT_TABLE_POINTER_REG: " << hex << v::setw(8) << tmp << v::endl;

  return (tmp);
}

// Write MMU Context Table Pointer Register
void mmu::write_mctpr(unsigned int * data) {

  unsigned int tmp = *data;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif

  // [1-0] reserved, must read as zero
  MMU_CONTEXT_TABLE_POINTER_REG = (tmp & ~0x3);

  v::debug << name() << "Write to MMU_CONTEXT_TABLE_POINTER_REG: " << hex << v::setw(8) << MMU_CONTEXT_TABLE_POINTER_REG << v::endl;
}

// Read MMU Context Register
unsigned int mmu::read_mctxr() {

  unsigned int tmp = MMU_CONTEXT_REG;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif
  
  v::debug << name() << "Read from MMU_CONTEXT_REG: " << hex << v::setw(8) << MMU_CONTEXT_REG << v::endl;

  return (tmp);
}

// Write MMU Context Register
void mmu::write_mctxr(unsigned int * data) {

  unsigned int tmp = *data;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif

  MMU_CONTEXT_REG = tmp;

  v::debug << name() << "Write to MMU_CONTEXT_REG: " << hex << v::setw(8) << MMU_CONTEXT_REG << v::endl;
}

// Read MMU Fault Status Register
unsigned int mmu::read_mfsr() {

  unsigned int tmp = MMU_FAULT_STATUS_REG;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif
  
  v::debug << name() << "Read from MMU_FAULT_STATUS_REG: " << hex << v::setw(8) << MMU_FAULT_STATUS_REG << v::endl;

  // Page 258 - Table H-8:  Reading the Fault Status Register clears it
  MMU_FAULT_STATUS_REG = 0;

  return (tmp);
}

// Read MMU Fault Address Register
unsigned int mmu::read_mfar() {

  unsigned int tmp = MMU_FAULT_ADDRESS_REG;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif
  
  v::debug << name() << "Read from MMU_FAULT_ADDRESS_REG: " << hex << v::setw(8) << MMU_FAULT_ADDRESS_REG << v::endl;

  return (tmp);
}

// Diagnostic PDC access (Page 255 SparcV8 Man):
// =============================================
// VA[31-12] Virtual address
// VA[11-4]  PDC entry
// VA[3-2]   Register Selector

// Register Selector:
// ==================
// 0 - D[31-20] context, D[19-0] address tag
// 1 - PTE
// 2 - control bits (e.g. V, Level, LRU) ???
// 3 - Read:  Start compare in every PDC entry
//            if hit, return the PTE; if miss, return 0.
//     Write: For each PDC entry: if the contents of its
//            LRU counter is less than the stored data
//            value, increment its counter. Otherwise,
//            leave the LRU counter unchanged. In any
//            case, zero the LRU counter of the addressed
//            PDC entry.


// Diagnostic read of instruction PDC (ASI 0x5)
void mmu::diag_read_itlb(unsigned int addr, unsigned int * data) {

  unsigned int tmp;

  t_VAT vpn = (addr >> (32 - m_vtag_width));

  v::debug << name() << "Diagnostic read instruction PDC with address: " << hex << addr << " VPN: " << vpn << v::endl;

  // diagnostic ITLB lookup (without bus access)
  if ((addr & 0x3) == 0x3) {

    pdciter = itlb->find(vpn);

    // found something ?
    if (pdciter != itlb->end()) {

      // hit
      tmp = ((pdciter->second).pte);

    } else {

      // miss
      tmp = 0;

    }
  }

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif

  *data = tmp;
}

// Selects a TLB entry for replacement (LRU or RANDOM replacement).
// Removes the selected entry from the TLB map
// and returns the 'number' of the TLB (which is now free).
unsigned int mmu::tlb_remove(std::map<t_VAT, t_PTE_context> * tlb, unsigned int tlb_size) {

  std::map<t_VAT, t_PTE_context>::iterator selector;
  std::map<t_VAT, t_PTE_context>::iterator lru_selector;

  unsigned int tlb_select=0;
  uint64_t min_lru = 0xffffffffffffffff;

  uint32_t count;

  switch(m_tlb_rep) {

    // LRU
    case 0:

      // LRU replaces the TLB, which hasn't been used for the longest time.

      // Find the TLB with the lowest LRU value
      for(selector = tlb->begin(); selector != tlb->end(); selector++) {

        if (selector->second.lru < min_lru) {
          lru_selector = selector;
          tlb_select = selector->second.tlb_no;
          min_lru = selector->second.lru;
        }
      }

      v::debug << this->name() << "Select TLB (LRU): " << tlb_select << " for replacement. " << v::endl;

      tlb->erase(lru_selector);
      v::debug << this->name() << "Erased TLB " << tlb_select << v::endl;

      break;

    // Pseudo Random
    default:

      // Random replacement is implemented through
      // modulo-N counter that selects the TLB entry
      // to be removed from the PDC.
      tlb_select = m_pseudo_rand % (tlb_size + 1);

      v::debug << this->name() << "Select TLB (Random): " << tlb_select << " for replacement. " << v::endl;

      count = 0;

      for(selector = tlb->begin(); selector != tlb->end(); selector++) {
	        if (count == tlb_select) {

	          tlb->erase(selector);
	          break;

	        }
      }

  }

  return tlb_select;

}

// LRU replacement history updater
void mmu::lru_update(t_VAT vpn, std::map<t_VAT, t_PTE_context> * tlb, unsigned int tlb_size) {

  std::map<t_VAT, t_PTE_context>::iterator selector;

  // The LRU counters of all TLBs, except the selected one (vpn),
  // are decrement. The counter of the selected TLB is set to the maximum.
  for (selector = tlb->begin(); selector != tlb->end(); selector++) {

    if (selector->first != vpn) {

      if (selector->second.lru != 0) {

        (selector->second.lru)--;

      }

    } else {

      selector->second.lru = 0xffffffffffffffff - 1;

    }

    v::debug << name() << "TLB " << selector->second.tlb_no << " LRU " << selector->second.lru << v::endl;

  }
}


// Diagnostic write of instruction PDC (ASI 0x5)
void mmu::diag_write_itlb(unsigned int addr, unsigned int * data) {
}

// Diagnostic read of data or shared instruction and data PDC (ASI 0x6)
void mmu::diag_read_dctlb(unsigned int addr, unsigned int * data) {

  unsigned int tmp;

  t_VAT vpn = (addr >> (32 - m_vtag_width));

  v::debug << name() << "Diagnostic read data/shared PDC with address: " << hex << addr << " VPN: " << vpn << v::endl;

  // diagnostic ITLB lookup (without bus access)
  if ((addr & 0x3) == 0x3) {

    pdciter = dtlb->find(vpn);

    // found something ?
    if (pdciter != dtlb->end()) {

      // hit
      tmp = (pdciter->second).pte;

    } else {

      // miss
      tmp = 0;

    }
  }

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif

  *data = tmp;
}

// Start of simulation
void mmu::start_of_simulation() {

  // Initialize power model
  if (m_pow_mon) {

    power_model();

  }
}

// Calculate power/energy values from normalized input data
void mmu::power_model() {

  unsigned long total_tlbs;
  unsigned long data_tlbs  = m_dtlbnum;
  unsigned long instr_tlbs = m_itlbnum;

  if (m_tlb_type == 0x0) {

    // split tlb
    total_tlbs = m_itlbnum + m_dtlbnum;

  } else {

    // shared instruction and data tlb
    total_tlbs = m_itlbnum;
    data_tlbs  = m_itlbnum;

  }

  // Static power = mmu controller + itlb + dtlb
  sta_power = sta_power_norm +
    sta_tlb_power_norm * total_tlbs;

  // Cell internal power = mmu controller + itlb + dtlb
  int_power = int_power_norm * 1/(clockcycle.to_seconds()) +
    int_tlb_power_norm * total_tlbs * 1/(clockcycle.to_seconds());

  // itlb read energy
  dyn_itlb_read_energy = dyn_tlb_read_energy_norm * instr_tlbs;

  // itlb write energy
  dyn_itlb_write_energy = dyn_tlb_write_energy_norm * instr_tlbs;

  // dtlb read energy
  dyn_dtlb_read_energy = dyn_tlb_read_energy_norm * data_tlbs;

  // dtlb write energy
  dyn_dtlb_write_energy = dyn_tlb_write_energy_norm * data_tlbs;

}

// Static power callback
gs::cnf::callback_return_type mmu::sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Static power of mmu is constant !!
  return GC_RETURN_OK;
}

// Internal power callback
gs::cnf::callback_return_type mmu::int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // MMU internal power is constant !!
  return GC_RETURN_OK;
}

// Switching power callback
gs::cnf::callback_return_type mmu::swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  swi_power = ((dyn_itlb_read_energy * dyn_itlb_reads) + (dyn_itlb_write_energy * dyn_itlb_writes) + (dyn_dtlb_read_energy * dyn_dtlb_reads) + (dyn_dtlb_write_energy * dyn_dtlb_writes)) / (sc_time_stamp() - power_frame_starting_time).to_seconds();
  return GC_RETURN_OK;

}

// Print execution statistic at end of simulation
void mmu::end_of_simulation() {

  uint64_t total_ihits = 0;
  uint64_t total_dhits = 0;

  v::report << name() << "******************************************** " << v::endl;
  v::report << name() << "* MMU statistic:                             " << v::endl;
  v::report << name() << "* -------------------" << v::endl;

  if (m_tlb_type == 0) {

    for (uint32_t i=0; i<m_itlbnum; i++) {

      v::report << name() << "* Hits in ITLB" << i << ": " << tihits[i] << v::endl;
      total_ihits += tihits[i];
    }

    v::report << name() << "* Misses in ITLB: " << timisses << v::endl;

    if (total_ihits+timisses != 0) {
      v::report << name() << "* ITLB hit rate: " << (total_ihits * 100) / (total_ihits+timisses) << "%" << v::endl;
    }

    for (uint32_t i=0; i<m_dtlbnum; i++) {

      v::report << name() << "* Hits in DTLB" << i << ": " << tdhits[i] << v::endl;
      total_dhits += tdhits[i];

    }

    v::report << name() << "* Misses in DTLB: " << tdmisses << v::endl;

    if (total_dhits+tdmisses != 0) {
      v::report << name() << "* DTLB hit rate: " << (total_dhits * 100) / (total_dhits+tdmisses) << "%" << v::endl;
    }

  } else {

    for (uint32_t i=0; i<m_itlbnum;i++) {

      v::report << name() << "* Hits in shared I/D TLB" << i << ": " << tihits[i] << v::endl;
      total_ihits += tihits[i];

    }

    v::report << name() << "* Misses in shared I/D TLB: " << timisses << v::endl;

    if (total_ihits+timisses != 0) {
      v::report << name() << "* I/D TLB hit rate: " << (total_ihits * 100) / (total_ihits+timisses) << "%" << v::endl;
    }

  }

  v::report << name() << " ******************************************** " << v::endl;

}

// Diagnostic write of data or shared instruction and data PDC (ASI 0x6)
void mmu::diag_write_dctlb(unsigned int addr, unsigned int * data) {

}

// Returns handle to itlb memory interface object
tlb_adaptor * mmu::get_itlb_if() {

    return (itlb_adaptor);

}

// Returns handle to dtlb memory interface object
tlb_adaptor * mmu::get_dtlb_if() {

    return (dtlb_adaptor);

}

// Helper for setting clock cycle latency using sc_clock argument
void mmu::clkcng(sc_core::sc_time &clk) {
  clockcycle = clk;
}

/// TLB flush complete
void mmu::tlb_flush() {
  itlb->clear();  
  dtlb->clear();  
  v::debug << name() << "TLB flush" << v::endl;
};

/// TLB flush certain entry
void mmu::tlb_flush(uint32_t vpn) {
  itlb->clear();  
  dtlb->clear();  
  v::debug << name() << "TLB flush" << v::endl;
}
/// @}
