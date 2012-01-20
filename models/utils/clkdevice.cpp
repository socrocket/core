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
// Title:      clkdevice.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    implementation of clkdevice base class
//
// Method:
//
// Modified on $Date: 2011-08-04 16:51:14 +0200 (Thu, 04 Aug 2011) $
//          at $Revision: 480 $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#include "clkdevice.h"
#include "verbose.h"

CLKDevice::CLKDevice() : 
    rst(&CLKDevice::onrst, "Reset"), 
    clk(&CLKDevice::onclk, "Clock"),
    clock_cycle(10, sc_core::SC_NS) {
}

CLKDevice::~CLKDevice() {
}

void CLKDevice::onrst(const bool &value, const sc_core::sc_time &time) {
    // The reset is active while the input is false.
    // On the rising edge we want to perform the reset:
    if(value==true) {
        v::debug << "CLKDevice" << "Initiating Reset" << v::endl;
        dorst();
    }
}

void CLKDevice::onclk(const sc_core::sc_time &value, const sc_core::sc_time &time) {
    clock_cycle = value;
    clkcng();
}

// Extract basic cycle rate from a sc_clock
void CLKDevice::set_clk(sc_core::sc_clock &clk) {
    clock_cycle = clk.period();
    clkcng();
}

// Extract basic cycle rate from a clock period
void CLKDevice::set_clk(sc_core::sc_time period) {
    clock_cycle = period;
    clkcng();
}

// Extract basic cycle rate from a clock period in double
void CLKDevice::set_clk(double period, sc_core::sc_time_unit base) {
    clock_cycle = sc_time(period, base);
    clkcng();
}

void CLKDevice::dorst() {
    v::debug << "CLKDevice" << "Doing Reset" << v::endl;
}
