// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache_base
/// @{
/// @file mmu_cache_base.cpp
/// Implementation of LEON2/3 cache-subsystem consisting of instruction cache,
/// data cache, i/d localrams and memory management unit. The mmu_cache_base class
/// provides two TLM slave sockets for connecting the cpu and an AHB master
/// interface for connecting the processor bus.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include "core/models/mmu_cache/lib/mmu_cache_base.h"
#include "core/common/report.h"
#include "core/common/vendian.h"

//SC_HAS_PROCESS(mmu_cache_base<>);
/// Constructor
mmu_cache_base::mmu_cache_base(
      ModuleName name,
      bool icen, 
      uint32_t irepl, 
      uint32_t isets,
      uint32_t ilinesize, 
      uint32_t isetsize,
      uint32_t isetlock,
      uint32_t dcen,
      uint32_t drepl,
      uint32_t dsets,
      uint32_t dlinesize,
      uint32_t dsetsize,
      bool dsetlock,
      bool dsnoop,
      bool ilram,
      uint32_t ilramsize,
      uint32_t ilramstart,
      uint32_t dlram,
      uint32_t dlramsize,
      uint32_t dlramstart,
      uint32_t cached,
      bool mmu_en,
      uint32_t itlb_num,
      uint32_t dtlb_num,
      uint32_t tlb_type,
      uint32_t tlb_rep,
      uint32_t mmupgsz,
      uint32_t hindex,
      bool pow_mon,
     AbstractionLayer abstractionLayer) :

  AHBMaster<>(name,
              hindex,
              0x01,   // vendor
              0x003,  // device
              3,      // version
              0,      // irq
              abstractionLayer), // LT or AT
  snoop(&mmu_cache_base::snoopingCallBack,"SNOOP"),
  irq("irq"),
  m_icen(icen),
  m_dcen(dcen),
  m_dsnoop(dsnoop),
  m_ilram(ilram),
  m_ilramstart(ilramstart),
  m_dlram(dlram),
  m_dlramstart(dlramstart),
  m_cached(cached),
  m_mmu_en(mmu_en),
  m_master_id(hindex),
  bus_in_fifo("bus_in_fifo",1),
  m_right_transactions("successful_transactions", 0ull, m_counters),
  m_total_transactions("total_transactions", 0ull, m_counters),
  m_pow_mon(pow_mon),
  m_abstractionLayer(abstractionLayer),
  ahb_response_event(),
  sta_power_norm("sta_power_norm", 1.16e+8, m_power), // Normalized static power of controller
  int_power_norm("int_power_norm", 0.0, m_power), // Normalized internal power of controller
  dyn_read_energy_norm("dyn_read_energy_norm", 1.465e-8, m_power), // Normalized read energy
  dyn_write_energy_norm("dyn_write_energy_norm", 1.465e-8, m_power), // Normalized write energy
  sta_power("sta_power", 0.0, m_power), // Static power
  int_power("int_power", 0.0, m_power), // Dynamic power
  swi_power("swi_power", 0.0, m_power), // Switching power
  power_frame_starting_time("power_frame_starting_time", SC_ZERO_TIME, m_power),
  dyn_read_energy("dyn_read_energy", 0.0, m_power), // Energy per read access
  dyn_write_energy("dyn_write_energy", 0.0, m_power), // Energy per write access
  dyn_reads("dyn_reads", 0ull, m_power), // Read access counter for power computation
  dyn_writes("dyn_writes", 0ull, m_power) // Write access counter for power computation
  {

    wb_pointer = 0;
    globl_count = 0;

    // Parameter checks
    // ----------------

    // check range of cacheability mask (0x0 - 0xffff)
    assert((m_cached>=0)&&(m_cached<=0xffff));

    // create mmu (if required)
    m_mmu = (mmu_en == 1)? new mmu("mmu",
                                   (mmu_cache_if *)this,
                                   itlb_num,
                                   dtlb_num,
                                   tlb_type,
                                   tlb_rep,
                                   mmupgsz,
                                   pow_mon) : NULL;

    // create icache
    icache = (icen == 1)? (cache_if*)new ivectorcache("ivectorcache",
            (mmu_cache_if *)this, (mmu_en)? (mem_if *)m_mmu->get_itlb_if()
                                           : (mem_if *)this, mmu_en,
            isets, isetsize, isetlock, ilinesize, irepl, ilram, ilramstart,
            ilramsize, m_pow_mon) : (cache_if*)new nocache("no_icache",
            (mmu_en)? (mem_if *)m_mmu->get_itlb_if() : (mem_if *)this);

    // create dcache
    dcache = (dcen == 1)? (cache_if*)new dvectorcache("dvectorcache",
            (mmu_cache_if *)this, (mmu_en)? (mem_if *)m_mmu->get_dtlb_if()
                                           : (mem_if *)this, mmu_en,
            dsets, dsetsize, dsetlock, dlinesize, drepl, dlram, dlramstart,
            dlramsize, m_pow_mon) : (cache_if*)new nocache("no_dcache",
            (mmu_en)? (mem_if *)m_mmu->get_dtlb_if() : (mem_if *)this);

    // Create instruction scratchpad
    // (! only allowed with mmu disabled !)
    ilocalram = ((ilram == 1) && (mmu_en == 0))? new localram("ilocalram",
            ilramsize, ilramstart) : NULL;

    // Create data scratchpad
    // (! only allowed with mmu disabled !)
    dlocalram = ((dlram == 1) && (mmu_en == 0))? new localram("dlocalram",
            dlramsize, dlramstart) : NULL;

    // Initialize cache control registers
    CACHE_CONTROL_REG = 0;

    SC_THREAD(mem_access);

    // Register power callback functions
    if (m_pow_mon) {

      GC_REGISTER_TYPED_PARAM_CALLBACK(&sta_power, gs::cnf::pre_read, mmu_cache_base, sta_power_cb);
      GC_REGISTER_TYPED_PARAM_CALLBACK(&int_power, gs::cnf::pre_read, mmu_cache_base, int_power_cb);
      GC_REGISTER_TYPED_PARAM_CALLBACK(&swi_power, gs::cnf::pre_read, mmu_cache_base, swi_power_cb);

    }

    // Module Configuration Report
    srInfo("/configuration/mmu_cache_base/generics")
      ("icen", icen)
      ("dcen", dcen)
      ("mmu_en", mmu_en)
      ("ilram", ilram)
      ("dlram", dlram)
      ("abstraction_layer", abstractionLayer)
      ("dsnoop", dsnoop)
      ("Creating mmu_cache_base with this generics");
}

mmu_cache_base::~mmu_cache_base() {

  GC_UNREGISTER_CALLBACKS();

}

void mmu_cache_base::dorst() {
  // Reset functionality executed on 0 to 1 edge
}

void mmu_cache_base::exec_instr(const unsigned int &addr, unsigned char *ptr, unsigned int *debug, const unsigned int &flush, sc_core::sc_time& delay, bool is_dbg) {
  v::debug << name() << "instr exec" << v::endl;
  // Instruction scratchpad enabled && address points into selected 16MB region
  bool cacheable = true;
  if (m_ilram && (((addr >> 24) & 0xff) == m_ilramstart)) {

    ilocalram->mem_read((unsigned int)addr, 0, ptr, 4, &delay, debug, is_dbg, cacheable);

  // Instruction cache access
  } else {

    icache->mem_read((unsigned int)addr, 0x8, ptr, 4, &delay, debug, is_dbg, cacheable, false);

  }
}

void mmu_cache_base::exec_data(const tlm::tlm_command cmd, const unsigned int &addr, unsigned char *ptr, unsigned int len, unsigned int asi, unsigned int *debug, unsigned int flush, unsigned int lock, sc_core::sc_time& delay, bool is_dbg, tlm::tlm_response_status &response) {
  v::debug << name() << "data exec" << v::endl;
  // Flush instruction
  if (flush) {

    v::debug << name() << "Received flush instruction - flushing both caches" << v::endl;

    // Simultaneous flush of both caches
    icache->flush(&delay, debug, is_dbg);
    dcache->flush(&delay, debug, is_dbg);

    response = (tlm::TLM_OK_RESPONSE);

    return;

  }

  bool cacheable = true;
  // ************************************************
  // * TLM_READ_COMMAND
  // ************************************************
  if (cmd == tlm::TLM_READ_COMMAND) {

    // ************************************************
    // * TLM_READ_COMMAND - MAIN ASI SWITCH
    // ************************************************
    v::debug << name() << "read with ASI 0x2 - addr:" << hex << addr << v::endl;

    switch (asi) {

    case 2:

      v::debug << name() << "System Registers read with ASI 0x2 - addr:" << hex << addr << v::endl;

      // Address decoder for system registers
      if (addr == 0) {

        // Cache Control Register
        *(unsigned int *)ptr = read_ccr(false);
        // Setting response status
        response = (tlm::TLM_OK_RESPONSE);

      } else if (addr == 8) {

        // Instruction Cache Configuration Register
        *(unsigned int *)ptr = icache->read_config_reg(&delay);
        // Setting response status
        response = (tlm::TLM_OK_RESPONSE);

      } else if (addr == 0x0c) {

        // Data Cache Configuration Register
        *(unsigned int *)ptr = dcache->read_config_reg(&delay);
        // Setting response status
        response = (tlm::TLM_OK_RESPONSE);

      } else {

        v::error << name() << "Address (" << v::uint32 << addr << ")  not valid for read with ASI 0x2" << v::endl;
        // Set TLM response
        response = (tlm::TLM_ADDRESS_ERROR_RESPONSE);
      }

      // Reading system registers has a delay of one clock cycle
      delay = clock_cycle;

      break;

    case 5:

      v::debug << name() << "Diagnostic read from instruction PDC (ASI 0x5)" << v::endl;

      // Only possible if mmu enabled
      if (m_mmu_en) {

        m_mmu->diag_read_itlb(addr, (unsigned int *)ptr);
        // Set TLM response
        response = (tlm::TLM_OK_RESPONSE);

      } else {

        v::error << name() << "MMU not present" << v::endl;
        // Set TLM response
        response = (tlm::TLM_ADDRESS_ERROR_RESPONSE);

      }

      // Reading the instruction PDC has a delay of one clock cycle
      delay = clock_cycle;

      break;

    case 6:

      v::debug << name() << "Diagnostic read from data (or shared) PDC (ASI 0x6)" << v::endl;

      // Only possible if mmu enabled
      if (m_mmu_en) {

        m_mmu->diag_read_dctlb(addr, (unsigned int *)ptr);
        // Set TLM response
        response = (tlm::TLM_OK_RESPONSE);

      } else {

        v::error << name() << "MMU not present" << v::endl;

        // Set TLM response
        response = (tlm::TLM_ADDRESS_ERROR_RESPONSE);

      }

      // Reading the data (or shared) PDC has a delay of one clock cycle
      delay = clock_cycle;

      break;

    case 0xc:

      v::debug << name() << "ASI read instruction cache tags" << v::endl;

      icache->read_cache_tag((unsigned int)addr, (unsigned int*)ptr, &delay);
      // Set TLM response
      response = (tlm::TLM_OK_RESPONSE);

      break;

    case 0xd:

      v::debug << name() << "ASI read instruction cache entry" << v::endl;

      icache->read_cache_entry((unsigned int)addr, (unsigned int*)ptr, &delay);
      // Set TLM response
      response = (tlm::TLM_OK_RESPONSE);

      break;

    case 0xe:

      v::debug << name() << "ASI read data cache tags" << v::endl;

      dcache->read_cache_tag((unsigned int)addr, (unsigned int*)ptr, &delay);
      // Set TLM response
      response = (tlm::TLM_OK_RESPONSE);

      break;

    case 0xf:

      v::debug << name() << "ASI read data cache entry" << v::endl;

      dcache->read_cache_entry((unsigned int)addr, (unsigned int*)ptr, &delay);
      // Set TLM response
      response = (tlm::TLM_OK_RESPONSE);

      break;

    case 0x19:

      // Only works if MMU present
      if (m_mmu_en == 0x1) {

        v::debug << name() << "MMU register read with ASI 0x19 - addr: " << hex << addr << v::endl;

        // Address decoder for MMU register access
        if (addr == 0x000) {

          // MMU Control Register
          v::debug << name() << "ASI read MMU Control Register" << v::endl;

          *(unsigned int *)ptr = m_mmu->read_mcr();
          // Set TLM response
          response = (tlm::TLM_OK_RESPONSE);

        } else if (addr == 0x100) {

          // Context Pointer Register
          v::debug << name() << "ASI read MMU Context Pointer Register" << v::endl;

          *(unsigned int *)ptr = m_mmu->read_mctpr();
          // Set TLM response
          response = (tlm::TLM_OK_RESPONSE);

        } else if (addr == 0x200) {

          // Context Register
          v::debug << name() << "ASI read MMU Context Register" << v::endl;

          *(unsigned int *)ptr = m_mmu->read_mctxr();
          // Set TLM response
          response = (tlm::TLM_OK_RESPONSE);

        } else if (addr == 0x300) {

          // Fault Status Register
          v::debug << name() << "ASI read MMU Fault Status Register" << v::endl;

          *(unsigned int *)ptr = m_mmu->read_mfsr();
          // Set TLM response
          response = (tlm::TLM_OK_RESPONSE);

        } else if (addr == 0x400) {

          // Fault Address Register
          v::debug << name() << "ASI read MMU Fault Address Register" << v::endl;

          *(unsigned int *)ptr = m_mmu->read_mfar();
          // Set TLM response
          response = (tlm::TLM_OK_RESPONSE);

        } else {

          v::error << name() << "Address not valid for read with ASI 0x19" << v::endl;

          // Setting TLM response
          response = (tlm::TLM_ADDRESS_ERROR_RESPONSE);

        }

      } else {

        v::error << name() << "Access to MMU registers, but MMU not present!" << v::endl;
        // Setting TLM response
        response = (tlm::TLM_ADDRESS_ERROR_RESPONSE);

      }

      // Reading the mmu internal registers has a delay of one clock cycle
      delay = clock_cycle;

      break;

    // ASIs 0, 1, 3 forces cache miss)
    case 0x0:
    case 0x1:
    case 0x3:
    // Regular memory ASIs
    case 0x8:
    case 0x9:
    case 0xa:
    case 0xb:
//    case 0x1c:
      
      v::debug << name() << "ASI 0x" << hex << asi << " read with address 0x" << hex << addr << v::endl;

      // Instruction scratchpad enabled && address points into selected 16 MB region
      if (m_ilram && (((addr >> 24) & 0xff) == m_ilramstart)) {

        ilocalram->mem_read((unsigned int)addr, asi, ptr, len, &delay, debug, is_dbg, cacheable);
        // Set TLM response
        response = (tlm::TLM_OK_RESPONSE);

      // Data scratchpad enabled && address points into selected 16MB region
      } else if (m_dlram && (((addr >> 24) & 0xff) == m_dlramstart)) {

        dlocalram->mem_read((unsigned int)addr, asi, ptr, len, &delay, debug, is_dbg, cacheable);
        // Set TLM response
        response = (tlm::TLM_OK_RESPONSE);

      // Cache access || bypass || direct mmu
      } else {

        dcache->mem_read((unsigned int)addr, asi, ptr, len, &delay, debug, is_dbg, cacheable, lock);
        // Set TLM response
        response = (tlm::TLM_OK_RESPONSE);

      }

      break;
    
    case 0x1c:
        v::debug << name() << "ASI 0x" << hex << asi << " read throug with address 0x" << hex << addr << v::endl;
        this->mem_read((unsigned int)addr, asi, ptr, len, &delay, debug, is_dbg, cacheable, lock);
        break;
      
    default:

      v::error << name() << "ASI at read not recognized: " << hex << asi << v::endl;
      // Setting TLM response
      response = (tlm::TLM_ADDRESS_ERROR_RESPONSE);

    }

  // ************************************************
  // * TLM_WRITE_COMMAND
  // ************************************************
  } else if (cmd == tlm::TLM_WRITE_COMMAND) {

    // ************************************************
    // * TLM_WRITE_COMMAND - MAIN ASI SWITCH
    // ************************************************
    //
    v::debug << name() << "write with ASI 0x2 - addr:" << hex << addr << v::endl;
    switch (asi) {

    case 2:

      v::debug << name() << "System Register write with ASI 0x2 - addr:" << hex << addr << v::endl;

      // Address decoder for system registers
      if (addr == 0) {

        // Cache Control Register
        write_ccr(ptr, len, &delay, debug, is_dbg);
        // Setting response status
        response = (tlm::TLM_OK_RESPONSE);

      // TRIGGER ICACHE DEBUG OUTPUT / NOT A SPARC SYSTEM REGISTER
      } else if (addr == 0xfe) {

        // icache debug output (arg: line)
        icache->dbg_out(*(unsigned int*)ptr);
        // Setting response status
        response = (tlm::TLM_OK_RESPONSE);

      // TRIGGER DCACHE DEBUG OUTPUT / NOT A SPARC SYSTEM REGISTER
      } else if (addr == 0xff) {

        // dcache debug output (arg: line)
        dcache->dbg_out(*(unsigned int*)ptr);
        // Setting response status
        response = (tlm::TLM_OK_RESPONSE);

      } else {

        v::error << name() << "Address not valid for write with ASI 0x2 (or read only)" << v::endl;

        // Set TLM response
        response = (tlm::TLM_ADDRESS_ERROR_RESPONSE);

      }

      // Writing system registers has a delay of one clock cycle
      delay = clock_cycle;

      break;

    case 5:

      v::debug << name() << "Diagnostic write to instruction PDC (ASI 0x5)" << v::endl;

      // Only possible if mmu enabled
      if (m_mmu_en) {

        m_mmu->diag_write_itlb(addr, (unsigned int *)ptr);
        // set TLM response
        response = (tlm::TLM_OK_RESPONSE);

      } else {

        v::error << name() << "MMU not present" << v::endl;
        // Set TLM response
        response = (tlm::TLM_ADDRESS_ERROR_RESPONSE);

      }

      // Writing the instruction PDC has a delay of one clock cycle
      delay = clock_cycle;

      break;

    case 6:

      v::debug << name() << "Diagnostic write to data (or shared) PDC (ASI 0x6)" << v::endl;

      // Only possible if mmu enabled
      if (m_mmu_en) {

        m_mmu->diag_write_dctlb(addr, (unsigned int *)ptr);
        // Set TLM response
        response = (tlm::TLM_OK_RESPONSE);

      } else {

        v::error << name() << "MMU not present" << v::endl;
        // Set TLM response
        response = (tlm::TLM_ADDRESS_ERROR_RESPONSE);

      }

      // Reading the data (or shared) PDC has a delay of one clock cycle
      delay = clock_cycle;

      break;

    case 0xc:

      v::debug << name() << "ASI write instruction cache tags" << v::endl;

      icache->write_cache_tag((unsigned int)addr, (unsigned int*)ptr, &delay);
      // Set TLM response
      response = (tlm::TLM_OK_RESPONSE);

      break;

    case 0xd:

      v::debug << name() << "ASI write instruction cache entry" << v::endl;

      icache->write_cache_entry((unsigned int)addr, (unsigned int*)ptr, &delay);

      // Set TLM response
      response = (tlm::TLM_OK_RESPONSE);

      break;

    case 0xe:

      v::debug << name() << "ASI write data cache tags" << v::endl;

      dcache->write_cache_tag((unsigned int)addr, (unsigned int*)ptr, &delay);
      // Set TLM response
      response = (tlm::TLM_OK_RESPONSE);

      break;

    case 0xf:

      v::debug << name() << "ASI write data cache entry" << v::endl;

      dcache->write_cache_entry((unsigned int)addr, (unsigned int*)ptr, &delay);
      // Set TLM response
      response = (tlm::TLM_OK_RESPONSE);

      break;

    case 0x11: // is this correct?

      // All write operations with ASI 0x11 flush the instruction and data cache
      v::debug << name() << "ASI flush instruction and data cache" << v::endl;

      icache->flush(&delay, debug, is_dbg);
      dcache->flush(&delay, debug, is_dbg);
      // Set TLM response
      response = (tlm::TLM_OK_RESPONSE);

      break;
    
    case 0x15:

      // All write operations with ASI 0x15 flush the instruction cache
      v::debug << name() << "ASI flush instruction cache" << v::endl;

      icache->flush(&delay, debug, is_dbg);
      // Set TLM response
      response = (tlm::TLM_OK_RESPONSE);

      break;

    case 0x16:

      // All write operations with ASI 0x16 flush the data cache
      v::debug << name() << "ASI flush data cache" << v::endl;

      dcache->flush(&delay, debug, is_dbg);
      // Set TLM response
      response = (tlm::TLM_OK_RESPONSE);

      break;

    case 0x18: // is this correct?

      // All write operations with ASI 0x18 flush the TLB
      v::debug << name() << "ASI flush TLB" << v::endl;

      m_mmu->tlb_flush();
      // Set TLM response
      response = (tlm::TLM_OK_RESPONSE);

      break;

    case 0x19:

      // Only works if MMU present
      if (m_mmu_en == 0x1) {

        v::debug << name() << "MMU register write with ASI 0x19 - addr: " << hex << addr << v::endl;

        // Address decoder for MMU register access
        if (addr == 0x000) {

          // MMU Control Register
          v::debug << name() << "ASI write MMU Control Register" << v::endl;

          m_mmu->write_mcr((unsigned int *)ptr);
          // Set TLM response
          response = (tlm::TLM_OK_RESPONSE);

        } else if (addr == 0x100) {

          // Context Table Pointer Register
          v::debug << name() << "ASI write MMU Context Table Pointer Register" << v::endl;

          m_mmu->write_mctpr((unsigned int*)ptr);
          // Set TLM response
          response = (tlm::TLM_OK_RESPONSE);

        } else if (addr == 0x200) {

          // Context Register
          v::debug << name() << "ASI write MMU Context Register" << v::endl;

          m_mmu->write_mctxr((unsigned int*)ptr);
          // Set TLM response
          response = (tlm::TLM_OK_RESPONSE);

        } else {

          v::error << name() << "Address not valid for write with ASI 0x19 (or read-only)" << v::endl;

          // Setting TLM response
          response = (tlm::TLM_ADDRESS_ERROR_RESPONSE);
        }

      } else {

        v::error << name() << "Access to MMU registers, but MMU not present!" << v::endl;
        // Setting TLM response
        response = (tlm::TLM_ADDRESS_ERROR_RESPONSE);

      }

      // Writing the mmu internal registers has a delay of one clock cycle
      delay = clock_cycle;

      break;

    // ASI <= 3 forces cache miss)
    case 0x0:
    case 0x1:
    case 0x3:
    // Regular memory ASIs
    case 0x8:
    case 0x9:
    case 0xa:
    case 0xb:
//    case 0x1c:
      
      v::debug << name() << "ASI 0x" << hex << asi << " write with address 0x" << hex << addr << v::endl;

      // Instruction scratchpad enabled && address points into selected 16 MB region
      if (m_ilram && (((addr >> 24) & 0xff) == m_ilramstart)) {

        ilocalram->mem_write((unsigned int)addr, asi, ptr, len, &delay, debug, is_dbg, cacheable);
        // set TLM response
        response = (tlm::TLM_OK_RESPONSE);

      // Data scratchpad enabled && address points into selected 16MB region
      } else if (m_dlram && (((addr >> 24) & 0xff) == m_dlramstart)) {

        dlocalram->mem_write((unsigned int)addr, asi, ptr, len, &delay, debug, is_dbg, cacheable);
        // Set TLM response
        response = (tlm::TLM_OK_RESPONSE);

      // Cache access (write through) || bypass || direct mmu
      } else {

        dcache->mem_write((unsigned int)addr, asi, ptr, len, &delay, debug, is_dbg, cacheable, lock);
        // Set TLM response
        response = (tlm::TLM_OK_RESPONSE);

      }

      break;
    
    case 0x1c:
        v::debug << name() << "ASI 0x" << hex << asi << " write through with address 0x" << hex << addr << v::endl;
        this->mem_write((unsigned int)addr, asi, ptr, len, &delay, debug, is_dbg, cacheable, lock);
        break;

    default:

      v::error << name() << "ASI at write not recognized: " << hex << asi << v::endl;
      // Setting TLM response
      response = (tlm::TLM_ADDRESS_ERROR_RESPONSE);

    }

  } else {

    v::error << name() << "TLM command not valid" << v::endl;
    // Setting TLM response
    response = (tlm::TLM_COMMAND_ERROR_RESPONSE);

  }

}

/// Called from AHB master to signal begin response
void mmu_cache_base::response_callback(tlm::tlm_generic_payload * trans) {

  // Check response status
  if(trans->get_response_status()!=tlm::TLM_OK_RESPONSE) {
    v::warn << name() << "Transaction response state of Transaction to address " << v::uint32 << trans->get_address() << " is not TLM_OK_RESPONSE" << v::endl;
  }

  // Let mem_read/mem_write function know that we received a response
  ahb_response_event.notify();

}

/// Function for write access to AHB master socket
void mmu_cache_base::mem_write(unsigned int addr, unsigned int asi, unsigned char * data,
                          unsigned int length, sc_core::sc_time * delay,
                          unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock) {

  // Allocate new transaction (reference counter = 1)
  tlm::tlm_generic_payload * trans = ahb.get_transaction();

  v::debug << this->name() << "Allocate new transaction (mem_write) " << hex << trans << " Acquire / Ref-Count = " << trans->get_ref_count() << v::endl;

  // Copy payload data
  memcpy(write_buf + wb_pointer, data, length);

  // Initialize transaction
  trans->set_command(tlm::TLM_WRITE_COMMAND);
  trans->set_address(addr);
  trans->set_data_length(length);
  trans->set_data_ptr(write_buf + wb_pointer);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  wb_pointer = (wb_pointer + length) % 256;

  if (!is_dbg) {

    if (is_lock) {
      ahb.validate_extension<amba::amba_lock>(*trans);
    }
 
    v::debug << this->name() << "Schedule transaction (WRITE) " << v::hex << trans << " FIFO level: " << bus_in_fifo.used() << v::endl;
    v::debug << this->name() << "Acquire " << trans << " Ref-Count before acquire (bus_in_fifo) " << trans->get_ref_count() << v::endl;
    trans->acquire();
    bus_in_fifo.put(trans);
    wait(SC_ZERO_TIME);
    v::debug << this->name() << "Done scheduling transaction (WRITE) " << v::hex << trans << " FIFO level: " << bus_in_fifo.used() << v::endl;

  } else {

    // Debug transport
    ahbaccess_dbg(trans);

  }

  v::debug << name() << "Release " << trans << " Ref-Count before calling release (mem_write) " << trans->get_ref_count() << v::endl;

  // Decrement reference counter
  trans->release();

}

// Function for read access to AHB master socket
bool mmu_cache_base::mem_read(unsigned int addr, unsigned int asi, unsigned char * data,
                         unsigned int length, sc_core::sc_time * delay,
                         unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock) {

  bool cacheable_local = true;

  // Allocate new transaction (reference counter = 1)
  tlm::tlm_generic_payload * trans = ahb.get_transaction();

  v::debug << this->name() << "Allocate new transaction (mem_read) " << v::hex << trans << " Acquire / Ref-Count = " << trans->get_ref_count() << v::endl;

  // Initialize transaction
  trans->set_command(tlm::TLM_READ_COMMAND);
  trans->set_address(addr);
  trans->set_data_length(length);
  trans->set_data_ptr(data);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  if (!is_dbg) {

    if (is_lock) {
      ahb.validate_extension<amba::amba_lock>(*trans);
    }

    v::debug << this->name() << "Schedule transaction (READ) " << v::hex << trans << " FIFO level: " <<  bus_in_fifo.used() << v::endl;
    v::debug << this->name() << "Acquire " << trans << " Ref-Count before acquire (bus_in_fifo) " << trans->get_ref_count() << v::endl;
    trans->acquire();
    bus_in_fifo.put(trans);
    v::debug << this->name() << "Done scheduling transaction (READ) " << v::hex << trans << v::endl;

    // Read misses are blocking the cache !!
    wait(bus_read_completed);
    v::debug << this->name() << "Done transaction (READ) / bus_read_completed event " << v::hex << trans << v::endl;
    // cacheable handling!!!
    cacheable = (ahb.get_extension<amba::amba_cacheable>(*trans)) ? true : false;

    // Check cacheability
    //if ((m_cached != 0) && (cacheable))  {
    if ((m_cached != 0))  {
      cacheable_local = (m_cached & (1 << (addr >> 28))) ? true : false;
    }

  } else {
    
    ahbaccess_dbg(trans);

  }

  v::debug << name() << "Release " << trans << " Ref-Count before calling release (mem_read) " << trans->get_ref_count() << v::endl;

  // Decrement reference counter
  trans->release();
  return cacheable_local && cacheable;

}

// Thread for serializing memory access
void mmu_cache_base::mem_access() {

  tlm::tlm_generic_payload * trans;
  
  while(1) {

    while(bus_in_fifo.nb_get(trans)) {

      v::debug << this->name() << "Transaction " << v::hex << trans << " issued to AHB" << v::endl;
      if (trans->is_read()) {
	v::analysis << name() << "ADDR=0x" << v::hex << trans->get_address() << " TYPE=AHB_READ"  << v::endl;
      } else {
	v::analysis << name() << "ADDR=0x" << v::hex << trans->get_address() << " TYPE=AHB_WRITE" << v::endl;
      }
      ahbaccess(trans);
      v::debug << this->name() << "Transaction " << v::hex << trans << " returned from AHB" << v::endl;

      if (m_abstractionLayer == amba::amba_AT) wait(ahb_response_event);
      if (trans->is_read()) bus_read_completed.notify();

      // Decrement ref counter
      v::debug << name() << "Release " << trans << " Ref-Count before calling release (bus_in_fifo) " << trans->get_ref_count() << v::endl;
      trans->release();
    }

    if (bus_in_fifo.used() == 0) {
      wait(bus_in_fifo.ok_to_get());
    }
  }
}

// Send an interrupt over the central IRQ interface
void mmu_cache_base::set_irq(uint32_t tt) {

  irq.write(std::pair<uint32_t, bool>(tt, true));

}

// Writes the cache control register and handles the commands
void mmu_cache_base::write_ccr(unsigned char * data, unsigned int len,
                          sc_core::sc_time * delay, unsigned int * debug, bool is_dbg) {

    unsigned int tmp1 = *(unsigned int *)data;
    unsigned int tmp;

    #ifdef LITTLE_ENDIAN_BO
    swap_Endianess(tmp1);
    #endif

    memcpy(&tmp, &tmp1, len);

    // [DS] data cache snoop enable (todo)
    if (tmp & (1 << 23)) {
    }
    // [FD] dcache flush (do not set; always reads as zero)
    if (tmp & (1 << 22)) {
        dcache->flush(delay, debug, is_dbg);
    }
    // [FI] icache flush (do not set; always reads as zero)
    if (tmp & (1 << 21)) {
        icache->flush(delay, debug, is_dbg);
    }
    // [IB] instruction burst fetch (todo)
    if (tmp & (1 << 16)) {
    }

    // [IP] Instruction cache flush pending (bit 15 - read only)
    // [DP] Data cache flush pending (bit 14 - read only)

    // [DF] data cache freeze on interrupt (todo)
    if (tmp & (1 << 5)) {
    }

    // [IF] instruction cache freeze on interrupt (todo)
    if (tmp & (1 << 4)) {
    }

    // [DCS] data cache state (bits 3:2)
    // [ICS] instruction cache state (bits 1:0)

    // read only masking: 1111 1111 1001 1111 0011 1111 1111 1111
    CACHE_CONTROL_REG = (tmp & 0xff9f3fff);

    v::debug << name() << "CACHE_CONTROL_REG write: " << hex << v::setw(8) << CACHE_CONTROL_REG << v::endl;
}

// Read the cache control register from processor interface
unsigned int mmu_cache_base::read_ccr(bool internal) {

  unsigned int tmp = CACHE_CONTROL_REG;
  v::debug << name() << "CACHE_CONTROL_REG read: " << hex << v::setw(8) << CACHE_CONTROL_REG << v::endl;

  if (!internal) {

    #ifdef LITTLE_ENDIAN_BO
    swap_Endianess(tmp);
    #endif

  }

  return (tmp);
}

// Snooping function
void mmu_cache_base::snoopingCallBack(const t_snoop& snoop, const sc_core::sc_time& delay) {

  v::debug << name() << "Snooping write operation on AHB interface (MASTER: " << snoop.master_id << " ADDR: "
   << v::uint32 << snoop.address << " LENGTH: " << snoop.length << ")" << v::endl;
  // Make sure we are not snooping ourself ;)
  if (snoop.master_id != m_master_id) {

    // If dcache and snooping enabled
    if (m_dcen && m_dsnoop) {

      dcache->snoop_invalidate(snoop, delay);
    }
  }
}


// Automatically called at the beginning of the simulation
void mmu_cache_base::start_of_simulation() {

  // Initialize power model
  if (m_pow_mon) {

    power_model();

  }
}

// Calculate power/energy values form normalized input data
void mmu_cache_base::power_model() {

  // Static power calculation (pW)
  sta_power = sta_power_norm;

  // Cell internal power (uW)
  int_power = int_power_norm * 1/(clock_cycle.to_seconds()*1.0e+6);

  // Energy per read access (uJ)
  dyn_read_energy = dyn_read_energy_norm;

  // Energy per write access (uJ)
  dyn_write_energy = dyn_write_energy_norm;

}

// Static power callback
gs::cnf::callback_return_type mmu_cache_base::sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Static power of mmu_cache_base is constant !!
  return GC_RETURN_OK;
}

// Internal power callback
gs::cnf::callback_return_type mmu_cache_base::int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Internal power of mmu_cache_base is constant !!
  return GC_RETURN_OK;
}

// Switching power callback
gs::cnf::callback_return_type mmu_cache_base::swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  swi_power = ((dyn_read_energy * dyn_reads) + (dyn_write_energy * dyn_writes)) / (sc_time_stamp() - power_frame_starting_time).to_seconds();
  return GC_RETURN_OK;
}

// Automatically called by SystemC scheduler at end of simulation
// Displays execution statistics.
void mmu_cache_base::end_of_simulation() {

    v::report << name() << " ********************************************" << v::endl;
    v::report << name() << " * mmu_cache_base Statistics: " << v::endl;
    v::report << name() << " * --------------------- " << v::endl;
    v::report << name() << " * Successful Transactions: " << m_right_transactions << v::endl;
    v::report << name() << " * Total Transactions: " << m_total_transactions << v::endl;
    v::report << name() << " * " << v::endl;
    v::report << name() << " * AHB Master interface reports: " << v::endl;
    print_transport_statistics(name());
    v::report << name() << " ********************************************" << v::endl;

}

sc_core::sc_time mmu_cache_base::get_clock() {

  return clock_cycle;

}

// Helper for setting clock cycle latency using a value-time_unit pair
void mmu_cache_base::clkcng() {

    // Set icache clock
    if(m_icen) {
        icache->clkcng(clock_cycle);
    }

    // Set dcache clock
    if(m_dcen) {
        dcache->clkcng(clock_cycle);
    }

    // Set mmu clock
    if(m_mmu_en) {
      m_mmu->clkcng(clock_cycle);
    }

    // Set dlocalram clock
    if(dlocalram) {
      dlocalram->clkcng(clock_cycle);
    }

    // Set ilocalram clcok
    if(ilocalram) {
      ilocalram->clkcng(clock_cycle);
    }
}
/// @}
