// ********************************************************************
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
// ********************************************************************
// Title:      timingmonitor.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    The timing monitor can be used to track
//             progress, performance and simulation time
//             of testbenches.
//             
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
// ********************************************************************

#include "timingmonitor.h"

// Initialize timing map (static - must be done outside class def)
TimingMonitor::t_timing_map TimingMonitor::timing_map;

// Create a new timing record and set starting time
void TimingMonitor::phase_start_timing(const unsigned int id, const char * name) {

  t_timing_rec tmp;

  tmp.name = (char *)name;
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
  assert(it != timing_map.end());

  tmp=it->second;

  tmp.st_end = sc_core::sc_time_stamp();
  tmp.rt_end = std::clock();

  timing_map[id] = tmp;

}

// Return simulation time of phase id
sc_core::sc_time TimingMonitor::phase_systime(const unsigned int id) {

  t_timing_it it;

  it = timing_map.find(id);

  // Phase/key must exist in timing_map
  assert(it != timing_map.end());

  return((it->second).st_end - (it->second).st_start);
}

// Return real time for processing phase id
double TimingMonitor::phase_realtime(const unsigned int id) {

  t_timing_it it;

  clock_t start_clock;
  clock_t end_clock;
  double  diff_sec;

  it = timing_map.find(id);

  // Phase/key must exist in timing_map
  assert(it != timing_map.end());

  start_clock = (it->second).rt_start;
  end_clock   = (it->second).rt_end;

  diff_sec = (end_clock - start_clock)/(double)CLOCKS_PER_SEC;

  return(diff_sec);
}

// Return name of phase id
const char * TimingMonitor::phase_get_name(const unsigned int id) {

  t_timing_it it;

  it = timing_map.find(id);

  // Phase/key must exist in timing_map
  assert(it != timing_map.end());

  return((it->second).name);

}

// Generate timing report for all phases at 'info' level.
void TimingMonitor::report_timing() {

  unsigned int id;
  t_timing_rec tmp;
  t_timing_it it;

  it = timing_map.begin();

  v::info << "TimingMonitor" << "******************************************************************" << v::endl;
  v::info << "TimingMonitor" << "* TIMING SUMMARY " << v::endl;
  v::info << "TimingMonitor" << "* -------------- " << v::endl;

  // Walk through timing map
  while(it != timing_map.end()) {

    id  = it->first;
    tmp = it->second;

    v::info << "TimingMonitor" << "* Phase: " << id << v::endl;
    v::info << "TimingMonitor" << "* Name: " << ((it->second).name) << v::endl;
    v::info << "TimingMonitor" << "* SystemC Time: " << TimingMonitor::phase_systime(id) \
	    << " (Start: " << tmp.st_start << " End: " << tmp.st_end << ")" << v::endl;
    v::info << "TimingMonitor" << "* Real Time: " << TimingMonitor::phase_realtime(id) << " sec " << v::endl;
    v::info << "TimingMonitor" << "* --------------------------------------------------------------" << v::endl;
  
    it++;
  }

  v::info << "TimingMonitor" << "******************************************************************" << v::endl;

}
