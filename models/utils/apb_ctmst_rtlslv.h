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
// Title:      apb_ctmst_rtlslv.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file defining a generic APB CT to RTL adapter
//             it enables the user to connect an grlib APB model
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

#ifndef APB_CTMST_RTLSLV_ADAPTER
#define APB_CTMST_RTLSLV_ADAPTER

#include <systemc.h>
#include <amba.h>
#include <adapters/APB_CT_RTL_Slave_Adapter.h>
#include "signalkit.h"
#include "verbose.h"

/// @addtogroup utils
/// @{

/// This class is a specific adapter to connect an
/// GRLIB APH Model to an AMBAKit APB CT Bus.
///
///>  Interrupt channels to the TLM components.
///>  |              clk  rst - needed by the adapter
///>  |                |  |
///>  | The TLM Bus  +------+
///>  |  ct.socket --| APB  |-- apbo
///>  |              |  CT  |        connected to the RTL Model
///>  pirqi, pirqo --|  RTL |-- apbi
///>                 +------+
///>                  | |  |
///>          pconfig_0/1  pindex
///>          Device mapper output
///
class CAPB_CT_RTL : public sc_module, public signalkit::signal_module<CAPB_CT_RTL> {
    public:
        /// A small subclass wich wraps the core functionality inhireted by amba::APB_CT_RTL_Slave_Adapter
        /// It has knowledge about addressdecoding and translates between the TLM Port and RTL Signals.
        /// But we need another class to map the RTL Signals to GRLIB Signals.
        class ct : public APB_CT_RTL_Slave_Adapter<ct> , public amba_slave_base {
            public:
                ct(sc_core::sc_module_name nm, sc_dt::uint64 base,
                   sc_dt::uint64 size) :
                    APB_CT_RTL_Slave_Adapter<ct, 32> (nm), m_base(base),
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

        SC_HAS_PROCESS( CAPB_CT_RTL);
        sc_core::sc_in_clk clk;
        signal<bool>::in rst;

        sc_core::sc_in<apb_slv_out_type> apbo;
        sc_core::sc_out<apb_slv_in_type> apbi;

        signal<bool>::infield        pirqi;
        signal<bool>::selector       pirqo;
        signal<uint32_t>::out        pconfig_0;
        signal<uint32_t>::out        pconfig_1;
        signal<uint16_t>::out        pindex;

    private:
        // Internal Signals to rerout the RTL Signals form the AMBA Adapter to the GRLIB format.
        sc_core::sc_signal<bool> m_reset;
        sc_core::sc_signal<bool> m_psel;
        sc_core::sc_signal<bool> m_penable;
        sc_core::sc_signal<bool> m_pwrite;
        sc_core::sc_signal<sc_dt::sc_uint<32> > m_paddr;
        sc_core::sc_signal<sc_dt::sc_uint<32> > m_pwdata;
        sc_core::sc_signal<bool> m_pready;
        sc_core::sc_signal<sc_dt::sc_uint<32> > m_prdata;
        sc_core::sc_signal<bool> m_pslverr;
        sc_core::sc_signal<sc_dt::sc_uint<32> > m_irqi;

    public:
        ct ct;

    public:
        /// Constructor: Simply give name, baseaddress and size as an argument.
        /// After construction ensure that interrupt ports apbi apbo and the TLM Port
        /// are connected before starting the simulation.
        CAPB_CT_RTL(sc_core::sc_module_name nm, sc_dt::uint64 base,
                    sc_dt::uint64 size);

        void onreset(const bool &value, const sc_core::sc_time &time);

        void onirq(const bool &value, const uint32_t &channel, const sc_core::sc_time &time);

        /// Takes apbo inputs and converts them into TLM communication and irq signals.
        void apbo_ctrl();

        /// Collectes all data from the input ports and writes them into the apbi record for the GRLIB Model.
        void apbi_ctrl();
};

CAPB_CT_RTL::CAPB_CT_RTL(sc_core::sc_module_name nm, sc_dt::uint64 base,
                         sc_dt::uint64 size) :
    sc_module(nm), clk("CLOCK"), rst(&CAPB_CT_RTL::onreset, "RESET"), 
    apbo("apbo"), apbi("apbi"), pirqi(&CAPB_CT_RTL::onirq, "GR_IRQ_IN"),
    pirqo("GR_IRQ_OUT"), pconfig_0("GR_CONFIG_0"), pconfig_1("GR_CONFIG_1"), 
    pindex("GR_INDEX"), m_psel("APB_SELECT"), m_penable("APB_ENABLE"), 
    m_pwrite("APB_WRITE"), m_paddr("APB_ADDRESS"), m_pwdata("APB_WRITE_DATA"), 
    m_pready("APB_READY"), m_prdata("APB_READ_DATA"), 
    m_pslverr("APB_SLAVE_ERROR"), m_reset("RESET_INTERN"), 
    m_irqi("IRQ_IN_INTERN"), ct("CT", base, size) {
      
    ct.m_clk(clk);
    ct.m_Reset(m_reset);
    m_reset = 1;
    ct.m_psel(m_psel);
    ct.m_penable(m_penable);
    ct.m_pwrite(m_pwrite);
    ct.m_paddr(m_paddr);
    ct.m_pwdata(m_pwdata);
    ct.m_pready(m_pready);
    m_pready = 1;
    ct.m_prdata(m_prdata);
    ct.m_pslverr(m_pslverr);
    m_pslverr = 0;

    SC_THREAD(apbo_ctrl);
    sensitive << apbo;

    SC_THREAD(apbi_ctrl);
    sensitive << m_psel << m_penable << m_paddr << m_pwrite << m_pwdata
            << m_irqi;  
}

void CAPB_CT_RTL::onreset(const bool &value, 
                          const sc_core::sc_time &time) {
    m_reset.write(value);
}

void CAPB_CT_RTL::onirq(const bool &value, const uint32_t &channel,
                        const sc_core::sc_time &time) {
    static uint32_t status = 0;
    status = (value << channel) | (status & ~(1 << channel));
    m_irqi.write(status);
}

void CAPB_CT_RTL::apbo_ctrl() {
    static uint32_t oldirq = 0;
    while(1) {
        apb_slv_out_type val = apbo.read();
        m_prdata.write(val.prdata);
        if(!(oldirq == val.pirq)) {
            for(int i=0;i<32;++i) {
                if(oldirq&(~val.pirq)&(1<<i)) {
                    pirqo.write((1 << i), false);
                }
                if(val.pirq&(~oldirq)&(1<<i)) {
                    pirqo.write((1 << i), true);
                }
            }
            oldirq = val.pirq;
        }
        wait();
    }
}


void CAPB_CT_RTL::apbi_ctrl() {
    while(1) {
        apb_slv_in_type val;
        val.psel = ((bool)m_psel.read())? 0xFFFF : 0x0;
        val.penable = m_penable.read();
        val.paddr = m_paddr.read();
        val.pwrite = m_pwrite.read();
        val.pwdata = m_pwdata.read();
        val.pirq = m_irqi.read();
        val.testen = false;
        val.scanen = false;
        val.testoen = false;
        apbi.write(val);
        wait();
    }
}

#endif
