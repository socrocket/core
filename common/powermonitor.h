// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file powermonitor.h
/// Demonstration of a power monitor for the SoCRocket Virtual Platform
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

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
  powermonitor(sc_core::sc_module_name name, sc_core::sc_time m_report_time = SC_ZERO_TIME, bool exram=false);

  // Local variables for constructor parameters
  sc_core::sc_time m_report_time;
  bool m_exram;
};

#endif // POWERMONITOR_H
/// @}