using namespace std;
using namespace sc_core;
using namespace tlm;

// Destructor (unregisters callbacks)
template<class BASE>
AHBSlave<BASE>::~AHBSlave() {
}

// TLM non-blocking forward transport function
template<class BASE>
tlm::tlm_sync_enum AHBSlave<BASE>::nb_transport_fw(tlm::tlm_generic_payload &trans, tlm::tlm_phase& phase, sc_core::sc_time& delay) {

  v::debug << this->name() << "nb_transport_fw received transaction " << hex << &trans << " with phase: " << phase << v::endl;

  // The master has sent BEGIN_REQ
  if(phase == tlm::BEGIN_REQ) {

    trans.acquire();

    m_AcceptPEQ.notify(trans, delay);

    delay = SC_ZERO_TIME;
    phase = tlm::END_REQ;

  } else if(phase == amba::BEGIN_DATA) {
    
    m_TransactionPEQ.notify(trans, delay);
    delay = SC_ZERO_TIME;

  } else if(phase == tlm::END_RESP) {
    
    // nothing to do

  } else {
  
    v::error << this->name() << "Invalid phase in call to nb_transport_fw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

  }
  
  msclogger::return_backward(this, &ahb, &trans, tlm::TLM_ACCEPTED, delay);
  return(tlm::TLM_UPDATED);
}

// Thread for modeling the AHB pipeline delay
template<class BASE>
void AHBSlave<BASE>::acceptTXN() {

  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;

  tlm::tlm_generic_payload * trans;

  while(1) {
    
    wait(m_AcceptPEQ.get_event());

    while((trans = m_AcceptPEQ.get_next_transaction())) {

      // Read transaction will be processed directly.
      // For write we wait for BEGIN_DATA (see nb_transport_fw)
      if(trans->get_command() == TLM_READ_COMMAND) {
        m_TransactionPEQ.notify(*trans);
      }

      // Check if new transaction can be accepted
      if(busy == true) {
        wait(unlock_event);
      }

      // Send END_REQ
      phase = tlm::END_REQ;
      delay = SC_ZERO_TIME;
      
      v::debug << this->name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << phase << v::endl;
      msclogger::backward(this, &ahb, trans, phase, delay);
      
      // Call to backward transport
      status = ahb->nb_transport_bw(*trans, phase, delay);
      assert(status==tlm::TLM_ACCEPTED);
    }
  }
}

// Process for interfacing the functional part of the model in AT mode
template<class BASE>
void AHBSlave<BASE>::processTXN() {
 
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;
  tlm::tlm_generic_payload *trans;

  while(1) {

    wait(m_TransactionPEQ.get_event());

    while((trans = m_TransactionPEQ.get_next_transaction())) {

      v::debug << this->name() << "Process transaction " << hex << trans << v::endl;

      // Reset delay
      delay = SC_ZERO_TIME;

      // Call the functional part of the model
      exec_func(*trans, delay);

      // Device busy (can not accept new transaction anymore)
      busy = true;

      // Consume component delay
      wait(delay);
      delay = SC_ZERO_TIME;
      
      // Device idle
      busy = false;

      // Ready to accept new transaction
      unlock_event.notify();

      if(trans->get_command() == tlm::TLM_WRITE_COMMAND) {

        // For write commands send END_DATA.
        // Transaction has been delayed until begin of Data Phase (see transport_fw)
        phase = amba::END_DATA;

        v::debug << this->name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << phase << " (delay: " << delay << ")" << v::endl;
        msclogger::backward(this, &ahb, trans, phase, delay);

        // Call backward transport
        status = ahb->nb_transport_bw(*trans, phase, delay);

        assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_COMPLETED));

      } else {

        // Read command - send BEGIN_RESP
        phase = tlm::BEGIN_RESP;
        
        v::debug << this->name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << phase << " (delay: " << delay << ")" << v::endl;
        msclogger::backward(this, &ahb, trans, phase, delay);

        // Call backward transport
        status = ahb->nb_transport_bw(*trans, phase, delay);
        assert(status==tlm::TLM_ACCEPTED);
        
      }

      // Decrement reference counter
      trans->release();
    }
  }
}
       

// TLM blocking transport function
template<class BASE>
void AHBSlave<BASE>::b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {

  // Call the functional part of the model
  // -------------------------------------
  exec_func(trans, delay);

  msclogger::return_backward(this, &ahb, &trans, tlm::TLM_ACCEPTED, delay);
}

// TLM blocking transport function
template<class BASE>
uint32_t AHBSlave<BASE>::transport_dbg(tlm::tlm_generic_payload& trans) {
  // Call the functional part of the model
  // -------------------------------------
  sc_time delay = SC_ZERO_TIME;
  return exec_func(trans, delay);
}


/* vim: set expandtab noai ts=4 sw=4: */
/* -*- mode: c-mode; tab-width: 4; indent-tabs-mode: nil; -*- */

