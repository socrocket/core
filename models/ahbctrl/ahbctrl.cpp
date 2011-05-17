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
// Maintainer: 
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
      clockcycle(10.0, sc_core::SC_NS) {

  if(ambaLayer==amba::amba_LT) {

    // register tlm blocking transport function
    ahbIN.register_b_transport(this, &AHBCtrl::b_transport);

  }

  // Register non blocking transport functions
  if(ambaLayer==amba::amba_AT) {

    // register tlm non blocking transport forward path
    ahbIN.register_nb_transport_fw(this, &AHBCtrl::nb_transport_fw, 0);

    // register tlm non blocking transport backward path
    ahbOUT.register_nb_transport_bw(this, &AHBCtrl::nb_transport_bw, 0);
  }

  // register debug transport
  ahbIN.register_transport_dbg(this, &AHBCtrl::transport_dbg);

}

// Destructor
AHBCtrl::~AHBCtrl() {

}

// Helper function for creating slave map decoder entries
void AHBCtrl::setAddressMap(const uint32_t i, const uint32_t addr, const uint32_t mask) {

  // Create slave map entry from slave ID and address range descriptor (slave_info_t)
  // Why std::map: Contains only the bar entries, which are actually valid.
  // A static array would have holes -> far slower, especially if number of slaves is small.
  slave_map.insert(std::pair<uint32_t, slave_info_t>(i, slave_info_t(addr, mask)));
}

// Find slave index by address
int AHBCtrl::get_index(const uint32_t address) {

  // Use 12 bit segment address for decoding
  uint32_t addr = address >> 20;

  for (it = slave_map.begin(); it != slave_map.end(); it++) {

      slave_info_t info = it->second;
  
      if (((addr ^ info.first) & info.second) == 0) {

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

int AHBCtrl::getMaster2Slave(const uint32_t slaveID) {
   std::map<uint32_t, int32_t>::iterator it;

   it = MstSlvMap.find(slaveID);
   return it->second;
}

// TLM blocking transport function (multi-sock)
void AHBCtrl::b_transport(uint32_t id, tlm::tlm_generic_payload& ahb_gp, sc_core::sc_time& delay) {

  // -- For Debug only --
  uint32_t a = 0;
  socket_t* other_socket = ahbIN.get_other_side(id, a);
  sc_core::sc_object *mstobj = other_socket->get_parent();
  // --------------------

  // Extract data pointer from payload
  unsigned int *data  = (unsigned int *)ahb_gp.get_data_ptr();
  // Extract address from payload
  unsigned int addr   = ahb_gp.get_address();
  // Extract length from payload
  unsigned int length = ahb_gp.get_data_length();

  // Is this an access to configuration area
  if (mfpnpen && ((((addr >> 20) ^ mcfgaddr) & mcfgmask)==0)) {

    // Configuration area is read only
    if (ahb_gp.get_command() == tlm::TLM_READ_COMMAND) {

      // No subword access supported here!
      assert(length%4==0);

      // Get registers from config area
      for (uint32_t i = 0; i < (length >> 2); i++) {

	data[i] = getPNPReg(addr);

	// one cycle delay per 32bit register
	delay += clockcycle;

      }
      
      ahb_gp.set_response_status(tlm::TLM_OK_RESPONSE);
      return;

    } else {

      v::error << name() << " Forbidden write to AHBCTRL configuration area (PNP)!" << v::endl;
      ahb_gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
      return;
    }
  }

  // Find slave by address / returns slave index or -1 for not mapped
  int index = get_index(ahb_gp.get_address());

  // For valid slave index
  if(index >= 0) {

    // -- For Debug only --
    other_socket = ahbOUT.get_other_side(index, a);
    sc_core::sc_object *obj = other_socket->get_parent();

    v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
             << v::setw(8) << ahb_gp.get_address() << ", from master:"
             << mstobj->name() << ", forwarded to slave:" << obj->name() << endl;

    // -------------------

    // Add delay for AHB address phase
    delay += clockcycle;

    // Wait for semaphore
    //SlvSemaphore.find(index)->second->wait();
    
    // Forward request to the selected slave
    ahbOUT[index]->b_transport(ahb_gp, delay);

    // Post to semaphore
    //SlvSemaphore.find(index)->second->post();

    return;

  } else {
    
    v::error << name() << "AHB Request 0x" << hex << v::setfill('0')
               << v::setw(8) << ahb_gp.get_address() << ", from master:"
               << mstobj->name() << ": Unmapped address space." << endl;

    // Invalid index
    ahb_gp.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    return;
  }
}

// TLM non blocking transport call forward path (from masters to slaves)
tlm::tlm_sync_enum AHBCtrl::nb_transport_fw(uint32_t id, tlm::tlm_generic_payload& gp,
                                                tlm::tlm_phase& phase, sc_core::sc_time& delay) {

   // Obtain slave index for the requested address
   int index = get_index(gp.get_address());

   uint32_t a = 0;
   socket_t* other_socket = ahbIN.get_other_side(id, a);
   sc_core::sc_object *mstobj = other_socket->get_parent();

   if(index>=0) {
      other_socket = ahbOUT.get_other_side(index,a);
      sc_core::sc_object *slvobj = other_socket->get_parent();

      // -2 indicates that the master is expected to finish a transaction by
      // calling with phase END_RESP, while the slave already finished by
      // returnen COMPLETED on intial call.
      if(getMaster2Slave(index)==-2) {
         MstSlvMap[index] = -1;
         SlvSemaphore.find(index)->second->post();
         return tlm::TLM_COMPLETED;
      }

      if((getMaster2Slave(index)==static_cast<int>(id)) ||
         (SlvSemaphore.find(index)->second->trywait()!=-1)) {
         tlm::tlm_sync_enum returnValue;
         MstSlvMap[index] = id;

         // Forward request
         returnValue = ahbOUT[index]->nb_transport_fw(gp, phase, delay);

         v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
                  << v::setw(8) << gp.get_address() << ", from master:"
                  << mstobj->name() << " forwarded to slave:" << slvobj->name()
                  << ", phase:" << phase << ", return:" << returnValue << endl;

         // Clear transaction mapping if transaction finishes
         if((returnValue==tlm::TLM_COMPLETED) ||
            ((phase==tlm::END_RESP) && (returnValue==tlm::TLM_ACCEPTED))) {
            MstSlvMap[index] = -1;
            SlvSemaphore.find(index)->second->post();
         }

         // return to initiator
         return returnValue;
      } else {
         // Requested slave is busy. Don't execute command and end
         // transaction.
         v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
                  << v::setw(8) << gp.get_address() << ", from master:"
                  << mstobj->name() << " to slave:" << slvobj->name()
                  << ": Slave busy => Request queued." << endl;

         // Requested slave is busy, spawn a new thread waiting to execute the
         // request
         // Create a new phase argument, since it's life time seems to end with termination of nb_tranport
         tlm::tlm_phase* newPhase = new tlm::tlm_phase(phase);
         sc_core::sc_spawn(sc_bind(&AHBCtrl::queuedTrans, this, id, index, sc_ref(gp),
                           sc_ref(*newPhase), sc_ref(delay)));

         // Request is queued. Return state to requesting master.
         phase = tlm::END_REQ;
         return tlm::TLM_UPDATED;
      }
    } else {
       // Invalid index
       gp.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);

       v::warn << name() << "AHB Request@0x" << hex << v::setfill('0')
               << v::setw(8) << gp.get_address() << ", from master:"
               << mstobj->name() << ": Unmapped address space." << endl;

       return tlm::TLM_COMPLETED;
    }
}

// TLM non-blocking transport call backward path (from slaves to masters)
tlm::tlm_sync_enum AHBCtrl::nb_transport_bw(uint32_t id, tlm::tlm_generic_payload& gp,
                                                tlm::tlm_phase& phase, sc_core::sc_time& delay) {
   int index = getMaster2Slave(id);

   uint32_t a = 0;
   socket_t* other_socket = ahbOUT.get_other_side(id, a);
   sc_core::sc_object *slvobj = other_socket->get_parent();

   if(index >= 0) {
      tlm::tlm_sync_enum returnValue;

      other_socket = ahbIN.get_other_side(index,a);
      sc_core::sc_object *mstobj = other_socket->get_parent();

      // Forward request
      returnValue = ahbIN[index]->nb_transport_bw(gp, phase, delay);

      v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
               << v::setw(8) << gp.get_address() << ", from slave:"
               << slvobj->name() << " forwarded to master:" << mstobj->name()
               << ", phase:" << phase << ", return:" << returnValue << endl;

      // Reset MstSlvMap if transaction finishes
      if((returnValue==tlm::TLM_COMPLETED) ||
         ((phase==tlm::END_RESP) && (returnValue==tlm::TLM_UPDATED))) {
         MstSlvMap[id] = -1;
         SlvSemaphore.find(id)->second->post();
      }

      // Return to initiator
      return returnValue;
   } else {
      // Invalid index
      v::error << name() << "Backward path by slave:" << slvobj->name()
               << ": No active connection found." << endl;
      return tlm::TLM_COMPLETED;
   }
}

void AHBCtrl::queuedTrans(const uint32_t mstID, const uint32_t slvID,
                              tlm::tlm_generic_payload& gp,
                              tlm::tlm_phase& phase,
                              sc_core::sc_time& delay) {

   tlm::tlm_sync_enum returnValue;

   // Wait for Semaphore
   SlvSemaphore.find(slvID)->second->wait();
   // Update master slave mapping
   MstSlvMap[slvID] = mstID;
   // Forward request to slave
   returnValue = ahbOUT[slvID]->nb_transport_fw(gp, phase, delay);

   v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
            << v::setw(8) << gp.get_address() << ", from master:"
            << mstID << " forwarded to slave:" << slvID
            << ", phase:" << phase << ", return:" << returnValue << endl;

   if(returnValue==tlm::TLM_COMPLETED) {
      // Adapt payload objects for backward path call
      phase = tlm::BEGIN_RESP;
      // Forward response to master
      returnValue = ahbIN[mstID]->nb_transport_bw(gp, phase, delay);

      v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
               << v::setw(8) << gp.get_address() << ", from slave:"
               << slvID << " forwarded to master:" << mstID
               << ", phase:" << phase << ", return:" << returnValue << endl;

      // Finish transaction according to protocol
      if(returnValue==tlm::TLM_ACCEPTED) {
         // Master will call forward path with end_resp, though slave
         // already finished transaction.
         MstSlvMap[slvID] = -2;
      } else {
         // Release slave from transaction
         MstSlvMap[slvID] = -1;
         SlvSemaphore.find(slvID)->second->post();
      }
   }

   if(((returnValue==tlm::TLM_UPDATED) && (phase == tlm::BEGIN_RESP))) {
      // Adapt payload objects for backward path call
      phase = tlm::BEGIN_RESP;
      // Forward response to master
      returnValue = ahbIN[mstID]->nb_transport_bw(gp, phase, delay);

      v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
               << v::setw(8) << gp.get_address() << ", from slave:"
               << slvID << " forwarded to master:" << mstID
               << ", phase:" << phase << ", return:" << returnValue << endl;

      if(((returnValue==tlm::TLM_UPDATED) && (phase == tlm::END_RESP)) ||
         (returnValue==tlm::TLM_COMPLETED)) {
         // Call forward path to finish transaction according to protocol
         phase = tlm::END_RESP;
         ahbOUT[slvID]->nb_transport_fw(gp, phase, delay);

         v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
                  << v::setw(8) << gp.get_address() << ", from master:"
                  << mstID << " forwarded to slave:" << slvID
                  << ", phase:" << phase << ", return:" << returnValue << endl;
      }
      // Release slave from transaction
      MstSlvMap[slvID] = -1;
      SlvSemaphore.find(slvID)->second->post();
   }
}

// Set up slave map and collect plug & play information
void AHBCtrl::start_of_simulation() {

 
  // Get number of bindings at master socket (number of connected slaves)
  uint32_t num_of_slave_bindings = ahbOUT.size();
  // Get number of bindings at slave socket (number of connected masters)
  uint32_t num_of_master_bindings = ahbIN.size();

  // max. 16 AHB slaves allowed
  assert(num_of_slave_bindings<=16);

  // max. 16 AHB masters allowed
  assert(num_of_master_bindings<=16);

  v::info << name() << "******************************************************************************* " << v::endl;
  v::info << name() << "* DECODER INITIALIZATION " << v::endl;
  v::info << name() << "* ---------------------- " << v::endl;

  // Iterate/detect the registered slaves
  // ------------------------------------
  for (uint32_t i = 0; i < (num_of_slave_bindings<<2); i+=4) {

    uint32_t a = 0;

    // get pointer to socket of slave i
    socket_t *other_socket = ahbOUT.get_other_side(i>>2, a);

    // get parent object containing slave socket i
    sc_core::sc_object *obj = other_socket->get_parent();

    // valid slaves implement the AHBDevice interface
    AHBDevice *slave = dynamic_cast<AHBDevice *> (obj);

    v::info << name() << "* SLAVE name: " << obj->name() << v::endl;

    // slave is valid (implements AHBDevice)
    if(slave) {

      // to be checked: do I need one 
      //sc_core::sc_semaphore* newSema = new sc_core::sc_semaphore(1);

      // Get pointer to device information
      const uint32_t * deviceinfo = slave->get_device_info();

      // Map device information into PNP region
      if (mfpnpen) {
	
	mSlaves[i] = deviceinfo;

      }

      // Each slave may have up to four subdevices (BARs)
      for (uint32_t j = 0; j < 4; j++) {

	// check 'type' field of bar[j] (must be != 0)
	if (slave->get_bar_type(j)) {

	  // get base address and maks from BAR i
	  uint32_t addr = slave->get_bar_base(j);
	  uint32_t mask = slave->get_bar_mask(j);

	  v::info << name() << "* BAR" << j << " with MSB addr: " << hex << addr << " and mask: " << mask <<  v::endl; 

	  // insert slave region into memory map
	  setAddressMap(i+j, addr, mask);

	  // What is this good for ???
	  MstSlvMap.insert(std::pair<uint32_t, int32_t>(i, -1));
	  //SlvSemaphore.insert(std::pair<uint32_t, sc_core::sc_semaphore*>(i, newSema));

	} else {

	  v::info << name() << "* BAR" << j << " not used." << v::endl;

	}
      
      } 

    } else {
      
      v::error << name() << "Slave bound to socket 'ahbout' is not a valid AHBDevice" << v::endl;
      assert(0);

    }
  }

  // Iterate/detect the registered masters
  // ------------------------------------
  for (uint32_t i = 0; i < (num_of_master_bindings<<2); i+=4) {

    uint32_t a = 0;

    // get pointer to socket of slave i
    socket_t *other_socket = ahbIN.get_other_side(i>>2, a);

    // get parent object containing slave socket i
    sc_core::sc_object *obj = other_socket->get_parent();

    // valid slaves implement the AHBDevice interface
    AHBDevice *master = dynamic_cast<AHBDevice *> (obj);

    v::info << name() << "* Master name: " << obj->name() << v::endl;

    // master is valid (implements AHBDevice)
    if(master) {

      // to be checked: do I need one 
      //sc_core::sc_semaphore* newSema = new sc_core::sc_semaphore(1);

      // Get pointer to device information
      const uint32_t * deviceinfo = master->get_device_info();

      // Map device information into PNP region
      if (mfpnpen) {
	
	mMasters[i] = deviceinfo;

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
}

// TLM debug interface
unsigned int AHBCtrl::transport_dbg(uint32_t id, tlm::tlm_generic_payload &ahb_gp) {

    // -- For Debug only --
    uint32_t a = 0;
    socket_t* other_socket = ahbIN.get_other_side(id, a);
    sc_core::sc_object *mstobj = other_socket->get_parent();
    // --------------------

    // Extract data pointer from payload
    unsigned int *data  = (unsigned int *)ahb_gp.get_data_ptr();
    // Extract address from payload
    unsigned int addr   = ahb_gp.get_address();
    // Extract length from payload
    unsigned int length = ahb_gp.get_data_length(); 

    // Is this an access to configuration area
    if (mfpnpen && ((((addr >> 20) ^ mcfgaddr) & mcfgmask)==0)) {

      // Configuration area is read only
      if (ahb_gp.get_command() == tlm::TLM_READ_COMMAND) {

	// No subword access supported here!
	assert(length%4==0);

	// Get registers from config area
	for (uint32_t i = 0 ; i < (length >> 2); i++) {

	  data[i] = getPNPReg(addr);

        }
	
	ahb_gp.set_response_status(tlm::TLM_OK_RESPONSE);
	return length;

      } else {

	v::error << name() << " Forbidden write to AHBCTRL configuration area (PNP)!" << v::endl;
	ahb_gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
	return 0;
      }
    }    

    // Find slave by address / returns slave index or -1 for not mapped
    int index = get_index(ahb_gp.get_address());

    // For valid slave index
    if(index >= 0) {

       // -- For Debug only --
       other_socket = ahbOUT.get_other_side(index, a);
       sc_core::sc_object *obj = other_socket->get_parent();

       v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
                << v::setw(8) << ahb_gp.get_address() << ", from master:"
                << mstobj->name() << ", forwarded to slave:" << obj->name() << endl;
       // --------------------

       // Forward request to the selected slave
       return ahbOUT[index]->transport_dbg(ahb_gp);

    } else {

       v::warn << name() << "AHB Request@0x" << hex << v::setfill('0')
               << v::setw(8) << ahb_gp.get_address() << ", from master:"
               << mstobj->name() << ": Unmapped address space." << endl;

       // Invalid index
       ahb_gp.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
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
