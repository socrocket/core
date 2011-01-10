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
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TU Braunschweig
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#include <systemc.h>
#include "ct-ahb2rtl-slv.h"

/// @addtogroup utils Model Utils
/// @{

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
