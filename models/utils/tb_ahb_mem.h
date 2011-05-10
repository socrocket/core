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
// Title:      tb_ahb_mem.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Provide a test bench memory class with AHB slave interface.
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

#ifndef TB_AHB_MEM_H
#define TB_AHB_MEM_H

#include <systemc.h>
#include <tlm.h>
#include <map>
#include <amba.h>

#if defined(MTI_SYSTEMC)
#include "peq_with_get.h"
#else
#include "tlm_utils/peq_with_get.h"
#endif

#include "ahbdevice.h"

class Ctb_ahb_mem : public sc_module, 
                    public AHBDevice {

    public:
        /// Constructor
        SC_HAS_PROCESS(Ctb_ahb_mem);
        /// @brief Constructor for the test bench memory class
        /// @param haddr AHB address of the AHB slave socket (12 bit)
        /// @param hmask AHB address mask (12 bit)
        /// @param ambaLayer Abstraction layer used (AT/LT)
        /// @param infile File name of a text file to initialize the memory from
        /// @param addr Start address for memory initilization
        Ctb_ahb_mem(const sc_core::sc_module_name nm, const uint16_t haddr_,
                    const uint16_t hmask_ = 0, const amba::amba_layer_ids ambaLayer = amba::amba_LT,
                    const char infile[] = NULL, uint32_t addr = 0);

        /// Destructor
        ~Ctb_ahb_mem();

        /// AMBA slave socket
        amba::amba_slave_socket<32, 0> ahb;

        /// @brief Method to read memory contents from a text file
        /// @param infile File name of a text file to initialize the memory from
        /// @param addr Start address for memory initilization
        /// @return 0 on success, 1 on error
        int readmem(const char infile_[], uint32_t addr = 0);

        /// @brief Method dumping the memory contents to a text file
        /// @param outfile File name to dump the memory in
        /// @return 0 on success, 1 on error
        int dumpmem(const char outfile_[]);

        /// TLM blocking transport function
        void b_transport(unsigned int id, tlm::tlm_generic_payload &gp,
                         sc_core::sc_time &delay);

        /// TLM non blocking transport function
        tlm::tlm_sync_enum nb_transport_fw(unsigned int id, tlm::tlm_generic_payload& gp,
                                           tlm::tlm_phase& phase, sc_core::sc_time& delay);

        /// TLM debug interface
        unsigned int transport_dbg(uint32_t id, tlm::tlm_generic_payload& gp);

        /// @brief Delete memory content
        void clear_mem() {
            mem.clear();
        }

        /// @brief Method returning the memory's base address at the AHB bus
        /// @return AHB base address of the memory module
        inline sc_dt::uint64 get_base_addr() {
            return ahbBaseAddress;
        }

        /// @brief Method returning the memory's size at the AHB bus
        /// @return Size of the address space occupied by the memory module in bytes
        inline sc_dt::uint64 get_size() {
            // ahbSize holds size in bytes. Add shift if other units are required.
            return ahbSize;
        }

        /// @brief Method to write a byte into the memory
        /// @param addr Write address
        /// @param byte Write data
        void writeByteDBG(const uint32_t addr, const uint8_t byte);

	/// Helper functions for definition of clock cycle
	void clk(sc_core::sc_clock &clk);
	void clk(sc_core::sc_time &period);
	void clk(double period, sc_core::sc_time_unit base);

    private:
        /// The actual memory
        std::map<uint32_t, uint8_t> mem;
        /// Method to convert ascii chars into their binary represenation
        uint8_t char2nibble(const char *ch) const;

        /// Thread processign transactions when they emerge from the PEQ
        void processTXN();

        /// @brief Method executing read/write commands
        /// @param gp Generic payload object to process
        /// @return 0 When read/write executed, 1 on unknown command
        bool execCmd(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay);

        sc_core::sc_event e_continueTXN;

        /// Payload event queue. Transactions accompanied with a non-zero
        /// delay argument are queued here in case of AT abstraction level.
        tlm_utils::peq_with_get<tlm::tlm_generic_payload> peq;

        /// AHB slave base address and size
        const uint32_t ahbBaseAddress;
        // size is saved in bytes
        const uint32_t ahbSize;
        const uint32_t hmask;
        const uint32_t haddr;

	/// 12 bit MSB address and mask (constructor parameters)
	const uint32_t mhaddr;
	const uint32_t mhmask;

	/// Clock cycle time
	sc_core::sc_time clockcycle;
};

#endif
