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
// Title:      tb_ahb_mem.cpp
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
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Soeren Brinkmann
// Reviewed:
// *****************************************************************************

#include "tb_ahb_mem.h"
#include <fstream>
#include <iostream>
#include "verbose.h"

/// Constructor
Ctb_ahb_mem::Ctb_ahb_mem(const sc_core::sc_module_name nm, // Module name
                         const uint16_t haddr_, // AMBA AHB address (12 bit)
                         const uint16_t hmask_, // AMBA AHB address mask (12 bit)
                         const amba::amba_layer_ids ambaLayer, // abstraction layer
                         const char infile[], // Memory initialization file
                         uint32_t addr, // Address for memory initialization
			 const uint32_t slave_id) :
            sc_module(nm),
            AHBDevice(slave_id,
		      0x04, // vendor_id: ESA
		      0, // device: TODO: get real device ID
		      0, //
		      0, // IRQ
		      BAR(AHBDevice::AHBMEM, hmask_, 0, 0, haddr_), // BAR 0
		      0, // BAR 1
		      0, // BAR 2
		      0  // BAR 3
            ),
            ahb("ahb", // sc_module_name
                    amba::amba_AHB, // bus type
                    ambaLayer, // abstraction level
                    false // is arbiter
            ),
            mTransactionPEQ("TransactionPEQ"),
            ahbBaseAddress(static_cast<uint32_t> (hmask_ & haddr_) << 20),
            ahbSize(~(static_cast<uint32_t> (hmask_) << 20) + 1), hmask(
                    static_cast<uint32_t> (hmask_ << 20)), haddr(
                    static_cast<uint32_t> (haddr_ << 20)),
	    mhaddr(haddr_),
	    mhmask(hmask_),
            clockcycle(10.0, sc_core::SC_NS) {

    // haddr and hmask must be 12 bit
    assert(!((mhaddr|mhmask)>>12));

    // Display AHB slave information
    v::info << name() << "********************************************************************" << v::endl;
    v::info << name() << "* Create AHB simulation memory with following parameters:           " << v::endl;
    v::info << name() << "* haddr/hmask: " << hex << mhaddr << "/" << mhmask                    << v::endl;
    v::info << name() << "* Slave base address: " << hex << get_base_addr()                     << v::endl;
    v::info << name() << "* Slave size (bytes): " << hex << get_size()                          << v::endl;
    v::info << name() << "********************************************************************" << v::endl;

    // For LT register blocking transport
    if(ambaLayer==amba::amba_LT) {

      ahb.register_b_transport(this, &Ctb_ahb_mem::b_transport);

    }

    // For AT register non-blocking transport
    if(ambaLayer==amba::amba_AT) {

      ahb.register_nb_transport_fw(this, &Ctb_ahb_mem::nb_transport_fw);

    }

    ahb.register_transport_dbg(this, &Ctb_ahb_mem::transport_dbg);

    SC_THREAD(processTXN);

    if(infile != NULL) {
      readmem(infile, addr);
    }
}


/// Destructor
Ctb_ahb_mem::~Ctb_ahb_mem() {

    // Delete memory contents
    mem.clear();
} 

/// TLM blocking transport function
void Ctb_ahb_mem::b_transport(tlm::tlm_generic_payload &trans,
                              sc_core::sc_time &delay) {

  // Is the address for me
  if(!((mhaddr ^ (trans.get_address() >> 20)) & mhmask)) {

    // warn if access exceeds slave memory region
    if((trans.get_address() + trans.get_data_length()) >

      (ahbBaseAddress + ahbSize)) {
       v::warn << name() << "Transaction exceeds slave memory region" << endl;
    }

    // consume router delay (address phase)
    wait(delay);

    // reset timing
    delay = sc_core::SC_ZERO_TIME;

    if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {

      for(uint32_t i = 0; i < trans.get_data_length(); i++) {
	    
	v::debug << name() << "Write with address: " << hex << trans.get_address() + i << " and data: " << hex << (unsigned int)*(trans.get_data_ptr() + i) << v::endl;

	// write simulation memory
        mem[trans.get_address() + i] = *(trans.get_data_ptr() + i);

      }

      delay = clockcycle * (trans.get_data_length() >> 2);
      trans.set_response_status(tlm::TLM_OK_RESPONSE);

    } else {

      for(uint32_t i = 0; i < trans.get_data_length(); i++) {
       
	v::debug << name() << "Read with address: " << hex << trans.get_address() + i << " to return: " << hex << (unsigned int)mem[trans.get_address() + i] << v::endl;

	// read simulation memory
	*(trans.get_data_ptr() + i) = mem[trans.get_address() + i];
        
      }

      delay = clockcycle * (trans.get_data_length() >> 2);
      trans.set_response_status(tlm::TLM_OK_RESPONSE);

    }

  // address not valid
  } else {

    v::error << name() << "Address not within permissable slave memory space" << v::endl;
    trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);

  }
}

tlm::tlm_sync_enum Ctb_ahb_mem::nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay) {

  v::debug << name() << "nb_transport_fw received transaction " << hex << &trans << " with phase: " << phase << v::endl;

  // The master has sent BEGIN_REQ
  if (phase == tlm::BEGIN_REQ) {

    // Writes have to wait for BEGIN_DATA
    if (trans.get_command() == tlm::TLM_READ_COMMAND) {

      mTransactionPEQ.notify(trans,delay);

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

void Ctb_ahb_mem::processTXN() {

  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;

  tlm::tlm_generic_payload * trans;

  while(1) {

    wait(mTransactionPEQ.get_event());

    while((trans = mTransactionPEQ.get_next_transaction())) {

      v::debug << name() << "Process transaction " << hex << trans << v::endl;

      if (trans->get_command() == tlm::TLM_WRITE_COMMAND) {

        for(uint32_t i = 0; i < trans->get_data_length(); i++) {
	    
	  v::debug << name() << "Write with address: " << hex << trans->get_address() + i << " and data: " << hex << (unsigned int)*(trans->get_data_ptr() + i) << v::endl;

	  // write simulation memory
          mem[trans->get_address() + i] = *(trans->get_data_ptr() + i);

        }

        // Send END_DATA
        phase = amba::END_DATA;
        delay = clockcycle * (trans->get_data_length() >> 2);

	v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << phase << " (delay: " << delay << ")" << v::endl;

        trans->set_response_status(tlm::TLM_OK_RESPONSE);

        status = ahb->nb_transport_bw(*trans, phase, delay);

	assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_COMPLETED));

      } else {

        for(uint32_t i = 0; i < trans->get_data_length(); i++) {
       
	  v::debug << name() << "Read with address: " << hex << trans->get_address() + i << " to return: " << hex << (unsigned int)mem[trans->get_address() + i] << v::endl;

	  // read simulation memory
	  *(trans->get_data_ptr() + i) = mem[trans->get_address() + i];
        
        }

        // Send BEGIN_RESP
        phase = tlm::BEGIN_RESP;
        delay = clockcycle * (trans->get_data_length() >> 2);

	v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << phase << " (delay: " << delay << ")" << v::endl; 

        trans->set_response_status(tlm::TLM_OK_RESPONSE);

        status = ahb->nb_transport_bw(*trans, phase, delay);

	assert(status==tlm::TLM_ACCEPTED);
      }
    }
  }
}

/// Method to initialize memory contents from a text file
int Ctb_ahb_mem::readmem(const char infile_[], uint32_t addr) {

    std::ifstream infile(infile_, ios::in);
    char buffer = 0;
    uint8_t nibbleBuffer = 0;
    unsigned char data = 0;
    unsigned int nibble = 0;
    bool processChar = 0;

    // Parse input file
    if(infile.good()) {
        while(!infile.eof()) {
            // Read one byte from file
            infile.read(&buffer, 1);
            // Convert char into binary
            nibbleBuffer = char2nibble(&buffer);

            // combine two ascii chars into one byte and write into memory
            if(!(nibbleBuffer & 0xf0)) {
                if(nibble) {
                    data |= nibbleBuffer;
                    mem[addr++] = data;
                    data = 0x0;
                    nibble = 0;
                } else {
                    data |= (nibbleBuffer << 4);
                    nibble++;
                }
                processChar = 0;
            }
        } // while(!infile.eof())

        // Warn if data stream ended unexpected
        if(nibble) {
            v::warn << name() << "Incomplete byte detected in memory file"
                    << endl;
        }

        // close file
        infile.close();
        return 0;
    } else {
        v::error << name() << "File \"" << infile_
                << "\" not found or readable" << endl;
        return 1;
    }
} // int Ctb_ahb_mem::readmem(char infile_[], unsigned int addr)


/// Method to convert ascii characters into their binary representation
uint8_t Ctb_ahb_mem::char2nibble(const char *ch) const {
    switch(*ch) {
        case '\n':
            return 0x10;
            break;
        case ' ':
            return 0x20;
            break;
        case '\t':
            return 0x20;
            break;
        case '0':
            return 0x00;
            break;
        case '1':
            return 0x01;
            break;
        case '2':
            return 0x02;
            break;
        case '3':
            return 0x03;
            break;
        case '4':
            return 0x04;
            break;
        case '5':
            return 0x05;
            break;
        case '6':
            return 0x06;
            break;
        case '7':
            return 0x07;
            break;
        case '8':
            return 0x08;
            break;
        case '9':
            return 0x09;
            break;
        case 'a':
            return 0x0a;
            break;
        case 'b':
            return 0x0b;
            break;
        case 'c':
            return 0x0c;
            break;
        case 'd':
            return 0x0d;
            break;
        case 'e':
            return 0x0e;
            break;
        case 'f':
            return 0x0f;
            break;
        case 'A':
            return 0x0a;
            break;
        case 'B':
            return 0x0b;
            break;
        case 'C':
            return 0x0c;
            break;
        case 'D':
            return 0x0d;
            break;
        case 'E':
            return 0x0e;
            break;
        case 'F':
            return 0x0f;
            break;
        default:
            v::warn << name() << "Illegal character in memory file: \'" << *ch
                    << "\'" << endl;
            return 0x80;
            break;
    }
} // uint8_t Ctb_ahb_mem::char2nibble(char *ch) const

void Ctb_ahb_mem::writeByteDBG(const uint32_t address, const uint8_t byte) {
   mem[address] = byte;
}

/// Method to dump the memory content into a text file
int Ctb_ahb_mem::dumpmem(const char outfile_[]) {

    // check if memory is filled
    if(mem.empty()) {
        v::debug << name() << "Memory is empty. Nothing do dump." << endl;
        return 1;
    }

    std::ofstream outfile(outfile_, ios::out);
    uint32_t word = 0;
    int position = 0;
    // create map iterator and initialize to first element
    std::map<uint32_t, uint8_t>::iterator it = mem.begin();
    std::map<uint32_t, uint8_t>::iterator it_next = mem.begin();
    it_next++;

    if(outfile.good()) {
        // print first address in file
        outfile << "@" << hex << v::setw(8) << v::setfill('0') << (it->first
                & 0xfffffffc) << endl;

        for(it = mem.begin(); it != mem.end(); it++, it_next++) {

            position = it->first % 4;
            word |= (it->second << (8 * position));

            // Next byte not within current word
            if((3 - position - static_cast<int> (it_next->first - it->first)) < 0) {
                // print word
                outfile << hex << v::setw(8) << v::setfill('0') << word << endl;
                word = 0;

                // print address if necessary
                if((7 - position - static_cast<int> (it_next->first
                        - it->first)) < 0) {
                    outfile << "@" << hex << v::setw(8) << v::setfill('0')
                            << (it_next->first & 0xfffffffc) << endl;
                }
            }
        }
        // print last word
        outfile << hex << v::setw(8) << v::setfill('0') << word << endl;

        outfile.close();
        return 0;
    } else {
        v::error << name() << "Unable to open dump file" << endl;
        return 1;
    }
} // int tb_ahb_mem::dumpmem(char outfile_[])

// TLM debug interface
unsigned int Ctb_ahb_mem::transport_dbg(tlm::tlm_generic_payload &gp) {
    // Check address befor before doing anything else
    if(!((haddr ^ (gp.get_address() & 0xfff00000)) & hmask)) {
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
                    *(gp.get_data_ptr() + i) = mem[gp.get_address() + i];
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
    } // if( !((haddr ^ (gp.get_address() & 0xfff00000)) & hmask) )

    return 0;
}

/// Helper for setting clock cycle latency using sc_clock argument
void Ctb_ahb_mem::clk(sc_core::sc_clock &clk) {

  clockcycle = clk.period();

}

/// Helper for setting clock cycle latency using sc_time argument
void Ctb_ahb_mem::clk(sc_core::sc_time &period) {

  clockcycle = period;

}

/// Helper for setting clock cycle latency using a value-time_unit pair
void Ctb_ahb_mem::clk(double period, sc_core::sc_time_unit base) {

  clockcycle = sc_core::sc_time(period, base);

}
