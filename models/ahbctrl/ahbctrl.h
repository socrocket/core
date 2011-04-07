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

class CAHBCTRL : public sc_core::sc_module {
    public:

        /// AMBA sockets
        amba::amba_slave_socket<32, 0>  ahbIN;
        amba::amba_master_socket<32, 0> ahbOUT;

        SC_HAS_PROCESS(CAHBCTRL);
        /// Constructor
        CAHBCTRL(sc_core::sc_module_name nm,
                    amba::amba_layer_ids ambaLayer = amba::amba_LT);

        /// Desctructor
        ~CAHBCTRL();

        void setAddressMap(const uint32_t i,
                           const uint32_t baseAddr,
                           const uint32_t size);


        /// TLM blocking transport method
        void b_transport(uint32_t id, tlm::tlm_generic_payload& gp, sc_core::sc_time& delay);

        /// TLM non blocking forward path
        tlm::tlm_sync_enum nb_transport_fw(uint32_t id, tlm::tlm_generic_payload& gp,
                                           tlm::tlm_phase& phase, sc_core::sc_time& delay);

        /// TLM non blocking backward path
        tlm::tlm_sync_enum nb_transport_bw(uint32_t id, tlm::tlm_generic_payload& gp,
                                           tlm::tlm_phase& phase, sc_core::sc_time& delay);

        /// TLM debug interface
        unsigned int transport_dbg(uint32_t id, tlm::tlm_generic_payload& gp);

        /// Check memory map for overlaps
        void checkMemMap();

	/// Helper functions for definition of clock cycle
	void clk(sc_core::sc_clock &clk);
	void clk(sc_core::sc_time &period);
	void clk(double period, sc_core::sc_time_unit base);

    private:
        typedef tlm::tlm_generic_payload payload_t;
        typedef gs::socket::bindability_base<tlm::tlm_base_protocol_types>
                socket_t;
        typedef std::pair<uint32_t, uint32_t> slave_info_t;
        std::map<uint32_t, slave_info_t> slave_map;
        std::map<uint32_t, int32_t> MstSlvMap;
        std::map<uint32_t, sc_core::sc_semaphore*> SlvSemaphore;

        // Get slave index for a given address
        int get_index(const uint32_t address);

        // Get master index for a given slave
        int getMaster2Slave(const uint32_t slaveID);

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
