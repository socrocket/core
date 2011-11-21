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
// Modified on $Date: 2011-05-09 20:31:53 +0200 (Mon, 09 May 2011) $
//          at $Revision: 416 $
//          by $Author: HWSWSIM $
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
#include <greensocket/target/single_socket.h>
#include <assert.h>
#include "verbose.h"
#include "memdevice.h"
#include "ext_erase.h"

class GenericMemory : public MEMDevice, public sc_core::sc_module {
    public:
        /// Slave socket: communication to the Mctrl
        gs::socket::target_socket<32> bus;

        GenericMemory(sc_module_name name, MEMDevice::device_type type, uint32_t banks, uint32_t bsize, uint32_t bits, uint32_t cols);
        ~GenericMemory();
        
        typedef std::map<uint32_t, uint8_t> type;
        type memory;

        void b_transport(tlm::tlm_generic_payload& gp, sc_time& delay);

        unsigned int transport_dbg(tlm::tlm_generic_payload& gp);
        
        uint8_t read(const uint32_t addr);
        void write(const uint32_t addr, const uint8_t byte);

        //erase sdram required for deep power down and PASR mode
        void erase(uint32_t start, uint32_t end);

        sc_core::sc_time calc_delay(bool write, uint32_t size, bool at, bool erase = false) const;
};

#endif
