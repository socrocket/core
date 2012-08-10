//*****************************************************************************
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
//*****************************************************************************
// Title:      powermonitor.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Demonstration of a power monitor for the SoCRocket
//             Virtual Platform
//
// Modified on $Date: 2011-06-09 08:49:53 +0200 (Thu, 09 Jun 2011) $
//          at $Revision: 452 $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*****************************************************************************

#ifndef POWERMONITOR_H
#define POWERMONITOR_H

#include <systemc>
#include <boost/algorithm/string.hpp>
#include "greencontrol/config.h"

#include "stdint.h"
#include "verbose.h"

#include <iomanip>

// Power monitor demonstrator
class powermonitor : public sc_module {

 public:

  // Generate report
  void gen_report();

  // Triggers report generation 
  void report_trigger();

  std::string get_model_name(std::string &param);

  std::vector<std::string> get_IP_params(std::vector<std::string> &params);

  // Called by systemc scheduler at end of simulation
  void end_of_simulation();

  SC_HAS_PROCESS(powermonitor);

  // Constructor
  powermonitor(sc_core::sc_module_name name, sc_core::sc_time m_report_time = SC_ZERO_TIME);

  sc_core::sc_time m_report_time;

};

#endif // POWERMONITOR_H
