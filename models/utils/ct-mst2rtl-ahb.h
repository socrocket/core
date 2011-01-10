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

#ifndef CT_AHB_MASTER_RTL_AHB_BUS_ADAPTER
#define CT_AHB_MASTER_RTL_AHB_BUS_ADAPTER
//#include "mmu_cache_wrapper.h"

#include <systemc.h>
#include <amba.h>
//#include <adapters/APB_CT_RTL_Slave_Adapter.h>
#include <adapters/AHB_Master_CT_RTL_Adapter.h>

/// @addtogroup utils Model Utils
/// @{

/// This class is a specific adapter to connect an
/// GRLIB CT Master to an AMBAKit AHB CT Bus.
///
///>  Interrupt channels to the TLM components.
///>  |              clk  rst - needed by the adapter
///>  |                |  |
///>  | The TLM Bus  +------+
///>  |  ct.socket --| AHB  |-- ahbo
///>  |              | RTL  |        connected to the RTL Model
///>  hirqi, hirqo --|  CT  |-- ahbi
///>                 +------+
///>                  | |  |
///>          pconfig_0/1  pindex
///>          Device mapper output
///

class Cmst_ct_rtl_ahb : public sc_module {
    public:

        SC_HAS_PROCESS( Cmst_ct_rtl_ahb);
        sc_core::sc_in_clk clk;
        sc_core::sc_in<bool> reset;

        sc_core::sc_in<ahb_mst_in_type> ahbi;
        sc_core::sc_out<ahb_mst_out_type> ahbo;

        sc_core::sc_in<sc_uint<32> > hirqi;
        sc_core::sc_out<sc_uint<32> > hirqo;
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
        class ct : public AHB_Master_CT_RTL_Adapter<32, ct> ,
                   public amba_slave_base {
            public:
                ct(sc_core::sc_module_name nm, sc_dt::uint64 base,
                   sc_dt::uint64 size) :
                    AHB_Master_CT_RTL_Adapter<32, ct> (nm), m_base(base),
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
        sc_core::sc_signal<bool> m_hbusreq;
        sc_core::sc_signal<bool> m_hlock;
        sc_core::sc_signal<sc_dt::sc_uint<2> > m_htrans;
        sc_core::sc_signal<sc_dt::sc_uint<32> > m_haddr;
        sc_core::sc_signal<sc_dt::sc_uint<3> > m_hsize;
        sc_core::sc_signal<sc_dt::sc_uint<3> > m_hburst;
        sc_core::sc_signal<sc_dt::sc_uint<4> > m_hprot;
        sc_core::sc_signal<bool> m_hwrite;
        sc_core::sc_signal<sc_dt::sc_uint<32> > m_hwdata;

        ///Outgoing signal to AHB RTL Master
        sc_core::sc_signal<sc_dt::sc_uint<32> > m_hrdata;
        sc_core::sc_signal<sc_dt::sc_uint<2> > m_hresp;
        sc_core::sc_signal<bool> m_hgrant;
        sc_core::sc_signal<bool> m_hreadyin;
    public:
        ct ct;

    public:
        /// Constructor: Simply give name, baseaddress and size as an argument.
        /// After construction ensure that interrupt ports ahbi ahbo and the TLM Port
        /// are connected before starting the simulation.
        Cmst_ct_rtl_ahb(sc_core::sc_module_name nm, sc_dt::uint64 base, sc_dt::uint64 size);

        /// Takes ahbo inputs and converts them into TLM communication and irq signals.
        void ahbo_ctrl();

        /// Collectes all data from the input ports and writes them into the ahbi record for the GRLIB Model.
        void ahbi_ctrl();
};

Cmst_ct_rtl_ahb::Cmst_ct_rtl_ahb(sc_core::sc_module_name nm, sc_dt::uint64 base,
                         sc_dt::uint64 size) :
    sc_module(nm),
    clk("CLOCK"),
    reset("RESET"),
    ahbo("ahbo"),
    ahbi("ahbi"),
    hirqi("GR_IRQ_IN"),
    hirqo("GR_IRQ_OUT"),
    hconfig_0("GR_CONFIG_0"),
    hconfig_1("GR_CONFIG_1"),
    hindex("GR_INDEX"),
    m_hbusreq("AHB_BUSREQ"),
    m_hlock("AHB_HLOCK"),
    m_htrans("AHB_HTRANS"),
    m_haddr("AHB_HADDR"),
    m_hsize("AHB_HSIZE"),
    m_hburst("AHB_HBURST"),
    m_hprot("AHB_HPROT"),
    m_hwrite("AHB_HWRITE"),
    m_hwdata("AHB_HWDATA"),
    m_hrdata("AHB_HRDATA"),
    m_hresp("AHB_HRESP"),
    m_hgrant("AHB_HGRANT"),
    m_hreadyin("AHB_HREADYIN"),
    ct("CT", base, size) {
    ct.m_clk(clk);
    ct.m_Reset(reset);
    ct.m_HBUSREQ(m_hbusreq);
    ct.m_HLOCK(m_hlock);
    ct.m_HTRANS(m_htrans);
    ct.m_HADDR(m_haddr);
    ct.m_HSIZE(m_hsize);
    ct.m_HBURST(m_hburst);
    ct.m_HPROT(m_hprot);
    ct.m_HWRITE(m_hwrite);
    ct.m_HWDATA(m_hwdata);
    ct.m_HRDATA(m_hrdata);
    ct.m_HRESP(m_hresp);
    ct.m_HGRANT(m_hgrant);
    ct.m_HREADYIN(m_hreadyin);

    SC_THREAD( ahbo_ctrl);
    sensitive << m_hbusreq << m_hlock << m_htrans << m_haddr << m_hwrite << m_hsize << m_hburst << m_hprot << m_hwdata;

    SC_THREAD(ahbi_ctrl);
    sensitive << ahbi;
}

void Cmst_ct_rtl_ahb::ahbo_ctrl() {
    while (1) {
        ahb_mst_out_type val;
        val.hbusreq = m_hbusreq.read();
        val.hlock   = m_hlock.read();
        val.htrans  = m_htrans.read();
        val.haddr   = m_haddr.read();
        val.hwrite  = m_hwrite.read();
        val.hsize   = m_hsize.read();
        val.hburst  = m_hburst.read();
        val.hprot   = m_hprot.read();
        val.hwdata  = m_hwdata.read();

        val.hirq = hirqi.read();
        val.hconfig[0] = hconfig_0.read();
        val.hconfig[1] = hconfig_1.read();
        val.hconfig[2] = hconfig_2.read();
        val.hconfig[3] = hconfig_3.read();
        val.hconfig[4] = hconfig_4.read();
        val.hconfig[5] = hconfig_5.read();
        val.hconfig[6] = hconfig_6.read();
        val.hconfig[7] = hconfig_7.read();
        val.hindex = hindex.read();
        ahbo.write(val);
        wait();
    }
}

void Cmst_ct_rtl_ahb::ahbi_ctrl() {
    while (1) {
        ahb_mst_in_type val = ahbi.read();
        if (val.hgrant) {
            m_hgrant.write(0xFFFF);
        }
        m_hreadyin.write(val.hready);
        m_hresp.write(val.hresp);
        m_hrdata.write(val.hrdata);
        hirqo.write(val.hirq);

        wait();
    }
}

/// @}

#endif
