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
// Title:      clock_gen.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Generates sc_logic clock and reset signals for
//             co-simulation of RTL components.
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#include "clock_gen.h"

// Constructor
clock_gen::clock_gen(sc_core::sc_module_name name, sc_core::sc_time clockcycle, sc_core::sc_time reset_switch, bool reset_init) :

  sc_module(name),
  clock("clock", clockcycle, 0.5, sc_core::SC_ZERO_TIME, true),
  m_clockcycle(clockcycle),
  m_reset_switch(reset_switch),
  m_reset_init(reset_init) {

    SC_THREAD(gen_signal_clk);
    sensitive << clock;

    SC_THREAD(gen_reset);

}

// Generates a SystemC signal corresponing to clock (for hdl-cosim)
void clock_gen::gen_signal_clk() {

  while(1) {

    wait();

    if (clock.posedge()) {

      signal_clk.write(sc_dt::SC_LOGIC_1);

    } else {

      signal_clk.write(sc_dt::SC_LOGIC_0);
    
    }
  }
}

// Generate boolean and sc_logic version of reset
void clock_gen::gen_reset() {

  // Initialize as selected in constructor
  rst.write(m_reset_init);
  signal_rst.write(sc_dt::sc_logic(m_reset_init));

  wait(m_reset_switch);

  // Do reset (switch)
  rst.write(!m_reset_init);
  signal_rst.write(sc_dt::sc_logic(!m_reset_init));

}

