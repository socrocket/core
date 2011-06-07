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
// Title:      clock_gen.h
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

#ifndef __CLOCK_GEN_H__
#define __CLOCK_GEN_H__

#include "tlm.h"

class clock_gen : public sc_core::sc_module {

 public:
  
  // ****** PORTS
  sc_core::sc_out<bool> rst;
  sc_core::sc_out<sc_dt::sc_logic> signal_rst;
  sc_core::sc_out<sc_dt::sc_logic> signal_clk;

  // Systemc master clock
  sc_core::sc_clock clock;

  // ****** Member functions
  void gen_signal_clk();
  void gen_reset();

  SC_HAS_PROCESS(clock_gen);

  // Constructor
  clock_gen(sc_core::sc_module_name name, sc_core::sc_time clockcycle, sc_core::sc_time reset_switch, bool reset_init);

  // Clock cycle time
  sc_core::sc_time m_clockcycle;
  sc_core::sc_time m_reset_switch;
  
  bool m_reset_init;

};

#endif // _CLOCK_GEN_H__ 
