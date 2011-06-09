// *****************************************************************************
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
// *****************************************************************************
// Title:      ahbctrl.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    AHB address decoder.
//             The decoder collects all AHB request from the masters and
//             forwards them to the appropriate slave.
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
// *****************************************************************************

#include "ahbctrl.h"
#include "verbose.h"

// Constructor of class AHBCtrl
AHBCtrl::AHBCtrl(sc_core::sc_module_name nm, // SystemC name
		 unsigned int ioaddr,  // The MSB address of the I/O area
		 unsigned int iomask,  // The I/O area address mask
		 unsigned int cfgaddr, // The MSB address of the configuration area (PNP)
		 unsigned int cfgmask, // The address mask of the configuration area
		 bool rrobin,          // 1 - round robin, 0 - fixed priority arbitration (only AT)
		 bool split,           // Enable support for AHB SPLIT response (only AT)
		 unsigned int defmast, // ID of the default master
		 bool ioen,            // AHB I/O area enable
		 bool fixbrst,         // Enable support for fixed-length bursts
		 bool fpnpen,          // Enable full decoding of PnP configuration records
		 bool mcheck,          // Check if there are any intersections between core memory regions
		 amba::amba_layer_ids ambaLayer) :
      sc_module(nm),      
      ahbIN("ahbIN", amba::amba_AHB, ambaLayer, false),
      ahbOUT("ahbOUT", amba::amba_AHB, ambaLayer, false),
      snoop("snoop"),
      mioaddr(ioaddr),
      miomask(iomask),
      mcfgaddr(cfgaddr),
      mcfgmask(cfgmask),
      mrrobin(rrobin),
      msplit(split),
      mdefmast(defmast),
      mioen(ioen),
      mfixbrst(fixbrst),
      mfpnpen(fpnpen),
      mmcheck(mcheck),
      AHBState(INIT),
      robin(0),
      mArbiterPEQ("ArbiterPEQ"),
      mRequestPEQ("RequestPEQ"),
      mResponsePEQ("ResponsePEQ"),
      clockcycle(10.0, sc_core::SC_NS) {

  if(ambaLayer==amba::amba_LT) {

    // Register tlm blocking transport function
    ahbIN.register_b_transport(this, &AHBCtrl::b_transport);

  }

  // Register non blocking transport functions
  if(ambaLayer==amba::amba_AT) {

    // Register tlm non blocking transport forward path
    ahbIN.register_nb_transport_fw(this, &AHBCtrl::nb_transport_fw, 0);

    // Register tlm non blocking transport backward path
    ahbOUT.register_nb_transport_bw(this, &AHBCtrl::nb_transport_bw, 0);
  }

  // Register debug transport
  ahbIN.register_transport_dbg(this, &AHBCtrl::transport_dbg);

  // Register arbiter thread
  SC_THREAD(ArbitrationThread);

  // Register request thread
  SC_THREAD(RequestThread);

  // Register response thread
  SC_THREAD(ResponseThread);

}

// Destructor
AHBCtrl::~AHBCtrl() {

}

// Helper function for creating slave map decoder entries
void AHBCtrl::setAddressMap(const uint32_t binding,const uint32_t hindex, const uint32_t haddr, const uint32_t hmask) {

  slave_info_t tmp;

  tmp.hindex = hindex;
  tmp.haddr  = haddr;
  tmp.hmask  = hmask;

  // Create slave map entry from slave ID and address range descriptor (slave_info_t)
  slave_map.insert(std::pair<uint32_t, slave_info_t>(binding, tmp));
}

// Find slave index by address
int AHBCtrl::get_index(const uint32_t address) {

  // Use 12 bit segment address for decoding
  uint32_t addr = address >> 20;

  for (it = slave_map.begin(); it != slave_map.end(); it++) {

      slave_info_t info = it->second;
  
      if (((addr ^ info.haddr) & info.hmask) == 0) {

	// There may be up to four BARs per device.
	// Only return device ID.
	return ((it->first)>>2);

      }
  }

  // no slave found
  return -1;
}

// Returns a PNP register from the slave configuration area
unsigned int AHBCtrl::getPNPReg(const uint32_t address) {

  // Calculate address offset in configuration area (slave info starts from 0x800)
  unsigned int addr   = address - ((mcfgaddr & mcfgmask) << 20);

  // Slave area
  if (addr >= 0x800) {

    addr -= 0x800;

    // Calculate index of the device in mSlaves pointer array (32 byte per device)
    unsigned int device = addr >> 3;
    // Calculate offset within device information
    unsigned int offset = addr & 0x7;

    return(mSlaves[device][offset]);

  } else {

    
    // Calculate index of the device in mMasters pointer array (32 byte per device)
    unsigned int device = addr >> 3;
    // Calculate offset within device information
    unsigned int offset = addr & 0x7;

    return(mMasters[device][offset]);

  }

}

// TLM blocking transport function (multi-sock)
void AHBCtrl::b_transport(uint32_t id, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {

  // master-address pair for dcache snooping
  t_snoop snoopy;

  // -- For Debug only --
  uint32_t a = 0;
  socket_t* other_socket = ahbIN.get_other_side(id, a);
  sc_core::sc_object *mstobj = other_socket->get_parent();
  // --------------------

  // Extract address from payload
  unsigned int addr   = trans.get_address();
  // Extract length from payload
  unsigned int length = trans.get_data_length();

  // Is this an access to configuration area
  if (mfpnpen && ((((addr >> 20) ^ mcfgaddr) & mcfgmask)==0)) {

    // Configuration area is read only
    if (trans.get_command() == tlm::TLM_READ_COMMAND) {

      // Extract data pointer from payload
      unsigned int *data  = (unsigned int *)trans.get_data_ptr();

      // No subword access supported here!
      assert(length%4==0);

      // Get registers from config area
      for (uint32_t i = 0; i < (length >> 2); i++) {

	data[i] = getPNPReg(addr);

	// one cycle delay per 32bit register
	delay += clockcycle;

      }
      
      // burn delay
      wait(delay);

      // and return
      trans.set_response_status(tlm::TLM_OK_RESPONSE);
      return;

    } else {

      v::error << name() << " Forbidden write to AHBCTRL configuration area (PNP)!" << v::endl;
      trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
      return;
    }
  }

  // Find slave by address / returns slave index or -1 for not mapped
  int index = get_index(trans.get_address());

  // For valid slave index
  if(index >= 0) {

    // -- For Debug only --
    other_socket = ahbOUT.get_other_side(index, a);
    sc_core::sc_object *obj = other_socket->get_parent();

    v::debug << name() << "AHB Request for address: " << hex << v::setfill('0')
             << v::setw(8) << trans.get_address() << ", from master: "
             << mstobj->name() << ", forwarded to slave: " << obj->name() << endl;

    // -------------------

    // Broadcast master_id and address for dcache snooping
    if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {

      snoopy.master_id  = id;
      snoopy.address = addr;
      snoopy.length = length;

      // Send to signal socket
      snoop.write(snoopy);

    }

    // Add delay for AHB address phase
    delay += clockcycle;

    // Forward request to the selected slave
    ahbOUT[index]->b_transport(trans, delay);

    return;

  } else {
    
    v::error << name() << "AHB Request 0x" << hex << v::setfill('0')
               << v::setw(8) << trans.get_address() << ", from master:"
               << mstobj->name() << ": Unmapped address space." << endl;

    // Invalid index
    trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    return;
  }
}

// Non-blocking forward transport function for ahb_slave multi-socket
// A master may send BEGIN_REQ or END_RESP. The model replies with
// TLM_ACCEPTED or TLM_COMPLETED, respectively.
tlm::tlm_sync_enum AHBCtrl::nb_transport_fw(uint32_t master_id, tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time &delay) {

  connection_t connection;

  v::debug << name() << "nb_transport_fw received phase: " << phase << v::endl;

  // The master has sent BEGIN_REQ
  if (phase == tlm::BEGIN_REQ) {

    // Memorize the master this was coming from
    // The slave info (second) will be filled in after decoding
    connection.first = master_id;
    connection.second = 0;
    addPendingTransaction(trans, connection);

    // All new transactions go in a PEQ to wait for sync
    mArbiterPEQ.notify(trans, delay);

    // Transaction accepted
    return tlm::TLM_ACCEPTED;

  } else if (phase == tlm::END_RESP) {

    // Let the response thread know that END_RESP
    // came in on the forward path.
    mEndResponseEvent.notify(delay);

    wait(delay);

    // Transaction completed
    return tlm::TLM_COMPLETED;

  } else {

    v::error << name() << "Invalid phase in call to nb_transport_fw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

  }

  return tlm::TLM_COMPLETED;
}

// Non-blocking backward transport function for ahb_master multi-socket
// A slave may send END_REQ or BEGIN_RESP. In both cases the model replies
// with TLM_ACCEPTED.
tlm::tlm_sync_enum AHBCtrl::nb_transport_bw(uint32_t id, tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time &delay) {

  v::debug << name() << "nb_transport_bw received phase: " << phase << v::endl;

  // The slave has sent END_REQ
  if (phase == tlm::END_REQ) {

    // Let the request thread know that END_REQ 
    // came in on return path.
    mEndRequestEvent.notify(delay);

  // New response - goes into response PEQ
  } else if (phase == tlm::BEGIN_RESP) {

    mEndRequestEvent.notify();
    mResponsePEQ.notify(trans, delay);

  } else {

    v::error << name() << "Invalid phase in call to nb_transport_bw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

  }

  return tlm::TLM_ACCEPTED;
}

// This thread models the arbitration schemes of the AHBCTRL.
// If the bus is BUSY all incoming requests are being delayed.
// If multiple masters request permission at the some simulation time,
// the one with the highest priority or the one pointed by the
// round 'robin' counter is select. All others get delayed.
void AHBCtrl::ArbitrationThread() {

  connection_t connection;
  int grand_id, master_id;
  payload_t* trans; 
  payload_t* selected_transaction = NULL;


  while(1) {

    // Wait for the next scheduled transaction
    wait(mArbiterPEQ.get_event());

    v::debug << name() << "ArbiterThread received new transaction" << v::endl;

    // Increment round robin pointer
    robin=(robin++)%(num_of_master_bindings-1);

    // Default: bus not granted
    grand_id = -1;

    // There may be multiple transactions (from multiple masters)
    // scheduled at the same time.
    while ((trans = mArbiterPEQ.get_next_transaction())!=0) {

      // Check if the bus is still busy
      if (AHBState==BUSY) {

	// Add one cycle of delay to transaction and throw back to PEQ
	mArbiterPEQ.notify(*trans, clockcycle);

      } else {

	// Get master id of transaction
	connection = pending_map[trans];
	master_id = connection.first;

	// Fixed priority arbitration
	if (mrrobin==0) {

	  // The master with the highest id has the highest priority
	  if (master_id > grand_id) {

	    // Throw back previously selected transaction
	    if (grand_id > -1) mArbiterPEQ.notify(*selected_transaction, clockcycle);

	    // Current transaction is new selected transaction
	    selected_transaction = trans;
	    grand_id = master_id;

	  } else {

	    // Throw back transaction (priority not high enough)
	    mArbiterPEQ.notify(*trans, clockcycle);

	  }
  
	// Round robin
	} else {

	  // The id matching the 'robin' pointer is granted
	  if (master_id == (int)robin) { 

	    selected_transaction = trans;
	    grand_id = master_id;

	  // All other transactions get delayed
	  } else {

	    mArbiterPEQ.notify(*trans, clockcycle);

	  }
	}
      }
    }

    // Is there a winner?
    if (grand_id != -1) { 

      v::debug << name() << "Master " << grand_id << " has won arbitration." << v::endl; 

      // Block all masters
      AHBState = BUSY;
      // Notify RequestThread
      mRequestPEQ.notify(*selected_transaction);

    }
  }
}

// The RequestThread is activated by the ArbitrationThread
// when a master has won arbitration. It takes care about
// address decoding (slave selection) and communication with
// the slaves nb_transport_fw interface.
void AHBCtrl::RequestThread() {

  payload_t* trans;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;
  connection_t connection;

  // Snooping info (master_id, address, length)
  t_snoop snoopy;

  while(true) {

    v::debug << name() << "Request thread waiting for new request" << v::endl;

    // Wait for arbiter to deliver next transaction
    wait(mRequestPEQ.get_event());

    // Get transaction from Queue
    trans = mRequestPEQ.get_next_transaction();

    v::debug << name() << "Request thread unblocked" << v::endl;
    
    // Extract address from payload
    unsigned int addr   = trans->get_address();
    // Extract length from payload
    unsigned int length = trans->get_data_length();

    // Is this an access to configuration area
    // ---------------------------------------
    if (mfpnpen && ((((addr >> 20) ^ mcfgaddr) & mcfgmask)==0)) {

      v::debug << name() << "Access to configuration area" << v::endl;

      // Configuration area is read only
      if (trans->get_command() == tlm::TLM_READ_COMMAND) {

        // Extract data pointer from payload
        unsigned int *data  = (unsigned int *)trans->get_data_ptr();

        // No subword access supported here!
        assert(length%4==0);

        // Get registers from config area
        for (uint32_t i = 0; i < (length >> 2); i++) {

	  data[i] = getPNPReg(addr);

	  // One cycle delay per 32bit register
	  delay += clockcycle;

        }

        trans->set_response_status(tlm::TLM_OK_RESPONSE);
	
	// Direct Response (Just like a slave returning TLM_COMPLETED
	// on BEGIN_REQ)
	mResponsePEQ.notify(*trans, delay);

      } else {

        v::error << name() << " Forbidden write to AHBCTRL configuration area (PNP)!" << v::endl;
        trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

      }

      // Ordinary access to slave domain
      // -------------------------------
      } else {

        // Find slave by address / returns slave index or -1 for not mapped
        int slave_id = get_index(trans->get_address());

        v::debug << name() << "Decoder: slave " << index << " address " << hex << addr << v::endl;

        if (index >= 0) {

	  // Add slave id to connection info
	  connection = pending_map[trans];
	  connection.second = slave_id;
	  pending_map[trans] = connection;

	  // Broadcast master_id, write address and length for snooping
	  if (trans->get_command()==tlm::TLM_WRITE_COMMAND) {

	    v::debug << name() << "Broadcast snooping info!" << v::endl;

	    snoopy.master_id = connection.first;
	    snoopy.address   = addr;
	    snoopy.length    = length;

	    // Broadcast snoop information
	    snoop.write(snoopy);
	  
	  }

          phase = tlm::BEGIN_REQ;

          // One cycle delay for addressing
          delay = sc_core::sc_time(10, SC_NS);
      
	  v::debug << name() << "Call to nb_transport_fw with phase " << phase << v::endl;

          // Forward request to the selected slave
          status = ahbOUT[slave_id]->nb_transport_fw(*trans, phase, delay);

	  v::debug << name() << "nb_transport_fw returned status " << status << v::endl;

          switch (status) {

            case tlm::TLM_ACCEPTED:
            case tlm::TLM_UPDATED:

	      if (phase == tlm::BEGIN_REQ) {

	        // Probably TLM_ACCEPTED.
	        // Request phase is not completed yet.
	        // Have to wait for END_REQ phase to come
	        // in on backward path.
	        // (mEndRequestEvent has delayed notification)

		v::debug << name() << "Request thread waiting for EndRequestEvent" << v::endl;
	        wait(mEndRequestEvent);

	      } else if (phase == tlm::END_REQ) {

	        // End of request via return path.
	        // Burn annotated delay.
	        wait(delay);

	      } else if (phase == tlm::BEGIN_RESP) {

	        // Begin of response initiated via return path.
	        // Put into response queue.

	        mResponsePEQ.notify(*trans, delay);
	  
              } else {

	        // END_RESP may only come from master
	        v::error << name() << "Invalid phase in return path from call to nb_transport_fw!" << v::endl;
	        trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

	      }

	      break;

            case tlm::TLM_COMPLETED:

	      // Slave directly jumps to TLM_COMPLETED (Pseudo AT).
	      // Put response into queue.
	      mResponsePEQ.notify(*trans, delay);
	      wait(delay);

	      break;

            default:

	      v::error << name() << "Invalid return value from call to nb_transport_fw" << v::endl;
	      trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

          }
     
        } else {

        v::error << name() << "No slave found for address: " << hex << addr << v::endl;
        trans->set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);

      }
    }
  }
}

// The ResponseThread can be activated by the nb_transport_bw function 
// (Slave sends BEGIN_RESP) or by the RequestThread (Slave returns 
// TLM_UPDATED with BEGIN_RESP or TLM_COMPLETED).
void AHBCtrl::ResponseThread() {

  payload_t* trans;
  unsigned int master_id, slave_id;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;
  connection_t connection;

  while(1) {

    v::debug << name() << "Response thread waiting for new response" << v::endl;

    // Wait for response from slave (inserted in transport_bw)
    wait(mResponsePEQ.get_event());

    // There can be only one response at the time,
    // because the arbiter serializes communication.
    // (no concurrent master-slave connections allowed)
    trans = mResponsePEQ.get_next_transaction();

    v::debug << name() << "Response thread unblocked" << v::endl;

    // Retrieve master id
    connection = pending_map[trans];
    master_id  = connection.first;
    slave_id   = connection.second;

    // Prepare BEGIN_REQ
    phase = tlm::BEGIN_RESP;
    delay = SC_ZERO_TIME;

    v::debug << name() << "Call to nb_transport_bw with phase " << phase << v::endl; 

    // Call nb_transport of master
    status = ahbIN[master_id]->nb_transport_bw(*trans, phase, delay);

    v::debug << name() << "nb_transport_bw returned with phase " << phase << " and status " << status << v::endl;

    switch (status) {

      case tlm::TLM_ACCEPTED:
      case tlm::TLM_UPDATED:

	if (phase == tlm::BEGIN_RESP) {

	  // Probably TLM_ACCEPTED.
	  // Wait for END_RESP to come in on forward path.
	  // (mEndResponseEvent has delayed notification)

	  v::debug << name() << "Response thread waiting for EndResponseEvent" << v::endl;
	  wait(mEndResponseEvent);

        } else if (phase == tlm::END_RESP) {

	  // Master returned END_RESP.
	  // Burn delay.
	  wait(delay);

	} else {

	  v::error << name() << "Invalid phase from call to nb_transport_bw!" << v::endl;
	  trans->set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

	}
	
	break;

      case tlm::TLM_COMPLETED:

	// Master returned TLM completed.
	// Burn delay.
	wait(delay);

	break;

      default:

	v::error << name() << "Invalid return value from call to nb_transport_bw!" << v::endl;

    }

    // Let slave know, we are done.
    // (Must be done, because we return TLM_ACCEPTED on BEGIN_RESP)
    phase=tlm::END_RESP;
    delay=SC_ZERO_TIME;
    status = ahbOUT[slave_id]->nb_transport_fw(*trans, phase, delay);
    assert((status==tlm::TLM_COMPLETED)||(status==tlm::TLM_ACCEPTED));

    // Cleanup
    // -------
    // Remove connection info form pending_map
  
    v::debug << name() << "Remove transaction from pending_map." << v::endl;
    pending_map.erase(trans);
    // Release transaction
    trans->release();
    // Ready for new transaction
    AHBState = IDLE;

  }
}

// Keeps track of master-payload relation. All incoming transactions
// are entered into the 'pending_map'. This way we can identify their origin
// once the fall out of the RequestPEQ.
void AHBCtrl::addPendingTransaction(payload_t& trans, connection_t connection) {

  // Transaction must be unique
  assert(pending_map.find(&trans) == pending_map.end());

  // Enter transaction
  pending_map[&trans] = connection;

}

// Set up slave map and collect plug & play information
void AHBCtrl::start_of_simulation() {

 
  // Get number of bindings at master socket (number of connected slaves)
  num_of_slave_bindings = ahbOUT.size();
  // Get number of bindings at slave socket (number of connected masters)
  num_of_master_bindings = ahbIN.size();

  // Max. 16 AHB slaves allowed
  assert(num_of_slave_bindings<=16);

  // Max. 16 AHB masters allowed
  assert(num_of_master_bindings<=16);

  v::info << name() << "******************************************************************************* " << v::endl;
  v::info << name() << "* DECODER INITIALIZATION " << v::endl;
  v::info << name() << "* ---------------------- " << v::endl;

  // Iterate/detect the registered slaves
  // ------------------------------------
  for (uint32_t i = 0; i < num_of_slave_bindings<<2; i+=4) {

    uint32_t a = 0;

    // Get pointer to socket of slave i
    socket_t *other_socket = ahbOUT.get_other_side(i>>2, a);

    // Get parent object containing slave socket i
    sc_core::sc_object *obj = other_socket->get_parent();

    // Valid slaves implement the AHBDevice interface
    AHBDevice *slave = dynamic_cast<AHBDevice *> (obj);

    v::info << name() << "* SLAVE name: " << obj->name() << v::endl;

    // Slave is valid (implements AHBDevice)
    if(slave) {

      // Get pointer to device information
      const uint32_t * deviceinfo = slave->get_device_info();
      
      // Get bus id (hindex oder master id)
      const uint32_t sbusid = slave->get_busid();
      assert(sbusid < 16);

      // Map device information into PNP region
      if (mfpnpen) {
	
	mSlaves[sbusid] = deviceinfo;

      }

      // Each slave may have up to four subdevices (BARs)
      for (uint32_t j = 0; j < 4; j++) {

	// Check 'type' field of bar[j] (must be != 0)
	if (slave->get_bar_type(j)) {

	  // Get base address and maks from BAR i
	  uint32_t addr = slave->get_bar_base(j);
	  uint32_t mask = slave->get_bar_mask(j);

	  v::info << name() << "* BAR" << j << " with MSB addr: " << hex << addr << " and mask: " << mask <<  v::endl; 

	  // Insert slave region into memory map
	  setAddressMap(i+j, sbusid, addr, mask);

	} else {

	  v::info << name() << "* BAR" << j << " not used." << v::endl;

	}
      
      } 

    } else {
      
      v::error << name() << "Slave bound to socket 'ahbOUT' is not a valid AHBDevice (no plug & play information)!" << v::endl;
      assert(0);

    }

    // Now ready for action
    AHBState = IDLE;

  }

  // Iterate/detect the registered masters
  // ------------------------------------
  for (uint32_t i = 0; i < (num_of_master_bindings<<2); i+=4) {

    uint32_t a = 0;

    // get pointer to socket of slave i
    socket_t *other_socket = ahbIN.get_other_side(i>>2, a);

    // get parent object containing slave socket i
    sc_core::sc_object *obj = other_socket->get_parent();

    // valid masters implement the AHBDevice interface
    AHBDevice *master = dynamic_cast<AHBDevice *> (obj);

    v::info << name() << "* Master name: " << obj->name() << v::endl;

    // master is valid (implements AHBDevice)
    if(master) {

      // Get pointer to device information
      const uint32_t * deviceinfo = master->get_device_info();

      // Get id of the master
      const uint32_t mbusid = master->get_busid();
      assert(mbusid < 16);

      // Map device information into PNP region
      if (mfpnpen) {
	
	mMasters[mbusid] = deviceinfo;

      }

      // Each master may have up to four subdevices (BARs)
      for (uint32_t j = 0; j < 4; j++) {

	// check 'type' field of bar[j] (must be != 0)
	if (master->get_bar_type(j)) {

	  // get base address and maks from BAR i
	  uint32_t addr = master->get_bar_base(j);
	  uint32_t mask = master->get_bar_mask(j);

	  v::info << name() << "* BAR" << j << " with MSB addr: " << hex << addr << " and mask: " << mask <<  v::endl; 

	} else {

	  v::info << name() << "* BAR" << j << " not used." << v::endl;

	}
      
      } 

    } else {
      
      v::error << name() << "Master bound to socket 'ahbin' is not a valid AHBDevice" << v::endl;
      assert(0);

    }
  }

  // End of decoder initialization
  v::debug << name() << "******************************************************************************* " << v::endl;

  // Check memory map for overlaps
  if (mmcheck) {

    //checkMemMap();

  }
}

// Check the memory map for overlaps
void AHBCtrl::checkMemMap() {
   std::map<uint32_t, slave_info_t>::iterator it;
   std::map<uint32_t, slave_info_t>::iterator it2;

   /*
   for(it=slave_map.begin(), it2=slave_map.begin(); it!=slave_map.end(); it++, it2++) {
      for(it2++; it2!=slave_map.end(); it2++) {
         if(((it2->second.first >= it->second.first) &&
            (it2->second.first < it->second.second)) ||
            (((it2->second.second - 1) >= it->second.first) &&
             ((it2->second.second - 1)< it->second.second))) {
            // Memory regions overlap output warning
             uint32_t a = 0;
             socket_t *other_socket = ahbOUT.get_other_side(it->first, a);
             sc_core::sc_object *obj = other_socket->get_parent();

             other_socket = ahbOUT.get_other_side(it2->first, a);
             sc_core::sc_object *obj2 = other_socket->get_parent();

             v::error << name() << "Overlap in AHB memory mapping." << endl;
             v::debug << name() << obj->name() << "@0x" << hex << v::setw(8)
                      << v::setfill('0') << it->second.first << ":0x" << hex
                      << v::setw(8) << v::setfill('0') << (it->second.second - 1) << endl;
             v::debug << name() << obj2->name() << "@0x" << hex << v::setw(8)
                      << v::setfill('0') << it2->second.first << ":0x" << hex
                      << v::setw(8) << v::setfill('0') << (it2->second.second - 1)
                      << endl;
         }
      }
   }
   */
}

// TLM debug interface
unsigned int AHBCtrl::transport_dbg(uint32_t id, tlm::tlm_generic_payload &trans) {

    // -- For Debug only --
    uint32_t a = 0;
    socket_t* other_socket = ahbIN.get_other_side(id, a);
    sc_core::sc_object *mstobj = other_socket->get_parent();
    // --------------------

    // Extract data pointer from payload
    unsigned int *data  = (unsigned int *)trans.get_data_ptr();
    // Extract address from payload
    unsigned int addr   = trans.get_address();
    // Extract length from payload
    unsigned int length = trans.get_data_length(); 

    // Is this an access to configuration area
    if (mfpnpen && ((((addr >> 20) ^ mcfgaddr) & mcfgmask)==0)) {

      // Configuration area is read only
      if (trans.get_command() == tlm::TLM_READ_COMMAND) {

	// No subword access supported here!
	assert(length%4==0);

	// Get registers from config area
	for (uint32_t i = 0 ; i < (length >> 2); i++) {

	  data[i] = getPNPReg(addr);

        }
	
	trans.set_response_status(tlm::TLM_OK_RESPONSE);
	return length;

      } else {

	v::error << name() << " Forbidden write to AHBCTRL configuration area (PNP)!" << v::endl;
	trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
	return 0;
      }
    }    

    // Find slave by address / returns slave index or -1 for not mapped
    int index = get_index(trans.get_address());

    // For valid slave index
    if(index >= 0) {

       // -- For Debug only --
       other_socket = ahbOUT.get_other_side(index, a);
       sc_core::sc_object *obj = other_socket->get_parent();

       v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
                << v::setw(8) << trans.get_address() << ", from master:"
                << mstobj->name() << ", forwarded to slave:" << obj->name() << endl;
       // --------------------

       // Forward request to the selected slave
       return ahbOUT[index]->transport_dbg(trans);

    } else {

       v::warn << name() << "AHB Request@0x" << hex << v::setfill('0')
               << v::setw(8) << trans.get_address() << ", from master:"
               << mstobj->name() << ": Unmapped address space." << endl;

       // Invalid index
       trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
       return 0;
    }
}

// Helper for setting clock cycle latency using sc_clock argument
void AHBCtrl::clk(sc_core::sc_clock &clk) {

  clockcycle = clk.period();

}

// Helper for setting clock cycle latency using sc_time argument
void AHBCtrl::clk(sc_core::sc_time &period) {

  clockcycle = period;

}

// Helper for setting clock cycle latency using a value-time_unit pair
void AHBCtrl::clk(double period, sc_core::sc_time_unit base) {

  clockcycle = sc_core::sc_time(period, base);

}
