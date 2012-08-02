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
// Title:      arraymemory.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Class defintion of the generic memory model to be used 
//             with the SoCRocket MCTRL. Can be configured as ROM, 
//             IO, SRAM or SDRAM. Underlying memory is implemented 
//             as a flat array.
//             Recommended for fast simulation of small memories.
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

#ifndef ARRAYMEMORY_H
#define ARRAYMEMORY_H

#include "memdevice.h"
#include "vmap.h"

#include <greencontrol/config.h>
#include <greensocket/target/single_socket.h>
#include "ext_erase.h"
#include <tlm.h>
#include <systemc.h>

#include "verbose.h"

/// @brief This class models a array memory. Depending on the configuration
/// it can be used as ROM, IO, SRAM or SDRAM, in conjunction with the SoCRocket MCTRL.
class ArrayMemory : public MEMDevice, 
	              public sc_core::sc_module {

    public:

        /// Slave socket -  for communication with Mctrl
        gs::socket::target_socket<32> bus;

        /// Creates a new Instance of ArrayMemory
        ///
        /// @param name The SystemC name of the component to be created
        /// @param type The type of memory to be modeled (0-ROM, 1-IO, 2-SRAM, 3-SDRAM)
        /// @param banks Number of parallel banks
        /// @param bsize Size of one memory bank in bytes (all banks always considered to have equal size)
        /// @param bits Bit width of memory
        /// @param cols Number of SDRAM cols.
        ArrayMemory(sc_module_name name, 
		      MEMDevice::device_type type, 
		      uint32_t banks, 
		      uint32_t bsize, 
		      uint32_t bits, 
		      uint32_t cols = 0,
		      bool pow_mon = false);

        /// Destructor
        ~ArrayMemory();
        
        uint8_t *memory;

        /// SystemC end of simulation
        void end_of_simulation();
        
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
  
        /// GreenControl API Pointer
        gs::cnf::cnf_api *m_api;

        /// Performance Counter Array
        gs::gs_param_array m_performance_counters;

        /// Performance counter to store transaction byte reads
        gs::gs_param<unsigned long long> m_reads;

        /// Performance counter to store the transaction byte writes
        gs::gs_param<unsigned long long> m_writes; 

        /// *****************************************************
        /// Power Modeling Parameters

        /// Normalized static power input
        gs::gs_param<double> sta_power_norm;

        /// Normalized dynamic power input (activation independent)
        gs::gs_param<double> dyn_power_norm;

        /// Normalized read access energy
        gs::gs_param<double> dyn_read_energy_norm;

        /// Normalized write access energy
        gs::gs_param<double> dyn_write_energy_norm;

        /// Parameter array for power data output
        gs::gs_param_array power;

        /// Static power of module
        gs::gs_param<double> sta_power;

        /// Dynamic power of module
        gs::gs_param<double> dyn_power;

        /// Dynamic energy per read access
        gs::gs_param<double> dyn_read_energy;

        /// Dynamic energy per write access
        gs::gs_param<double> dyn_write_energy;

        /// Number of reads from memory (read & reset by monitor)
        gs::gs_param<unsigned long long> dyn_reads;

        /// Number of writes to memory (read & reset by monitor)
        gs::gs_param<unsigned long long> dyn_writes;
};

#endif
