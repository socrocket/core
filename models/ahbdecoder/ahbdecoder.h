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
// Title:      ahbdecoder.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    AHB address decoder.
//             The decoder collects all AHB request from the masters and
//             forwards them to the appropriate slave.
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Soeren Brinkmann
// Reviewed:
// ********************************************************************

#ifndef AHBDECODER_H
#define AHBDECODER_H

#include <systemc>
#include "amba.h"

class CAHBDecoder : public sc_core::sc_module {
    public:
        SC_HAS_PROCESS(CAHBDecoder);
        /// Constructor
        CAHBDecoder(sc_core::sc_module_name nm,
                    amba::amba_layer_ids ambaLayer = amba::amba_LT);

        /// Desctructor
        ~CAHBDecoder();

        /// AMBA interfaces
        amba::amba_slave_socket<32, 0>  ahbIN;
        amba::amba_master_socket<32, 0> ahbOUT;

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

        /// Check memory map for overlaps
        void checkMemMap();

    private:
        typedef tlm::tlm_generic_payload payload_t;
        typedef gs::socket::bindability_base<tlm::tlm_base_protocol_types>
                socket_t;
        typedef std::pair<uint32_t, uint32_t> slave_info_t;
        std::map<uint32_t, slave_info_t> slave_map;
        std::map<uint32_t, int32_t> MstSlvMap;

        // Get slave index for a given address
        int get_index(const uint32_t address);

        // Get master index for a given slave
        int getMaster2Slave(const uint32_t slaveID);

        void start_of_simulation();
};

#endif // AHBDECODER_H
