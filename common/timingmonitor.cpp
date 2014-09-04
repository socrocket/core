// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file timingmonitor.cpp
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

#include "core/common/timingmonitor.h"

// Initialize timing map (static - must be done outside class def)
TimingMonitor::t_timing_map TimingMonitor::timing_map;

// Create a new timing record and set starting time
void TimingMonitor::phase_start_timing(const unsigned int id, const char *name) {
  t_timing_rec tmp;

  tmp.name = const_cast<char *>(name);
  tmp.st_start = sc_core::sc_time_stamp();
  tmp.rt_start = std::clock();
  tmp.st_end = sc_core::SC_ZERO_TIME;
  tmp.rt_end = std::clock();

  timing_map[id] = tmp;
}

// Enter phase finishing time
void TimingMonitor::phase_end_timing(const unsigned int id) {
  t_timing_rec tmp;
  t_timing_it it;

  it = timing_map.find(id);

  // Forbidden to set end timing before creating phase (calling phase_start_timing)
  if (it == timing_map.end()) {
    v::error << "TimingMonitor" << "No such phase: " << id << v::endl;
  }

  tmp = it->second;

  tmp.st_end = sc_core::sc_time_stamp();
  tmp.rt_end = std::clock();

  timing_map[id] = tmp;
}

// Return simulation time of phase id
sc_core::sc_time TimingMonitor::phase_systime(const unsigned int id) {
  t_timing_it it;

  it = timing_map.find(id);

  // Phase/key must exist in timing_map
  if (it == timing_map.end()) {
    v::error << "TimingMonitor" << "No such phase: " << id << v::endl;

    return sc_core::sc_time(0, sc_core::SC_NS);
  }

  return (it->second).st_end - (it->second).st_start;
}

// Return real time for processing phase id
double TimingMonitor::phase_realtime(const unsigned int id) {
  t_timing_it it;

  clock_t start_clock;
  clock_t end_clock;
  double diff_sec;

  it = timing_map.find(id);

  // Phase/key must exist in timing_map
  if (it == timing_map.end()) {
    v::error << "TimingMonitor" << "No such phase: " << id << v::endl;

    return 0;
  }

  start_clock = (it->second).rt_start;
  end_clock   = (it->second).rt_end;

  diff_sec = (end_clock - start_clock) / static_cast<double>(CLOCKS_PER_SEC);

  return diff_sec;
}

// Return name of phase id
const char *TimingMonitor::phase_get_name(const unsigned int id) {
  t_timing_it it;

  it = timing_map.find(id);

  // Phase/key must exist in timing_map
  if (it == timing_map.end()) {
    v::error << "TimingMonitor" << "No such phase: " << id << v::endl;

    return "Unknown Phase";
  }

  return (it->second).name;
}

// Generate timing report for all phases at 'info' level.
void TimingMonitor::report_timing() {
  unsigned int id;
  t_timing_rec tmp;
  t_timing_it it;

  it = timing_map.begin();

  v::report << "TimingMonitor" << "******************************************************************" << v::endl;
  v::report << "TimingMonitor" << "* TIMING SUMMARY " << v::endl;
  v::report << "TimingMonitor" << "* -------------- " << v::endl;
  v::report << "TimingMonitor" << " Phase, Name, SystemC Time, SystemC Start, SystemC End, Real Time (sec)" << v::endl;

  // Walk through timing map
  while (it != timing_map.end()) {
    id  = it->first;
    tmp = it->second;

    v::report << "TimingMonitor" << id << ", " << ((it->second).name) << ", " << TimingMonitor::phase_systime(id) <<
    ", " << tmp.st_start << ", " << tmp.st_end << ", " << TimingMonitor::phase_realtime(id) << v::endl;
    // v::report << "TimingMonitor" << "* Phase: " << id << v::endl;
    // v::report << "TimingMonitor" << "* Name: " << ((it->second).name) << v::endl;
    // v::report << "TimingMonitor" << "* SystemC Time: " << TimingMonitor::phase_systime(id)
    //  << " (Start: " << tmp.st_start << " End: " << tmp.st_end << ")" << v::endl;
    // v::report << "TimingMonitor" << "* Real Time: " << TimingMonitor::phase_realtime(id) << " sec " << v::endl;
    // v::report << "TimingMonitor" << "* --------------------------------------------------------------" << v::endl;

    it++;
  }

  v::report << "TimingMonitor" << "******************************************************************" << v::endl;
}
/// @}
