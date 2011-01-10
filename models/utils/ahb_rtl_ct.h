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
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TU Braunschweig
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef AHB_CT_RTL_ADAPTER
#define AHB_CT_RTL_ADAPTER

#include <systemc.h>
#include <amba.h>
#include <adapters/AHB_Master_RTL_CT_Adapter.h>
#include <signalkit.h>

/// @addtogroup utils Model Utils
/// @{

/// This class is a specific adapter to connect an
/// GRLIB AHH Master to an AMBAKit AHB CT Bus.
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

class CAHB_RTL_CT : public sc_module, public signalkit::signal_module<CAHB_RTL_CT> {
    public:

        SC_HAS_PROCESS(CAHB_RTL_CT);
        sc_core::sc_in_clk clk;
        signal<bool>::in reset;

        sc_core::sc_in<ahb_mst_out_type_adapter> ahbo;
        sc_core::sc_out<ahb_mst_in_type> ahbi;

        signal<uint32_t>::in hirqi;
        signal<uint32_t>::out hirqo;
        signal<uint32_t>::out hconfig_0;
        signal<uint32_t>::out hconfig_1;
        signal<uint16_t>::out hindex;

        /// A small subclass which wraps the core functionality inhireted by amba::AHB_Master_RTL_CT_Adapter
        /// It has knowledge about addressdecoding and translates between the TLM Port and RTL Signals.
        /// But we need another class to map the RTL Signals to GRLIB Signals.
        class ct : public AHB_Master_RTL_CT_Adapter<32, ct> ,
                   public amba_slave_base {
            public:
                ct(sc_core::sc_module_name nm, sc_dt::uint64 base,
                   sc_dt::uint64 size) :
                    AHB_Master_RTL_CT_Adapter<32, ct> (nm, 1), m_base(base),
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
        sc_core::sc_signal<bool> m_reset;
        sc_core::sc_signal<bool> m_hbusreq;
        sc_core::sc_signal<bool> m_hlock;
        sc_core::sc_signal<sc_dt::sc_uint<2> > m_htrans;
        sc_core::sc_signal<sc_dt::sc_uint<32> > m_haddr;
        sc_core::sc_signal<sc_dt::sc_uint<32> > m_hirqi;
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
        CAHB_RTL_CT(sc_core::sc_module_name nm, sc_dt::uint64 base, sc_dt::uint64 size);

        void onreset(const bool &value, const sc_core::sc_time &time);

        void onirq(const uint32_t &value, const sc_core::sc_time &time);
        
        /// Takes ahbo inputs and converts them into TLM communication and irq signals.
        void ahbo_ctrl();

        /// Collectes all data from the input ports and writes them into the ahbi record for the GRLIB Model.
        void ahbi_ctrl();
};

/// @}

CAHB_RTL_CT::CAHB_RTL_CT(sc_core::sc_module_name nm, sc_dt::uint64 base,
                         sc_dt::uint64 size) :
    sc_module(nm), clk("CLOCK"), reset(&CAHB_RTL_CT::onreset, "RESET"), 
    ahbo("ahbo"), ahbi("ahbi"), hirqi(&CAHB_RTL_CT::onirq, "GR_IRQ_IN"), 
    hirqo("GR_IRQ_OUT"), hconfig_0("GR_CONFIG_0"), hconfig_1("GR_CONFIG_1"), 
    hindex("GR_INDEX"), m_reset("RESET_INTERN"), m_hbusreq("AHB_BUSREQ"), 
    m_hlock("AHB_LOCK"), m_htrans("AHB_TRANS"), m_haddr("AHB_ADDRESS"), m_hirqi("GR_IRQ_IN_INTERN"), 
    m_hsize("AHB_SIZE"), m_hburst("AHB_BURST"),m_hprot("AHB_PROT"), 
    m_hwrite("AHB_WRITE"), m_hwdata("AHB_WRITE_DATA"), 
    m_hrdata("AHB_READ_DATA"), m_hresp("AHB_RESPONSE"), m_hgrant("AHB_GRANT"), 
    m_hreadyin("AHB_READY_IN"), ct("CT", base, size) {
    ct.m_clk(clk);
    ct.m_Reset(m_reset);
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
    sensitive << ahbo;

    SC_THREAD(ahbi_ctrl);
    sensitive << m_hrdata << m_hresp << m_hgrant << m_hreadyin << m_hirqi;
}

void CAHB_RTL_CT::onreset(const bool &value, 
                          const sc_core::sc_time &time) {
      m_reset.write(value);
}

void CAHB_RTL_CT::onirq(const uint32_t &value, 
                        const sc_core::sc_time &time) {
      m_hirqi.write(value);
}

void CAHB_RTL_CT::ahbo_ctrl() {
    while (1) {
        ahb_mst_out_type_adapter val = ahbo.read();
        m_hbusreq.write(val.hbusreq.to_bool());
        m_hlock.write(val.hlock.to_bool());
        m_htrans.write(val.htrans);
        m_haddr.write(val.haddr);
        m_hwrite.write(val.hwrite.to_bool());
        m_hsize.write(val.hsize);
        m_hburst.write(val.hburst);
        m_hprot.write(val.hprot);
        m_hwdata.write(val.hwdata);

        hirqo.write(val.hirq.to_uint());
        hconfig_0.write(val.hconfig[0].to_uint());
        hconfig_1.write(val.hconfig[1].to_uint());
        hindex.write(val.hindex);
        wait();
    }
}

void CAHB_RTL_CT::ahbi_ctrl() {
    while (1) {
        ahb_mst_in_type val;
        val.hgrant = (m_hgrant.read())? 0xFFFFFFFF : 0x0;
        val.hready = m_hreadyin.read();
        val.hresp = m_hresp.read();
        val.hrdata = m_hrdata.read();
        val.hcache = 0;
        val.hirq = hirqi.read();
        val.testen = false;
        val.testrst = false;
        val.scanen = false;
        val.testoen = false;
        ahbi.write(val);
        wait();
    }
}

#endif
