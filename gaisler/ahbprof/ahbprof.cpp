// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup ahbprof
/// @{
/// @file ahbprof.cpp
///
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#include <ctime>
#include <fstream>
#include <iostream>
#include "gaisler/ahbprof/ahbprof.h"
#include "core/common/vendian.h"
#include "core/common/verbose.h"

SR_HAS_MODULE(AHBProf);

// / Constructor
AHBProf::AHBProf(const ModuleName nm,  // Module name
  uint32_t index,
  uint16_t addr,                                    // AMBA AHB address (12 bit)
  uint16_t mask,                                    // AMBA AHB address mask (12 bit)
  AbstractionLayer ambaLayer) :                 // abstraction layer
  AHBSlave<>(nm,
    index,
    0xce,                                           // Vendor: c3e
    0x0FE,                                          // Device: OutFileDevice,
    0,
    0,
    ambaLayer,
    BAR(AHBMEM, mask, 0, 0, addr)),
  m_addr(addr),
  m_mask(mask) {
  // haddr and hmask must be 12 bit
  assert(!((m_addr | m_mask) >> 12));

  // Display AHB slave information
  v::info << name() << "********************************************************************" << v::endl;
  v::info << name() << "* Create AHB Profiling device with following parameters:            " << v::endl;
  v::info << name() << "* haddr/hmask: " << v::uint32 << m_addr << "/" << v::uint32 << m_mask << v::endl;
  v::info << name() << "* Slave base address: 0x" << std::setw(8) << std::setfill('0') << hex <<
    get_ahb_base_addr()                     << v::endl;
  v::info << name() << "* Slave size (bytes): 0x" << std::setw(8) << std::setfill('0') << hex <<
    get_ahb_size()                          << v::endl;
  v::info << name() << "********************************************************************" << v::endl;
}

// / Destructor
AHBProf::~AHBProf() {
}

// / Encapsulated functionality
uint32_t AHBProf::exec_func(
    tlm::tlm_generic_payload &trans,  // NOLINT(runtime/references)
    sc_core::sc_time &delay,          // NOLINT(runtime/references)
    bool debug) {
  if (!((m_addr ^ (trans.get_address() >> 20)) & m_mask)) {
    uint32_t address = (trans.get_address() - ((m_addr & m_mask) << 20)) >> 2;
    if (address > 16) {
      v::warn << name() << "Address offset bigger than 16" << v::endl;
    }

    if (trans.get_data_length() > 4) {
      v::warn << name() << "Transaction exceeds slave memory region" << v::endl;
    }

    if (trans.is_write()) {
      unsigned int state = *((unsigned int *)trans.get_data_ptr());
      swap_Endianess(state);
      info[address].state = state;
      // v::info << name() << "Address: " << address << " -- State: " << state << v::endl;
      switch (state) {
      case 1:
        info[address].real_start = std::clock();
        info[address].sim_start = sc_time_stamp();
        break;
      case 2:
        info[address].real_end = std::clock();
        info[address].sim_end = sc_time_stamp();
        break;
      case 3:
        v::report << name() << "********************************************************************" << v::endl;
        v::report << name() << "* real time (" << address << "): " << info[address].real_end << " - " <<
          info[address].real_start << " = " << (info[address].real_end - info[address].real_start) << v::endl;
        v::report << name() << "* simulated time (" << address << "): " << info[address].sim_end << " - " <<
          info[address].sim_start << " = " << (info[address].sim_end - info[address].sim_start) << v::endl;
        v::report << name() << "********************************************************************" << v::endl;

        break;
      case 255:
        sc_core::sc_stop();
        break;
      default: {
        }
      }

      delay += clock_cycle * 4;
      trans.set_response_status(tlm::TLM_OK_RESPONSE);
    } else {
      // We don't support reading. We are an output device.
      // So return 0x0
      for (uint32_t i = 0; i < trans.get_data_length(); i++) {
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
  return trans.get_data_length();
}

sc_core::sc_time AHBProf::get_clock() {
  return clock_cycle;
}
// / @}
