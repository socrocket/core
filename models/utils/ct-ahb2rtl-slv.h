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
// Title:      ahb_rtl_ct.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file defining a generic AHB RTL to CT adapter
//             it enables the user to connect an grlib AHB model
//             to an AMBAKit CT bus
//
// Modified on $Date$
//          at $Revision$
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TU Braunschweig
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef CT_AHB_TO_RTL_SLV_ADAPTER
#define CT_AHB_TO_RTL_SLV_ADAPTER

#include <systemc.h>
#include <amba.h>
#include <adapters/AHB_CT_RTL_Slave_Adapter.h>
#include "gaisler_record_types.h"

/// @addtogroup utils Model Utils
/// @{

/// This class is a specific adapter to connect an
/// AMBAKit AHB CT Bus to a GRLIB RTL Slave.
///
///>  Interrupt channels to the TLM components.
///>  |              clk  rst - needed by the adapter
///>  |                |  |
///>  | The TLM Bus  +------+
///>  |  ct.socket --| AHB  |-- ahsbo
///>  |              | RTL  |        connected to the RTL Model
///>  hirqi, hirqo --|  CT  |-- ahsbi
///>                 +------+
///>                  | |  |
///>          pconfig_0/1  pindex
///>          Device mapper output
///

class Cct_ahb2rtl_slv : public sc_module {
    public:

        SC_HAS_PROCESS (Cct_ahb2rtl_slv);
        sc_core::sc_in_clk clk;
        sc_core::sc_in<bool> reset;

        sc_core::sc_out<ahb_slv_in_type> ahbsi;
        sc_core::sc_in<ahb_slv_out_type> ahbso;

        sc_core::sc_in<sc_uint<32> > hirqi;
        sc_core::sc_out<sc_uint<32> > hirqo;
        sc_core::sc_out<sc_uint<4> > hmbsel;
        sc_core::sc_out<sc_uint<32> > hconfig_0;
        sc_core::sc_out<sc_uint<32> > hconfig_1;
        sc_core::sc_out<sc_uint<32> > hconfig_2;
        sc_core::sc_out<sc_uint<32> > hconfig_3;
        sc_core::sc_out<sc_uint<32> > hconfig_4;
        sc_core::sc_out<sc_uint<32> > hconfig_5;
        sc_core::sc_out<sc_uint<32> > hconfig_6;
        sc_core::sc_out<sc_uint<32> > hconfig_7;
        sc_core::sc_out<sc_uint<16> > hindex;

        /// A small subclass which wraps the core functionality inhireted by amba::AHB_Master_RTL_CT_Adapter
        /// It has knowledge about addressdecoding and translates between the TLM Port and RTL Signals.
        /// But we need another class to map the RTL Signals to GRLIB Signals.
        class ct : public AHB_CT_RTL_Slave_Adapter<32, ct> ,
                   public amba_slave_base {
            public:
                ct(sc_core::sc_module_name nm, sc_dt::uint64 base,
                   sc_dt::uint64 size) :
                    AHB_CT_RTL_Slave_Adapter<32, ct> (nm), m_base(base),
                            m_size(size) {
                }

                sc_dt::uint64 get_base_addr() {
                    return m_base;
                }
                sc_dt::uint64 get_size() {
                    return m_size;
                }
            private:
                sc_dt::uint64 m_base;
                sc_dt::uint64 m_size;
        };

    private:
        //sc_out of ahb_ct_rtl_slv_adapter (output must be ahbsi signals)
        sc_core::sc_signal<bool> m_hsel;
        sc_core::sc_signal<bool> m_hwrite;
        sc_core::sc_signal<sc_dt::sc_uint<2> > m_htrans;
        sc_core::sc_signal<sc_dt::sc_uint<32> > m_haddr;
        sc_core::sc_signal<sc_dt::sc_uint<3> > m_hsize;
        sc_core::sc_signal<sc_dt::sc_uint<3> > m_hburst;
        sc_core::sc_signal<sc_dt::sc_uint<4> > m_hprot;
        sc_core::sc_signal<bool> m_hreadyin;
        sc_core::sc_signal<sc_dt::sc_uint<32> > m_hwdata;
        sc_core::sc_signal<sc_dt::sc_uint<4> > m_hmaster;
        sc_core::sc_signal<bool> m_hmastlock;

        //sc_in of ahb_ct_rtl_slv_adapter (input must be ahbso signals)
        sc_core::sc_signal<bool> m_hreadyout;
        sc_core::sc_signal<sc_dt::sc_uint<2> > m_hresp;
        sc_core::sc_signal<sc_dt::sc_uint<32> > m_hrdata;
        sc_core::sc_signal<sc_dt::sc_uint<16> > m_hsplit;
    public:
        ct ct;

    public:
        /// Constructor: Simply give name, baseaddress and size as an argument.
        /// After construction ensure that interrupt ports ahbsi ahbso and the TLM Port
        /// are connected before starting the simulation.
        Cct_ahb2rtl_slv(sc_core::sc_module_name nm, sc_dt::uint64 base, sc_dt::uint64 size);

        /// Takes ahbo inputs and converts them into TLM communication and irq signals.
        void ahbso_ctrl();

        /// Collectes all data from the input ports and writes them into the ahbsi record for the GRLIB Model.
        void ahbsi_ctrl();
};

/// @}

#endif
