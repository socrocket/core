// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file ahbmaster.tpp
///
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

// Constructor (sc_module version)
template<class BASE>
AHBMaster<BASE>::AHBMaster(
    ModuleName nm,
    uint8_t hindex,
    uint8_t vendor,
    uint8_t device,
    uint8_t version,
    uint8_t irq,
    AbstractionLayer ambaLayer,
    BAR bar0,
    BAR bar1,
    BAR bar2,
    BAR bar3) :
  AHBDevice<BASE>(nm, hindex, vendor, device, version, irq, bar0, bar1, bar2, bar3),
  ahb("ahb", ::amba::amba_AHB, ambaLayer, false),
  m_ResponsePEQ("ResponsePEQ"),
  m_ambaLayer(ambaLayer),
  m_reads("bytes_read", 0llu, this->m_counters),
  m_writes("bytes_written", 0llu, this->m_counters),
  response_error(false) {
  if (ambaLayer == amba::amba_AT) {
    // Register backward transport function
    ahb.register_nb_transport_bw(this, &AHBMaster::nb_transport_bw);

    // Thread for response processing (read)
    SC_THREAD(ResponseThread);
  }
}


// Destructor (unregisters callbacks)
template<class BASE>
AHBMaster<BASE>::~AHBMaster() {
}

// TLM non-blocking backward transport function
template<class BASE>
tlm::tlm_sync_enum AHBMaster<BASE>::nb_transport_bw(
    tlm::tlm_generic_payload &trans,  // NOLINT(runtime/references)
    tlm::tlm_phase &phase,            // NOLINT(runtime/references)
    sc_core::sc_time &delay) {        // NOLINT(runtime/references)
  v::debug << this->name() << "nb_transport_bw received transaction " << hex << &trans << " with phase " << phase << v::endl;

  v::debug << this->name() << "Acquire " << hex << &trans << " Ref-Count before acquire (nb_transport_bw) " << trans.get_ref_count() << v::endl;
  trans.acquire();

  if (phase == tlm::END_REQ) {
    // END_REQ marks the time at which the address or the last address of a burst is sampled by the slave.
    // Let the ahbread function know that END_REQ came in.
    m_EndRequestEvent.notify(delay);
    delay = SC_ZERO_TIME;
  } else if (phase == tlm::BEGIN_RESP) {
    // BEGIN_RESP indicates that the data is ready (begin of AHB data phase)
    // Send the transaction to the thread for response processing;

    uint32_t data_phase_base;

    // Calculate length of data phase
    data_phase_base = (((trans.get_data_length() - 1) >> 2) + 1);
    delay = data_phase_base * get_clock();
    
    // Increment reference counter
    v::debug << this->name() << "Acquire " << hex << &trans << " Ref-Count before acquire (m_ResponsePEQ) " << trans.get_ref_count() << v::endl;
    trans.acquire();
    m_ResponsePEQ.notify(trans, delay);
    delay = SC_ZERO_TIME;

  } else {
    v::error << this->name() << "Invalid phase in call to nb_transport_bw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    assert(-1);
  }

  // Return arrow for msc
  msclogger::return_forward(this, &ahb, &trans, tlm::TLM_ACCEPTED, delay);

  v::debug << this->name() << "Release " << hex << &trans << " Ref-Count before release (nb_transport_bw) " << trans.get_ref_count() << v::endl;
  trans.release();

  return tlm::TLM_ACCEPTED;
}

template<class BASE>
void AHBMaster<BASE>::ahbread(uint32_t addr, unsigned char *data, uint32_t length) {
  bool cacheable;
  tlm::tlm_response_status response;
  sc_core::sc_time delay = SC_ZERO_TIME;

  ahbread(addr, data, length, delay, cacheable, response);
}

template<class BASE>
void AHBMaster<BASE>::ahbread(
    uint32_t addr,
    unsigned char *data,
    uint32_t length,
    sc_core::sc_time &delay,               // NOLINT(runtime/references)
    bool &cacheable,                       // NOLINT(runtime/references)
    tlm::tlm_response_status &response) {  // NOLINT(runtime/references)
  ahbread(addr, data, length, delay, cacheable, false, response);
}

template<class BASE>
void AHBMaster<BASE>::ahbwrite(uint32_t addr, unsigned char *data, uint32_t length) {
  sc_core::sc_time delay = SC_ZERO_TIME;
  tlm::tlm_response_status response;

  ahbwrite(addr, data, length, delay, response);
}

template<class BASE>
void AHBMaster<BASE>::ahbwrite(
    uint32_t addr,
    unsigned char *data,
    uint32_t length,
    sc_core::sc_time &delay,               // NOLINT(runtime/references)
    tlm::tlm_response_status &response) {  // NOLINT(runtime/references)
  ahbwrite(addr, data, length, delay, false, response);
}

// Function for read access to AHB master socket
template<class BASE>
void AHBMaster<BASE>::ahbread(
    uint32_t addr,
    unsigned char *data,
    uint32_t length,
    sc_core::sc_time &delay,               // NOLINT(runtime/references)
    bool &cacheable,                       // NOLINT(runtime/references)
    bool is_lock,
    tlm::tlm_response_status &response) {  // NOLINT(runtime/references)

  // Allocate new transactin (reference counter = 1)
  tlm::tlm_generic_payload *trans = ahb.get_transaction();

  v::debug << this->name() << "Allocate new transaction: " << hex << trans << " Acquire / Ref-Count = " <<
  trans->get_ref_count() << v::endl;

  // Initialize transaction
  trans->set_command(tlm::TLM_READ_COMMAND);
  trans->set_address(addr);
  trans->set_data_length(length);
  trans->set_data_ptr(data);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  if (is_lock) {
    ahb.template validate_extension<amba::amba_lock>(*trans);
  }

  // Call generic transport function
  ahbaccess(trans);

  // Check response
  response = trans->get_response_status();
  if (response != tlm::TLM_OK_RESPONSE) {
    response_error = true;
  }
  
  cacheable = (ahb.get_extension<amba::amba_cacheable>(*trans)) ? true : false;

  // Decrement reference counter
  ahb.release_transaction(trans);

}

// Function for write access to AHB master socket
template<class BASE>
void AHBMaster<BASE>::ahbwrite(
    uint32_t addr,
    unsigned char *data,
    uint32_t length,
    sc_core::sc_time &delay,               // NOLINT(runtime/references)
    bool is_lock,
    tlm::tlm_response_status &response) {  // NOLINT(runtime/references)

  // Allocate new transactin (reference counter = 1)
  tlm::tlm_generic_payload *trans = ahb.get_transaction();

  v::debug << this->name() << "Allocate new transaction " << hex << trans << "Acquire / Ref-Count = " <<
  trans->get_ref_count() << v::endl;

  // Initialize transaction
  trans->set_command(tlm::TLM_WRITE_COMMAND);
  trans->set_address(addr);
  trans->set_data_length(length);
  trans->set_data_ptr(data);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  if (is_lock) {
    ahb.template validate_extension<amba::amba_lock>(*trans);
  }

  // Call generic transport function
  ahbaccess(trans);

  // Check response
  response = trans->get_response_status();
  if (response != tlm::TLM_OK_RESPONSE) {
    response_error = true;
  }

  // Decrement reference counter
  ahb.release_transaction(trans);

}

template<class BASE>
void AHBMaster<BASE>::ahbaccess(tlm::tlm_generic_payload *trans) {

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;
  sc_core::sc_time delay;

  // Increment reference counter
  v::debug << this->name() << "Acquire " << trans << " Ref-Count before acquire (ahbaccess) " << trans->get_ref_count() << v::endl;
  trans->acquire();

  if (m_ambaLayer == amba::amba_LT) {

    v::debug << this->name() << "Transaction " << hex << trans << " call to b_transport" << v::endl;
    
    // Forward arrow for MSC
    msclogger::forward(this, &ahb, trans, tlm::BEGIN_REQ);
    // Start blocking transport
    ahb->b_transport(*trans, delay);

    if (trans->get_response_status() != tlm::TLM_OK_RESPONSE) {
        response_error = true;
    }
 
    // Consume transfer delay
    wait(delay);
    delay=SC_ZERO_TIME;

    // For read-data checking
    if (trans->is_read()) {
        response_callback(trans);
    }

  } else {

    // Initial phase for AT
    phase = tlm::BEGIN_REQ;
    // Forward arrow for MSC
    msclogger::forward(this, &ahb, trans, phase, delay);
    v::debug << this->name() << "Transaction " << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;

    // Start non-blocking transaction
    status = ahb->nb_transport_fw(*trans, phase, delay);

    assert((status == tlm::TLM_ACCEPTED) || (status == tlm::TLM_UPDATED));

    if (phase == tlm::BEGIN_REQ) {
      // Wait for END_REQ to come in on backward path
      wait(m_EndRequestEvent);
    } else if (phase == tlm::END_REQ) {
      // The bus has sent END_REQ on the return path
      wait(delay);
      delay = SC_ZERO_TIME;
    } else {
      // Forbidden phase
      v::error << this->name() << "Invalid phase in return path (from call to nb_transport_fw)!" << status << v::endl;
      trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
      assert(-1);
    }
  }

  // Decrement reference counter
  v::debug << this->name() << "Release " << trans << " Ref-Count before release (ahbaccess) " << trans->get_ref_count() << v::endl;
  trans->release();
}

// Perform AHB debug read
template<class BASE>
uint32_t AHBMaster<BASE>::ahbread_dbg(uint32_t addr, unsigned char *data, unsigned int length) {
  uint32_t length_dbg;

  // Allocate new transaction (reference counter = 1
  tlm::tlm_generic_payload *trans = ahb.get_transaction();

  v::debug << this->name() << "Allocate new transaction " << hex << trans << v::endl;

  // Initialize transaction
  trans->set_command(tlm::TLM_READ_COMMAND);
  trans->set_address(addr);
  trans->set_data_length(length);
  trans->set_data_ptr(data);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  // Call generic debug transport
  length_dbg = ahbaccess_dbg(trans);
  
  // Decrement reference count
  trans->release();

  return length_dbg;
}

// Perform AHB debug write
template<class BASE>
uint32_t AHBMaster<BASE>::ahbwrite_dbg(uint32_t addr, unsigned char *data, unsigned int length) {
  uint32_t length_dbg;

  // Allocate new transactin (reference counter = 1)
  tlm::tlm_generic_payload *trans = ahb.get_transaction();

  v::debug << this->name() << "Allocate new transaction " << hex << trans << v::endl;

  // Initialize transaction
  trans->set_command(tlm::TLM_WRITE_COMMAND);
  trans->set_address(addr);
  trans->set_data_length(length);
  trans->set_data_ptr(data);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  // Call generic debug transport
  length_dbg = ahbaccess_dbg(trans);
  
  // Decrement reference count
  trans->release();

  return length_dbg;
}

template<class BASE>
uint32_t AHBMaster<BASE>::ahbaccess_dbg(tlm::tlm_generic_payload * trans) {
  return(ahb->transport_dbg(*trans));
}
   
// Thread for response synchronization (sync and send END_RESP)
template<class BASE>
void AHBMaster<BASE>::ResponseThread() {
  tlm::tlm_generic_payload *trans;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;

  // uint32_t data_phase_base;

  while (1) {
    // Wait for response from slave
    wait(m_ResponsePEQ.get_event());

    // Get transaction from PEQ
    while ((trans = m_ResponsePEQ.get_next_transaction())) {
      v::debug << name() << "Response Thread running for transaction: " << trans << v::endl;

      if (trans->get_response_status() != tlm::TLM_OK_RESPONSE) {
        v::error << this->name() << "Error in Response for transaction: " << trans << v::endl;

        // This variable is visible within response_callback.
        // Needs to be externally resetted.
        response_error = true;
      }

      // Check result
      response_callback(trans);

      // END_RESP marks the end of the data phase
      phase = tlm::END_RESP;
      delay = sc_core::SC_ZERO_TIME;

      v::debug << this->name() << "Transaction " << hex << trans << " call to nb_transport_fw with phase " << phase
               << v::endl;

      // Forward arrow for msc
      msclogger::forward(this, &ahb, trans, phase, delay);

      // Call to nb_transport_fw
      status = ahb->nb_transport_fw(*trans, phase, delay);

      // Return value must be TLM_COMPLETED or TLM_ACCEPTED
      assert((status == tlm::TLM_COMPLETED) || (status == tlm::TLM_ACCEPTED));

      v::debug << name() << "Release " << trans << " Ref-Count before calling release (ResponseThread) " << trans->get_ref_count() << " Status: "
               << status << v::endl;

      // Decrement reference count
      trans->release();
    }
  }
}

template<class BASE>
void AHBMaster<BASE>::transport_statistics(tlm::tlm_generic_payload &gp) throw() {  // NOLINT(runtime/references)
  if (gp.is_write()) {
    m_writes += gp.get_data_length();
  } else if (gp.is_read()) {
    m_reads += gp.get_data_length();
  }
}

template<class BASE>
void AHBMaster<BASE>::print_transport_statistics(const char *name) const throw() {
  v::report << name << " * Bytes read: " << m_reads << v::endl;
  v::report << name << " * Bytes written: " << m_writes << v::endl;
}
/// @}
