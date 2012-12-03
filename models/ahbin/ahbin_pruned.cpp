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
// Title:      input_device.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Class definition of a cache-subsystem.
//             The cache-subsystem envelopes an instruction cache,
//             a data cache and a memory management unit.
//             The input_device class provides two TLM slave interfaces
//             for connecting the cpu to the caches and an AHB master
//             interface for connection to the main memory.
//
// Method:
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#include "input_device.h"
#include "vendian.h"

/// Constructor
input_device::input_device(sc_core::sc_module_name name,
                           unsigned int hindex,
                           unsigned int hirq,
                           bool pow_mon,
                           amba::amba_layer_ids abstractionLayer) :

    sc_module(name),
    m_irq(hirq),
    AHBDevice(hindex,
	      0x01,  // vendor: Gaisler Research (Fake the LEON)
	      0x003,  // 
	      0,
	      m_irq,
	      0,
	      0,
	      0,
	      0),

    ahb("ahb_socket", amba::amba_AHB, abstractionLayer, false),
    irq("irq"),
    m_master_id(hindex), 
    m_performance_counters("performance_counters"),
    m_right_transactions("successful_transactions", 0llu, m_performance_counters),
    m_total_transactions("total_transactions", 0llu, m_performance_counters),
    m_pow_mon(pow_mon),
    m_abstractionLayer(abstractionLayer), 
    mResponsePEQ("ResponsePEQ"),
    mDataPEQ("DataPEQ"),
    mEndTransactionPEQ("EndTransactionPEQ") {

    // Parameter checks
    // ----------------

    // Register GreenConfig api instance
    m_api = gs::cnf::GCnf_Api::getApiInstance(this);

    // Approximately-Timed
    if (abstractionLayer==amba::amba_AT) {

      // Register non-blocking backward transport function for ahb socket
      ahb.register_nb_transport_bw(this, &input_device::ahb_nb_transport_bw);

      SC_THREAD(ResponseThread);

      SC_THREAD(DataThread);

      // Delayed transaction release (for AT)
      SC_THREAD(cleanUP);

    }

    // Register power monitor
    PM::registerIP(this,"input_device",m_pow_mon);
    PM::send_idle(this,"idle",sc_time_stamp(),m_pow_mon);

    // Module Configuration Report
    v::info << this->name() << " ************************************************** " << v::endl;
    v::info << this->name() << " * Created INPUT_DEVICE in following configuration: " << v::endl;
    v::info << this->name() << " * --------------------------------------------- " << v::endl;
    v::info << this->name() << " * abstraction Layer (LT = 8 / AT = 4): " << abstractionLayer << v::endl;
    v::info << this->name() << " ************************************************** " << v::endl;   
}

void input_device::dorst() {
  // Reset functionality executed on 0 to 1 edge
}

// Delayed release of transactions (AT only)
void input_device::cleanUP() {

  tlm::tlm_generic_payload * trans;

  while(1) {

    wait(mEndTransactionPEQ.get_event());

    while((trans = mEndTransactionPEQ.get_next_transaction())) {

      // Check TLM RESPONSE
      if (trans->get_response_status() != tlm::TLM_OK_RESPONSE) {

	v::error << name() << "Transaction " << hex << trans << " failed with " << trans->get_response_status() << v::endl;
	  
      } else {

	m_right_transactions++;

      }

      v::debug << name() << "Release transaction: " << hex << trans << v::endl;
      ahb.release_transaction(trans);

    }
  }
}

// TLM non-blocking backward transport function for ahb socket
tlm::tlm_sync_enum input_device::ahb_nb_transport_bw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  v::debug << name() << "nb_transport_bw received transaction " << hex << &trans << " with phase " << phase << " and delay " << delay << v::endl;

  // The slave has sent END_REQ
  if (phase == tlm::END_REQ) {

    // In case END_REQ comes via backward path:
    // Notify interface functions that request phase is over.
    mEndRequestEvent.notify();

    // Slave is ready for BEGIN_DATA (writes only)
    if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {

      // Put into DataPEQ for data phase processing
      mDataPEQ.notify(trans);

    }

    // Reset delay
    delay = SC_ZERO_TIME;

  } else if (phase == amba::DATA_SPLIT) {

    // Master never reacts on data split!!
    v::warn << name() << "Master received DATA_SPLIT" << v::endl;

  // New response (read operations only)
  } else if (phase == tlm::BEGIN_RESP) {

    // Put into ResponsePEQ for response processing
    mResponsePEQ.notify(trans, delay);

    // Reset delay
    delay = SC_ZERO_TIME;

  // Data phase completed
  } else if (phase == amba::END_DATA) {

    // Add some delay and remove transaction
    delay = 1000*clock_cycle;
    // Release transaction
    mEndTransactionPEQ.notify(trans, delay);

    // Reset delay
    delay = SC_ZERO_TIME;

  // Phase not valid
  } else {

    v::error << name() << "Invalid phase in call to nb_transport_bw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

  }

  return tlm::TLM_ACCEPTED;

}

/// Function for write access to AHB master socket
void input_device::mem_write(unsigned int addr, unsigned int asi, unsigned char * data,
                          unsigned int length, sc_core::sc_time * t,
                          unsigned int * debug, bool is_dbg) {

    tlm::tlm_phase phase;
    tlm::tlm_sync_enum status;
    sc_core::sc_time delay;

    // Allocate new transaction
    tlm::tlm_generic_payload *trans = ahb.get_transaction();
    m_total_transactions++;

    v::debug << name() << "Allocate new transaction " << hex << trans << v::endl;

    // Initialize transaction
    trans->set_command(tlm::TLM_WRITE_COMMAND);
    trans->set_address(addr);
    trans->set_data_length(length);
    trans->set_data_ptr(data);
    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    // Set burst size extension
    amba::amba_burst_size* size_ext;
    ahb.validate_extension<amba::amba_burst_size> (*trans);
    ahb.get_extension<amba::amba_burst_size> (size_ext, *trans);
    size_ext->value = (length < 4)? length : 4;

    // Set master id extension
    amba::amba_id* m_id;
    ahb.validate_extension<amba::amba_id> (*trans);
    ahb.get_extension<amba::amba_id> (m_id, *trans);
    m_id->value = m_master_id;
 
    // Set transfer type extension
    amba::amba_trans_type * trans_ext;
    ahb.validate_extension<amba::amba_trans_type>(*trans);
    ahb.get_extension<amba::amba_trans_type> (trans_ext, *trans);
    trans_ext->value = amba::NON_SEQUENTIAL;

    // Collect transport statistics
    transport_statistics(*trans);

    // Initialize delay
    delay = SC_ZERO_TIME;

    // Timed transport
    if (!is_dbg) {

      if (m_abstractionLayer == amba::amba_LT) {

        // Blocking transport
        ahb->b_transport(*trans, delay);

	//v::debug << name() << "Delay after return from b_transport: " << delay << v::endl;

        // Consume delay
        wait(delay);

	// Check TLM RESPONSE
	if (trans->get_response_status()!=tlm::TLM_OK_RESPONSE) {
	  
	  v::error << name() << "Transaction " << hex << trans << " failed with " << trans->get_response_status() << v::endl;
	  
	} else {

	  m_right_transactions++;

	}

	v::debug << name() << "Release transaction: " << hex << trans << v::endl;

	// Release transaction
	ahb.release_transaction(trans);

      } else {

        // Initial phase for AT
        phase = tlm::BEGIN_REQ;

        v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;

        // Non-blocking transport
        status = ahb->nb_transport_fw(*trans, phase, delay);

        switch (status) {

          case tlm::TLM_ACCEPTED:
          case tlm::TLM_UPDATED:

	    if (phase == tlm::BEGIN_REQ) {

	      // The slave returned TLM_ACCEPTED.
	      // Wait until END_REQ comes in on backward path
	      // before starting DATA phase.
	      wait(mEndRequestEvent);

	    } else if (phase == tlm::END_REQ) {

	      // The slave returned TLM_UPDATED with END_REQ
	      mDataPEQ.notify(*trans, delay);

	    } else if (phase == amba::END_DATA) {

	      // Done return control to user.

	    } else {

	      // Forbidden phase
	      v::error << name() << "Invalid phase in return path (from call to nb_transport_fw)!" << v::endl;
	      trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

	    }

	    break;

            case tlm::TLM_COMPLETED:

	      // Slave directly jumps to TLM_COMPLETED (Pseudo AT).
	      // Don't send END_RESP
	      // wait(delay)
	  
	      break;

            default:

	      v::error << name() << "Invalid return value from call to nb_transport_fw!" << v::endl;
	      trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

	}
      }

    } else {

      // Debug transport
      ahb->transport_dbg(*trans);

      v::debug << name() << "Release transaction: " << hex << trans << v::endl;

      // Release transaction
      ahb.release_transaction(trans);

    }
}

/// Function for read access to AHB master socket
void input_device::mem_read(unsigned int addr, unsigned int asi, unsigned char * data,
                         unsigned int length, sc_core::sc_time * t,
                         unsigned int * debug, bool is_dbg) {

    tlm::tlm_phase phase;
    tlm::tlm_sync_enum status;
    sc_core::sc_time delay;

    // Allocate new transaction
    tlm::tlm_generic_payload *trans = ahb.get_transaction();
    m_total_transactions++;

    v::debug << name() << "Allocate new transaction: " << hex << trans << v::endl;

    // Initialize transaction
    trans->set_command(tlm::TLM_READ_COMMAND);
    trans->set_address(addr);
    trans->set_data_length(length);
    trans->set_data_ptr(data);
    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    // Set burst size extension
    amba::amba_burst_size* size_ext;
    ahb.validate_extension<amba::amba_burst_size> (*trans);
    ahb.get_extension<amba::amba_burst_size> (size_ext, *trans);
    size_ext->value = (length < 4)? length : 4;

    // Set master id extension
    amba::amba_id* m_id;
    ahb.validate_extension<amba::amba_id> (*trans);
    ahb.get_extension<amba::amba_id> (m_id, *trans);
    m_id->value = m_master_id;

    // Set transfer type extension
    amba::amba_trans_type * trans_ext;
    ahb.validate_extension<amba::amba_trans_type> (*trans);
    ahb.get_extension<amba::amba_trans_type> (trans_ext, *trans);
    trans_ext->value = amba::NON_SEQUENTIAL;

    // Collect transport statistics
    transport_statistics(*trans);
    
    // Init delay
    delay = SC_ZERO_TIME;

    // Timed transport
    if (!is_dbg) {

      if (m_abstractionLayer == amba::amba_LT) {

	// Blocking transport
	ahb->b_transport(*trans, delay);

	v::debug << name() << "Release transaction: " << hex << trans << v::endl;

	// Check TLM RESPONSE
	if (trans->get_response_status()!=tlm::TLM_OK_RESPONSE) {
	  
	  v::error << name() << "Transaction " << hex << trans << " failed with " << trans->get_response_status() << v::endl;
	  
	} else {

	  m_right_transactions++;

	}

	// Consume delay
	wait(delay);
	delay = SC_ZERO_TIME;

      } else {

	// Initial phase for AT
	phase = tlm::BEGIN_REQ;
      
	v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;

	// Non-blocking transport
	status = ahb->nb_transport_fw(*trans, phase, delay);

        v::debug << name() << "Transaction returned with phase: " << phase << " and status " << status << v::endl;

	switch(status) {

          case tlm::TLM_ACCEPTED:
          case tlm::TLM_UPDATED:

	    if (phase == tlm::BEGIN_REQ) {

	      // The slave returned TLM_ACCEPTED.
	      // Wait until BEGIN_RESP before giving control
	      // to the user (for sending next transaction).
 	      wait(mEndRequestEvent);

	    } else if (phase == tlm::END_REQ) {

	      // The slave returned TLM_UPDATED with END_REQ

	      wait(mEndRequestEvent);

	    } else if (phase == tlm::BEGIN_RESP) {

	      // Slave directly jumped to BEGIN_RESP
	      // Notify the response thread and return control to user
	      mResponsePEQ.notify(*trans, delay);

	    } else {

	      // Forbidden phase
	      v::error << name() << "Invalid phase in return path (from call to nb_transport_fw)!" << v::endl;
	      trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

	    }

	    break;

	  case tlm::TLM_COMPLETED:
	
	    // Slave directly jumps to TLM_COMPLETED (Pseudo AT).
	    // Don't send END_RESP
	    // wait(delay)

	    break;

          default:

	    v::error << name() << "Invalid return value from call to nb_transport_fw!" << v::endl;
	    trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

	}

	// Wait for result to be ready before return
	//wait(mResponsePEQ.get_event());

      }

    } else {

      // Debug transport
      ahb->transport_dbg(*trans);

      v::debug << name() << "Release transaction: " << hex << trans << v::endl;

    }

    if (is_dbg || m_abstractionLayer == amba::amba_LT) {

      // Release transaction
      ahb.release_transaction(trans);

    }

}

// Thread for data phase processing in write operations (sends BEGIN_DATA)
void input_device::DataThread() {

  tlm::tlm_generic_payload* trans;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;

  while(1) {

    // v::debug << name() << "Data thread waiting for new data phase." << v::endl;

    // Wait for new data phase
    wait(mDataPEQ.get_event());

    // v::debug << name() << "DataPEQ Event" << v::endl;

    // Get transaction from PEQ
    trans = mDataPEQ.get_next_transaction();

    // Prepare BEGIN_DATA
    phase = amba::BEGIN_DATA;
    delay = SC_ZERO_TIME;

    v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;

    // Call to nb_transport_fw with BEGIN_DATA
    status = ahb->nb_transport_fw(*trans, phase, delay);

    switch(status) {
      
      case tlm::TLM_ACCEPTED:
      case tlm::TLM_UPDATED:

	if (phase == amba::BEGIN_DATA) {

	  // The slave returned TLM_ACCEPTED.
	  // Wait for END_DATA to come in on backward path.

	  // v::debug << name() << "Waiting mEndDataEvent" << v::endl;
	  //wait(mEndDataEvent);
	  // v::debug << name() << "mEndDataEvent" << v::endl;

        } else if (phase == amba::END_DATA) {

	  // Slave returned TLM_UPDATED with END_DATA
	  // Data phase completed.
	  wait(delay);

	  // Add some delay and return transaction
	  delay = 100*clock_cycle;
	  mEndTransactionPEQ.notify(*trans, delay);

	} else {

	  // Forbidden phase
	  v::error << name() << "Invalid phase in return path (from call to nb_transport_fw)!" << v::endl;

	}
	
	break;

      case tlm::TLM_COMPLETED:

	// Slave directly jumps to TLM_COMPLETED (Pseudo AT).
	// wait(delay);

	break;

    }
  }
}


// Thread for response synchronization (sync and send END_RESP)
void input_device::ResponseThread() {

  tlm::tlm_generic_payload* trans;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;

  while(1) {

    // Wait for response from slave
    wait(mResponsePEQ.get_event());

    // Get transaction from PEQ
    trans = mResponsePEQ.get_next_transaction();

    // Prepare END_RESP
    phase = tlm::END_RESP;
    delay = clock_cycle;

    v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;

    // Call to nb_transport_fw
    status = ahb->nb_transport_fw(*trans, phase, delay);

    // Return value must be TLM_COMPLETED or TLM_ACCEPTED
    assert((status==tlm::TLM_COMPLETED)||(status==tlm::TLM_ACCEPTED));

    // Add some delay and remove transaction
    delay = 100*clock_cycle;
    mEndTransactionPEQ.notify(*trans, delay);

  }
}


// Automatically called by SystemC scheduler at end of simulation
// Displays execution statistics.
void input_device::end_of_simulation() {

    v::report << name() << " ********************************************" << v::endl;
    v::report << name() << " * INPUT_DEVICE Statistics: " << v::endl;
    v::report << name() << " * --------------------- " << v::endl;
    v::report << name() << " * Successful Transactions: " << m_right_transactions << v::endl;
    v::report << name() << " * Total Transactions: " << m_total_transactions << v::endl;
    v::report << name() << " * " << v::endl;
    v::report << name() << " * AHB Master interface reports: " << v::endl;
    print_transport_statistics(name());
    v::report << name() << " ********************************************" << v::endl;    

}

// Helper for setting clock cycle latency using a value-time_unit pair
void input_device::clkcng() {

}
