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
// Title:      ahbctrl_test.cpp
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
ahbctrl_test::ahbctrl_test(sc_core::sc_module_name name,
			   unsigned int haddr, // haddr for random instr. generation
			   unsigned int hmask, // hmask for random instr. generation
			   unsigned int master_id, // id of the bus master
			   sc_core::sc_time inter, // interval of random instructions (waiting period)
			   amba::amba_layer_ids abstractionLayer) : sc_module(name),
  AHBDevice(
      master_id, // bus id (hindex)
      0x01,      // vendor: Gaisler Research (Fake the Leon)
      0x003,      // device
      0,
      0,
      0,
      0,
      0,
      0),
  ahb("ahb", amba::amba_AHB, abstractionLayer, false),
  snoop(&ahbctrl_test::snoopingCallBack,"SNOOP"),
  m_haddr(haddr),
  m_hmask(hmask),
  m_master_id(master_id),
  m_inter(inter),
  m_abstractionLayer(abstractionLayer),
  mResponsePEQ("ResponsePEQ") {

  // Calculate address bound for random instruction generation
  // from haddr/hmask
  m_addr_range_lower_bound = (m_haddr & m_hmask) << 20;
  m_addr_range_upper_bound = m_addr_range_lower_bound + (((~m_hmask & 0xfff) + 1) << 20);

  // Sanity check range
  assert(m_addr_range_upper_bound > m_addr_range_lower_bound);

  v::info << this->name() << "***************************************************       " << v::endl;
  v::info << this->name() << "* Testbench master " << m_master_id << " configured to    " << v::endl;
  v::info << this->name() << "* generate instructions with following parameters:        " << v::endl;
  v::info << this->name() << "* Lower address bound: " << hex << m_addr_range_lower_bound << v::endl;
  v::info << this->name() << "* Upper address bound: " << hex << m_addr_range_upper_bound << v::endl;
  v::info << this->name() << "* Interval: " << m_inter << v::endl;
  v::info << this->name() << "***************************************************" << v::endl;


  // For AT abstraction layer
  if (m_abstractionLayer == amba::amba_AT) {

    // Register non-blocking backward transport
    ahb.register_nb_transport_bw(this, &ahbctrl_test::nb_transport_bw);

    // Register thread for response synchronization
    SC_THREAD(ResponseThread);

  }
}

// TLM non-blocking backward transport function
tlm::tlm_sync_enum ahbctrl_test::nb_transport_bw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

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

  v::debug << name() << "Aquire transaction: " << trans << v::endl;

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

  // Set transfer type extension
  amba::amba_trans_type * trans_ext;
  ahb.get_extension<amba::amba_trans_type> (trans_ext, *trans);
  trans_ext->value = amba::NON_SEQUENTIAL;
  ahb.validate_extension<amba::amba_trans_type> (*trans);

  v::debug << name() << "AHB write to addr: " << hex << addr << v::endl;

  // Start transaction processing
  processTXN(trans);

}

// Read operation / result will be checked against locally cached data
void ahbctrl_test::check_read(unsigned int addr, unsigned char* data, unsigned int length) {

  // Read from AHB
  ahbread(addr, data, length, 4);

}

// Generates random read operations within haddr/hmask region
void ahbctrl_test::random_read(unsigned int length) {

  unsigned char data[4];

  // Random address
  unsigned int addr = (rand() % (m_addr_range_upper_bound-m_addr_range_lower_bound)) + m_addr_range_lower_bound;

  // Align address with respect to data length
  switch (length) {

  case 1:
    // byte address
    break;

  case 2:
    // half-word boundary
    addr = addr & 0xfffffffe;
    break;

  case 4:
    // word boundary
    addr = addr & 0xfffffffc;
    break;

  default:

    v::error << "Length not valid in random_write!" << v::endl;
  }

  // Read from AHB
  ahbread(addr, data, length, 4);

}

// Write operation / write data will be cached in local storage
void ahbctrl_test::check_write(unsigned int addr, unsigned char * data, unsigned int length) {

  unsigned int i;
  t_entry entry;

  // Write to AHB
  ahbwrite(addr, data, length, 4);  

  // Keep track in local cache
  for (i=0;i<length;i++) {

    entry.data = data[i];
    entry.valid = true;

    localcache[addr+i] = entry;

  }
}

// Generates random write operations within haddr/hmask region
void ahbctrl_test::random_write(unsigned int length) {

  unsigned int i;
  unsigned char data[4];

  t_entry entry;

  // Random address
  unsigned int addr = (rand() % (m_addr_range_upper_bound-m_addr_range_lower_bound)) + m_addr_range_lower_bound;

  // Align address with respect to data length
  switch (length) {

  case 1:
    // byte address
    break;

  case 2:
    // half-word boundary
    addr = addr & 0xfffffffe;
    break;

  case 4:
    // word boundary
    addr = addr & 0xfffffffc;
    break;

  default:

    v::error << "Length not valid in random_write!" << v::endl;
  }
  
  v::debug << name() << "New random write!" << v::endl;

  // Random data
  for (i=0;i<4;i++) {

    if (i < length) {

      data[i] = rand() % 256;

      v::debug << "addr: " << hex << addr+i << " data: " << (unsigned int)data[i] << v::endl;

    } else {

      data[i] = 0;

    }
  
  }

  // Write to AHB
  ahbwrite(addr, data, length, 4);

  // Keep track in local cache
  for (i=0;i<length;i++) {

    entry.data = data[i];
    entry.valid = true;

    localcache[addr+i] = entry;

  }

}

// Snooping function - invalidates dirty cache entries
void ahbctrl_test::snoopingCallBack(const t_snoop & snoop, const sc_core::sc_time & delay) {

  v::debug << name() << "Snooping write operation on AHB interface (MASTER: " << snoop.master_id << " ADDR: " \
	   << hex << snoop.address << " LENGTH: " << snoop.length << ")" << v::endl;

  // Make sure we are not snooping ourself ;)
  // Check if address was used for a random instruction from this master.
  if (snoop.master_id != m_master_id) {

    for (unsigned int i = 0; i < snoop.length; i++) {

      // Look up local cache
      it = localcache.find(snoop.address + i);

      if (it != localcache.end()) {

	v::debug << name() << "Invalidate data at address: " << hex << snoop.address + i << v::endl;

	// Delete the valid bit
	(it->second).valid = false;

      }
    }
  }
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

    // Get transaction from Queue
    trans = mResponsePEQ.get_next_transaction();

    // Check result
    checkTXN(trans);
  
    // Prepare END_RESP
    phase = tlm::END_RESP;
    delay = sc_core::SC_ZERO_TIME;

    v::debug << name() << "Call to nb_transport_fw with phase " << phase << v::endl;

    // Call nb_transport_fw with END_RESP
    status = ahb->nb_transport_fw(*trans, phase, delay);

    v::debug << name() << "nb_transport_fw returned with phase: " << phase << " and status " << status << v::endl;

    // Return value must be completed or accepted
    assert((status==tlm::TLM_COMPLETED)||(status==tlm::TLM_ACCEPTED));

    v::debug << name() << "Release transaction: " << trans << v::endl;

    // Cleanup
    ahb.release_transaction(trans);
  }
}

// Check transaction
void ahbctrl_test::checkTXN(tlm::tlm_generic_payload * trans) {

  // Cache data
  t_entry tmp;
  tmp.data = 0;
  tmp.valid = 0;

  // Unpack transaction
  tlm::tlm_command cmd = trans->get_command();
  unsigned int addr    = trans->get_address();
  unsigned char * data = trans->get_data_ptr();
  unsigned int length  = trans->get_data_length();

  // Verify reads against localcache
  if (cmd == tlm::TLM_READ_COMMAND) {

      // Check result
      for (unsigned int i=0; i<length; i++) {

	// Look up local cache
	it = localcache.find(addr+i);

	// Assume 0, if location was never written before
	if (it != localcache.end()) {

	  // We (this master) wrote to this address before.
	  tmp = it->second;

	} else {

	  // Address was not written by this master before.
	  tmp.data = 0;
	  tmp.valid = false;

	}

	v::debug << name() << "ADDR: " << hex << addr+i << " DATA: " << (unsigned int)data[i] << " EXPECTED: " << tmp.data << " VALID: " << tmp.valid << v::endl;
   
	if (tmp.valid == true) {

	  //assert(data[i] == tmp.data);

	} else {

	  v::debug << name() << "No local reference for checking or data invalidated by snooping!" << v::endl;

	}
      }
    }
    

}
