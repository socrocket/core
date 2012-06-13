// *****************************************************************************
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
// *****************************************************************************

#include "ahbout.h"
#include <fstream>
#include <iostream>
#include "verbose.h"

/// Constructor
AHBOut::AHBOut(const sc_core::sc_module_name nm, // Module name
               uint16_t haddr_,                  // AMBA AHB address (12 bit)
               uint16_t hmask_,                  // AMBA AHB address mask (12 bit)
               amba::amba_layer_ids ambaLayer,   // abstraction layer
               uint32_t slave_id,
               char *outfile_) :
            AHBSlave<>(nm,
                       slave_id, 
                       0xce, // Vendor: c3e
                       0x0FD,// Device: OutFileDevice,
                       0, 
                       0,
                       ambaLayer,
                       BAR(AHBDevice::AHBMEM, hmask_, 0, 0, haddr_)),
            mhaddr(haddr_),
            mhmask(hmask_) {

    // haddr and hmask must be 12 bit
    assert(!((mhaddr|mhmask)>>12));

    outfile.open(outfile_);
    // Display AHB slave information
    v::info << name() << "********************************************************************" << v::endl;
    v::info << name() << "* Create AHB simulation output device with following parameters:           " << v::endl;
    v::info << name() << "* haddr/hmask: " << v::uint32 << mhaddr << "/" << v::uint32 << mhmask << v::endl;
    v::info << name() << "* Slave base address: 0x" << std::setw(8) << std::setfill('0') << hex << get_base_addr()                     << v::endl;
    v::info << name() << "* Slave size (bytes): 0x" << std::setw(8) << std::setfill('0') << hex << get_size()                          << v::endl;
    v::info << name() << "********************************************************************" << v::endl;

}

/// Destructor
AHBOut::~AHBOut() {
  outfile.close();
} 

/// Encapsulated functionality
uint32_t AHBOut::exec_func(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay, bool debug) {

  uint32_t words_transferred;

  // Is the address for me
  if(!((mhaddr ^ (trans.get_address() >> 20)) & mhmask)) {
  
    // Warn if access exceeds slave memory region
    // We only do have one register!
    if(trans.get_data_length() > 4) { 
       v::warn << name() << "Transaction exceeds slave memory region" << endl;
    }

    if(trans.is_write()) {
      if(outfile.is_open()) {
      char c = *trans.get_data_ptr();
      outfile << c;
      } else {
        v::warn << name() << "File not open" << endl;
      }

      // Total delay is base delay + 4 wait states
      delay += clock_cycle * 4;
      trans.set_response_status(tlm::TLM_OK_RESPONSE);

    } else {
      // We don't support reading. We are an output device.
      // So return 0x0
      for(uint32_t i = 0; i < trans.get_data_length(); i++) {
        *(trans.get_data_ptr() + i) = 0;
      }

      // Total delay is base delay + 4 wait states
      delay += clock_cycle * 4;
      trans.set_response_status(tlm::TLM_OK_RESPONSE);

    }

  } else {
    // address not valid
    v::error << name() << "Address not within permissable slave memory space" << v::endl;
    trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
  }
  return(trans.get_data_length());
}

sc_core::sc_time AHBOut::get_clock() {
  return clock_cycle;
}
