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
// Title:      apbctrl.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implements an LT/AT AHB APB Bridge
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
// ********************************************************************

#include "apbctrl.h"
#include "verbose.h"


/// Constructor of class APBCtrl
APBCtrl::APBCtrl(sc_core::sc_module_name nm, // SystemC name
		 uint32_t haddr_,            // The MSB address of the AHB area. Sets the 12 MSBs in the AHB address
                 uint32_t hmask_,            // The 12bit AHB area address mask
		 bool mcheck,                // Check if there are any intersections between APB slave memory regions
		 amba::amba_layer_ids ambaLayer) :

      sc_module(nm),
      AHBDevice(0x04,  // vendor_id: ESA
                0x006, // device_id: APBCtrl (p. 92 GRIP) 
                0,     //
                0,     // IRQ
                BAR(AHBDevice::APBIO, hmask_, 0, 0, haddr_), // BAR 0
                0,     // BAR 1
                0,     // BAR 2
                0),    // BAR 3
      ahb("ahb", amba::amba_AHB, amba::amba_LT, false),  // TODO set arbiter flags to true as soon bug in ambasockets is fixed
      apb("apb", amba::amba_APB, amba::amba_LT, false),
      mhaddr(haddr_),
      mhmask(hmask_),
      mmcheck(mcheck),
      mambaLayer(ambaLayer),
      clockcycle(10.0, sc_core::SC_NS) {

    // Assert generics are withing allowed ranges
    assert(haddr_ <= 0xfff);
    assert(hmask_ <= 0xfff);

    // Calculate base address of PNP APB device records
    // (Upper 4kb of device address space)
    mpnpbase = get_base_addr_()+get_size_()-0x1000;

    if (mambaLayer==amba::amba_LT) {

      // register tlm blocking transport function
      ahb.register_b_transport(this, &APBCtrl::ahb_custom_b_transport);

    } else if (mambaLayer==amba::amba_AT) {

      // register non-blocking forward transport function for ahb slave
      ahb.register_nb_transport_fw(this, &APBCtrl::ahb_custom_nb_transport_fw);

      // register non-blocking backward transport function for apb master socket
      apb.register_nb_transport_bw(this, &APBCtrl::apb_custom_nb_transport_bw);

    } else {

      v::error << name() << "Abstraction Layer not valid!!" << v::endl;
      assert(0);

    }
}

// Destructor
APBCtrl::~APBCtrl() {

}

/// Helper function for creating slave map decoder entries
void APBCtrl::setAddressMap(const uint32_t i, const uint32_t addr, const uint32_t mask) {

  // Create slave map entry from slave ID and address range descriptor (slave_info_t)
  // Why std::map: Contains only the bar entries, which are actually valid.
  // A static array would have holes -> far slower, especially if number of slaves is small.
  slave_map.insert(std::pair<uint32_t, slave_info_t>(i, slave_info_t(addr, mask)));
}

/// Find slave index by address
int APBCtrl::get_index(const uint32_t address) {

  // Use 12 bit segment address for decoding
  uint32_t addr = address >> 20;

  for (it = slave_map.begin(); it != slave_map.end(); it++) {

    slave_info_t info = it->second;

    if (((addr ^ info.first) & info.second) == 0) {

      // APB: Device == BAR)
      return(it->first);

    }

  }

  // no slave found
  return -1;
}

/// Returns a PNP register from the APB configuration area (upper 4kb of address space)
unsigned int APBCtrl::getPNPReg(const uint32_t address) {

  // Calculate address offset in configuration area
  unsigned int addr = address - mpnpbase;
  // Calculate index of the device in mSlaves pointer array (8 byte per device)
  unsigned int device = addr >> 1;
  // Calculate offset within device information
  unsigned int offset = addr & 0x1;

  return(mSlaves[device][offset]);

}

/// TLM blocking transport function
void APBCtrl::ahb_custom_b_transport(tlm::tlm_generic_payload& ahb_gp,sc_core::sc_time& delay) {

  // Extract data pointer from payload
  unsigned int *data = (unsigned int *) ahb_gp.get_data_ptr();
  // Extract address from payload
  unsigned int addr = ahb_gp.get_address();
  // Extract length from payload
  unsigned int length = ahb_gp.get_data_length();

  // Is this for me?
  if (!(((addr >> 20) ^ mhaddr) & mhmask)) {
 
    // Access configuration area (upper most 4kb)?
    if (addr >= mpnpbase) {

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

	v::error << name() << " Forbidden write to APBCTRL configuration area (PNP)!" << v::endl;
	ahb_gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
	return;
      }
    }
	
    // Find slave by address / returns slave index or -1 for not mapped
    int index = get_index(ahb_gp.get_address());

    // For valid slave index
    if(index >= 0) {

       // -- For Debug only --
       uint32_t a = 0;
       socket_t *other_socket = apb.get_other_side(index, a);
       sc_core::sc_object *obj = other_socket->get_parent();

       v::debug << name() << "Forwarding request to APB slave:" << obj->name()
                << "@0x" << hex << v::setfill('0') << v::setw(8)
                << (ahb_gp.get_address() & 0x000fffff) << endl;
       // --------------------

       // Take APB transaction from pool
       payload_t *apb_gp = apb.get_transaction();

       // Build APB transaction from incoming AHB transaction:
       // ----------------------------------------------------
       apb_gp->set_command(ahb_gp.get_command());
       // Substract the base address of the bridge
       apb_gp->set_address(ahb_gp.get_address() & 0x000fffff);
       apb_gp->set_data_length(ahb_gp.get_data_length());
       apb_gp->set_byte_enable_ptr(ahb_gp.get_byte_enable_ptr());
       apb_gp->set_data_ptr(ahb_gp.get_data_ptr());

       // Forward request to the selected slave
       apb[index]->b_transport(*apb_gp, delay);

       // Copy back response message
       ahb_gp.set_response_status(apb_gp->get_response_status());

       // Add delay for APB setup cycle
       delay += clockcycle;

       // Release transaction
       apb.release_transaction(apb_gp);

    } else {

       v::error << name() << "Access to unmapped APB address space." << endl;
       ahb_gp.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    }
  }
}

tlm::tlm_sync_enum APBCtrl::ahb_custom_nb_transport_fw(tlm::tlm_generic_payload& gp, tlm::tlm_phase& phase, sc_core::sc_time& delay) {

  // todo
  
}

tlm::tlm_sync_enum APBCtrl::apb_custom_nb_transport_bw(uint32_t id, tlm::tlm_generic_payload& gp, tlm::tlm_phase& phase, sc_core::sc_time& delay) {

  // todo

}

unsigned int APBCtrl::transport_dbg(uint32_t id, tlm::tlm_generic_payload &gp) {

  // todo

}

/// Set up slave map and collect plug & play information
void APBCtrl::start_of_simulation() {

  // Get number of bindings at master socket (number of connected slaves)
  uint32_t num_of_bindings = apb.size();

  // max. 16 APB slaves allowed
  assert(num_of_bindings<=16);

  v::info << name() << "******************************************************************************* " << v::endl;
  v::info << name() << "* DECODER INITIALIZATION " << v::endl;
  v::info << name() << "* ---------------------- " << v::endl;

  // iterate the registered slaves
  for (uint32_t i = 0; i < num_of_bindings; i++) {

    uint32_t a = 0;

    // get pointer to socket of slave i
    socket_t *other_socket = apb.get_other_side(i, a);

    // get parent object containing slave socket i
    sc_core::sc_object *obj = other_socket->get_parent();

    // valid slaves implement the APBDevice interface
    APBDevice *slave = dynamic_cast<APBDevice *> (obj);

    v::info << name() << "* Slave name: " << obj->name() << v::endl;

    // slave is valid (implements APBDevice)
    if(slave) {

      // Get pointer to device information
      const uint32_t * deviceinfo = slave->get_device_info();

      // Map device information into PNP region
      mSlaves[i] = deviceinfo;

      // check 'type'filed of bar[i] (must be != 0)
      if (slave->get_type()) {

	// get base address and mask from BAR
        uint32_t addr = slave->get_base();
        uint32_t mask = slave->get_mask();

	v::info << name() << "* BAR with MSB addr: " << hex << addr << " and mask: " << mask << v::endl;

	// insert slave region into memory map
        setAddressMap(i, addr, mask);

      }

    } else {

       v::warn << name() << "Slave bound to socket 'apb' is not a valid APBDevice." << v::endl;
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

/// Check the memory map for overlaps
void APBCtrl::checkMemMap() {
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
             socket_t *other_socket = apb.get_other_side(it->first, a);
             sc_core::sc_object *obj = other_socket->get_parent();

             other_socket = apb.get_other_side(it2->first, a);
             sc_core::sc_object *obj2 = other_socket->get_parent();

             v::error << name() << "Overlap in APB memory mapping." << endl;
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

/// Helper for setting clock cycle latency using sc_clock argument
void APBCtrl::clk(sc_core::sc_clock &clk) {

  clockcycle = clk.period();

}

/// Helper for setting clock cycle latency using sc_time argument
void APBCtrl::clk(sc_core::sc_time &period) {

  clockcycle = period;

}

/// Helper for setting clock cycle latency using a value-time_unit pair
void APBCtrl::clk(double period, sc_core::sc_time_unit base) {

  clockcycle = sc_core::sc_time(period, base);

}

