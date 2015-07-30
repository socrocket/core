// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file powermonitor.h
/// Demonstration of a power monitor for the SoCRocket Virtual Platform
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef COMMON_POWERMONITOR_H_
#define COMMON_POWERMONITOR_H_

#include <stdint.h>
#include <boost/algorithm/string.hpp>
#include <iomanip>
#include <string>
#include <vector>

#include "core/common/sr_param.h"
#include "core/common/systemc.h"
#include "core/common/verbose.h"

// Power monitor demonstrator
class powermonitor : public sc_core::sc_module {
  public:
    // Generate report
    void gen_report();

    // Triggers report generation
    void report_trigger();

    std::string get_model_name(std::string &param);  // NOLINT(runtime/references)

    std::vector<std::string> get_IP_params(std::vector<std::string> &params);  // NOLINT(runtime/references)

    // Called by systemc scheduler at end of simulation
    void end_of_simulation();

    SC_HAS_PROCESS(powermonitor);

    // Constructor
    powermonitor(sc_core::sc_module_name name, sc_core::sc_time m_report_time = sc_core::SC_ZERO_TIME, bool exram = false);

    // Local variables for constructor parameters
    sc_core::sc_time m_report_time;
    bool m_exram;
};

#endif  // COMMON_POWERMONITOR_H_
/// @}
