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
#include "vendian.h"

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
      mRequestPEQ("RequestPEQ"),
      mDataPEQ("DataPEQ"),
      mEndDataPEQ("EndDataPEQ"),
      mResponsePEQ("ResponsePEQ"),
      m_total_wait(SC_ZERO_TIME),
      m_arbitrated(0),
      m_max_wait(SC_ZERO_TIME),
      m_max_wait_master(defmast),
      m_idle_count(0),
      m_total_transactions(0), 
      m_right_transactions(0),
      m_writes(0),
      m_reads(0),
      m_ambaLayer(ambaLayer) 

{

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
  PM::registerIP(this, "ahbctrl", m_pow_mon);
  PM::send_idle(this, "idle", sc_time_stamp(), m_pow_mon);

  // Module configuration report
  v::info << name() << " ******************************************************************************* " << v::endl;
  v::info << name() << " * Created AHBCTRL with following parameters: " << v::endl;
  v::info << name() << " * ------------------------------------------ " << v::endl;
  v::info << name() << " * ioaddr/iomask: " << hex << ioaddr << "/" << iomask << v::endl;
  v::info << name() << " * cfgaddr/cfmask: " << hex << cfgaddr << "/" << cfgmask << v::endl;
  v::info << name() << " * rrobin: " << rrobin << v::endl;
  v::info << name() << " * split: " << split << v::endl;
  v::info << name() << " * defmast: " << defmast << v::endl;
  v::info << name() << " * ioen: " << ioen << v::endl;
  v::info << name() << " * fixbrst: " << fixbrst << v::endl;
  v::info << name() << " * fpnpen: " << fpnpen << v::endl;
  v::info << name() << " * mcheck: " << mcheck << v::endl;
  v::info << name() << " * pow_mon: " << pow_mon << v::endl;
  v::info << name() << " * ambaLayer (LT = 8 / AT = 4):  " << ambaLayer << v::endl;
  v::info << name() << " ******************************************************************************* " << v::endl; 

}

void AHBCtrl::dorst() {
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
  m_total_transactions++;
  // Use 12 bit segment address for decoding
  uint32_t addr = address >> 20;

  for (it = slave_map.begin(); it != slave_map.end(); it++) {

      slave_info_t info = it->second;
  
      if (((addr ^ info.haddr) & info.hmask) == 0) {

	// There may be up to four BARs per device.
	// Only return device ID.
  m_right_transactions++;
	return ((it->first)>>2);
 
      }
  }

  // no slave found
  return -1;
}

// Returns a PNP register from the slave configuration area
unsigned int AHBCtrl::getPNPReg(const uint32_t address) {

  m_total_transactions++;
  // Calculate address offset in configuration area (slave info starts from 0x800)
  unsigned int addr   = address - (((mioaddr << 20) | (mcfgaddr << 8)) & ((miomask << 20) | (mcfgmask << 8)));
  v::debug << name() << "Accessing PNP area at " << addr << v::endl;

  // Slave area
  if (addr >= 0x800) {

    addr -= 0x800;

    // Calculate index of the device in mSlaves pointer array (32 byte per device)
    // Shift first to get word addresses.
    unsigned int device = (addr >> 2) >> 3;
    // Calculate offset within device information
    unsigned int offset = (addr >> 2) & 0x7;

    if(device>=num_of_slave_bindings) {
        v::warn << name() << "Access to unregistered PNP Slave Register!" << v::endl;
        return 0;
    }
    uint32_t result =  mSlaves[device][offset];
    #ifdef LITTLE_ENDIAN_BO
    swap_Endianess(result);
    #endif
    m_right_transactions++;
    return result;

  } else {

    
    // Calculate index of the device in mMasters pointer array (32 byte per device)
    unsigned int device = (addr >> 2) >> 3;
    // Calculate offset within device information
    unsigned int offset = (addr >> 2) & 0x7;

    if(device>=num_of_master_bindings) {
        v::warn << name() << "Access to unregistered PNP Master Register!" << v::endl;
        return 0;
    }
    uint32_t result = mMasters[device][offset];
    #ifdef LITTLE_ENDIAN_BO
    swap_Endianess(result);
    #endif
    return result;

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

  // Collect transport statistics
  transport_statistics(trans);

  // Extract address from payload
  unsigned int addr   = trans.get_address();
  // Extract length from payload
  unsigned int length = trans.get_data_length();

  // Is this an access to configuration area
  if (mfpnpen && ((((addr ^ ((mioaddr << 20) | (mcfgaddr << 8))) & ((miomask << 20) | (mcfgmask << 8))))==0)) {

    // Configuration area is read only
    if (trans.get_command() == tlm::TLM_READ_COMMAND) {

      // Extract data pointer from payload
      unsigned int *data  = (unsigned int *)trans.get_data_ptr();

      // No subword access supported here!
      //assert(length%4==0);

      // Get registers from config area
      for (uint32_t i = 0; i < (length >> 2); i++) {
          data[i] = getPNPReg(addr + (i<<2));

          // one cycle delay per 32bit register
          delay += clock_cycle;
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

    v::debug  << name() << "AHB Request for address: 0x" << hex << v::setfill('0')
	      << v::setw(8) << trans.get_address() << ", from master: "
	      << mstobj->name() << ", forwarded to slave: " << obj->name() << endl;
    // --------------------

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
    delay += clock_cycle;
    
    // Forward request to the selected slave
    ahbOUT[index]->b_transport(trans, delay);

    // Power event end
    PM::send(this,"ahb_trans",0,sc_time_stamp()+delay,(unsigned int)trans.get_data_ptr(),m_pow_mon);

    // !!!! TMP FOR BUGFIXING
    unsigned char * test_data;
    test_data = trans.get_data_ptr();

    v::debug << name() << "Data after call to slave: " << v::uint32 << *((uint32_t *)test_data) << v::endl;

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

  v::debug << name() << "nb_transport_fw received transaction 0x" << hex << &trans << " with phase " << phase << v::endl;

  // The master has sent BEGIN_REQ
  if (phase == tlm::BEGIN_REQ) {

    // Memorize the master this was coming from
    // The slave info (second) will be filled in after decoding
    connection.master_id  = master_id;
    connection.slave_id   = 0;
    connection.start_time = sc_time_stamp();
    connection.state      = PENDING;

    // Collect transport statistics
    transport_statistics(trans);

    // Add to arbiter queue
    addPendingTransaction(trans, connection);

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

    v::error << name() << "Illegal phase in call to nb_transport_fw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

  }

  return tlm::TLM_COMPLETED;
}

// Non-blocking backward transport function for ahb_master multi-socket
// A slave may send END_REQ or BEGIN_RESP. In both cases the model replies
// with TLM_ACCEPTED.
tlm::tlm_sync_enum AHBCtrl::nb_transport_bw(uint32_t id, tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time &delay) {

  v::debug << name() << "nb_transport_bw received transaction 0x" << hex << &trans << " with phase: " << phase << v::endl;

  // The slave has sent END_REQ
  if (phase == tlm::END_REQ) {

    // Let the request thread know that END_REQ 
    // came in on return path.
    //mEndRequestEvent.notify(delay);
    mEndRequestEvent.notify();

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
  
  // The arbiter waiting time of a certain transaction
  sc_time waiting_time;

  connection_t connection;
  int grand_id;
  
  wait(1,SC_PS);

  while(1) {

    wait(clock_cycle);

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

	  v::debug << name() << "Arbiter selects master " << grand_id << " (Trans. 0x" << hex << selected_transaction << ")" << v::endl;

	}
      }
    }

    // There is a winner
    if (grand_id > -1) {

      // Get the selected transaction from pending map
      connection = pending_map[selected_transaction];

      // Calculate the waiting time of the transaction
      waiting_time = sc_time_stamp() - connection.start_time;

      if (waiting_time > m_max_wait) {

	// New maximum waiting time
	m_max_wait = waiting_time;
	m_max_wait_master = grand_id;

      }

      // Accumulate total waiting time 
      m_total_wait += waiting_time;
      // Increment absolute number of arbitrated transactions
      m_arbitrated++;

      connection.state = BUSY;
      pending_map[selected_transaction] = connection;

      mRequestPEQ.notify(*selected_transaction);

    } else {

      m_idle_count++;

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
    if (mfpnpen && ((((addr ^ ((mioaddr << 20) | (mcfgaddr << 8))) & ((miomask << 20) | (mcfgmask << 8))))==0)) {

      v::debug << name() << "Access to configuration area" << v::endl;

      // Notify response thread
      mResponsePEQ.notify(*trans, clock_cycle);

    // Access to slave domain
    // Do address decoding and send BEGIN_REQ
    // --------------------------------------
    } else {

      // Find slave by address / returns slave index or -1 for not mapped
      int slave_id = get_index(trans->get_address());

      v::debug << name() << "Decoder: slave " << dec << slave_id << " address 0x" << hex << addr << v::endl;

      if (slave_id >= 0) {

	// Add slave id to connection info
	connection.slave_id = slave_id;
	pending_map[trans] = connection;

	// Power event start
	//PM::send(this, "ahb_trans", 1, sc_time_stamp(),(unsigned int)trans->get_data_ptr(),m_pow_mon);

	// Send BEGIN_REQ to slave
	phase = tlm::BEGIN_REQ;
	delay = sc_time(1, SC_PS);

	v::debug << name() << "Transaction 0x" << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;
	  
	status = ahbOUT[slave_id]->nb_transport_fw(*trans, phase, delay);

	assert((status==tlm::TLM_UPDATED)||(status==tlm::TLM_ACCEPTED));

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
	  
    v::debug << name() << "Transaction 0x" << hex << trans << " call to nb_transport_bw with phase " << phase << v::endl;

    status = ahbIN[connection.master_id]->nb_transport_bw(*trans, phase, delay);

    assert(status == tlm::TLM_ACCEPTED);    

  }
}

// Thread for modeling the AHB data phase for write transactions.
// Sends BEGIN_DATA to slave and handles snooping.
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
    if (mfpnpen && ((((addr ^ ((mioaddr << 20) | (mcfgaddr << 8))) & ((miomask << 20) | (mcfgmask << 8))))==0)) {

      v::error << name() << "Configuration area is read-only" << v::endl;    
    
    } else {

      connection = pending_map[trans];

      // Send BEGIN_DATA to slave
      phase = amba::BEGIN_DATA;

      v::debug << name() << "Transaction 0x" << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;

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

    v::debug << name() << "Transaction 0x" << hex << trans << " call to nb_transport_bw with phase " << phase << v::endl;

    status = ahbIN[connection.master_id]->nb_transport_bw(*trans, phase, delay);

    assert((status == tlm::TLM_ACCEPTED)||(status == tlm::TLM_COMPLETED));

    // Power event end
    //PM::send(this,"ahb_trans", 0, sc_time_stamp()+sc_time(1, SC_PS),(unsigned int)trans->get_data_ptr(),m_pow_mon);

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
    if (mfpnpen && ((((addr ^ ((mioaddr << 20) | (mcfgaddr << 8))) & ((miomask << 20) | (mcfgmask << 8))))==0)) {


      v::error << name() << "Reading configuration area" << v::endl;    
    
      // Extract data pointer from payload
      unsigned int *data  = (unsigned int *)trans->get_data_ptr();
      unsigned int length = trans->get_data_length();

      // No subword access supported here!
      assert(length%4==0);

      // Get registers from config area
      for (uint32_t i = 0; i < (length >> 2); i++) {

	data[i] = getPNPReg(addr + (i<<2));

	// One cycle delay per 32bit register
	delay += clock_cycle;

      }

      trans->set_response_status(tlm::TLM_OK_RESPONSE);

    }

    // Find back connect info
    connection = pending_map[trans];

    // Send BEGIN_RESP to master
    phase = tlm::BEGIN_RESP;
    delay = SC_ZERO_TIME;

    v::debug << name() << "Transaction 0x" << hex << trans << " call to nb_transport_bw with phase " << phase << v::endl; 

    // Call nb_transport_bw of master
    status = ahbIN[connection.master_id]->nb_transport_bw(*trans, phase, delay);

    assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_UPDATED)||(status==tlm::TLM_COMPLETED));

    if (status == tlm::TLM_ACCEPTED) {

      wait(mEndResponseEvent);

    }

    // If not config, send END_RESP
    if(!(mfpnpen && ((((addr ^ ((mioaddr << 20) | (mcfgaddr << 8))) & ((miomask << 20) | (mcfgmask << 8))))==0))) {

      // Send END_RESP to slave
      phase = tlm::END_RESP;
      delay = SC_ZERO_TIME;

      // Call nb_transport_fw of slave
      status = ahbOUT[connection.slave_id]->nb_transport_fw(*trans, phase, delay);

      assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_COMPLETED));

      wait(delay);
  
    }

    // End power event
    //PM::send(this,"ahb_trans", 0, sc_time_stamp(),(unsigned int)trans->get_data_ptr(),m_pow_mon);

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
  v::info << name() << "* AHB DECODER INITIALIZATION " << v::endl;
  v::info << name() << "* -------------------------- " << v::endl;

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

	  v::info << name() << "* BAR" << dec << j << " with MSB addr: 0x" << hex << addr << " and mask: 0x" << hex << mask <<  v::endl; 

	  // Insert slave region into memory map
	  setAddressMap(i+j, sbusid, addr, mask);

	} else {

	  v::info << name() << "* BAR" << dec << j << " not used." << v::endl;

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

	  v::info << name() << "* BAR" << dec << j << " with MSB addr: 0x" << hex << addr << " and mask: 0x" << hex << mask <<  v::endl; 

	} else {

	  v::info << name() << "* BAR" << dec << j << " not used." << v::endl;

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
    checkMemMap();
  }
}

// Print execution statistic at end of simulation
void AHBCtrl::end_of_simulation() {

  double busy_cycles;

    v::report << name() << " ********************************************" << v::endl;
    v::report << name() << " * AHBCtrl Statistic:" << v::endl;
    v::report << name() << " * ------------------" << v::endl;
    v::report << name() << " * Successful Transactions: " << m_right_transactions << v::endl;
    v::report << name() << " * Total Transactions:      " << m_total_transactions << v::endl;
    v::report << name() << " * " << v::endl;

    if (m_ambaLayer == amba::amba_AT) {

      busy_cycles = sc_time_stamp() / clock_cycle;

      v::report << name() << " * Simulation cycles: " << busy_cycles << v::endl;
      v::report << name() << " * Idle cycles: " << m_idle_count << v::endl;
      v::report << name() << " * Bus utilization: " << (busy_cycles - m_idle_count)/busy_cycles << v::endl;
      v::report << name() << " * Maximum arbiter waiting time: " << m_max_wait << " (" << m_max_wait/clock_cycle << " cycles)" << v::endl;
      v::report << name() << " * Master with maximum waiting time: " << m_max_wait_master << v::endl;
      v::report << name() << " * Average arbitration time / transaction: " << m_total_wait / m_arbitrated << " (" << (m_total_wait / m_arbitrated) / clock_cycle << " cycles)" << v::endl;
      v::report << name() << " * " << v::endl;
    
    }

      v::report << name() << " * AHB Master interface reports: " << v::endl;
      print_transport_statistics(name());

    v::report << name() << " ******************************************** " << v::endl;
}


struct ahb_check_slave_type {
    uint32_t start;
    uint32_t end;
    uint32_t index;
};
// Check the memory map for overlaps
void AHBCtrl::checkMemMap() {
   std::map<uint32_t, ahb_check_slave_type> slaves;
   typedef std::map<uint32_t, ahb_check_slave_type>::iterator iter_t;
   struct ahb_check_slave_type last;
   last.start = 0;
   last.end = 0;
   last.index = ~0;

   for(slave_iter iter = slave_map.begin(); iter!=slave_map.end(); iter++) {
       uint32_t start_addr = (iter->second.haddr & iter->second.hmask) << 20;
       uint32_t size = (((~iter->second.hmask) & 0xFFF) + 1) << 20;
       struct ahb_check_slave_type obj;
       obj.start = start_addr;
       obj.end = start_addr + size -1;
       obj.index = iter->first >> 2;
       slaves.insert(make_pair(start_addr, obj));
   }
   for(iter_t iter=slaves.begin(); iter != slaves.end(); iter++) {
      // First Slave need it in last to start 
      if(last.index!=~0u) {
          // All other elements
          // See if the last element is begining and end befor the current
          if(last.start>=iter->second.start || last.end >= iter->second.start) {
              uint32_t a = 0;
              socket_t *other_socket = ahbOUT.get_other_side(last.index, a);
              sc_core::sc_object *obj = other_socket->get_parent();

              other_socket = ahbOUT.get_other_side(iter->second.index, a);
              sc_core::sc_object *obj2 = other_socket->get_parent();

              v::error << name() << "Overlap in AHB memory mapping." << v::endl;
              v::error << name() << obj->name() << ": " << v::uint32 << last.start << " - " << v::uint32 << last.end << endl;
              v::error << name() << obj2->name() << ": " << v::uint32 << iter->second.start << " - " << v::uint32 << iter->second.end << endl;
          }
      }
      last = iter->second;
  }
}

// TLM debug interface
unsigned int AHBCtrl::transport_dbg(uint32_t id, tlm::tlm_generic_payload &trans) {

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
    if(mfpnpen && ((((addr ^ ((mioaddr << 20) | (mcfgaddr << 8))) & ((miomask << 20) | (mcfgmask << 8))))==0)) {

        // Configuration area is read only
        if (trans.get_command() == tlm::TLM_READ_COMMAND) {

            // Extract data pointer from payload
            unsigned int *data  = (unsigned int *)trans.get_data_ptr();
            
            // No subword access supported here!
            //assert(length%4==0);

            // Get registers from config area
            for(uint32_t i = 0 ; i < (length >> 2); i++) {
                data[i] = getPNPReg(addr + (i<<2));
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

// Collect common transport statistics
void AHBCtrl::transport_statistics(tlm::tlm_generic_payload &gp) {
  if(gp.is_write()) {
    m_writes += gp.get_data_length();
  } else if(gp.is_read()){
    m_reads += gp.get_data_length();
  }
}

// Displays common transport statistics
void AHBCtrl::print_transport_statistics(const char *name) const {
  v::report << name << " * Bytes read: " << m_reads << v::endl;
  v::report << name << " * Bytes written: " << m_writes << v::endl;
}
