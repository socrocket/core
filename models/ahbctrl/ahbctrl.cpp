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
		 bool pow_mon,         // Enable power monitoring
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
      m_pow_mon(pow_mon),
      robin(0),
      mArbiterPEQ("ArbiterPEQ"),
      mRequestPEQ("RequestPEQ"),
      mDataPEQ("DataPEQ"),
      mEndDataPEQ("EndDataPEQ"),
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

    // Register arbiter thread
    SC_THREAD(arbitrate_me);

    // Register request thread
    SC_THREAD(RequestThread);

    // Register data thread
    SC_THREAD(DataThread);

    SC_THREAD(EndData);

    // Register response thread
    SC_THREAD(ResponseThread);

    selected_transaction = NULL;

  }

  // Register debug transport
  ahbIN.register_transport_dbg(this, &AHBCtrl::transport_dbg);

  // Register power monitor
  PM::registerIP(this,"ahbctrl",m_pow_mon);
  PM::send_idle(this,"idle",sc_time_stamp(),m_pow_mon);

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

    // Power event start
    PM::send(this,"ahb_trans",1,sc_time_stamp(),(unsigned int)trans.get_data_ptr(),m_pow_mon);
    
    // Add delay for AHB address phase
    delay += clockcycle;

    // Forward request to the selected slave
    ahbOUT[index]->b_transport(trans, delay);

    // Power event end
    PM::send(this,"ahb_trans",0,sc_time_stamp(),(unsigned int)trans.get_data_ptr(),m_pow_mon);

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

  v::debug << name() << "nb_transport_fw received transaction " << hex << &trans << " with phase " << phase << v::endl;

  // The master has sent BEGIN_REQ
  if (phase == tlm::BEGIN_REQ) {

    // Memorize the master this was coming from
    // The slave info (second) will be filled in after decoding
    connection.master_id = master_id;
    connection.slave_id  = 0;
    connection.state     = PENDING;

    addPendingTransaction(trans, connection);

    // All new transactions go in a PEQ to wait for sync
    mArbiterPEQ.notify(trans, delay);

    // Transaction accepted
    return tlm::TLM_ACCEPTED;

  } else if (phase == amba::BEGIN_DATA) {
    
    mDataPEQ.notify(trans, delay);

    // Will send END_DATA later
    return tlm::TLM_ACCEPTED;

  } else if (phase == tlm::END_RESP) {

    // Let the response thread know that END_RESP
    // came in on the forward path.
    mEndResponseEvent.notify(delay);

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

  v::debug << name() << "nb_transport_bw received transaction " << hex << &trans << " with phase: " << phase << v::endl;

  // The slave has sent END_REQ
  if (phase == tlm::END_REQ) {

    // Let the request thread know that END_REQ 
    // came in on return path.
    mEndRequestEvent.notify(delay);

  // New response - goes into response PEQ
  } else if (phase == tlm::BEGIN_RESP) {

    mEndRequestEvent.notify();
    mResponsePEQ.notify(trans, delay);

  } else if (phase == amba::END_DATA) {

    mEndDataPEQ.notify(trans, delay);

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
void AHBCtrl::arbitrate_me() {

  connection_t connection;
  int grand_id;
  
  wait(1,SC_PS);

  while(1) {

    wait(clockcycle);

    grand_id = -1;

    // Increment round robin pointer
    robin=(robin++) % num_of_master_bindings;

    for(pm_itr = pending_map.begin(); pm_itr != pending_map.end(); pm_itr++) {

      connection = pm_itr->second;

      if (((int)connection.master_id) >= grand_id) {

	// make sure same transaction is not arbitrated twice
	if (connection.state == PENDING) {

	  selected_transaction = pm_itr->first;
	  grand_id = connection.master_id;

	  v::debug << name() << "Arbiter selects master " << grand_id << " (Trans. " << hex << selected_transaction << ")" << v::endl;

	}
      }
    }

    // There is a winner
    if (grand_id > -1) {

      connection = pending_map[selected_transaction];
      connection.state = BUSY;
      pending_map[selected_transaction] = connection;

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

  while(true) {

    //v::debug << name() << "Request thread waiting for new event" << v::endl;

    wait(mRequestPEQ.get_event());

    // Get transaction from request PEQ
    trans = mRequestPEQ.get_next_transaction();
    assert(trans != NULL);

    // Read connection info
    connection = pending_map[trans];

    // Extract address from payload
    unsigned int addr       = trans->get_address();

    // Access to configuration area?
    // Do not need to send BEGIN_REQ to slave
    // ---------------------------------------
    if (mfpnpen && ((((addr >> 20) ^ mcfgaddr) & mcfgmask)==0)) {

      v::debug << name() << "Access to configuration area" << v::endl;

      // Notify response thread
      mResponsePEQ.notify(*trans, clockcycle);

    // Access to slave domain
    // Do address decoding and send BEGIN_REQ
    // --------------------------------------
    } else {

      // Find slave by address / returns slave index or -1 for not mapped
      int slave_id = get_index(trans->get_address());

      v::debug << name() << "Decoder: slave " << index << " address " << hex << addr << v::endl;

      if (index >= 0) {

	// Add slave id to connection info
	connection.slave_id = slave_id;
	pending_map[trans] = connection;

	// Power event start
	PM::send(this, "ahb_trans", 1, sc_time_stamp(),(unsigned int)trans->get_data_ptr(),m_pow_mon);

	// Send BEGIN_REQ to slave
	phase = tlm::BEGIN_REQ;
	delay = sc_time(1, SC_PS);

	v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;
	  
	status = ahbOUT[slave_id]->nb_transport_fw(*trans, phase, delay);

	assert((status==tlm::TLM_UPDATED)||(status=tlm::TLM_ACCEPTED));

	// In case the slave did not return END_REQ,
	// wait for it to come in on the backward path.
	if (phase != tlm::END_REQ) {

	  wait(mEndRequestEvent);

	} else {

	  // Consume accept delay
	  wait(delay);
	  delay = SC_ZERO_TIME;

	}
      }
    }

    // Send END_REQ to the master
    phase = tlm::END_REQ;
    delay = SC_ZERO_TIME;
	  
    v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << phase << v::endl;

    status = ahbIN[connection.master_id]->nb_transport_bw(*trans, phase, delay);

    assert(status == tlm::TLM_ACCEPTED);    

  }
}

void AHBCtrl::DataThread() {

  payload_t* trans;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;
  connection_t connection;
  t_snoop snoopy;

  while(1) {

    //v::debug << name() << "Data thread waiting for new event" << v::endl;

    wait(mDataPEQ.get_event());
    
    trans = mDataPEQ.get_next_transaction();

    assert(trans != NULL);

    // Extract address from payload
    unsigned int addr       = trans->get_address();

    // Access to configuration area?
    // Do not need to send BEGIN_REQ to slave
    // ---------------------------------------
    if (mfpnpen && ((((addr >> 20) ^ mcfgaddr) & mcfgmask)==0)) {

      v::error << name() << "Configuration area is read-only" << v::endl;    
    
    } else {

      connection = pending_map[trans];

      // Send BEGIN_DATA to slave
      phase = amba::BEGIN_DATA;

      v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;

      status = ahbOUT[connection.slave_id]->nb_transport_fw(*trans, phase, delay);

      assert((status == tlm::TLM_ACCEPTED)||(status == tlm::TLM_UPDATED));

      wait(delay);

      // Broadcast master_id, write address and length for snooping
      // ----------------------------------------------------------
      v::debug << name() << "Broadcast snooping info!" << v::endl;

      snoopy.master_id = connection.master_id;
      snoopy.address   = addr;
      snoopy.length    = trans->get_data_length();

      // Broadcast snoop information
      snoop.write(snoopy);
      // ----------------------------------------------------------
    }
  }
}

void AHBCtrl::EndData() {

  payload_t* trans;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;
  connection_t connection;

  while(1) {

    wait(mEndDataPEQ.get_event());

    trans = mEndDataPEQ.get_next_transaction();
    assert(trans!=NULL);

    // read connection info
    connection = pending_map[trans];

    // Send END_DATA to master
    phase = amba::END_DATA;
    delay = SC_ZERO_TIME;

    v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << phase << v::endl;

    status = ahbIN[connection.master_id]->nb_transport_bw(*trans, phase, delay);

    assert((status == tlm::TLM_ACCEPTED)||(status == tlm::TLM_COMPLETED));

    // Power event end
    PM::send(this,"ahb_trans", 0, sc_time_stamp(),(unsigned int)trans->get_data_ptr(),m_pow_mon);

    // Cleanup
    // -------
    // Remove connection info form pending_map
    v::debug << name() << "Remove transaction from pending_map." << v::endl;

    pending_map.erase(trans);

  }

}

// The ResponseThread can be activated by the nb_transport_bw function 
// (Slave sends BEGIN_RESP) or by the RequestThread (Slave returns 
// TLM_UPDATED with BEGIN_RESP or TLM_COMPLETED).
void AHBCtrl::ResponseThread() {

  payload_t* trans;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;
  connection_t connection;

  while(1) {

    //v::debug << name() << "Response thread waiting for new response" << v::endl;

    // Wait for response from slave (inserted in transport_bw)
    wait(mResponsePEQ.get_event());

    trans = mResponsePEQ.get_next_transaction();

    // Extract address from payload
    unsigned int addr       = trans->get_address();

    // Access to configuration area?
    // ---------------------------------------
    if (mfpnpen && ((((addr >> 20) ^ mcfgaddr) & mcfgmask)==0)) {

      v::error << name() << "Reading configuration area" << v::endl;    
    
      // Extract data pointer from payload
      unsigned int *data  = (unsigned int *)trans->get_data_ptr();
      unsigned int length = trans->get_data_length();

      // No subword access supported here!
      assert(length%4==0);

      // Get registers from config area
      for (uint32_t i = 0; i < (length >> 2); i++) {

	data[i] = getPNPReg(addr);

	// One cycle delay per 32bit register
	delay += clockcycle;

      }

      trans->set_response_status(tlm::TLM_OK_RESPONSE);

    }

    // Find back connect info
    connection = pending_map[trans];

    // Send BEGIN_RESP to master
    phase = tlm::BEGIN_RESP;
    delay = SC_ZERO_TIME;

    v::debug << name() << "Call to nb_transport_bw with phase " << phase << v::endl; 

    // Call nb_transport_bw of master
    status = ahbIN[connection.master_id]->nb_transport_bw(*trans, phase, delay);

    assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_UPDATED)||(status==tlm::TLM_COMPLETED));

    if (status == tlm::TLM_ACCEPTED) {

      wait(mEndResponseEvent);

    }

    // If not config, send END_RESP
    if (!(mfpnpen && ((((addr >> 20) ^ mcfgaddr) & mcfgmask)==0))) {

      // Send END_RESP to slave
      phase = tlm::END_RESP;
      delay = SC_ZERO_TIME;

      // Call nb_transport_fw of slave
      status = ahbOUT[connection.slave_id]->nb_transport_fw(*trans, phase, delay);

      assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_COMPLETED));

      wait(delay);
  
    }

    // End power event
    PM::send(this,"ahb_trans", 0, sc_time_stamp(),(unsigned int)trans->get_data_ptr(),m_pow_mon);

    // Cleanup
    // -------
    // Remove connection info form pending_map
    v::debug << name() << "Remove transaction from pending_map." << v::endl;

    pending_map.erase(trans);

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

    v::info << name() << "******************************************************************************* " << v::endl;

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
  v::info << name() << "******************************************************************************* " << v::endl;

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
