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
