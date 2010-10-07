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
// Title:      ahb_rtl_ct.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file defining a generic AHB RTL to CT adapter
//             it enables the user to connect an grlib AHB model
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
#include "ahb_rtl_ct.h"

/// @addtogroup utils Model Utils
/// @{

CAHB_RTL_CT::CAHB_CT_RTL(sc_core::sc_module_name nm, sc_dt::uint64 base, sc_dt::uint64 size)
    : sc_module(nm), clk("CLOCK"), reset("RESET"), ahbo("ahbo"), ahbi("ahbi"),
      hirqi("GR_IRQ_IN"), hirqo("GR_IRQ_OUT"), hconfig_0("GR_CONFIG_0"),
      hconfig_1("GR_CONFIG_1"), hindex("GR_INDEX"), m_hbusreq("AHB_BUSREQ"),
      m_hlock("AHB_LOCK"), m_htrans("AHB_TRANS"), m_haddr("AHB_ADDRESS"),
      m_hsize("AHB_SIZE"), m_hburst("AHB_BURST"), m_prot("AHB_PROT"),
      m_hwrite("AHB_WRITE"), m_hwdata("AHB_WRITE_DATA"), m_hrdata("AHB_READ_DATA"),
      m_hresp("AHB_RESPONSE"), m_hgrant("AHB_GRANT"), m_hreadyin("AHB_READY_IN"),
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

    SC_THREAD(ahbo_ctrl);
    sensitive << ahbo;

    SC_THREAD(ahbi_ctrl);
    sensitive << m_hrdata << m_hresp << m_hgrand << m_hreadyin << hirqi;
}

void CAHB_RTL_CT::ahbo_ctrl() {
    while(1) {
        ahb_mst_out_type val = ahbo.read();
        m_hbusreq.write(val.hbusreq);
        m_hlock.write(val.hlock);
        m_htrans.write(val.htrans);
        m_haddr.write(val.haddr);
        m_hwrite.write(val.hwrite);
        m_hsize.write(val.hsize);
        m_hburst.write(val.hburst);
        m_hprot.write(val.hprot);
        m_hwdata.write(val.hwdata);

        hirqo.write(val.hwirq);
        hconfig_0.write(val.hconfig[0]);
        hconfig_1.write(val.hconfig[1]);
        hindex.write(val.hindex);
        wait();
    }
}

void CAHB_RTL_CT::ahbi_ctrl() {
    while(1) {
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
        apbi.write(val);
        wait();
    }
}


/// @}
