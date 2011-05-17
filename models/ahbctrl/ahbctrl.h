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
// Title:      ahbctrl.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    AHB Controller / Bus model.
//             The decoder collects all AHB request from the masters and
//             forwards them to the appropriate slave.
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: 
// Reviewed:
// ********************************************************************

#ifndef AHBCTRL_H
#define AHBCTRL_H

#include <deque>
#include <systemc>
#include "amba.h"
#include "ahbdevice.h"

class AHBCtrl : public sc_core::sc_module {
    public:

        /// AMBA multi-sockets
        amba::amba_slave_socket<32, 0>  ahbIN;
        amba::amba_master_socket<32, 0> ahbOUT;

	/// Member functions

        /// TLM blocking transport method
        void b_transport(uint32_t id, tlm::tlm_generic_payload& gp, sc_core::sc_time& delay);

        /// TLM non-blocking transport forward (for AHB slave multi-sock)
        tlm::tlm_sync_enum nb_transport_fw(uint32_t id, tlm::tlm_generic_payload& gp,
                                           tlm::tlm_phase& phase, sc_core::sc_time& delay);

        /// TLM non-blocking transport backward (for AHB master multi-sock)
        tlm::tlm_sync_enum nb_transport_bw(uint32_t id, tlm::tlm_generic_payload& gp,
                                           tlm::tlm_phase& phase, sc_core::sc_time& delay);

        /// TLM debug interface
        unsigned int transport_dbg(uint32_t id, tlm::tlm_generic_payload& gp);	

	/// Helper function for creating slave map decoder entries
        void setAddressMap(const uint32_t i, const uint32_t addr, const uint32_t mask);

	/// Get slave index for a given address
        int get_index(const uint32_t address);

	/// Returns a PNP register from the slave configuration area
	unsigned int getPNPReg(const uint32_t address);
	
        /// Check memory map for overlaps
        void checkMemMap();

	/// Helper functions for definition of clock cycle
	void clk(sc_core::sc_clock &clk);
	void clk(sc_core::sc_time &period);
	void clk(double period, sc_core::sc_time_unit base);

        SC_HAS_PROCESS(AHBCtrl);

        /// Constructor
        AHBCtrl(sc_core::sc_module_name nm, // SystemC name
		 unsigned int ioaddr,  // The MSB address of the I/O area
		 unsigned int iomask,  // The I/O area address mask
		 unsigned int cfgaddr, // The MSB address of the configuration area (PNP)
		 unsigned int cfgmask, // The address mask of the configuration area
		 bool rrobin,          // 1 - round robin, 0 - fixed priority arbitration (only AT)
		 bool split,           // Enable support for AHB SPLIT response (only AT)
		 unsigned int defmast, // ID of the default master
		 bool ioen,            // AHB I/O area enable
		 bool fixbrst,         // Enable support for fixed-length bursts
		 bool fpnpen,          // Enable full decoding of PnP configuration records.
		 bool mcheck,          // Check if there are any intersections between core memory regions.
                 amba::amba_layer_ids ambaLayer);		 
		 
		 
	// Omitted parameters:
	// -------------------
	// nahbm  - Number of AHB masters
	// nahbs  - Number of AHB slaves
	// It is checked that the number of binding does not raise above 16.
	// Apart from that the parameters are not required.
	// debug  - Print configuration
	// Not required. Use verbosity outputs instead.
	// icheck - Check bus index
	// Not required.
	// enbusmon - Enable AHB bus monitor
	// assertwarn - Enable assertions for AMBA recommendations.
	// asserterr - Enable assertion for AMBA requirements

        /// Desctructor
        ~AHBCtrl();


    private:

	// The MSB address of the I/O area
	unsigned int mioaddr;
	// The I/O area address mask
	unsigned int miomask;
	// The MSB address of the configuration area (PNP)
	unsigned int mcfgaddr;
	// The address mask of the configuration area
	unsigned int mcfgmask;
	// 1 - round robin, 0 - fixed priority arbitration (only AT)
	bool mrrobin;
	// Enable support for AHB SPLIT response (only AT)
	bool msplit;
	// ID of the default master
	unsigned int mdefmast;
	// AHB I/O area enable
	bool mioen;
	// Enable support for fixed-length bursts
	bool mfixbrst;
	// Enable support for fixed-length bursts
	bool mfpnpen;
	// Check if there are any intersections between core memory regions
	bool mmcheck;

        typedef tlm::tlm_generic_payload payload_t;
        typedef gs::socket::bindability_base<tlm::tlm_base_protocol_types> socket_t;
        typedef std::pair<uint32_t, uint32_t> slave_info_t;

	/// Address decoder table (slave index, (bar addr, mask))
        std::map<uint32_t, slave_info_t> slave_map;
	/// Iterator for slave map
	std::map<uint32_t, slave_info_t>::iterator it;

        std::map<uint32_t, int32_t> MstSlvMap;
        std::map<uint32_t, sc_core::sc_semaphore*> SlvSemaphore;

	/// Array of slave device information (PNP)
	const uint32_t *mSlaves[64];

	/// Array of master device information (PNP)
	const uint32_t *mMasters[64];

        /// Get master index for a given slave
        int getMaster2Slave(const uint32_t slaveID);

	/// Set up slave map and collect plug & play information
        void start_of_simulation();

        // Thread which is spawned in AT model when a busy slave is requested.
        void queuedTrans(const uint32_t mstID, const uint32_t slvID,
                         tlm::tlm_generic_payload& gp,
                         tlm::tlm_phase &phase,
                         sc_core::sc_time &delay);

	/// Clock cycle time
	sc_core::sc_time clockcycle;
};

#endif // AHBCTRL_H
