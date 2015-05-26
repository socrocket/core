// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file ahbslave.tpp
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
using namespace std;
using namespace sc_core;
using namespace tlm;

// Constructor
template<class BASE>
AHBSlave<BASE>::AHBSlave(
    ModuleName mn,
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
  AHBDevice<BASE>(
    mn,
    hindex,
    vendor,
    device,
    version,
    irq,
    bar0,
    bar1,
    bar2,
    bar3),
  ahb("ahb", ::amba::amba_AHB, ambaLayer, false  /* Arbitration */),
  m_RequestPEQ("RequestPEQ"),
  m_ResponsePEQ("ResponsePEQ"),
  busy(false),
  m_reads("bytes_read", 0llu, this->m_counters),
  m_writes("bytes_written", 0llu, this->m_counters) {
  // Register transport functions to sockets
  ahb.register_b_transport(this, &AHBSlave::b_transport);
  ahb.register_transport_dbg(this, &AHBSlave::transport_dbg);

  if (ambaLayer == amba::amba_AT) {
    // Register non-blocking transport for AT
    ahb.register_nb_transport_fw(this, &AHBSlave::nb_transport_fw);

    // Thread for modeling AHB pipeline delay
    SC_THREAD(requestThread);

    // Thread for interfacing functional part of the model
    // in AT mode.
    SC_THREAD(responseThread);
  }
}

// Destructor (unregisters callbacks)
template<class BASE>
AHBSlave<BASE>::~AHBSlave() {
}

// TLM non-blocking forward transport function
template<class BASE>
tlm::tlm_sync_enum AHBSlave<BASE>::nb_transport_fw(tlm::tlm_generic_payload &trans,
  tlm::tlm_phase &phase,
  sc_core::sc_time &delay) {
  sc_core::sc_time request_delay;
  sc_core::sc_time response_delay;

  v::debug << this->name() << "nb_transport_fw received transaction " << hex << &trans << " with phase: " << phase <<
  v::endl;

  if (phase == tlm::BEGIN_REQ) {
    // Increment reference counter
    trans.acquire();
    transport_statistics(trans);
    if (this->get_ahb_bar_cachable(0)) {
      ahb.validate_extension<amba::amba_cacheable>(trans);
    }

    v::debug << this->name() << "Acquire " << hex << &trans << " Ref-Count = " << trans.get_ref_count() << v::endl;

    uint32_t address_cycle_base;

    // v::debug << this->name() << "Delay before calling exec_func: " << delay << v::endl;

    // Call the functional part of the model
    // ! The functional part may not call wait !
    // ! Overload this function (nb_transport_fw) if necessary !
    exec_func(trans, delay);

    if (trans.get_response_status() != tlm::TLM_OK_RESPONSE) {
      v::warn << name() << "Target did not return tlm::TLM_OK_RESPONSE" << v::endl;
    }

    // The delay returned by the function model relates to the time for delivering
    // the data + wait states.
    address_cycle_base = (((trans.get_data_length() - 1) >> 2) + 1);

    // In case unvalid access (TLM_ERROR_RESPONSE)
    // some models might return an unvalid delay.
    // Set delay to sane default for safe reponse processing
    if (delay == SC_ZERO_TIME) {
      delay = address_cycle_base * get_clock() + sc_core::sc_time(1, SC_PS);
    }

    // v::debug << this->name() << "Total delay: " << delay << v::endl;

    // Calculating delay for sending END_REQ
    request_delay = delay - sc_core::sc_time(1, SC_PS);

    v::debug << this->name() << "Request Delay: " << request_delay << v::endl;

    // Consume request_delay and forward to request thread
    m_RequestPEQ.notify(trans, request_delay);

    // Calculating delay for sending BEGIN_REQ
    response_delay = (delay - (get_clock() * (address_cycle_base - 1)) - sc_core::sc_time(1, SC_PS));

    v::debug << this->name() << "Response Delay: " << response_delay << v::endl;

    // Consume response_delay and forward to response thread
    m_ResponsePEQ.notify(trans, response_delay);

    delay = SC_ZERO_TIME;

    msclogger::return_backward(this, &ahb, &trans, tlm::TLM_ACCEPTED, delay);

    return tlm::TLM_ACCEPTED;
  } else if (phase == tlm::END_RESP) {
    msclogger::return_backward(this, &ahb, &trans, tlm::TLM_COMPLETED, delay);
    v::debug << this->name() << "Release " << &trans << " Ref-Count before calling release " << trans.get_ref_count() <<
    v::endl;

    // END_RESP corresponds to the end of the AHB data phase.
    // ! For this to work master must send END_RESP via forward path !
    trans.release();

    return tlm::TLM_COMPLETED;
  } else {
    v::error << this->name() << "Invalid phase in call to nb_transport_fw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
  }

  return tlm::TLM_ACCEPTED;
}

// Thread for modeling the AHB pipeline delay
template<class BASE>
void AHBSlave<BASE>::requestThread() {
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
#ifndef NDEBUG // assert specific macro
  tlm::tlm_sync_enum status;
#endif

  tlm::tlm_generic_payload *trans;

  while (1) {
    wait(m_RequestPEQ.get_event());

    trans = m_RequestPEQ.get_next_transaction();

    // Send END_REQ
    phase = tlm::END_REQ;
    delay = SC_ZERO_TIME;

    v::debug << this->name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << phase <<
    v::endl;

    // Backward arrow for msc
    msclogger::backward(this, &ahb, trans, phase, delay);

    // Call to backward transport
#ifdef NDEBUG // assert specific macro
    ahb->nb_transport_bw(*trans, phase, delay);
#else
    status = ahb->nb_transport_bw(*trans, phase, delay);
    assert(status == tlm::TLM_ACCEPTED);
#endif
  }
}

template<class BASE>
void AHBSlave<BASE>::responseThread() {
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;

  tlm::tlm_generic_payload *trans;

  while (1) {
    wait(m_ResponsePEQ.get_event());

    trans = m_ResponsePEQ.get_next_transaction();

    // Send BEGIN_RESP
    phase = tlm::BEGIN_RESP;
    delay = SC_ZERO_TIME;

    v::debug << this->name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << status << phase <<
    v::endl;

    // Backward arrow for msc
    msclogger::backward(this, &ahb, trans, phase, delay);

    // Call to backward transport
    status = ahb->nb_transport_bw(*trans, phase, delay);

    if ((phase == tlm::END_RESP) || (status == tlm::TLM_COMPLETED)) {
      // Decrement reference counter
      trans->release();
    }
  }
}

// TLM blocking transport function
template<class BASE>
void AHBSlave<BASE>::b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {
  // Call the functional part of the model
  // -------------------------------------
  transport_statistics(trans);
  if (this->get_ahb_bar_cachable(0)) {
    ahb.validate_extension<amba::amba_cacheable>(trans);
  }
  exec_func(trans, delay);

  msclogger::return_backward(this, &ahb, &trans, tlm::TLM_ACCEPTED, delay);
}

// TLM blocking transport function
template<class BASE>
uint32_t AHBSlave<BASE>::transport_dbg(tlm::tlm_generic_payload &trans) {
  // Call the functional part of the model
  // -------------------------------------
  sc_time delay = SC_ZERO_TIME;
  return exec_func(trans, delay);
}

template<class BASE>
void AHBSlave<BASE>::transport_statistics(tlm::tlm_generic_payload &gp) throw() {
  if (gp.is_write()) {
    m_writes += gp.get_data_length();
  } else if (gp.is_read()) {
    m_reads += gp.get_data_length();
  }
}

template<class BASE>
void AHBSlave<BASE>::print_transport_statistics(const char *name) const throw() {
  v::report << name << " * Bytes read: " << m_reads << v::endl;
  v::report << name << " * Bytes written: " << m_writes << v::endl;
}

/* vim: set expandtab noai ts=4 sw=4: */
/* -*- mode: c-mode; tab-width: 4; indent-tabs-mode: nil; -*- */

/// @}
