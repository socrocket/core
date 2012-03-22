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
// Title:      ahbmem.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Provide a test bench memory class with AHB slave interface.
//
// Method:     Memory is modeled with a map.
//             DMI and streaming width fields are ignored.
//             Delays are not modeled.
//             Address checking is performed in that way, that transactions
//             are only executed if the slave select condition defined in
//             grlib user manual holds.
//             Transactions generating a correct slave select but exceeding
//             the memory region due to their length are reported as warning
//             and executed anyhow.
//
// Modified on $Date: 2011-08-18 15:18:53 +0200 (Thu, 18 Aug 2011) $
//          at $Revision: 492 $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
// *****************************************************************************

#include "ahbmem.h"
#include <fstream>
#include <iostream>
#include "verbose.h"

/// Constructor
AHBMem::AHBMem(const sc_core::sc_module_name nm, // Module name
               uint16_t haddr_, // AMBA AHB address (12 bit)
               uint16_t hmask_, // AMBA AHB address mask (12 bit)
               amba::amba_layer_ids ambaLayer, // abstraction layer
               uint32_t slave_id,
	       bool cacheable) :
            sc_module(nm),
            AHBDevice(slave_id, /*Gaisler*/ 0x01, /*AHBMem*/0x00E, 0, 0, BAR(AHBDevice::AHBMEM, hmask_, cacheable, 0, haddr_)),
            ahb("ahb", amba::amba_AHB, ambaLayer, false /* arbiter? */),
            m_performance_counters("performance_counters"),
            m_bytes_read("bytes_read", 0ull, m_performance_counters),
            m_bytes_written("bytes_written", 0ull, m_performance_counters),
            mTransactionPEQ("TransactionPEQ"),
            ahbBaseAddress(static_cast<uint32_t> (hmask_ & haddr_) << 20),
            ahbSize(~(static_cast<uint32_t> (hmask_) << 20) + 1), 
            mhaddr(haddr_),
            mhmask(hmask_),
	    mcacheable(cacheable) {

    // haddr and hmask must be 12 bit
    assert(!((mhaddr|mhmask)>>12));

    // Display AHB slave information
    v::info << name() << "********************************************************************" << v::endl;
    v::info << name() << "* Create AHB simulation memory with following parameters:           " << v::endl;
    v::info << name() << "* haddr/hmask: " << v::uint32 << mhaddr << "/" << v::uint32 << mhmask << v::endl;
    v::info << name() << "* Slave base address: 0x" << std::setw(8) << std::setfill('0') << hex << get_base_addr()                     << v::endl;
    v::info << name() << "* Slave size (bytes): 0x" << std::setw(8) << std::setfill('0') << hex << get_size()                          << v::endl;
    v::info << name() << "********************************************************************" << v::endl;

    // For LT register blocking transport
    if(ambaLayer==amba::amba_LT) {
      ahb.register_b_transport(this, &AHBMem::b_transport);
    }

    // For AT register non-blocking transport
    if(ambaLayer==amba::amba_AT) {
      ahb.register_nb_transport_fw(this, &AHBMem::nb_transport_fw);
    }

    ahb.register_transport_dbg(this, &AHBMem::transport_dbg);

    SC_THREAD(processTXN);
}

void AHBMem::dorst() {
    mem.clear();
}

/// Destructor
AHBMem::~AHBMem() {
    // Delete memory contents
    mem.clear();
} 

/// TLM blocking transport function
void AHBMem::b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {
  // Is the address for me
  if(!((mhaddr ^ (trans.get_address() >> 20)) & mhmask)) {
    // warn if access exceeds slave memory region
    if((trans.get_address() + trans.get_data_length()) >
      (ahbBaseAddress + ahbSize)) {
       v::warn << name() << "Transaction exceeds slave memory region" << endl;
    }

    if(trans.is_write()) {
      for(uint32_t i = 0; i < trans.get_data_length(); i++) {

	v::debug << name() << "mem[" << hex << trans.get_address() + i << "] = 0x" << hex << v::setw(2) << (unsigned int)*(trans.get_data_ptr() + i) << v::endl;
        // write simulation memory
        mem[trans.get_address() + i] = *(trans.get_data_ptr() + i);
      }

      // Update statistics
      m_bytes_written += trans.get_data_length();

      // delay a clock cycle per word
      delay += clock_cycle * (trans.get_data_length() >> 2);
      trans.set_response_status(tlm::TLM_OK_RESPONSE);
    } else {
      for(uint32_t i = 0; i < trans.get_data_length(); i++) {

	v::debug << name() << "0x" << hex << v::setw(2) << (unsigned int)mem[trans.get_address() + i] << "= mem[" << hex << trans.get_address() + i << "]" << v::endl;
        // read simulation memory
        *(trans.get_data_ptr() + i) = mem[trans.get_address() + i];
      }

      // Update statistics
      m_bytes_read += trans.get_data_length();

      // delay a clock cycle per word
      delay += clock_cycle * (trans.get_data_length() >> 2);
      trans.set_response_status(tlm::TLM_OK_RESPONSE);

      // set cacheability
      if (mcacheable) {

	ahb.validate_extension<amba::amba_cacheable> (trans);

      }

    }

    v::debug << name() << "Delay increment: " << delay << v::endl;

  } else {
    // address not valid
    v::error << name() << "Address not within permissable slave memory space" << v::endl;
    trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);

  }
}

tlm::tlm_sync_enum AHBMem::nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay) {
  v::debug << name() << "nb_transport_fw received transaction " << hex << &trans << " with phase: " << phase << " and delay " << delay << v::endl;

  // The master has sent BEGIN_REQ
  if (phase == tlm::BEGIN_REQ) {

    // Writes have to wait for BEGIN_DATA
    if (trans.get_command() == tlm::TLM_READ_COMMAND) {
      mTransactionPEQ.notify(trans,delay);
    }

    // set cacheability
    if (mcacheable) {

	ahb.validate_extension<amba::amba_cacheable> (trans);

    }

    phase = tlm::END_REQ;
    delay = SC_ZERO_TIME;
    return(tlm::TLM_UPDATED);

  } else if (phase == amba::BEGIN_DATA) {
    mTransactionPEQ.notify(trans,delay);
    
  } else if (phase == tlm::END_RESP) {
    // nothing to do
  } else {

    v::error << name() << "Invalid phase in call to nb_transport_fw!" << v::endl;
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
  }

  return(tlm::TLM_ACCEPTED);
}

void AHBMem::processTXN() {

  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;

  tlm::tlm_generic_payload * trans;

  while(1) {
    wait(mTransactionPEQ.get_event());
    while((trans = mTransactionPEQ.get_next_transaction())) {
      v::debug << name() << "Process transaction " << hex << trans << v::endl;
      if(trans->get_command() == tlm::TLM_WRITE_COMMAND) {
        for(uint32_t i = 0; i < trans->get_data_length(); i++) {
	  v::debug << name() << "Write with address: 0x" << std::setw(8) << std::setfill('0') << hex << trans->get_address() + i << " and data: 0x" << std::setw(2) << std::setfill('0') << hex << (unsigned int)*(trans->get_data_ptr() + i) << v::endl;

	  // write simulation memory
          mem[trans->get_address() + i] = *(trans->get_data_ptr() + i);

        }

        // Update statistics
        m_bytes_written += trans->get_data_length();

        // Send END_DATA
        phase = amba::END_DATA;
        delay = clock_cycle * (trans->get_data_length() >> 2);

	v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << phase << " (delay: " << delay << ")" << v::endl;

        trans->set_response_status(tlm::TLM_OK_RESPONSE);
        status = ahb->nb_transport_bw(*trans, phase, delay);
	assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_COMPLETED));
      } else {
        for(uint32_t i = 0; i < trans->get_data_length(); i++) {
	  v::debug << name() << "Read with address: 0x" << std::setw(8) << std::setfill('0') << hex << trans->get_address() + i << " to return: 0x" << std::setw(2) << std::setfill('0') << hex << (unsigned int)mem[trans->get_address() + i] << v::endl;

	  // read simulation memory
	  *(trans->get_data_ptr() + i) = mem[trans->get_address() + i];
        
        }

        // Update statistics
        m_bytes_read += trans->get_data_length();

        // Send BEGIN_RESP
        phase = tlm::BEGIN_RESP;
        delay = clock_cycle * (trans->get_data_length() >> 2);

	v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << phase << " (delay: " << delay << ")" << v::endl; 

        trans->set_response_status(tlm::TLM_OK_RESPONSE);

        status = ahb->nb_transport_bw(*trans, phase, delay);

	assert(status==tlm::TLM_ACCEPTED);
      }
    }
  }
}

void AHBMem::writeByteDBG(const uint32_t address, const uint8_t byte) {
   mem[address] = byte;
}

// TLM debug interface
unsigned int AHBMem::transport_dbg(tlm::tlm_generic_payload &gp) {
    // Check address befor before doing anything else
    if(!((mhaddr ^ (gp.get_address() >> 20)) & mhmask)) {
        // warn if access exceeds slave memory region
        if((gp.get_address() + gp.get_data_length()) >
           (ahbBaseAddress + ahbSize)) {
            v::warn << name() << "Transaction exceeds slave memory region"
                    << endl;
        }

        switch(gp.get_command()) {
            // Read command
            case tlm::TLM_READ_COMMAND:
                for(uint32_t i = 0; i < gp.get_data_length(); i++) {
                    gp.get_data_ptr()[i] = mem[gp.get_address() + i];
                }
                gp.set_response_status(tlm::TLM_OK_RESPONSE);
                return gp.get_data_length();
                break;

                // Write command
            case tlm::TLM_WRITE_COMMAND:
                for(uint32_t i = 0; i < gp.get_data_length(); i++) {
                    mem[gp.get_address() + i] = *(gp.get_data_ptr() + i);
                }
                gp.set_response_status(tlm::TLM_OK_RESPONSE);
                return gp.get_data_length();
                break;

            // Neither read nor write command
            default:
                v::warn << name() << "Received unknown command." << endl;
                gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
                return 0;
                break;
        }
    }
    return 0;
}

void AHBMem::end_of_simulation() {

  v::report << name() << " **************************************************** " << v::endl;
  v::report << name() << " * AHBMem Statistics: " << v::endl;
  v::report << name() << " * ------------------ " << v::endl;
  v::report << name() << " * Bytes read: " << m_bytes_read << v::endl;
  v::report << name() << " * Bytes written: " << m_bytes_written << v::endl;
  v::report << name() << " * ************************************************** " << v::endl;

}
