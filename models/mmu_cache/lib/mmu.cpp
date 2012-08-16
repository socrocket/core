//*********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is 
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      mmu.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implementation of a memory management unit.
//             The mmu can be configured to have split or combined
//             TLBs for instructions and data. The TLB size can be
//             configured as well. The memory page size is currently
//             currently fixed to 4kB.
//
//
// Method:
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#include "mmu.h"
#include "verbose.h"

mmu::mmu(sc_core::sc_module_name name, // sysc module name,
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
    MMU_CONTROL_REG = ((m_itlblog2 << 21) | (m_dtlblog2 << 18));

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

// look up a tlb (page descriptor cache)
// and return physical address
unsigned int mmu::tlb_lookup(unsigned int addr,
                             std::map<t_VAT, t_PTE_context> * tlb,
                             unsigned int tlb_size, sc_core::sc_time * t,
                             unsigned int * debug, bool is_dbg) {

    // According to the SparcV8 Manual: Pages of the Reference MMU are always aligned on 4K-byte boundaries; hence, the lower-order
    // 12 bits of a physical address are always the same as the low-order 12 bits of
    // the virtual address. The Gaisler MMU additionally supports 8k, 16k and 32k alignment,
    unsigned int offset = ((addr << m_vtag_width) >> m_vtag_width);
    t_VAT vpn = (addr >> (32 - m_vtag_width));

    // The Virtual Address Tag consists of three indices, which are used to look
    // up the three level page table in main memory. The indices are extracted from
    // the tag and translated to byte addresses (shift left 2 bit)
    unsigned int idx1 = (vpn >> (m_idx2 + m_idx3) << 2);
    unsigned int idx2 = (vpn << (32 - m_idx2 - m_idx3)) >> (30 - m_idx3);
    unsigned int idx3 = (vpn << (32 - m_idx3)) >> (30 - m_idx3);

    // Locals for intermediate results
    t_PTE_context tmp;
    unsigned int paddr;
    unsigned int data;

    unsigned int slot_no;

    bool context_miss = false;

    // Search virtual address tag in ipdc (associative)
    v::info << this->name() << "lookup with VPN: " << std::hex << vpn
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

      v::info << this->name() << "Virtual Address Tag Hit in TLB " << tmp.tlb_no << " for address: "
                << hex << addr << v::endl;

        // Check the context tag
        if (tmp.context == MMU_CONTEXT_REG) {

            v::info << this->name() << "CONTEXT hit" << v::endl;

            // Build physical address from PTE and offset, and return
            paddr = (((tmp.pte >> 8) << (32 - m_vtag_width)) | offset);

            // Update debug information
            TLBHIT_SET(*debug);

	    if (tlb == itlb) {

	      tihits[tmp.tlb_no]++;

	    } else {

	      tdhits[tmp.tlb_no]++;

	    }

	    // Update LRU history
	    if (m_tlb_rep==1) {

	      lru_update(vpn, tlb, tlb_size);
	      
	    }

            return (paddr);

        } else {

            v::info << this->name() << "CONTEXT miss" << v::endl;

            // Update debug information
            TLBMISS_SET(*debug);

	    if (tlb == itlb) {

	      timisses++;

	    } else {

	      tdmisses++;

	    }

            context_miss = true;

        }
    } else {

        v::info << this->name() << "Virtual Address Tag miss" << v::endl;

        // Update debug information
        TLBMISS_SET(*debug);

	if (tlb == itlb) {

	  timisses++;

	} else {

	  tdmisses++;

	}

    }

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
    v::info << this->name()
            << "START TLB MISS PROCESSING FOR VIRTUAL ADDRESS: " << std::hex
            << addr << v::endl;

    // ***************************************
    // 3-Level TLB table walk
    // ***************************************

    // 1. load from 1st-level page table
    m_mmu_cache->mem_read(MMU_CONTEXT_TABLE_POINTER_REG + idx1, 0x8,
                          (unsigned char *)&data, 4, t, debug, is_dbg, false);

    #ifdef LITTLE_ENDIAN_BO
    swap_Endianess(data);
    #endif

    v::info << this->name() << "Back from read addr: " << std::hex
            << (MMU_CONTEXT_TABLE_POINTER_REG + idx1) << " data: " << std::hex
            << data << v::endl;

    // page table entry (PTE) or page table descriptor (PTD) (to level 2)
    if ((data & 0x3) == 0x2) {

        v::info << this->name() << "1-Level Page Table returned PTE: "
                << std::hex << data << v::endl;

        // In case of a virtual address tag miss a new PDC entry is created.
        // For context miss the existing entry will be replaced.
        if ((!context_miss) && (tlb->size() == tlb_size)) {

            v::info << this->name() << "PDC full" << std::hex << data
                    << v::endl;

	    // Remove a TLB entry, with respect to replacement strategy
	    slot_no = tlb_remove(tlb, tlb_size);
	    
	    tmp.tlb_no = slot_no;

   
        } else {

	  v::info << this->name() << "Create new entry PDC entry - TLB number: " << tlb->size() << v::endl;
	  tmp.tlb_no = tlb->size();

	}

        // add to PDC
        tmp.context = MMU_CONTEXT_REG;
        tmp.pte = data;
	tmp.lru = 7;

        // Log TLB writes for power monitoring
        if (m_pow_mon) {

          if (tlb == itlb) {

            dyn_itlb_writes++;

          } else {

            dyn_dtlb_writes++;

          }
        }

	(*tlb)[vpn] = tmp;

        // build physical address from PTE and offset
        paddr = (((tmp.pte >> 8) << (32 - m_vtag_width)) | offset);

        v::info << this->name() << "Mapping complete - Virtual Addr: "
                << std::hex << addr << " Physical Addr: " << std::hex << paddr
                << v::endl;
        return (paddr);

    } else if ((data & 0x3) == 0x1) {

        v::info << this->name() << "1st-Level Page Table returned PTD: "
                << std::hex << data << v::endl;

    } else {

        v::error << this->name()
                << "Error in 1st-Level Page Table / Entry type not valid"
                << v::endl;
        
        // Set fault status and fault address
        MMU_FAULT_STATUS_REG = 0;
        MMU_FAULT_STATUS_REG |= 1 << 8; // L  - Level of Error
        MMU_FAULT_STATUS_REG |= 4 << 2; // FT - Translation Error
        MMU_FAULT_STATUS_REG |= 1 << 1; // FAV - Fault Address Register valid
        
        MMU_FAULT_ADDRESS_REG = addr;

        if (tlb == itlb) {

          v::error << this->name() << "Trap encountered (instruction_access_mmu_miss) tt = 0x3c" << v::endl;
          m_mmu_cache->set_irq(0x3c);

        } else {

          v::error << this->name() << "Trap encountered (data_access_mmu_miss) tt = 0x2c" << v::endl;
          m_mmu_cache->set_irq(0x2c);

        }        

        return (0);
    }

    // 2. load from 2nd-level page table
    m_mmu_cache->mem_read((((data) >> 2) << 2) + idx2, 0x8, (unsigned char *)&data,
                          4, t, debug, is_dbg, false);

    #ifdef LITTLE_ENDIAN_BO
    swap_Endianess(data);
    #endif

    // page table entry (PTE) or page table descriptor (PTD) (to level 3)
    if ((data & 0x3) == 0x2) {

        v::info << this->name() << "2-Level Page Table returned PTE: "
                << std::hex << data << v::endl;

        // In case of a virtual address tag miss a new PDC entry is created.
        // For context miss the existing entry will be replaced.
        if ((!context_miss) && (tlb->size() == tlb_size)) {

          v::info << this->name() << "PDC full" << std::hex << data
                  << v::endl;

	  // Remove a TLB entry, with respect to replacement strategy
	  slot_no = tlb_remove(tlb, tlb_size);
	    
	  tmp.tlb_no = slot_no;
	    
        } else {

	  v::info << this->name() << "Create new entry PDC entry - TLB number: " << tlb->size() << v::endl;
	  tmp.tlb_no = tlb->size();

        }

        // add to PDC
        tmp.context = MMU_CONTEXT_REG;
        tmp.pte = data;
	tmp.lru = 7;

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
        paddr = (((tmp.pte >> 8) << (32 - m_vtag_width)) | offset);

        v::info << this->name() << "Mapping complete - Virtual Addr: "
                << std::hex << addr << " Physical Addr: " << std::hex << paddr
                << v::endl;
        return (paddr);

    } else if ((data & 0x3) == 0x1) {

        v::info << this->name() << "2-Level Page Table returned PTD: "
                << std::hex << data << v::endl;

    } else {

        v::error << this->name()
                 << "Error in 2-Level Page Table / Entry type not valid"
                 << v::endl;

        // Set fault status and fault address
        MMU_FAULT_STATUS_REG = 0;
        MMU_FAULT_STATUS_REG |= 2 << 8; // L  - Level of Error
        MMU_FAULT_STATUS_REG |= 4 << 2; // FT - Translation Error
        MMU_FAULT_STATUS_REG |= 1 << 1; // FAV - Fault Address Register valid
        
        MMU_FAULT_ADDRESS_REG = addr;

        if (tlb == itlb) {

          v::error << this->name() << "Trap encountered (instruction_access_mmu_miss) tt = 0x3c" << v::endl;
          m_mmu_cache->set_irq(0x3c);

        } else {

          v::error << this->name() << "Trap encountered (data_access_mmu_miss) tt = 0x2c" << v::endl;
          m_mmu_cache->set_irq(0x2c);

        }

        return (0);
    }

    // 3. load from 3rd-level page table
    m_mmu_cache->mem_read((((data) >> 2) << 2) + idx3, 0x8, (unsigned char *)&data,
                          4, t, debug, is_dbg, false);

    #ifdef LITTLE_ENDIAN_BO
    swap_Endianess(data);
    #endif

    // 3rd-level page table must contain PTE (PTD not allowed)
    if ((data & 0x3) == 0x2) {

        v::info << this->name() << "3-Level Page Table returned PTE: "
                << std::hex << data << v::endl;

        // In case of a virtual address tag miss a new PDC entry is created.
        // For context miss the existing entry will be replaced.
        if ((!context_miss) && (tlb->size() == tlb_size)) {

            v::info << this->name() << "PDC full" << std::hex << data
                    << v::endl;

	    // Remove a TLB entry, with respect to replacement strategy
	    slot_no = tlb_remove(tlb, tlb_size);

	    tmp.tlb_no = slot_no;
	    
        } else {

	  v::info << this->name() << "Create new entry PDC entry - TLB number: " << tlb->size() << v::endl;
	  tmp.tlb_no = tlb->size();

        }

        // add to PDC
        tmp.context = MMU_CONTEXT_REG;
        tmp.pte = data;
	tmp.lru = 7;

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
        paddr = (((tmp.pte >> 8) << (32 - m_vtag_width)) | offset);

        v::info << this->name() << "Mapping complete - Virtual Addr: "
                << std::hex << addr << " Physical Addr: " << std::hex << paddr
                << v::endl;
        return (paddr);
    } else {

        v::error << this->name()
                 << "Error in 3-Level Page Table / Entry type not valid"
                 << v::endl;

        // Set fault status and fault address
        MMU_FAULT_STATUS_REG = 0;
        MMU_FAULT_STATUS_REG |= 3 << 8; // L  - Level of Error
        MMU_FAULT_STATUS_REG |= 4 << 2; // FT - Translation Error
        MMU_FAULT_STATUS_REG |= 1 << 1; // FAV - Fault Address Register valid
        
        MMU_FAULT_ADDRESS_REG = addr;

        if (tlb == itlb) {

          v::error << this->name() << "Trap encountered (instruction_access_mmu_miss) tt = 0x3c" << v::endl;
          m_mmu_cache->set_irq(0x3c);

        } else {

          v::error << this->name() << "Trap encountered (data_access_mmu_miss) tt = 0x2c" << v::endl;
          m_mmu_cache->set_irq(0x2c);

        }

        return (0);
    }
}

// Read MMU Control Register
unsigned int mmu::read_mcr() {

  unsigned int tmp = MMU_CONTROL_REG;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif

  return (tmp);

}

// Write MMU Control Register
void mmu::write_mcr(unsigned int * data) {

  unsigned int tmp = *data;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif

  // Only TD [15], NF [1] and E [0] are writable
  MMU_CONTROL_REG = (tmp & 0x00008003);

  v::debug << name() << "Write to MMU_CONTROL_REG: " << hex << v::setw(8) << MMU_CONTROL_REG << v::endl;

}

// Read MMU Context Table Pointer Register
unsigned int mmu::read_mctpr() {

  unsigned int tmp = MMU_CONTEXT_TABLE_POINTER_REG;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif  

  return (tmp);

}

// Write MMU Context Table Pointer Register
void mmu::write_mctpr(unsigned int * data) {

  unsigned int tmp = *data;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif

  // [1-0] reserved, must read as zero
  MMU_CONTEXT_TABLE_POINTER_REG = (tmp & 0xfffffffc);

  v::debug << name() << "Write to MMU_CONTEXT_TABLE_POINTER_REG: " << hex << v::setw(8) << MMU_CONTEXT_TABLE_POINTER_REG << v::endl;

}

// Read MMU Context Register
unsigned int mmu::read_mctxr() {

  unsigned int tmp = MMU_CONTEXT_REG;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif

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
  
  return (tmp);

}

// Read MMU Fault Address Register
unsigned int mmu::read_mfar() {

  unsigned int tmp = MMU_FAULT_ADDRESS_REG;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif

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

  v::info << name() << "Diagnostic read instruction PDC with address: " << hex << addr << " VPN: " << vpn << v::endl;

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
  unsigned int min_lru=7;

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

	}
      }

      v::info << this->name() << "Select TLB (LRU): " << tlb_select << " for replacement. " << v::endl;

      tlb->erase(lru_selector);

      break;

    // Pseudo Random
    default:

      // Random replacement is implemented through
      // modulo-N counter that selects the TLB entry
      // to be removed from the PDC.
      tlb_select = m_pseudo_rand % (tlb_size + 1);

      v::info << this->name() << "Select TLB (Random): " << tlb_select << " for replacement. " << v::endl;

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

  v::info << name() << "LRU_UPDATE for VPN: " << hex << vpn << v::endl;

  // The LRU counters of all TLBs, except the selected one (vpn),
  // are decrement. The counter of the selected TLB is set to the maximum.
  for (selector = tlb->begin(); selector != tlb->end(); selector++) {

    if (selector->first != vpn) {

      if (selector->second.lru != 0) {

	(selector->second.lru)--;

      }
 
    } else {

	selector->second.lru = 7;

    }

    v::info << name() << "TLB " << selector->second.tlb_no << " LRU " << selector->second.lru << v::endl;

  }
}


// Diagnostic write of instruction PDC (ASI 0x5)
void mmu::diag_write_itlb(unsigned int addr, unsigned int * data) {
}

// Diagnostic read of data or shared instruction and data PDC (ASI 0x6)
void mmu::diag_read_dctlb(unsigned int addr, unsigned int * data) {

  unsigned int tmp;

  t_VAT vpn = (addr >> (32 - m_vtag_width));

  v::info << name() << "Diagnostic read data/shared PDC with address: " << hex << addr << " VPN: " << vpn << v::endl;

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
  int_power = int_power_norm * 1/(clockcycle.to_seconds()*1.0e+6) +
    int_tlb_power_norm * total_tlbs * 1/(clockcycle.to_seconds()*1.0e+6);

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
void mmu::sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Static power of mmu is constant !!

}

// Internal power callback
void mmu::int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // MMU internal power is constant !!

}

// Switching power callback
void mmu::swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  swi_power = ((dyn_itlb_read_energy * dyn_itlb_reads) + (dyn_itlb_write_energy * dyn_itlb_writes) + (dyn_dtlb_read_energy * dyn_dtlb_reads) + (dyn_dtlb_write_energy * dyn_dtlb_writes)) / (sc_time_stamp() - power_frame_starting_time).to_seconds();

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
