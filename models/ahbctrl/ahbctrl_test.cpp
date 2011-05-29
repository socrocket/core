// ********************************************************************
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
// ********************************************************************
// Title:      ahbctrl.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    This class contains functions common to all AHBCTRL
//             tests. It implements the blocking and non-blocking
//             TLM-AHB master interface and respective read/write 
//             routines.
//             
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
// ********************************************************************

#include "ahbctrl_test.h"

// Constructor
ahbctrl_test::ahbctrl_test(sc_core::sc_module_name name, amba::amba_layer_ids abstractionLayer, unsigned int master_id) : 

  sc_module(name),
  AHBDevice(
      0x04, // vendor: ESA
      0x00,  // device: ??
      0,
      0,
      0,
      0,
      0,
      0),
  ahb("ahb", amba::amba_AHB, abstractionLayer, false),
  m_abstractionLayer(abstractionLayer),
  m_master_id(master_id),
  mResponsePEQ("ResponsePEQ") {

  // For AT abstraction layer
  if (m_abstractionLayer == amba::amba_AT) {

    // Register non-blocking backward transport
    ahb.register_nb_transport_bw(this, &ahbctrl_test::nb_transport_bw, 0);

    // Register thread for response synchronization
    SC_THREAD(ResponseThread);

  }
}

// TLM non-blocking backward transport function
tlm::tlm_sync_enum ahbctrl_test::nb_transport_bw(unsigned int id, tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  v::debug << name() << "nb_transport_bw received phase: " << phase << v::endl;

  // The slave has sent END_REQ
  if (phase == tlm::END_REQ) {

    // Let the processTXN know that END_REQ
    // came in on return path
    mEndRequestEvent.notify(delay);

  // New response - goes into response PEQ
  } else if (phase == tlm::BEGIN_RESP) {

    // Notify processTXN (just in case END_REQ was skipped)
    mEndRequestEvent.notify();

    // Response processing
    mResponsePEQ.notify(trans, delay);

  } else {

    v::error << name() << "Invalid phase in call to nb_transport_bw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

  }

  return tlm::TLM_ACCEPTED;
  
}

// Function for read access to AHB master socket
void ahbctrl_test::ahbread(unsigned int addr, unsigned char * data, unsigned int length, unsigned int burst_size) {

  // Acquire/init transaction
  tlm::tlm_generic_payload *trans = ahb.get_transaction();

  trans->set_command(tlm::TLM_READ_COMMAND);
  trans->set_address(addr);
  trans->set_data_ptr(data);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  trans->set_data_length(length);

  // Set the burst size
  amba::amba_burst_size* size_ext;
  ahb.validate_extension<amba::amba_burst_size> (*trans);
  ahb.get_extension<amba::amba_burst_size> (size_ext, *trans);
    
  if (length < 4) {

    size_ext->value = length;

  } else {

    size_ext->value = 4;

  }

  // Set id extension
  amba::amba_id* m_id;
  ahb.get_extension<amba::amba_id> (m_id, *trans);
  m_id->value = m_master_id;
  ahb.validate_extension<amba::amba_id> (*trans);

  v::debug << name() << "AHB read from addr: " << hex << addr << v::endl;

  // Start transaction processing
  processTXN(trans);

  ahb.release_transaction(trans);

}

// Function for write access to AHB master socket
void ahbctrl_test::ahbwrite(unsigned int addr, unsigned char * data, unsigned int length, unsigned int burst_size) {

  // Acquire/init transaction
  tlm::tlm_generic_payload *trans = ahb.get_transaction();

  trans->set_command(tlm::TLM_WRITE_COMMAND);
  trans->set_address(addr);
  trans->set_data_ptr(data);
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  trans->set_data_length(length);

  // Set the burst size
  amba::amba_burst_size* size_ext;
  ahb.validate_extension<amba::amba_burst_size> (*trans);
  ahb.get_extension<amba::amba_burst_size> (size_ext, *trans);

  if (length < 4) {

    size_ext->value = length;

  } else {

    size_ext->value = 4;

  }

  // Set id extension
  amba::amba_id* m_id;
  ahb.get_extension<amba::amba_id> (m_id, *trans);
  m_id->value = m_master_id;
  ahb.validate_extension<amba::amba_id> (*trans);

  v::debug << name() << "AHB write to addr: " << hex << addr << v::endl;

  // Start transaction processing
  processTXN(trans);

  ahb.release_transaction(trans);
 
}

// The transaction processor
void ahbctrl_test::processTXN(tlm::tlm_generic_payload* trans) {

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;
  sc_core::sc_time delay;
  
  if (m_abstractionLayer == amba::amba_LT) {

    // Blocking transport
    ahb->b_transport(*trans, delay);

  } else {

    // Initial phase for AT
    phase = tlm::BEGIN_REQ;
    delay = SC_ZERO_TIME;

    v::debug << name() << "Call to nb_transport_fw with phase " << phase << v::endl;

    // non-blocking transport
    status = ahb->nb_transport_fw(*trans, phase, delay);

    v::debug << name() << "nb_transport_fw returned with phase " << phase << " and status " << status << v::endl;

    switch (status) {

      case tlm::TLM_ACCEPTED:
      case tlm::TLM_UPDATED:

	if (phase == tlm::BEGIN_REQ) {

	  // Probably TLM_ACCEPTED
	  // Request phase is not completed yet.
	  // Have to wait for END_REQ phase to come
	  // in on backward path.
	  // (mEndRequestEvent has delayed notification)
	  v::debug << name() << "processTXN waiting for EndRequestEvent" << v::endl;
	  
	  wait(mEndRequestEvent);

	  v::debug << name() << "processTXN received EndRequestEvent" << v::endl;
	
	} else if (phase == tlm::END_REQ) {

	  // End of request via return path
	  // Burn annotated delay.
	  wait(delay);

	} else if (phase == tlm::BEGIN_RESP) {

	  // Begin of response initiated via return path.
	  mResponsePEQ.notify(*trans, delay);

	} else {

	  // forbidden phase
	  v::error << name() << "Invalid phase in return path from call to nb_transport_fw!" << v::endl;
	  trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

	}

	v::debug << name() << "processTXN waiting for EndResponseEvent" << v::endl;
	
	wait(mEndResponseEvent);

	v::debug << name() << "processTXN received EndResponseEvent" << v::endl;

	break;

      case tlm::TLM_COMPLETED:

	// Slave directly jumps to TLM_COMPLETED (Pseudo AT).
	// Don't send END_RESP
	wait(delay);

	break;
      
      default:

	v::error << name() << "Invalid return value from call to nb_transport_fw" << v::endl;
	trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

    }
  }
}

// Thread for response synchronization (sync and send END_RESP)
void ahbctrl_test::ResponseThread() {

  tlm::tlm_generic_payload* trans;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;

  while(1) {

    v::debug << name() << "Response thread waiting for new response" << v::endl;

    // Wait for response from slave (inserted in transport_bw)
    wait(mResponsePEQ.get_event());

    // There can be only one response at the time,
    // because the arbiter serializes communication.
    // (no concurrent master-slave connections allowed)
    trans = mResponsePEQ.get_next_transaction();

    // Prepare END_RESP
    phase = tlm::END_RESP;
    delay = sc_core::SC_ZERO_TIME;

    v::debug << name() << "Call to nb_transport_fw with phase " << phase << v::endl;

    // Call nb_transport_fw with END_RESP
    status = ahb->nb_transport_fw(*trans, phase, delay);

    v::debug << name() << "nb_transport_fw returned with phase: " << phase << " and status " << status << v::endl;

    // Return value must be completed or accepted
    assert((status==tlm::TLM_COMPLETED)||(status==tlm::TLM_ACCEPTED));

    // Send EndResponse to unblock processTXN
    mEndResponseEvent.notify();
  }
}

/// Use this function to record system time and realtime at the beginning of a test phase
void ahbctrl_test::phase_start_timing() {

  phase_systime_start = sc_core::sc_time_stamp();
  phase_realtime_start = std::clock();

}

/// Use this function to record system time and realtime at the end of a test phase
void ahbctrl_test::phase_end_timing() {

  phase_systime_end = sc_core::sc_time_stamp();
  phase_realtime_end = std::clock();
	
}

/// Returns the difference between phase_systime_end and phase_systime_start
sc_core::sc_time ahbctrl_test::phase_systime() {

  return(phase_systime_end - phase_systime_start);

}

/// Returns the difference between phase_realtime_end and phase_realtime_start in seconds.
double ahbctrl_test::phase_realtime() {

  return((phase_realtime_end - phase_realtime_start)/(double)CLOCKS_PER_SEC);
	
}
 

