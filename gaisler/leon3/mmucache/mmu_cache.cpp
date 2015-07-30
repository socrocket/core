// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file mmu_cache.cpp
/// Implementation of LEON2/3 cache-subsystem consisting of instruction cache,
/// data cache, i/d localrams and memory management unit. The mmu_cache class
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

#include "gaisler/leon3/mmucache/mmu_cache.h"

//SC_HAS_PROCESS(mmu_cache<>);
/// Constructor
mmu_cache::mmu_cache(
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
  mmu_cache_base(
      name,
      icen, 
      irepl, 
      isets,
      ilinesize, 
      isetsize,
      isetlock,
      dcen,
      drepl,
      dsets,
      dlinesize,
      dsetsize,
      dsetlock,
      dsnoop,
      ilram,
      ilramsize,
      ilramstart,
      dlram,
      dlramsize,
      dlramstart,
      cached,
      mmu_en,
      itlb_num,
      dtlb_num,
      tlb_type,
      tlb_rep,
      mmupgsz,
      hindex,
      pow_mon,
      abstractionLayer),
  icio("icio"),
  dcio("dcio"),
  icio_PEQ("icio_PEQ"),
  dcio_PEQ("dcio_PEQ") {
    // Loosely-Timed
    if (abstractionLayer==amba::amba_LT) {

      // Register blocking forward transport functions for icio and dcio sockets (slave)
      icio.register_b_transport(this, &mmu_cache::icio_b_transport);
      dcio.register_b_transport(this, &mmu_cache::dcio_b_transport);

    // Approximately-Timed
    } else if (abstractionLayer==amba::amba_AT) {

      // Register non-blocking forward transport functions for icio and dcio sockets
      icio.register_nb_transport_fw(this, &mmu_cache::icio_nb_transport_fw);
      dcio.register_nb_transport_fw(this, &mmu_cache::dcio_nb_transport_fw);

      // Register icio service thread (for AT)
      SC_THREAD(icio_service_thread);
      sensitive << icio_PEQ.get_event();
      dont_initialize();

      // Register dcio service thread (for AT)
      SC_THREAD(dcio_service_thread);
      sensitive << dcio_PEQ.get_event();
      dont_initialize();

    } else {

      srError(name)
        ("Abstraction Layer not valid!!");
      assert(0);

    }

    // Instruction debug transport
    icio.register_transport_dbg(this, &mmu_cache::icio_transport_dbg);
    // Data debug transport
    dcio.register_transport_dbg(this, &mmu_cache::dcio_transport_dbg);
}

mmu_cache::~mmu_cache() {
  GC_UNREGISTER_CALLBACKS();

}

// Instruction interface to functional part of the model
void mmu_cache::exec_instr(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay, bool is_dbg) {

  // Vars for payload decoding
  tlm::tlm_command cmd = trans.get_command();
  sc_dt::uint64 addr   = trans.get_address();
  unsigned char * ptr  = trans.get_data_ptr();

  // Log instruction reads for power monitoring
  if (m_pow_mon) {
    dyn_reads += (trans.get_data_length() >> 2) + 1;
  }

  // Extract extension
  icio_payload_extension * iext;
  trans.get_extension(iext);

  unsigned int *debug;
  unsigned int flush;

  // Check/extract instruction payload extension
  if (iext!=NULL) {

    debug = iext->debug;
    flush = iext->flush;

  } else {

    // No iext
    debug = NULL;
    flush = 0;

    v::error << name() << "IEXT Payload extension missing" << v::endl;
  }

  // Flush instruction
  if (flush) {

    v::debug << name() << "Received flush instruction - flushing both caches" << v::endl;

    // Simultaneous flush of both caches
    icache->flush(&delay, debug, is_dbg);
    dcache->flush(&delay, debug, is_dbg);

    trans.set_response_status(tlm::TLM_OK_RESPONSE);

    return;

  }

  if (cmd == tlm::TLM_READ_COMMAND) {

    assert( 1 ); // fix asi -> priv / unpriv

    mmu_cache_base::exec_instr(addr, ptr, 0x8, debug, flush, delay, is_dbg); // ToDo: fix ASI! 0x8 -> unprivileged instruction

    // Set response status
    trans.set_response_status(tlm::TLM_OK_RESPONSE);

  } else {

    v::error << name() << " Command not valid for instruction cache (tlm_write)" << v::endl;
    // Set response status
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

  }
}

// Data interface to functional part of the model
void mmu_cache::exec_data(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay, bool is_dbg) {

  // Vars for payload decoding
  tlm::tlm_command cmd = trans.get_command();
  sc_dt::uint64 addr   = trans.get_address();
  unsigned char * ptr  = trans.get_data_ptr();
  unsigned int len     = trans.get_data_length();

  // Log number of reads and writes for power monitoring
  if (m_pow_mon) {

    if (cmd == tlm::TLM_READ_COMMAND) {

      dyn_reads += (len >> 2) + 1;

    } else {

      dyn_writes += (len >> 2) + 1;

    }
  }

  // Extract extension
  dcio_payload_extension * dext;
  trans.get_extension(dext);

  unsigned int asi;
  unsigned int *debug;
  unsigned int flush;
  unsigned int lock;
  //unsigned int flushl;

  // Check/extract data payload extension
  if(dext!=NULL) {

      asi    = dext->asi;
      debug  = dext->debug;
      flush  = dext->flush;
      //flushl = dext->flushl;
      lock   = dext->lock;

  } else {
      // No dext extension
      // assuming normal access
      asi    = 0xA;
      debug  = NULL;
      flush  = 0;
      //flushl = 0;
      lock   = 0;

      v::error << name() << "DEXT Payload extension missing - assume ASI 0xA for unprivileged data" << v::endl;

      assert( 1 ); // fix asi -> priv / unpriv
  }

  tlm::tlm_response_status response = tlm::TLM_COMMAND_ERROR_RESPONSE;
  mmu_cache_base::exec_data(cmd, addr, ptr, len, asi, debug, flush, lock, delay, is_dbg, response);
  trans.set_response_status(response);

}

/// TLM blocking forward transport function for icio socket
void mmu_cache::icio_b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {

  v::analysis << name() << "ADDR=0x" << v::hex << trans.get_address() << " TYPE=ICACHE_READ"  << v::endl;
  
  v::debug << name() << "TRANS: " << globl_count++ << " icio_b_transport received trans " << hex << &trans << " (0x" << v::hex << v::setfill('0') << v::setw(8) << trans.get_address() << ") with delay " << delay << v::endl;

  //if (trans.get_address()==0x40000000) v::info << "Boot completed - jump to main" << v::endl;

  // Call the functional part of the model
  // ---------------------------
  exec_instr(trans, delay, false);
  // ---------------------------

  v::debug << name() << "Transaction " << hex << &trans << "returned from exec_instr with delay " << delay << v::endl;

}

/// TLM forward blocking transport function for dcio socket
void mmu_cache::dcio_b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {

  if (trans.is_read()) {
    v::debug << name() << "TRANS: " << globl_count++ << " dcio_b_transport (READ) received trans " << hex << &trans << " (0x" << v::hex << v::setfill('0') << v::setw(8) << trans.get_address() << ") with delay " << delay << v::endl;
    v::analysis << name() << "ADDR=0x" << v::hex << trans.get_address() << " TYPE=DCACHE_READ"  << v::endl;    

  } else {
    v::debug << name() << "TRANS: " << globl_count++ << " dcio_b_transport (WRITE) received trans " << hex << &trans << " (0x" << v::hex << v::setfill('0') << v::setw(8) << trans.get_address() << ") with delay " << delay << v::endl;
    v::analysis << name() << "ADDR=0x" << v::hex << trans.get_address() << " TYPE=DCACHE_WRITE"  << v::endl;
  }

  // Call the functional part of the model
  // -----------------------
  exec_data(trans, delay, false);
  // -----------------------

  v::debug << name() << "Transaction " << hex << &trans << "returned from exec_data with delay " << delay << v::endl;

}

/// TLM non-blocking forward transport function for icio socket
tlm::tlm_sync_enum mmu_cache::icio_nb_transport_fw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  v::debug << name() << "ICIO nb_transport forward received transaction: " << hex << &trans << " with phase " << phase << v::endl;

  if (trans.get_address()==0x40000000) v::info << "Boot completed - jump to main" << v::endl;
 
  // The master has sent BEGIN_REQ
  if (phase == tlm::BEGIN_REQ) {

    v::debug << name() << "TRANS: " << globl_count++ << " icio_nb_transport received trans " << hex << &trans << " (0x" << v::hex << v::setfill('0') << v::setw(8) << trans.get_address() << ") with delay " << delay << v::endl;
    v::analysis << name() << "ADDR=0x" << v::hex << trans.get_address() << " TYPE=ICACHE_READ"  << v::endl;

    // Put transaction in PEQ
    icio_PEQ.notify(trans, delay);

    // Reset delay
    delay = SC_ZERO_TIME;

    // Advance transaction state.
    phase = tlm::END_REQ;

    // Return state
    return tlm::TLM_UPDATED;

  } else if (phase == tlm::END_RESP) {

    // Return state
    return tlm::TLM_COMPLETED;

  } else {

    v::error << name() << "Illegal phase in call to icio_nb_transport_fw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

  }

  return tlm::TLM_COMPLETED;
}

/// Processes transactions from the icio_PEQ.
/// Contains a state machine to manage the communication path back to instruction initiator
/// Is registered as an SC_THREAD and sensitive to icio_PEQ.get_event()
void mmu_cache::icio_service_thread() {

  tlm::tlm_generic_payload *trans;
  tlm::tlm_sync_enum status;

  sc_core::sc_time delay;

  while(1) {

    // Process all icio transactions scheduled for the current time.
    // A return value of NULL indicates that the PEQ is empty at this time.

    while((trans = icio_PEQ.get_next_transaction()) != 0) {

      v::debug << name() << "Start ICIO transaction processing." << v::endl;

      delay = SC_ZERO_TIME;

      // Call the functional part of the model
      // -------------------------------------
      exec_instr(*trans, delay, false);
      // -------------------------------------

      v::debug << name() << "Consume component delay: " << delay << v::endl;

      // Consume delay
      wait(delay);
      delay = SC_ZERO_TIME;

      // Call master with phase BEGIN_RESP
      tlm::tlm_phase phase = tlm::BEGIN_RESP;

      v::debug << name() << "Call icio backward transport with phase " << phase << v::endl;
      status = icio->nb_transport_bw(*trans, phase, delay);

      // Check return status
      switch (status) {

      case tlm::TLM_COMPLETED:

        wait(delay);
        delay = SC_ZERO_TIME;
        break;

      case tlm::TLM_ACCEPTED:

        wait(delay);
        delay = SC_ZERO_TIME;
        break;

      default:

        v::error << name() << "TLM return status undefined or not valid!! " << v::endl;
        assert(0);

      } // switch

    } // while PEQ

    wait();

  }  // while thread
}

/// TLM non-blocking forward transport function for dcio socket
tlm::tlm_sync_enum mmu_cache::dcio_nb_transport_fw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  v::debug << name() << "DCIO nb_transport forward received transaction: " << hex << &trans << " with  phase " << phase << " and delay: " << delay << v::endl;
  
  // The master has sent BEGIN_REQ
  if (phase == tlm::BEGIN_REQ) {

    if (trans.is_read()) {
      v::debug << name() << "TRANS: " << globl_count++ << " dcio_nb_transport (READ) received trans " << hex << &trans << " (0x" << v::hex << v::setfill('0') << v::setw(8) << trans.get_address() << ") with delay " << delay << v::endl;
      v::analysis << name() << "ADDR=0x" << v::hex << trans.get_address() << " TYPE=DCACHE_READ"  << v::endl;  
    } else {
      v::debug << name() << "TRANS: " << globl_count++ << " dcio_nb_transport (WRITE) received trans " << hex << &trans << " (0x" << v::hex << v::setfill('0') << v::setw(8) << trans.get_address() << ") with delay " << delay << v::endl;
      v::analysis << name() << "ADDR=0x" << v::hex << trans.get_address() << " TYPE=DCACHE_WRITE"  << v::endl;
    }

    // Put transaction into PEQ
    dcio_PEQ.notify(trans, delay);
    delay = SC_ZERO_TIME;

    // Advance transaction state
    phase = tlm::END_REQ;

    // Return state
    return tlm::TLM_UPDATED;

  } else if (phase == tlm::END_RESP) {

    // Return state
    return tlm::TLM_COMPLETED;

  } else {

    v::error << name() << "Illegal phase in call to dcio_nb_transport_fw: " << phase << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

  }

  return tlm::TLM_COMPLETED;

}

/// Processes transactions from the dcio_PEQ.
/// Contains a state machine to manage the communication path back to the data initiator
/// Is registered as an SC_THREAD and sensitive to dcio_PEQ.get_event()
void mmu_cache::dcio_service_thread() {

  tlm::tlm_generic_payload *trans;
  tlm::tlm_sync_enum status;

  sc_core::sc_time delay;

  while(1) {

    // Process all dcio transactions scheduled for the current time.
    // A return value of NULL indicates that the PEQ is empty at this time.

    while((trans = dcio_PEQ.get_next_transaction()) != 0) {

      v::debug << name() << "Start dcio transaction processing." << v::endl;

      delay = SC_ZERO_TIME;

      // Call the functional part of the model
      // -------------------------------------
      exec_data(*trans, delay, false);
      // -------------------------------------

      v::debug << name() << "Consume component delay: " << delay << v::endl;

      // Consume delay
      wait(delay);
      delay = SC_ZERO_TIME;

      // Call master with phase BEGIN_RESP
      tlm::tlm_phase phase = tlm::BEGIN_RESP;

      v::debug << name() << "Call to dcio backward transport with phase " << phase << v::endl;
      status = dcio->nb_transport_bw(*trans, phase, delay);

      // Check return status
      switch (status) {

        case tlm::TLM_COMPLETED:

          wait(delay);
          delay = SC_ZERO_TIME;
          break;

        case tlm::TLM_ACCEPTED:

          wait(delay);
          delay = SC_ZERO_TIME;
          break;

        default:

          v::error << name() << "TLM return status undefined or not valid!! " << v::endl;
          break;

      } // switch

    } // while PEQ

    wait();

  } // while thread
}

// Instruction debug transport
unsigned int mmu_cache::icio_transport_dbg(tlm::tlm_generic_payload &trans) {

  sc_core::sc_time zero_delay = SC_ZERO_TIME;

  // Call the functional part of the model (in debug mode)
  // ----------------------------------
  exec_instr(trans, zero_delay, true);
  // -----------------------------------

  return trans.get_data_length();

}

// Data debug transport
unsigned int mmu_cache::dcio_transport_dbg(tlm::tlm_generic_payload &trans) {

  sc_core::sc_time zero_delay = SC_ZERO_TIME;

  // Call the functional part of the model (in debug mode)
  // ---------------------------------
  exec_data(trans, zero_delay, true);
  // ---------------------------------

  return trans.get_data_length();

}

/// @}
