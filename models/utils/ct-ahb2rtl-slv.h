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
// Title:      ct-ahb2rtl-slv.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file defining generic AHB CT to RTL SLV adapter
//             it enables the user to connect a grlib AHB slave model
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

Cct_ahb2rtl_slv::Cct_ahb2rtl_slv(sc_core::sc_module_name nm, sc_dt::uint64 base,
                         sc_dt::uint64 size) :
    sc_module(nm),
    clk("CLOCK"),
    reset("RESET"),
    ahbso("ahbso"),
    ahbsi("ahbsi"),
    hirqi("GR_IRQ_IN"),
    hirqo("GR_IRQ_OUT"),
    hconfig_0("GR_CONFIG_0"),
    hconfig_1("GR_CONFIG_1"),
    hconfig_2("GR_CONFIG_2"),
    hconfig_3("GR_CONFIG_3"),
    hconfig_4("GR_CONFIG_4"),
    hconfig_5("GR_CONFIG_5"),
    hconfig_6("GR_CONFIG_6"),
    hconfig_7("GR_CONFIG_7"),
    hindex("GR_INDEX"),
	  m_hsel("AHB_HSEL"),
	  m_hwrite("AHB_HWRITE"),
	  m_htrans("AHB_HTRANS"),
	  m_haddr("AHB_HADDR"),
	  m_hsize("AHB_HSIZE"),
	  m_hburst("AHB_HBURST"),
	  m_hprot("AHB_HPROT"),
	  m_hreadyin("AHB_HREADYIN"),
	  m_hwdata("AHB_HWDATA"),
	  m_hreadyout("AHB_HREADYOUT"),
	  m_hresp("AHB_HRESP"),
	  m_hrdata("AHB_HRDATA"),
	  m_hmaster("AHB_HMASTER"),
	  m_hmastlock("AHB_MASTLOCK"),
	  m_hsplit("AHB_HSPLIT"),
    ct("CT", base, size) {
    ct.m_clk(clk);
    ct.m_Reset(reset);
    ct.m_HSEL(m_hsel);
    ct.m_HWRITE(m_hwrite);
    ct.m_HTRANS(m_htrans);
    ct.m_HADDR(m_haddr);
    ct.m_HSIZE(m_hsize);
    ct.m_HBURST(m_hburst);
    ct.m_HPROT(m_hprot);
    ct.m_HREADYIN(m_hreadyin);
    ct.m_HWDATA(m_hwdata);
    ct.m_HMASTER(m_hmaster);
    ct.m_HMASTLOCK(m_hmastlock);
    ct.m_HREADYOUT(m_hreadyout);
    ct.m_HRESP(m_hresp);
    ct.m_HRDATA(m_hrdata);
    ct.m_HSPLIT(m_hsplit);

    SC_THREAD(ahbso_ctrl);
    sensitive << ahbso;

    SC_THREAD(ahbsi_ctrl);
    sensitive << m_hsel << m_hwrite << m_htrans << m_haddr << m_hsize << m_hburst << m_hprot << m_hreadyin << m_hwdata << m_hmaster << m_hmastlock;
}

void Cct_ahb2rtl_slv::ahbso_ctrl() {
    while (1) {
        ahb_slv_out_type val = ahbso.read();

        m_hreadyout.write(!val.hready);
        m_hresp.write(val.hresp);
        m_hrdata.write(val.hrdata);
        m_hsplit.write(val.hsplit);

        hirqo.write(val.hirq);
        hconfig_0.write(val.hconfig[0]);
        hconfig_1.write(val.hconfig[1]);
        hconfig_2.write(val.hconfig[2]);
        hconfig_3.write(val.hconfig[3]);
        hconfig_4.write(val.hconfig[4]);
        hconfig_5.write(val.hconfig[5]);
        hconfig_6.write(val.hconfig[6]);
        hconfig_7.write(val.hconfig[7]);
        hindex.write(val.hindex);

        wait();
    }
}

void Cct_ahb2rtl_slv::ahbsi_ctrl() {
    while (1) {
        ahb_slv_in_type val;

        val.hsel      = m_hsel.read();
        val.hwrite    = m_hwrite.read();
        val.htrans    = m_htrans.read();
        val.haddr     = m_haddr.read();
        val.hsize     = m_hsize.read();
        val.hburst    = m_hburst.read();
        val.hprot     = m_hprot.read();
        val.hready    = m_hreadyin.read();
        val.hwdata    = m_hwdata.read();
        val.hmaster   = m_hmaster.read();
        val.hmastlock = m_hmastlock.read();

        val.hirq       = hirqi.read();
        val.hmbsel     = hmbsel.read();
//        val.hirq       = 0;
//        val.hmbsel     = 0xF;

        ahbsi.write(val);

        wait();
    }
}

/// @}

#endif
