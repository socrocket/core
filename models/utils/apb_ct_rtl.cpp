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
// Title:      apb_ct_rtl.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file defining a generic APB CT to RTL adapter
//             it enables the user to connect an grlib APB model
//             to an AMBAKit CT bus
//
// Method:
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

#include <systemc.h>
#include "apb_ct_rtl.h"

/// @addtogroup utils
/// @{

CAPB_CT_RTL::CAPB_CT_RTL(sc_core::sc_module_name nm, sc_dt::uint64 base, sc_dt::uint64 size)
      : sc_module(nm), clk("CLOCK"), rst(&APB_CT_RTL::onreset, "RESET"), apbo("apbo"), apbi("apbi"),
        pirqi(&APB_CT_RTL::onirq, "GR_IRQ_IN"), pirqo("GR_IRQ_OUT"), pconfig_0("GR_CONFIG_0"),
        pconfig_1("GR_CONFIG_1"), pindex("GR_INDEX"), m_psel("APB_SELECT"),
        m_penable("APB_ENABLE"), m_pwrite("APB_WRITE"), m_paddr("APB_ADDRESS"),
        m_pwdata("APB_WRITE_DATA"), m_pready("APB_READY"), m_prdata("APB_READ_DATA"),
        m_pslverr("APB_SLAVE_ERROR"), m_reset("RESET_INTERN"), m_irqi("IRQ_IN_INTERN"), ct("CT", base, size) {
    ct.m_clk(clk);
    ct.m_Reset(m_reset);   m_reset = 1;
    ct.psel(m_psel);
    ct.penable(m_penable);
    ct.pwrite(m_pwrite);
    ct.paddr(m_paddr);
    ct.pwdata(m_pwdata);
    ct.pready(m_pready);   m_pready = 1;
    ct.prdata(m_prdata);
    ct.pslverr(m_pslverr); m_pslverr = 0;

    SC_THREAD(apbo_ctrl);
    sensitive << apbo;

    SC_THREAD(apbi_ctrl);
    sensitive << m_psel << m_penable << m_paddr << m_pwrite << m_pwdata << m_irqi;
}

void CAPB_CT_RTL::onreset(const bool &value, signalkit::signal_in_if<bool> *signal, signalkit::signal_out_if<bool> *sender, const sc_core::sc_time &time) {
    m_reset.write(value);
}

void CAPB_CT_RTL::onirq(const uint32_t &value, signalkit::signal_in_if<uint32_t> *signal, signalkit::signal_out_if<uint32_t> *sender, const sc_core::sc_time &time) {
    m_irqi.write(value);
}

void CAPB_CT_RTL::apbo_ctrl() {
    while(1) {
        apb_slv_out_type val = apbo.read();
        m_prdata.write(val.prdata);
        if(!pirqo==val.pirq) {
          pirqo.write(val.pirq);
        }
        pconfig_0.write(val.pconfig[0]);
        pconfig_1.write(val.pconfig[1]);
        pindex.write(val.pindex);
        wait();
    }
}

void CAPB_CT_RTL::apbi_ctrl() {
    while(1) {
        apb_slv_in_type val;
        val.psel    = ((bool)m_psel.read())? 0xFFFF : 0x0;
        val.penable = m_penable.read();
        val.paddr   = m_paddr.read();
        val.pwrite  = m_pwrite.read();
        val.pwdata  = m_pwdata.read();
        val.pirq    = m_irqi.read();
        val.testen  = false;
        val.scanen  = false;
        val.testoen = false;
        apbi.write(val);
        wait();
    }
}

/// @}
