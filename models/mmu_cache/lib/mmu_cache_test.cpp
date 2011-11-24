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
// The program is provided "as is", ther is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      mmu_cache_test.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implementation of top-level class for all mmu_cache tests.
//             All tests of this module inherit from this class.
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

#include "mmu_cache_test.h"

// Constructor
mmu_cache_test::mmu_cache_test(sc_core::sc_module_name name,
			       amba::amba_layer_ids abstractionLayer) :
  sc_module(name),
  icio("icio"),
  dcio("dcio"),
  m_InstrResponsePEQ("InstrResponsePEQ"),
  m_DataResponsePEQ("DataResponsePEQ"),
  m_DataPEQ("DataPEQ"),
  m_EndTransactionPEQ("EndTransactionPEQ"),
  m_CheckPEQ("CheckPEQ"),
  m_abstractionLayer(abstractionLayer) {
  // For AT abstraction layer
  if (m_abstractionLayer == amba::amba_AT) {

    // Register non-blocking backward transport functions
    icio.register_nb_transport_bw(this, &mmu_cache_test::icio_nb_transport_bw);
    dcio.register_nb_transport_bw(this, &mmu_cache_test::dcio_nb_transport_bw);

    // Register threads for response processing
    SC_THREAD(InstrResponseThread);
    SC_THREAD(DataResponseThread);
    SC_THREAD(DataThread);
    SC_THREAD(cleanUP);
    SC_THREAD(check_delayed);

  }

  // Reset test counter and error counter
  tc = 0;
  ec = 0;

}

unsigned char * mmu_cache_test::get_datap() {

  return(data+tc);

}

unsigned char * mmu_cache_test::get_datap_word(unsigned int value) {

  memset(data+tc,0,4);
  
  // Assume LE host simulating BE master
  data[tc+3] = value & 0xff;
  data[tc+2] = (value >> 8) & 0xff;
  data[tc+1] = (value >> 16) & 0xff;
  data[tc+0] = (value >> 24) & 0xff;

  return(data+tc);

}

unsigned char * mmu_cache_test::get_datap_short(unsigned int value) {

  memset(data+tc,0,2);

  // Assume LE host simulating BE master
  data[tc+1] = value & 0xff; 
  data[tc+0] = (value >> 8) & 0xff;

  return(data+tc);

}

unsigned char * mmu_cache_test::get_datap_byte(unsigned int value) {
 
  memset(data+tc,0,1);

  data[tc] = value;

  return(data+tc);

}

unsigned char * mmu_cache_test::get_refp() {

  return(ref+tc);

}

unsigned char * mmu_cache_test::get_refp_word(unsigned int value) {

  memset(ref+tc,0,4);
  
  // Assume LE host simulating BE master
  ref[tc+3]   = value & 0xff;
  ref[tc+2] = (value >> 8) & 0xff;
  ref[tc+1] = (value >> 16) & 0xff;
  ref[tc+0] = (value >> 24) & 0xff;

  return(ref+tc);

}

unsigned char * mmu_cache_test::get_refp_short(unsigned int value) {

  memset(ref+tc,0,4);

  // Assume LE host simulating BE master
  ref[tc+1] = value & 0xff;
  ref[tc+0] = (value >> 8) & 0xff;

  return(ref+tc);

}

unsigned char * mmu_cache_test::get_refp_byte(unsigned int value) {

  memset(ref+tc,0,1);

  ref[tc] = value;

  return(ref+tc);

}

unsigned int * mmu_cache_test::get_debugp() {

  return(debug+(tc>>2));

}

unsigned int * mmu_cache_test::get_debugp_clean() {

  memset(debug+(tc>>2),0,4);
  return(debug+(tc>>2));
}

// Increment test counter (tc)
// tc is offset for data, ref and debug arrays
void mmu_cache_test::inc_tptr() {

  tc = (tc + 4) % 1024;
  
}

// Increment error counter 
void mmu_cache_test::inc_ec() {

  ec++;

}

unsigned int mmu_cache_test::error_stat() {

  v::info << name() << "Total number of errors during test: " << ec << v::endl;

  return(ec);

}

// Result checking without debug pointer
void mmu_cache_test::check(unsigned char * result, unsigned char * refer, unsigned int len) {

  unsigned int dummy = 0;

  check(result, refer, len, &dummy, NOCHECK);

}

// Function for result checking / Prototype with debug checking
void mmu_cache_test::check(unsigned char * result, unsigned char * refer, unsigned int len, unsigned int * debug, check_t check) {

  checkpair_type* checkpair;
  unsigned int i;
  bool is_error = false;

  if (m_abstractionLayer == amba::amba_LT) {

    // 1. Check Data
    // =============

    // For blocking communication - check immediately
    for (i=0; i<len; i++) {

      if (result[i] != refer[i]) {

	is_error = true;
	ec++;

      }
    }

    if (is_error) {

      v::error << name() << "Testbench Error (Expected/Received): " << v::endl;

      for (i=0; i<len; i++) {

	v::error << name() << hex << (unsigned int)refer[i] << "/" << (unsigned int)result[i] << v::endl;

      }
    }

    // 2. Check Debug Info
    // ===================

    switch (check) {

      case FROZENMISS:

	if (!FROZENMISS_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no FROZEN MISS!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

        }

	break;

      case NOTFROZENMISS:

	if (FROZENMISS_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - FROZEN MISS!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

	}

	break;

      case CACHEBYPASS:

	if (!CACHEBYPASS_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no CACHE BYPASS!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

        }

	break;

      case SCRATCHPAD:

        if (!SCRATCHPAD_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no SCRATCHPAD ACCESS!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

        }

	break;     

      case CACHEREADHIT: 

	if (!CACHEREADHIT_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no READ HIT!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

	}

	break;

      case CACHEREADMISS: 

	if (!CACHEREADMISS_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no READ MISS!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

	}
	  
	break;

      case CACHEWRITEHIT: 

	if (!CACHEWRITEHIT_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no WRITE HIT!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

	}

	break;

      case CACHEWRITEMISS: 

	if (!CACHEWRITEMISS_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no WRITE MISS!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

	}
	
	break;

      case TLBHIT:

	if (!TLBHIT_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no TLB HIT!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

	}
	
	break;

      case TLBMISS:

	if (!TLBMISS_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no TLB MISS!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

	}
	
	break;
	

      default:
	break;

    }

  } else {

    sc_core::sc_time delay;
    delay = sc_time(100, SC_NS);

    checkpair = new checkpair_type;

    // For non-blocking communication - use PEQ and check later
    checkpair->result     = result;
    checkpair->refer      = refer;
    checkpair->len        = len;
    checkpair->check_time = sc_time_stamp();
    checkpair->debug      = debug;
    checkpair->check      = check;

    m_CheckPEQ.notify(*checkpair, delay);

  }
}

// Thread for delayed result checking (AT pipeline)
void mmu_cache_test::check_delayed() {

  unsigned int i, j;
  checkpair_type* checkpair;

  while(1) {

    wait(m_CheckPEQ.get_event());

    checkpair = m_CheckPEQ.get_next_transaction();

    // 1. Check Data
    // =============

    for (i=0; i<checkpair->len; i++) {

      if (checkpair->result[i] != checkpair->refer[i]) {

	v::error << name() << "Testbench Error (Expected/Received) from check @ " << checkpair->check_time << v::endl;
	
	for (j=0; j<checkpair->len; j++) {

	  v::error << name() << hex << (unsigned int)checkpair->refer[j] << "/" << hex << (unsigned int)checkpair->result[j] << v::endl;
	  ec++;

	}
      } 
    }

    // 2. Check Debug Info
    // ===================
    
    switch (checkpair->check) {

      case FROZENMISS:

	if (!FROZENMISS_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no FROZEN MISS!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

        }

	break;

      case NOTFROZENMISS:

	if (FROZENMISS_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - FROZEN MISS!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

	}

	break;

      case CACHEBYPASS:

	if (!CACHEBYPASS_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no CACHE BYPASS!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

        }

	break;

      case SCRATCHPAD:

        if (!SCRATCHPAD_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no SCRATCHPAD ACCESS!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

        }

	break;     

      case CACHEREADHIT: 

	if (!CACHEREADHIT_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no READ HIT!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

	}

	break;

      case CACHEREADMISS: 

	if (!CACHEREADMISS_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no READ MISS!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

	}
	  
	break;

      case CACHEWRITEHIT: 

	if (!CACHEWRITEHIT_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no WRITE HIT!!" << v::endl;
	  ec++;

	}

	break;

      case CACHEWRITEMISS: 

	if (!CACHEWRITEMISS_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no WRITE MISS!!" << v::endl;
	  ec++;

	}
	
	break;

      case TLBHIT:

	if (!TLBHIT_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no TLB HIT!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

	}
	
	break;

      case TLBMISS:

	if (!TLBMISS_CHECK(*debug)) {

	  v::error << name() << "Unexpected type of access - no TLB MISS!! (debug = " << hex << *debug << ")" << v::endl;
	  ec++;

	}
	
	break;

      default:

	break;

    }    
  }
}

// Delayed release of transactions
void mmu_cache_test::cleanUP() {

  tlm::tlm_generic_payload * trans;

  while(1) {

    wait(m_EndTransactionPEQ.get_event());

    while((trans = m_EndTransactionPEQ.get_next_transaction())) {

      v::debug << name() << "Release transaction: " << hex << trans << v::endl;

      // remove transaction
      delete(trans);

    }
  }
}

// TLM non-blocking backward transport function for icio socket
tlm::tlm_sync_enum mmu_cache_test::icio_nb_transport_bw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  v::debug << name() << "icio_nb_transport_bw received transaction " << hex << &trans << " with phase " << phase << v::endl;

  // The slave has sent END_REQ
  if (phase == tlm::END_REQ) {

    // Inform request thread about END_REQ
    m_EndInstrRequestEvent.notify();
    
    // Reset delay
    delay = SC_ZERO_TIME;

  // New response
  } else if (phase == tlm::BEGIN_RESP) {

    // Put new response into InstrResponsePEQ
    m_InstrResponsePEQ.notify(trans, delay);

    // Reset delay
    delay = SC_ZERO_TIME;
    
  } else {

    v::error << name() << "Invalid phase in call to icio_nb_transport_bw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

  }

  return(tlm::TLM_ACCEPTED);
}

// TLM non-blocking backward transport function for dcio socket
tlm::tlm_sync_enum mmu_cache_test::dcio_nb_transport_bw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  v::debug << name() << "dcio_nb_transport_bw received transaction " << hex << &trans << " with phase " << phase << v::endl;

  // The slave has sent END_REQ
  if (phase == tlm::END_REQ) {

    // Inform request thread about END_REQ
    m_EndDataRequestEvent.notify();

    // For writes there will be no BEGIN_RESP.
    // Notify response thread to send BEGIN_DATA.
    if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {

      m_DataPEQ.notify(trans);

    }

    // Reset delay
    delay = SC_ZERO_TIME;

  // New response
  } else if (phase == tlm::BEGIN_RESP) {

    v::debug << name() << "TBM RECEIVED DATA: " << hex << *(unsigned int*)trans.get_data_ptr() << v::endl;

    // Put new response into DataResponsePEQ
    m_DataResponsePEQ.notify(trans, delay);

  // Data phase completed
  } else if (phase == amba::END_DATA) {

    // Release transaction
    m_EndTransactionPEQ.notify(trans, delay);

    // Reset delay
    delay = SC_ZERO_TIME;

  } else {

    v::error << name() << "Invalid phase in call to dcio_nb_transport_bw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

  }

  return(tlm::TLM_ACCEPTED);

}

// Instruction read
void mmu_cache_test::iread(unsigned int addr, unsigned char * data, unsigned int flush, unsigned int flushl, unsigned int fline, unsigned int *debug) {

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;
  sc_core::sc_time delay;

  // Allocate new transaction + extension set
  tlm::tlm_generic_payload *trans = new tlm::tlm_generic_payload();
  icio_payload_extension *iext = new icio_payload_extension();

  v::debug << name() << "Allocate new transaction (iread): " << hex << trans << v::endl;

  // Clear debug field
  *debug = 0;

  // Initialize transaction
  trans->set_command(tlm::TLM_READ_COMMAND);
  trans->set_address(addr);
  trans->set_data_length(4);
  trans->set_data_ptr(data);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  // Initialize extensions
  iext->flush  = flush;
  iext->flushl = flushl;
  iext->fline  = fline;
  iext->debug  = debug;

  // Hook extension to payload
  trans->set_extension(iext);

  // Init delay
  delay = SC_ZERO_TIME;

  if (m_abstractionLayer == amba::amba_LT) {

    // Blocking transport
    icio->b_transport(*trans, delay);

  } else {

    // Initial phase for AT
    phase = tlm::BEGIN_REQ;

    v::debug << name() << "Transaction (icio) " << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;

    // Non-blocking transport
    status = icio->nb_transport_fw(*trans, phase, delay);

    switch (status) {

      case tlm::TLM_ACCEPTED:
      case tlm::TLM_UPDATED:

	if (phase == tlm::BEGIN_REQ) {

	  // The slave returned TLM_ACCEPTED.
	  // Wait until END_REQ before giving control
	  // to the user (for sending next transaction).

	  wait(m_EndInstrRequestEvent);

        } else if (phase == tlm::END_REQ) {

	  // The slave returned TLM_UPDATED with END_REQ
	  // Wait until BEGIN_RESP comes in on the backward path
	  // and the return control to the user (for putting next
	  // transaction into the pipeline.

	  wait(m_EndInstrRequestEvent);
	
	} else if (phase == tlm::BEGIN_RESP) {

	  // Slave directly jumped to BEGIN_RESP
	  // Notify the response thread and return control to user
	  m_InstrResponsePEQ.notify(*trans, delay);

	} else {

 	  // Forbidden phase
	  v::error << name() << "Invalid phase from call to nb_transport_fw (icio)!" << v::endl;
	  trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

	}

	break;

      case tlm::TLM_COMPLETED:

	// Slave directly jumps to TLM_COMPLETED
  
	break;

      default:
	
	v::error << name() << "Invalid return value from call to nb_transport_fw (icio)!" << v::endl;
	trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

    }
  }
}

// Data read
void mmu_cache_test::dread(unsigned int addr, unsigned char * data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock, unsigned int *debug) {

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;
  sc_core::sc_time delay;

  // Allocate new transaction + extension set
  // (ICIO is used to obtain data transaction, in order to maintain only one memory pool.)
  tlm::tlm_generic_payload *trans = new tlm::tlm_generic_payload();
  dcio_payload_extension *dext = new dcio_payload_extension();

  v::debug << name() << "Allocate new transaction (dread): " << hex << trans << v::endl;

  // Clear debug field
  *debug = 0;

  // Initialize transaction
  trans->set_command(tlm::TLM_READ_COMMAND);
  trans->set_address(addr);
  trans->set_data_length(length);
  trans->set_data_ptr(data);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  // Initialize extensions
  dext->asi    = asi;
  dext->flush  = flush;
  dext->flushl = flushl;
  dext->lock   = lock;
  dext->debug  = debug;

  // Hook extension to payload
  trans->set_extension(dext);

  // Init delay
  delay = SC_ZERO_TIME;

  if (m_abstractionLayer == amba::amba_LT) {

    // Blocking transport
    dcio->b_transport(*trans, delay);

  } else {

    // Initial phase for AT
    phase = tlm::BEGIN_REQ;

    v::debug << name() << "Transaction (dcio) " << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;

    // Non-blocking transport
    status = dcio->nb_transport_fw(*trans, phase, delay);
    
    switch (status) {

      case tlm::TLM_ACCEPTED:
      case tlm::TLM_UPDATED:

	if (phase == tlm::BEGIN_REQ) {

	  // The slave returned TLM_ACCEPTED.
	  // Wait until END_REQ before giving control
	  // to the user (for sending next transaction).

	  wait(m_EndDataRequestEvent);

        } else if (phase == tlm::END_REQ) {

	  // The slave returned TLM_UPDATED with END_REQ
	  // Wait until BEGIN_RESP comes in on the backward path
	  // and the return control to the user (for putting next
	  // transaction into the pipeline.

	  wait(m_EndDataRequestEvent);
	
	} else if (phase == tlm::BEGIN_RESP) {

	  // Slave directly jumped to BEGIN_RESP
	  // Notify the response thread and return control to user
	  m_DataResponsePEQ.notify(*trans, delay);

	} else {

 	  // Forbidden phase
	  v::error << name() << "Invalid phase from call to nb_transport_fw (dcio)!" << v::endl;
	  trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

	}

	break;

      case tlm::TLM_COMPLETED:

	// Slave directly jumps to TLM_COMPLETED
  
	break;

      default:
	
	v::error << name() << "Invalid return value from call to nb_transport_fw (dcio)!" << v::endl;
	trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

    }
  }
}

// Data write
void mmu_cache_test::dwrite(unsigned int addr, unsigned char * data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock, unsigned int *debug) {

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;
  sc_core::sc_time delay;

  // Allocate new transaction + extension set
  tlm::tlm_generic_payload *trans = new tlm::tlm_generic_payload();
  dcio_payload_extension *dext = new dcio_payload_extension();

  // Clear debug field
  *debug = 0;

  // Initialize transaction
  trans->set_command(tlm::TLM_WRITE_COMMAND);
  trans->set_address(addr);
  trans->set_data_length(length);
  trans->set_data_ptr(data);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  // Initialize extensions
  dext->asi    = asi;
  dext->flush  = flush;
  dext->flushl = flushl;
  dext->lock   = lock;
  dext->debug  = debug;

  // Hook extension to payload
  trans->set_extension(dext);

  // Init delay
  delay = SC_ZERO_TIME;
  
    if (m_abstractionLayer == amba::amba_LT) {

    // Blocking transport
    dcio->b_transport(*trans, delay);

  } else {

    // Initial phase for AT
    phase = tlm::BEGIN_REQ;

    v::debug << name() << "Transaction (dcio) " << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;

    // Non-blocking transport
    status = dcio->nb_transport_fw(*trans, phase, delay);
    
    switch (status) {

      case tlm::TLM_ACCEPTED:
      case tlm::TLM_UPDATED:

	if (phase == tlm::BEGIN_REQ) {

	  // The slave returned TLM_ACCEPTED.
	  // Wait until END_REQ comes in on backward path
	  // before starting DATA phase.
	  wait(m_EndDataRequestEvent);

        } else if (phase == tlm::END_REQ) {

	  // The slave returned TLM_UPDATED with END_REQ
	
	} else if (phase == amba::END_DATA) {

	  // Done - return control to user

	} else {

 	  // Forbidden phase
	  v::error << name() << "Invalid phase from call to nb_transport_fw (dcio)!" << v::endl;
	  trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

	}

	break;

      case tlm::TLM_COMPLETED:

	// Slave directly jumps to TLM_COMPLETED
  
	break;

      default:
	
	v::error << name() << "Invalid return value from call to nb_transport_fw (dcio)!" << v::endl;
	trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

    }
  }
}

// Thread for instruction response synchronization (sync and send END_RESP)
void mmu_cache_test::InstrResponseThread() {

  tlm::tlm_generic_payload* trans;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;

  while(1) {

    // Wait for response from slave
    wait(m_InstrResponsePEQ.get_event());

    // Get transaction from PEQ
    trans = m_InstrResponsePEQ.get_next_transaction();

    // Prepare END_RESP
    phase = tlm::END_RESP;
    delay = SC_ZERO_TIME;

    v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_fw with phase (icio) with phase " << phase << v::endl;

    // Call nb_transport_fw with END_RESP
    status = icio->nb_transport_fw(*trans, phase, delay);

    // Return value must be TLM_COMPLETED or TLM_ACCEPTED
    assert((status==tlm::TLM_COMPLETED)||(status==tlm::TLM_ACCEPTED));

    // Cleanup
    m_EndTransactionPEQ.notify(*trans, delay);
  }
}

// Thread for data response synchronization (sync and send END_RESP)
void mmu_cache_test::DataResponseThread() {

  tlm::tlm_generic_payload* trans;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;

  while(1) {

    // Wait for response from slave
    wait(m_DataResponsePEQ.get_event());

    // Get transaction from PEQ
    trans = m_DataResponsePEQ.get_next_transaction();

    // Prepare END_RESP
    phase = tlm::END_RESP;
    delay = SC_ZERO_TIME;

    v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_fw (dcio) with phase " << phase << v::endl;

    // Call nb_transport_fw with END_RESP
    status = dcio->nb_transport_fw(*trans, phase, delay);

    // Return value must be TLM_COMPLETED or TLM_ACCEPTED
    assert((status==tlm::TLM_COMPLETED)||(status==tlm::TLM_ACCEPTED));

    // Cleanup
    m_EndTransactionPEQ.notify(*trans, delay);

  }  
}

// Thread for data phase processing in write operations (sends BEGIN_DATA)
void mmu_cache_test::DataThread() {

  tlm::tlm_generic_payload* trans;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;

  while(1) {

    wait(m_DataPEQ.get_event());

    // Get transaction from PEQ
    trans = m_DataPEQ.get_next_transaction();

    // Prepare BEGIN_DATA
    phase = amba::BEGIN_DATA;
    delay = SC_ZERO_TIME;

    v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_fw (dcio) with phase " << phase << v::endl;

    // Call nb_transport_fw with BEGIN_DATA
    status = dcio->nb_transport_fw(*trans, phase, delay);

    // Return value must be TLM_COMPLETED or TLM_ACCEPTED
    assert((status==tlm::TLM_COMPLETED)||(status==tlm::TLM_ACCEPTED));

    if (status == tlm::TLM_COMPLETED) {

      // Cleanup
      m_EndTransactionPEQ.notify(*trans, delay);

    }

  }
}
