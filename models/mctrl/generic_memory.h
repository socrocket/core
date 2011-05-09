//*********************************************************************
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
//*********************************************************************
// Title:      Generic_memory.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    memory module modeling all types of memory supported by
//             mctrl: RAM, std I/O, SRAM, SDRAM
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Dennis Bode
// Reviewed:
//*********************************************************************

#ifndef GENERIC_MEMORY_H
#define GENERIC_MEMORY_H

#include <map>
#include <systemc.h>
#include <tlm.h>
#include "tlm_utils/simple_target_socket.h"
#include "ext_erase.h"
#include <assert.h>
#include "verbose.h"

template<typename T = uint8_t>
class Generic_memory : public sc_core::sc_module {
    public:
        //Slave socket: communication with mctrl
        tlm_utils::simple_target_socket<Generic_memory> slave_socket;

        //constructor / destructor
        Generic_memory(sc_module_name name);
        ~Generic_memory();

        //memory instance (key: int address, value: int data)
        //ROM and SRAM words are modelled as 4 single bytes for easier management,
        //i.e. we need the template parameter T to be uint32_t (SDRAM, IO) or uint8_t (ROM, SRAM)
        std::map<uint32_t, T> memory;

        //blocking transport functions
        void b_transport(tlm::tlm_generic_payload& gp, sc_time& delay);

        // Debug interface
        unsigned int transport_dbg(tlm::tlm_generic_payload& gp);

        //read from memory
        //ROM, SRAM: byte addressable
        void read_8(uint32_t address, unsigned char* data_ptr, uint8_t length);
        //IO, SDRAM: word addressable
        void read_32(uint32_t address, uint32_t* data_ptr, uint8_t length);

        uint8_t readByteDBG(const uint32_t addr);

        //write into memory
        //ROM, SRAM: byte addressable
        void write_8(uint32_t address, unsigned char* data, uint8_t length);
        //IO, SDRAM: word addressable
        void write_32(uint32_t address, uint32_t* data, uint8_t length);

        void writeByteDBG(const uint32_t addr, const uint8_t byte);

        //erase sdram required for deep power down and PASR mode
        void erase_memory(uint32_t start_address, uint32_t end_address,
                          unsigned int length);

};

#include "generic_memory.tpp"

#endif
