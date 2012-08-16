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
#include <string>

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
      arbiter_eval_delay(1,SC_PS),
      busy(false),
      m_pow_mon(pow_mon),
      robin(0),
      address_bus_owner(-1),
      m_AcceptPEQ("AcceptPEQ"),
      m_RequestPEQ("RequestPEQ"),
      m_ResponsePEQ("ResponsePEQ"),
      m_EndResponsePEQ("EndResponsePEQ"),
      m_performance_counters("performance_counters"),
      m_total_wait("total_wait", SC_ZERO_TIME, m_performance_counters),
      m_arbitrated("arbitrated", 0ull, m_performance_counters),
      m_max_wait("maximum_waiting_time", SC_ZERO_TIME, m_performance_counters),
      m_max_wait_master("maximum_wating_master_id", defmast, m_performance_counters),
      m_idle_count("idle_cycles", 0ull, m_performance_counters),
      m_total_transactions("total_transactions", 0ull, m_performance_counters), 
      m_right_transactions("successful_transactions", 0ull, m_performance_counters),
      m_writes("bytes_written", 0ull, m_performance_counters),
      m_reads("bytes_read", 0ull, m_performance_counters),
      is_lock(false),
      lock_master(0),
      m_ambaLayer(ambaLayer),
      sta_power_norm("power.ahbctrl.sta_power_norm", 10714285.71, true), // Normalized static power input
      int_power_norm("power.ahbctrl.int_power_norm", 0.0, true), // Normalized dyn power input (activation indep.)
      dyn_read_energy_norm("power.ahbctrl.dyn_read_energy_norm", 9.10714e-10, true), // Normalized read energy input
      dyn_write_energy_norm("power.ahbctrl.dyn_write_energy_norm", 9.10714e-10, true), // Normalized write energy input
      power("power"),
      sta_power("sta_power", 0.0, power), // Static power output
      int_power("int_power", 0.0, power), // Internal power of module (dyn. switching independent)
      swi_power("swi_power", 0.0, power), // Switching power of module
      power_frame_starting_time("power_frame_starting_time", SC_ZERO_TIME, power),
      dyn_read_energy("dyn_read_energy", 0.0, power), // Energy per read access
      dyn_write_energy("dyn_write_energy", 0.0, power), // Energy per write access
      dyn_reads("dyn_reads", 0ull, power), // Read access counter for power computation
      dyn_writes("dyn_writes", 0ull, power) // Write access counter for power computation

{

  // GreenControl API
  m_api = gs::cnf::GCnf_Api::getApiInstance(this);

  // Initialize slave and master table
  // (Pointers to deviceinfo fields will be set in start_of_simulation)
  for(int i = 0; i < 64; i++) {

    mSlaves[i] = NULL;
    mMasters[i] = NULL;

  }

  if(ambaLayer==amba::amba_LT) {

    // Register tlm blocking transport function
    ahbIN.register_b_transport(this, &AHBCtrl::b_transport);

  }

  // Register non blocking transport functions
  if(ambaLayer==amba::amba_AT) {

    memset(request_map, 0, 16*sizeof(connection_t));
    memset(response_map, 0, 16*sizeof(connection_t));

    // Register tlm non blocking transport forward path
    ahbIN.register_nb_transport_fw(this, &AHBCtrl::nb_transport_fw, 0);

    // Register tlm non blocking transport backward path
    ahbOUT.register_nb_transport_bw(this, &AHBCtrl::nb_transport_bw, 0);

    // Register arbiter thread
    SC_THREAD(arbitrate);

    SC_THREAD(AcceptThread);

    // Register request thread
    SC_THREAD(RequestThread);

    // Register response thread
    SC_THREAD(ResponseThread);

    SC_THREAD(EndResponseThread);

  }

  // Register debug transport
  ahbIN.register_transport_dbg(this, &AHBCtrl::transport_dbg);

  // Register power callback functions
  if (m_pow_mon) {

    GC_REGISTER_TYPED_PARAM_CALLBACK(&sta_power, gs::cnf::pre_read, AHBCtrl, sta_power_cb);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&int_power, gs::cnf::pre_read, AHBCtrl, int_power_cb);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&swi_power, gs::cnf::pre_read, AHBCtrl, swi_power_cb);

  }

  requests_pending = 0;

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

// Reset handler
void AHBCtrl::dorst() {

  // nothing to do

}

// Destructor
AHBCtrl::~AHBCtrl() {

  GC_UNREGISTER_CALLBACKS();

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
  uint32_t result;

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

    v::debug << name() << "Access mSlaves - device: " << device << " offset: " << offset << v::endl;

    if(device>=num_of_slave_bindings) {
        v::debug << name() << "Access to unregistered PNP Slave Register!" << v::endl;
        return 0;
    }

    // If the device exists, access deviceinfo (otherwise 0)
    if (mSlaves[device] != NULL) {
      result =  mSlaves[device][offset];
    } else {
      result = 0;
    }
 
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
        v::debug << name() << "Access to unregistered PNP Master Register!" << v::endl;
        return 0;
    }

    if (mMasters[device] != NULL) {
      
      result = mMasters[device][offset];

    } else {

      result = 0;

    }

    #ifdef LITTLE_ENDIAN_BO
    swap_Endianess(result);
    #endif
    return result;

  }

}

// TLM blocking transport function (multi-sock)
void AHBCtrl::b_transport(uint32_t id, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {

  static uint32_t old_lock = 0;
  // master-address pair for dcache snooping
  t_snoop snoopy;

  uint32_t a = 0;
  socket_t* other_socket;
  sc_core::sc_object *slvobj = NULL;
  sc_core::sc_object *mstobj = NULL;

  if (v::debug) {
    other_socket = ahbIN.get_other_side(id, a);
    mstobj = other_socket->get_parent();
  }

  // Bus occupied or locked by other master
  while(busy || (is_lock && (id != lock_master))) {

    wait(clock_cycle);

  }

  busy = true;
  old_lock = is_lock;
  is_lock = ahbIN.get_extension<amba::amba_lock>(lock, trans);
  
  lock_master = id;

  // Collect transport statistics
  transport_statistics(trans);

  // Extract address from payload
  uint32_t addr   = trans.get_address();
  // Extract length from payload
  uint32_t length = trans.get_data_length();

  // Is this an access to configuration area
  if (mfpnpen && ((((addr ^ ((mioaddr << 20) | (mcfgaddr << 8))) & ((miomask << 20) | (mcfgmask << 8))))==0)) {

    // Configuration area is read only
    if (trans.get_command() == tlm::TLM_READ_COMMAND) {
      //addr = addr - (((mioaddr << 20) | (mcfgaddr << 8)) & ((miomask << 20) | (mcfgmask << 8)));
      // Extract data pointer from payload
      uint8_t *data  = trans.get_data_ptr();

      // Get registers from config area
      for(uint32_t i = 0; i < length; i++) {
          //uint32_t word = (addr + i) >> 2;
          uint32_t byte = (addr + i) & 0x3;
          uint32_t reg = getPNPReg(addr + i);
          data[i] = ((uint8_t*)&reg)[byte];

          // one cycle delay per 32bit register
          delay += clock_cycle;
      }
      
      // and return
      trans.set_response_status(tlm::TLM_OK_RESPONSE);

      wait(delay);
      delay=SC_ZERO_TIME;

      msclogger::return_backward(this, &ahbIN, &trans, tlm::TLM_COMPLETED, delay);

      busy = false;
      return;

    } else {

      v::error << name() << " Forbidden write to AHBCTRL configuration area (PNP)!" << v::endl;
      trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
      
      delay=SC_ZERO_TIME;

      msclogger::return_backward(this, &ahbIN, &trans, tlm::TLM_COMPLETED, delay);

      busy = false;
      return;
    }
  }

  // Find slave by address / returns slave index or -1 for not mapped
  int index = get_index(trans.get_address());

  // For valid slave index
  if(index >= 0) {

    if (v::debug) {

      other_socket = ahbOUT.get_other_side(index, a);
      slvobj = other_socket->get_parent();

      v::debug  << name() << "AHB Request for address: 0x" << hex << v::setfill('0')
		<< v::setw(8) << trans.get_address() << ", from master: "
		<< mstobj->name() << ", forwarded to slave: " << slvobj->name() << endl;

    }

    // Broadcast master_id and address for dcache snooping
    if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {

      snoopy.master_id  = id;
      snoopy.address = addr;
      snoopy.length = length;

      // Send to signal socket
      snoop.write(snoopy);

    }

    // Power event start
    //const char *event_name = "ahb_trans";
    //size_t data_int = (size_t)trans.get_data_ptr();
    //uint32_t id = data_int & 0xFFFFFFFF;
    //PM::send(this,event_name,1,sc_time_stamp(),id,m_pow_mon);

    // Forward request to the selected slave
    ahbOUT[index]->b_transport(trans, delay);

    //v::debug << name() << "Delay after return from slave: " << delay << v::endl;

    // Power event end
    //PM::send(this,event_name,0,sc_time_stamp()+delay,id,m_pow_mon);
    
    wait(delay);
    delay=SC_ZERO_TIME;

    busy = false;
    return;

  } else {
    
    other_socket = ahbIN.get_other_side(id, a);
    mstobj = other_socket->get_parent();
    
    v::error << name() << "AHB Request 0x" << hex << v::setfill('0')
               << v::setw(8) << trans.get_address() << ", from master:"
               << mstobj->name() << ": Unmapped address space." << endl;

    // Invalid index
    trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);

    wait(delay);
    delay=SC_ZERO_TIME;

    busy = false;
    return;
  }
}

// Non-blocking forward transport function for ahb_slave multi-socket
// A master may send BEGIN_REQ or END_RESP. The model replies with
// TLM_ACCEPTED or TLM_COMPLETED, respectively.
tlm::tlm_sync_enum AHBCtrl::nb_transport_fw(uint32_t master_id, tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time &delay) {

  v::debug << name() << "nb_transport_fw received transaction 0x" << hex << &trans << " with phase " << phase << " and delay " << delay << v::endl;

  if (phase == tlm::BEGIN_REQ) {

    // Increment reference counter
    trans.acquire();

    // Validate master_id extension
    amba::amba_id * m_id;
    ahbIN.validate_extension<amba::amba_id>(trans);
    ahbIN.get_extension<amba::amba_id>(m_id, trans);
    m_id->value = master_id;    

    v::debug << name() << "Acquire " << hex << &trans << " Master ID = " << master_id << " Ref-Count = " << trans.get_ref_count() << v::endl;

    // In communication with the ahbctrl, BEGIN_REQ marks the begin of the bus request.
    // Transaction is send to request thread, where it is going to be decoded and put in PENDING state. 
    m_AcceptPEQ.notify(trans, delay);

    // Collect transport statistics
    transport_statistics(trans);

    // Reset delay
    delay = SC_ZERO_TIME;

    // Draw msc retrun arrow
    msclogger::return_backward(this, &ahbIN, &trans, tlm::TLM_ACCEPTED, delay, master_id);

     // Transaction accepted
    return tlm::TLM_ACCEPTED;    

  } else if (phase == tlm::END_RESP) {

    // Let the response thread know that END_RESP
    // came in on the forward path.
    m_EndResponsePEQ.notify(trans, delay);

    msclogger::return_backward(this, &ahbIN, &trans, tlm::TLM_COMPLETED, delay, master_id);

    delay = SC_ZERO_TIME;

    // Transaction completed
    return tlm::TLM_COMPLETED;

  } else {

    v::error << name() << "Illegal phase in call to nb_transport_fw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

  }

  msclogger::return_backward(this, &ahbIN, &trans, tlm::TLM_ACCEPTED, delay, master_id);

  return tlm::TLM_COMPLETED;
}

// Non-blocking backward transport function for ahb_master multi-socket
// A slave may send END_REQ or BEGIN_RESP. In both cases the model replies
// with TLM_ACCEPTED.
tlm::tlm_sync_enum AHBCtrl::nb_transport_bw(uint32_t id, tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time &delay) {

  v::debug << name() << "nb_transport_bw received transaction 0x" << hex << &trans << " with phase: " << phase << " and delay: " << delay << v::endl;

  // The slave has sent END_REQ
  if (phase == tlm::END_REQ) {

    m_RequestPEQ.notify(trans, delay);
    delay = SC_ZERO_TIME;

  } else if (phase == amba::DATA_SPLIT) {

    // Ignore DATA_SPLIT!
    // Current version of AHBCTRL does not rearbitrate on DATA_SPLIT!
    // Slave expected to continue with BEGIN_RESP.

  // New response - goes into response PEQ
  } else if (phase == tlm::BEGIN_RESP) {

    m_ResponsePEQ.notify(trans, delay);
    delay = SC_ZERO_TIME;

  } else {

    v::error << name() << "Invalid phase in call to nb_transport_bw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

  }

  msclogger::return_forward(this, &ahbOUT, &trans, tlm::TLM_ACCEPTED, delay, id);

  return tlm::TLM_ACCEPTED;
}

// Helper function for printing requests
void AHBCtrl::print_requests() {

  v::info << name() << " ---------------------------------------------------- " << v::endl;
  v::info << name() << "NEW ARBITER CYCLE:" << v::endl;

  for(int i=0;i<16;i++) {

    v::info << name() << "Master " << i << " Transaction: " <<  request_map[i].trans << " State: " << request_map[i].state << v::endl;
    
  }

  v::info << name() << " ---------------------------------------------------- " << v::endl;

}

// Arbitration thread (AT only)
void AHBCtrl::arbitrate() {

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;
  sc_core::sc_time delay;

  payload_t * trans;
  int slave_id = 0;

  sc_core::sc_time tmp;
  sc_core::sc_time waiting_time;
  sc_core::sc_time request_delay;
  sc_core::sc_time response_delay;

  wait(1, SC_PS);

  // Arbiter phase shift:
  // --------------------
  while(true) {

    wait(clock_cycle);

    //print_requests();

    // Last address of current transfer must have been sampled.
    // Last data sample is on the way.
    if ((address_bus_owner == -1) && ((data_bus_state == RESPONSE) || (data_bus_state == IDLE))) {

      // Priority arbitration
      if (mrrobin == 0) {

        if (!is_lock) {

          // Master with the highest ID has the highest priority
          for(int i=15; i>=0; i--) {

            if (request_map[i].state == TRANS_PENDING) {

              address_bus_owner = i;
              request_map[i].state = TRANS_SCHEDULED;
              trans = request_map[i].trans;
              slave_id = request_map[i].slave_id;

              is_lock = ahbIN.get_extension<amba::amba_lock>(lock, *trans);
              lock_master = i;

              break;

            }
          }

        } else {

          if (request_map[lock_master].state == TRANS_PENDING) {
            
            address_bus_owner = lock_master;
            request_map[lock_master].state = TRANS_SCHEDULED;
            trans = request_map[lock_master].trans;
            slave_id = request_map[lock_master].slave_id;

            is_lock = ahbIN.get_extension<amba::amba_lock>(lock, *trans);

            break;
          }
        }
        
      // Round-robin
      } else {

        if (!is_lock) {

          for(uint32_t i=0; i<num_of_master_bindings; i++) {
            
            robin=(++robin) % num_of_master_bindings;
          
            v::debug << name() << "Robin: " << robin << v::endl;

            if (request_map[robin].state == TRANS_PENDING) {

              v::debug << name() << "Select for robin: " << robin << v::endl;

              address_bus_owner = robin;
              request_map[robin].state = TRANS_SCHEDULED;
              trans = request_map[robin].trans;
              slave_id = request_map[robin].slave_id;

              is_lock = ahbIN.get_extension<amba::amba_lock>(lock, *trans);
              lock_master = i;

              break;
            }
          }
        } else {
          
          if (request_map[lock_master].state == TRANS_PENDING) {

            v::debug << name() << "Select for robin: " << robin << v::endl;
            
            address_bus_owner = lock_master;
            request_map[lock_master].state = TRANS_SCHEDULED;
            trans = request_map[lock_master].trans;
            slave_id = request_map[lock_master].slave_id;

            is_lock = ahbOUT.get_extension<amba::amba_lock>(lock, *trans);
              
          }
        }
      }
    
      if (address_bus_owner != -1) {

        waiting_time = sc_time_stamp() - request_map[address_bus_owner].start_time;

        // Statistic
        if (waiting_time > m_max_wait) {

          m_max_wait = waiting_time;
          m_max_wait_master = address_bus_owner;

        }

        tmp = m_total_wait; 
        tmp += waiting_time;

        m_total_wait = tmp;
        m_arbitrated++;

        // Is this an access to the configuration area
        if (slave_id == 16) {

          uint32_t addr = trans->get_address();
          uint8_t *data  = trans->get_data_ptr();

          // Get registers from config area
          for(uint32_t i = 0; i < trans->get_data_length(); i++) {

            uint32_t byte = (addr + i) & 0x3;
            uint32_t reg = getPNPReg(addr + i);
            data[i] = ((uint8_t*)&reg)[byte];
          
          }

          // Calculate delay for sending END_REQ
          request_delay = (trans->get_data_length() > 3) ? (trans->get_data_length() >> 2)*clock_cycle - sc_core::sc_time(1, SC_PS) : clock_cycle - sc_core::sc_time(1, SC_PS);
          m_RequestPEQ.notify(*trans, request_delay);

          // Calculate delay for sending BEGIN_RESP (no wait states for PNP)
          response_delay = clock_cycle - sc_core::sc_time(1, SC_PS);
          m_ResponsePEQ.notify(*trans, response_delay);
          
        } else {

          // Forward request to slave
          phase = tlm::BEGIN_REQ;
          delay = SC_ZERO_TIME;

          v::debug << name() << "Transaction 0x" << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;

          // Forward arrow for msc
          msclogger::forward(this, &ahbOUT, trans, phase, delay, slave_id);

          status = ahbOUT[slave_id]->nb_transport_fw(*trans, phase, delay);
          assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_UPDATED));

          if (phase == tlm::END_REQ) {
            
            m_RequestPEQ.notify(*trans, delay);
            delay = SC_ZERO_TIME;

          }
        }
      }
    } else {

      m_idle_count++;

    }
  }
}

// Queue incoming master transactions
void AHBCtrl::AcceptThread() {

  int slave_id = 16;

  payload_t *trans;
  connection_t connection;

  while(true) {

    wait(m_AcceptPEQ.get_event());

    // Get new transaction from AcceptPEQ (nb_transport_fw)
    while((trans = m_AcceptPEQ.get_next_transaction())) {

      // Extract master id from payload
      amba::amba_id * master_id;
      ahbIN.get_extension<amba::amba_id>(master_id, *trans);

      // Is PNP access
      if (mfpnpen && ((((trans->get_address() ^ ((mioaddr << 20) | (mcfgaddr << 8))) & ((miomask << 20) | (mcfgmask << 8))))==0)) {

        if (trans->get_command() == tlm::TLM_WRITE_COMMAND) {

          v::warn << name() << "PNP area is read-only. Write operation ignored" << v::endl;

        }

        // Reserved PNP id
        slave_id = 16;

      } else {

        // Find the target (slave)
        slave_id = get_index(trans->get_address());

      }

      v::debug << name() << "Decoding (" << hex << trans << ")" << " - Master: " << master_id->value << " Slave : " << dec << slave_id << " Address: " << hex << trans->get_address() << v::endl;

      if (slave_id >= 0) {

        // Initialize connection record
        connection.master_id  = master_id->value;
        connection.slave_id   = slave_id;
        connection.start_time = sc_time_stamp();
        connection.state      = TRANS_PENDING;
        connection.trans      = trans;

        request_map[master_id->value] = connection;
        response_map[master_id->value] = connection;

      } else {

        v::error << name() << "DECODING ERROR" << v::endl;
          
      }
    }
  }
}

// Send END_REQ to master
void AHBCtrl::RequestThread() {

  payload_t *trans;
  connection_t connection;

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;
  sc_core::sc_time delay;

  while(true) {

    wait(m_RequestPEQ.get_event());

    // Get new transaction from RequestPEQ (nb_transport_bw or arbitrate)
    while((trans = m_RequestPEQ.get_next_transaction())) {

      amba::amba_id * master_id; 
      ahbIN.get_extension<amba::amba_id>(master_id, *trans);

      connection = request_map[master_id->value];

      // We don't need the address bus anymore
      address_bus_owner = -1;

      if (data_bus_state != RESPONSE) {

        data_bus_state = WAITSTATES;

      }

      // Send END_REQ to the master
      phase = tlm::END_REQ;
      delay = SC_ZERO_TIME;

      v::debug << name() << "Transaction 0x" << hex << trans << " call to nb_transport_bw with phase " << phase << v::endl;

      // Backward arrow for msc
      msclogger::backward(this, &ahbIN, trans, phase, delay, connection.master_id);
    
      status = ahbIN[connection.master_id]->nb_transport_bw(*trans, phase, delay);

      assert(status==tlm::TLM_ACCEPTED);

    }
  }
}


void AHBCtrl::ResponseThread() {

  payload_t *trans;
  connection_t connection;

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;
  sc_core::sc_time delay;

  while(true) {

    wait(m_ResponsePEQ.get_event());

    // Get new transaction from ResponsePEQ (nb_transport_bw)
    while((trans = m_ResponsePEQ.get_next_transaction())) {

      amba::amba_id * master_id;
      ahbIN.get_extension<amba::amba_id>(master_id, *trans);

      connection = response_map[master_id->value];

      // Data bus is response mode (data is being transferred)
      data_bus_state = RESPONSE;

      // Send BEGIN_RESP to master
      phase = tlm::BEGIN_RESP;
      delay = SC_ZERO_TIME;

      v::debug << name() << "Transaction 0x" << hex << trans << " call to nb_transport_bw with phase " << phase << v::endl;

      // Backward arrow for msc
      msclogger::backward(this, &ahbIN, trans, phase, delay, connection.master_id);

      status = ahbIN[connection.master_id]->nb_transport_bw(*trans, phase, delay);

      assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_UPDATED));

      if (phase == tlm::END_RESP) {

        m_EndResponsePEQ.notify(*trans, delay);
        delay = SC_ZERO_TIME;

      }
    }
  }
}

// Send END_RESP to slave
void AHBCtrl::EndResponseThread() {

  payload_t *trans;
  connection_t connection;

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;
  sc_core::sc_time delay;

  while(true) {

    wait(m_EndResponsePEQ.get_event());

    // Get new transaction from EndResponsePEQ (nb_transport_fw or ResponseThread)
    while((trans = m_EndResponsePEQ.get_next_transaction())) {

      assert(trans != NULL);

      amba::amba_id *master_id; 
      ahbIN.get_extension<amba::amba_id>(master_id, *trans);

      connection = response_map[master_id->value];

      // Is PNP access
      if (connection.slave_id == 16) {

        // Data bus is now idle
        data_bus_state = IDLE;

        v::debug << name() << "Release " << trans << " Ref-Count before calling release " << trans->get_ref_count() << v::endl;

        // Decrement reference counter
        trans->release();

      } else {

        // Data bus is now idle
        data_bus_state = IDLE;

        // Send END_RESP to slave
        phase = tlm::END_RESP;
        delay = SC_ZERO_TIME;

        v::debug << name() << "Transaction 0x" << hex << trans << " call to nb_transport_fw with phase " << phase << v::endl;

        // Forward arrow for msc
        msclogger::forward(this, &ahbOUT, trans, phase, delay, connection.slave_id);

        status = ahbOUT[connection.slave_id]->nb_transport_fw(*trans, phase, delay);

        assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_COMPLETED));

        v::debug << name() << "Release " << trans << " Ref-Count before calling release " << trans->get_ref_count() << v::endl;

        // Decrement reference counter
        trans->release();

      }
    }
  }
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

  // Initialize power model
  if (m_pow_mon) {

    power_model();

  }
}

// Calculate power/energy values from normalized input data
void AHBCtrl::power_model() {

  // Static power calculation (pW)
  sta_power = sta_power_norm * (num_of_slave_bindings + num_of_master_bindings);

  // Dynamic power (switching independent internal power)
  int_power = int_power_norm * (num_of_slave_bindings + num_of_master_bindings) * 1/(clock_cycle.to_seconds());

  // Energy per read access (uJ)
  dyn_read_energy = dyn_read_energy_norm * (num_of_slave_bindings + num_of_master_bindings);

  // Energy per write access (uJ)
  dyn_write_energy = dyn_write_energy_norm * (num_of_slave_bindings + num_of_master_bindings);  
  
}

// Static power callback
void AHBCtrl::sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Static power of AHBCTRL is constant !!

}

// Internal power callback
void AHBCtrl::int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // RTL AHBCTRL has no internal power - constant.

}

// Switching power callback
void AHBCtrl::swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  swi_power = ((dyn_read_energy * dyn_reads) + (dyn_write_energy * dyn_writes)) / (sc_time_stamp() - power_frame_starting_time).to_seconds();

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
      sc_time max_wait = m_max_wait;
      sc_time total_wait = m_total_wait;

      v::report << name() << " * Simulation cycles: " << busy_cycles << v::endl;
      v::report << name() << " * Idle cycles: " << m_idle_count << v::endl;
      v::report << name() << " * Bus utilization: " << (busy_cycles - m_idle_count)/busy_cycles << v::endl;
      v::report << name() << " * Maximum arbiter waiting time: " << m_max_wait << " (" << max_wait/clock_cycle << " cycles)" << v::endl;
      v::report << name() << " * Master with maximum waiting time: " << m_max_wait_master << v::endl;
      v::report << name() << " * Average arbitration time / transaction: " << total_wait / m_arbitrated << " (" << (total_wait / m_arbitrated) / clock_cycle << " cycles)" << v::endl;
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
       slaves.insert(std::make_pair(start_addr, obj));
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
    uint32_t addr   = trans.get_address();
    // Extract length from payload
    uint32_t length = trans.get_data_length(); 
    uint8_t *data  = trans.get_data_ptr();

    if (mfpnpen && ((((addr ^ ((mioaddr << 20) | (mcfgaddr << 8))) & ((miomask << 20) | (mcfgmask << 8))))==0)) {

        // Configuration area is read only
        if(trans.get_command() == tlm::TLM_READ_COMMAND) {
            //addr = addr - (((mioaddr << 20) | (mcfgaddr << 8)) & ((miomask << 20) | (mcfgmask << 8)));
            // Extract data pointer from payload

            // Get registers from config area
            for(uint32_t i = 0; i < length; i++) {
                //uint32_t word = (addr + i) >> 2;
                uint32_t byte = (addr + i) & 0x3;
                uint32_t reg = getPNPReg(addr + i);
                data[i] = ((uint8_t*)&reg)[byte];
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
    
    // Total number of bytes written
    m_writes += gp.get_data_length();

    // Number of write transfers (clock cycles with bus busy)
    if(m_pow_mon) dyn_writes += (gp.get_data_length() >> 2) + 1;

  } else if(gp.is_read()){

    // Total number of bytes read
    m_reads += gp.get_data_length();

    // Number of read transfers (clock cycles with bus busy)
    if(m_pow_mon) dyn_reads += (gp.get_data_length() >> 2) + 1;
  }
}

// Displays common transport statistics
void AHBCtrl::print_transport_statistics(const char *name) const {
  v::report << name << " * Bytes read: " << m_reads << v::endl;
  v::report << name << " * Bytes written: " << m_writes << v::endl;
}
