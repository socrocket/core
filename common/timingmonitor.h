// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file timingmonitor.h
/// The timing monitor can be used to track progress, performance and simulation
/// time of testbenches.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef COMMON_TIMINGMONITOR_H_
#define COMMON_TIMINGMONITOR_H_

#include <tlm.h>
#include <ctime>
#include <map>

#include "core/common/verbose.h"

class TimingMonitor {
  public:
    /// Content of a timing_map entry
    typedef struct {
      char *name;                 // name of phase
      sc_core::sc_time st_start;  // sim time start
      sc_core::sc_time st_end;    // sim time end
      clock_t rt_start;           // real time start
      clock_t rt_end;             // real time end
    } t_timing_rec;

    /// Data structure for timing information
    typedef std::map<unsigned int, t_timing_rec> t_timing_map;
    /// Iterator for timing information
    typedef t_timing_map::iterator t_timing_it;

    /// The map for tracking phases & time
    static t_timing_map timing_map;

    /// Create a new timing record and set starting time
    static void phase_start_timing(const unsigned int id, const char *name = "");
    /// Enter phase finishing time
    static void phase_end_timing(const unsigned int id);
    /// Return simulation time of phase id
    static sc_core::sc_time phase_systime(const unsigned int id);
    /// Return real time for processing phase id
    static double phase_realtime(const unsigned int id);
    /// Return name of phase id
    static const char*phase_get_name(const unsigned int id);
    /// Generate timing report for all phases at 'info' level
    static void report_timing();
};

#endif  // COMMON_TIMINGMONITOR_H_
/// @}
