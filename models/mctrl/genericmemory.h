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
// Title:      genericmemory.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Generic memory model to be used with the SoCRocket
//             MCTRL. Can be configured as ROM, IO, SRAM or SDRAM.
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

#include "verbose.h"
#include "memdevice.h"
#include "ext_erase.h"
#include <map>
#include <systemc.h>
#include <tlm.h>
#include <greensocket/target/single_socket.h>

/// @brief This class models a generic memory. Depending on the configuration
/// it can be used as ROM, IO, SRAM or SDRAM, in conjunction with the SoCRocket MCTRL.
class GenericMemory : public MEMDevice, 
	              public sc_core::sc_module {

    public:

        /// Slave socket -  for communication with Mctrl
        gs::socket::target_socket<32> bus;

	/// Creates a new Instance of GenericMemory
	///
	/// @param name The SystemC name of the component to be created
	/// @param type The type of memory to be modeled (0-ROM, 1-IO, 2-SRAM, 3-SDRAM)
	/// @param banks Number of parallel banks
	/// @param bsize Size of one memory bank in bytes (all banks always considered to have equal size)
	/// @param bits Bit width of memory
	/// @param cols Number of SDRAM cols.
        GenericMemory(sc_module_name name, 
		      MEMDevice::device_type type, 
		      uint32_t banks, 
		      uint32_t bsize, 
		      uint32_t bits, 
		      uint32_t cols,
		      bool pow_mon = false);

	/// Destructor
        ~GenericMemory();
        
        typedef std::map<uint32_t, uint8_t> type;
        type memory;

	/// TLM 2.0 blocking transport function
        void b_transport(tlm::tlm_generic_payload& gp, sc_time& delay);

	/// TLM 2.0 debug transport function
        unsigned int transport_dbg(tlm::tlm_generic_payload& gp);
        
	/// Read byte from functional memory
        uint8_t read(const uint32_t addr);

	/// Write byte to functional memory
        void write(const uint32_t addr, const uint8_t byte);

        /// Erase sdram - Required for deep power down and PASR mode
        void erase(uint32_t start, uint32_t end);

	/// Power monitoring
	bool g_powmon;
};

#endif
