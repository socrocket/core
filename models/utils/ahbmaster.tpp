#include "ahbmaster.h"

using namespace std;
using namespace sc_core;
using namespace tlm;

// Destructor (unregisters callbacks)
template<class BASE>
AHBMaster<BASE>::~AHBMaster() {
}

// TLM non-blocking backward transport function
template<class BASE>
tlm::tlm_sync_enum AHBMaster<BASE>::nb_transport_bw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  v::debug << this->name() << "nb_transport_bw received transaction " << hex << &trans << " with phase " << phase << v::endl;

  // The slave has sent END_REQ
  if (phase == tlm::END_REQ) {

    // Usually the slave would send TLM_UPDATED/END_REQ
    // on the return path. In case END_REQ comes via backward path,
    // notify ahbwrite that request phase is over.
    m_EndRequestEvent.notify();

    // For writes there will be no BEGIN_RESP.
    // Notify response thread to send BEGIN_DATA.
    if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {

      m_DataPEQ.notify(trans);

    }

    // Reset delay
    delay = SC_ZERO_TIME;

  } else if (phase == amba::DATA_SPLIT) {

    // Master never reacts on data split!!
    v::warn << this->name() << "Master received DATA_SPLIT" << v::endl;

  // New response - goes into response PEQ
  } else if (phase == tlm::BEGIN_RESP) {

    // Put new response into ResponsePEQ
    m_ResponsePEQ.notify(trans, delay);

    // Reset delay
    delay = SC_ZERO_TIME;

  // Data phase completed
  } else if (phase == amba::END_DATA) {

    // Reset delay
    delay = SC_ZERO_TIME;

  } else {

    v::error << this->name() << "Invalid phase in call to nb_transport_bw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

  }

  msclogger::return_forward(this, &ahb, &trans, tlm::TLM_ACCEPTED, delay);
  return tlm::TLM_ACCEPTED;
  
}

template<class BASE>
void AHBMaster<BASE>::ahbread(uint32_t addr, unsigned char * data, uint32_t length) {

  bool cacheable;
  tlm::tlm_response_status response;
  sc_core::sc_time delay = SC_ZERO_TIME;

  ahbread(addr, data, length, delay, cacheable, response);

}

template<class BASE>
void AHBMaster<BASE>::ahbwrite(uint32_t addr, unsigned char * data, uint32_t length) {

  sc_core::sc_time delay = SC_ZERO_TIME;
  tlm::tlm_response_status response;

  ahbwrite(addr, data, length, delay, response);

}

// Function for read access to AHB master socket
template<class BASE>
void AHBMaster<BASE>::ahbread(uint32_t addr, unsigned char * data, uint32_t length, sc_core::sc_time &delay, bool &cacheable, tlm::tlm_response_status &response) {

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;

  // Allocate new transaction (reference counter = 1)
  tlm::tlm_generic_payload *trans = ahb.get_transaction();

  v::debug << this->name() << "Allocate new transaction: " << hex << trans << v::endl;

  // Initialize transaction
  trans->set_command(tlm::TLM_READ_COMMAND);
  trans->set_address(addr);
  trans->set_data_length(length);
  trans->set_data_ptr(data);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  // Set transfer type extension
  amba::amba_trans_type * trans_ext;
  ahb.validate_extension<amba::amba_trans_type> (*trans);
  ahb.get_extension<amba::amba_trans_type> (trans_ext, *trans);
  trans_ext->value = amba::NON_SEQUENTIAL;

  if (m_ambaLayer == amba::amba_LT) {

    // Blocking transport
    msclogger::forward(this, &ahb, trans, tlm::BEGIN_REQ);
    ahb->b_transport(*trans, delay);

    // Check the data
    checkTXN(trans);

    // Consume transfer delay
    wait(delay);
    delay = SC_ZERO_TIME;

    cacheable = (ahb.get_extension<amba::amba_cacheable>(*trans)) ? true : false;

    response = trans->get_response_status();    

    // Decrement reference counter
    trans->release();

  } else {

    // Initial phase for AT
    phase = tlm::BEGIN_REQ;

    v::debug << this->name() << "Transaction " << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;
    msclogger::forward(this, &ahb, trans, phase, delay);

    // Non-blocking transport
    status = ahb->nb_transport_fw(*trans, phase, delay);

    switch (status) {

      case tlm::TLM_ACCEPTED:
      case tlm::TLM_UPDATED:

	if (phase == tlm::BEGIN_REQ) {

	  // The slave returned TLM_ACCEPTED.
	  // Wait until END_REQ before giving control
	  // to the user (for sending next transaction).

	  wait(m_EndRequestEvent);
	  
	} else if (phase == tlm::END_REQ) {

	  // The slave returned TLM_UPDATED with END_REQ.
	  // Wait until BEGIN_RESP comes in on the backward path
	  // and then return control to the user (for putting next
	  // transaction into pipeline).

	  wait(m_EndRequestEvent);
	  
	} else if (phase == tlm::BEGIN_RESP) {

	  // Slave directly jumped to BEGIN_RESP.
	  // Notify the response thread and return control to user.
	  m_ResponsePEQ.notify(*trans, delay);

	} else {

	  // Forbidden phase
	  v::error << this->name() << "Invalid phase in return path (from call to nb_transport_fw)!" << v::endl;
	  trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

	}

	break;

      case tlm::TLM_COMPLETED:

	// Slave directly jumps to TLM_COMPLETED

	break;
      
      default:

	v::error << this->name() << "Invalid return value from call to nb_transport_fw!" << v::endl;
	trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

    }

    response = trans->get_response_status();
    cacheable = (ahb.get_extension<amba::amba_cacheable>(*trans)) ? true : false;
  }
}

// Perform AHB debug read
template<class BASE>
uint32_t AHBMaster<BASE>::ahbread_dbg(uint32_t addr, unsigned char * data, unsigned int length) {

  uint32_t length_dbg;

  // Allocate new transaction (reference counter = 1)
  tlm::tlm_generic_payload *trans = ahb.get_transaction();

  v::debug << this->name() << "Allocate new transaction " << hex << trans << v::endl;

  // Initialize transaction
  trans->set_command(tlm::TLM_READ_COMMAND);
  trans->set_address(addr);
  trans->set_data_length(length);
  trans->set_data_ptr(data);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);  
  
  amba::amba_trans_type * trans_ext;
  ahb.validate_extension<amba::amba_trans_type> (*trans);
  ahb.get_extension<amba::amba_trans_type> (trans_ext, *trans);
  trans_ext->value = amba::NON_SEQUENTIAL;

  // Collect transport statistics
  transport_statistics(*trans);

  // Debug transport
  length_dbg = ahb->transport_dbg(*trans);
  
  // Decrement reference count
  trans->release();

  return length_dbg;

}

// Function for write access to AHB master socket
template<class BASE>
void AHBMaster<BASE>::ahbwrite(uint32_t addr, unsigned char * data, uint32_t length, sc_core::sc_time &delay, tlm::tlm_response_status &response) {

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;

  // Allocate new transaction (reference counter = 1)
  tlm::tlm_generic_payload *trans = ahb.get_transaction();

  v::debug << this->name() << "Allocate new transaction " << hex << trans << v::endl;

  // Initialize transaction
  trans->set_command(tlm::TLM_WRITE_COMMAND);
  trans->set_address(addr);
  trans->set_data_length(length);
  trans->set_data_ptr(data);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  // Set transfer type extension
  amba::amba_trans_type * trans_ext;
  ahb.validate_extension<amba::amba_trans_type> (*trans);
  ahb.get_extension<amba::amba_trans_type> (trans_ext, *trans);
  trans_ext->value = amba::NON_SEQUENTIAL;

  if (m_ambaLayer == amba::amba_LT) {

    // Blocking transport
    msclogger::forward(this, &ahb, trans, tlm::BEGIN_REQ);
    ahb->b_transport(*trans, delay);

    // Consume transfer delay
    wait(delay);
    delay = SC_ZERO_TIME;

    response = trans->get_response_status();

    // Decrement reference counter
    trans->release();

  } else {

    // Initial phase for AT
    phase = tlm::BEGIN_REQ;

    v::debug << this->name() << "Transaction " << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;
    msclogger::forward(this, &ahb, trans, phase, delay);

    // Non-blocking transport
    status = ahb->nb_transport_fw(*trans, phase, delay);

    switch (status) {

      case tlm::TLM_ACCEPTED:
      case tlm::TLM_UPDATED:

	if (phase == tlm::BEGIN_REQ) {

	  // The slave returned TLM_ACCEPTED
	  // Wait until END_REQ comes in on backward path
	  // before starting DATA phase.
	  wait(m_EndRequestEvent);

	} else if (phase == tlm::END_REQ) {

	  // The slave returned TLM_UPDATED with END_REQ

	} else if (phase == amba::END_DATA) {

	  // Done - return control to user.

	} else {

	  // Forbidden phase
	  v::error << this->name() << "Invalid phase in return path (from call to nb_transport_fw)!" << v::endl;
	  trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

	}

	break;

      case tlm::TLM_COMPLETED:

	// Slave directly jumps to TLM_COMPLETED (Pseudo AT).
	// Don't send END_RESP
	//wait(delay);

	break;
      
      default:

	v::error << this->name() << "Invalid return value from call to nb_transport_fw!" << v::endl;
	trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

    }

    response = trans->get_response_status();
  }
}


// Perform AHB debug write
template<class BASE>
uint32_t AHBMaster<BASE>::ahbwrite_dbg(uint32_t addr, unsigned char * data, unsigned int length) {

  uint32_t length_dbg;

  // Allocate new transaction (reference counter = 1)
  tlm::tlm_generic_payload *trans = ahb.get_transaction();

  v::debug << this->name() << "Allocate new transaction " << hex << trans << v::endl;

  // Initialize transaction
  trans->set_command(tlm::TLM_WRITE_COMMAND);
  trans->set_address(addr);
  trans->set_data_length(length);
  trans->set_data_ptr(data);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);  
  
  amba::amba_trans_type * trans_ext;
  ahb.validate_extension<amba::amba_trans_type> (*trans);
  ahb.get_extension<amba::amba_trans_type> (trans_ext, *trans);
  trans_ext->value = amba::NON_SEQUENTIAL;

  // Collect transport statistics
  transport_statistics(*trans);

  // Debug transport
  length_dbg = ahb->transport_dbg(*trans);
  
  // Decrement reference count
  trans->release();

  return length_dbg;

}


// Thread for response synchronization (sync and send END_RESP)
template<class BASE>
void AHBMaster<BASE>::ResponseThread() {

  tlm::tlm_generic_payload* trans;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;

  while(1) {

    // Wait for response from slave
    wait(m_ResponsePEQ.get_event());

    // Get transaction from PEQ
    trans = m_ResponsePEQ.get_next_transaction();

    // Check result
    checkTXN(trans);
  
    // Prepare END_RESP
    phase = tlm::END_RESP;
    delay = sc_core::SC_ZERO_TIME;

    v::debug << this->name() << "Transaction " << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;
    msclogger::forward(this, &ahb, trans, phase, delay);

    // Call nb_transport_fw with END_RESP
    status = ahb->nb_transport_fw(*trans, phase, delay);

    // Return value must be TLM_COMPLETED or TLM_ACCEPTED
    assert((status==tlm::TLM_COMPLETED)||(status==tlm::TLM_ACCEPTED));

    // Cleanup (add some additional delay, to allow checkTXN to complete)

  }
}

// Thread for data phase processing in write operations (sends BEGIN_DATA)
template<class BASE>
void AHBMaster<BASE>::DataThread() {

  tlm::tlm_generic_payload* trans;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;

  while(1) {

    // Wait for new data phase
    wait(m_DataPEQ.get_event());

    // Get transaction from PEQ
    trans = m_DataPEQ.get_next_transaction();

    // Prepare BEGIN_DATA
    phase = amba::BEGIN_DATA;
    delay = SC_ZERO_TIME;

    v::debug << this->name() << "Transaction " << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;
    msclogger::forward(this, &ahb, trans, phase, delay);

    // Call nb_transport_fw with BEGIN_DATA
    status = ahb->nb_transport_fw(*trans, phase, delay);

    switch (status) {

      case tlm::TLM_ACCEPTED:
      case tlm::TLM_UPDATED:

	if (phase == amba::BEGIN_DATA) {

	  // The slave returned TLM_ACCEPTED.
	  // Wait for END_DATA to come in on backward path.

	  // v::debug << this->name() << "Waiting mEndDataEvent" << v::endl;
	  // wait(mEndDataEvent);
	  // v::debug << this->name() << "mEndDataEvent" << v::endl;

	} else if (phase == amba::END_DATA) {

	  // Slave sent TLM_UPDATED/END_DATA.
	  // Data phase completed.
	  // wait(delay);

	} else {

	  // Forbidden phase
	  v::error << this->name() << "Invalid phase in return path (from call to nb_transport_fw)!" << v::endl;

	}

	break;

      case tlm::TLM_COMPLETED:

	// Slave directly jumps to TLM_COMPLETED (Pseudo AT).
	//wait(delay);
	
	break;

    }
  }
}
