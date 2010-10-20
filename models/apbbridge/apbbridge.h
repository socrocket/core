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
// Title:      apbbridge.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implements an LT/AT AHB APB Bridge
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#ifndef APBBRIDGE_H
#define APBBRIDGE_H

#include <systemc>
#include "amba.h"
#include "verbose.h"

/// 
class apb_bridge : public sc_core::sc_module, public amba_slave_base {
    public:
        SC_HAS_PROCESS(apb_bridge);
        apb_bridge(sc_core::sc_module_name nm, sc_dt::uint64 bAddr,
                   sc_dt::uint64 mSize);
        ~apb_bridge();

        amba::amba_slave_socket<32> ahb;
        amba::amba_master_socket<32, 0> apb;
        inline sc_dt::uint64 get_size();
        inline sc_dt::uint64 get_base_addr();
        inline void setAddressMap(uint32_t i, sc_dt::uint64 baseAddr,
                                  sc_dt::uint64 size);
        inline uint32_t get_index(uint32_t address);

        virtual void b_transport(tlm::tlm_generic_payload& gp, sc_time& delay);

    private:
        typedef tlm::tlm_generic_payload payload_t;
        typedef gs::socket::bindability_base<tlm::tlm_base_protocol_types>
                socket_t;
        typedef std::pair<uint32_t, uint32_t> slave_info_t;
        std::map<uint32_t, slave_info_t> slave_map;

        // -- ahb slave --
        sc_dt::uint64 baseAddr; //base address of the slave
        sc_dt::uint64 mem_size; //size of the memory

        void start_of_simulation();
};


#endif // APBBRIDGE_H
