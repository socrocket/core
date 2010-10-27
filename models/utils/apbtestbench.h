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
// Title:      apbtestbench.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file defining the a generic APB testbench
//             template to use for systemc or vhdl simulation
//             all implementations are included for
//             maximum inline optiisatzion
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TU Braunschweig
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef APB_TESTBENCH_H
#define APB_TESTBENCH_H

#include "amba.h"

/// @addtogroup utils
/// @{

///macro to print a register value
#define REG(name) { \
            << ": 0x" \
            << std::hex \
            << std::setfill('0') \
            << std::setw(2) \
            << read(name, 4); \
}

/// Testbench for GPTimer Models.
class CAPBTestbench : public sc_core::sc_module {
    public:
        /// TLM Interface
        amba::amba_master_socket<32> master_sock;

        /// Constructor
        CAPBTestbench(sc_core::sc_module_name nm);
        /// Destructor
        ~CAPBTestbench();

        /// Define TLM write transactions
        inline void write(uint32_t addr, uint32_t data, uint32_t width) {
            sc_core::sc_time t;
            tlm::tlm_generic_payload *gp = master_sock.get_transaction();
            gp->set_command(tlm::TLM_WRITE_COMMAND);
            gp->set_address(addr);
            gp->set_data_length(width);
            gp->set_streaming_width(4);
            gp->set_byte_enable_ptr(NULL);
            gp->set_data_ptr((unsigned char*)&data);
            master_sock->b_transport(*gp, t);
            wait(t);
            master_sock.release_transaction(gp);
        }

        /// Define TLM read transactions
        inline uint32_t read(uint32_t addr, uint32_t width) {
            sc_core::sc_time t;
            uint32_t data;
            tlm::tlm_generic_payload *gp = master_sock.get_transaction();
            gp->set_command(tlm::TLM_READ_COMMAND);
            gp->set_address(addr);
            gp->set_data_length(width);
            gp->set_streaming_width(4);
            gp->set_byte_enable_ptr(NULL);
            gp->set_data_ptr((unsigned char*)&data);
            gp->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
            master_sock->b_transport(*gp, t);
            wait(t);
            master_sock.release_transaction(gp);
            return data;
        }

        inline void set(const uint32_t &addr, const uint32_t &data) {
            write(addr, data, 4);
        }

        inline uint32_t get(const uint32_t &addr) {
            return read(addr, 4);
        }
};

/// @}

#endif // APB_TESTBENCH_H
