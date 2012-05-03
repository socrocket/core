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

#ifndef TIMINGMONITOR_H
#define TIMINGMONITOR_H

#include <tlm.h>
#include <map>
#include <ctime>

#include "verbose.h"

class TimingMonitor {

 public:

  /// Content of a timing_map entry
  typedef struct {

    char * name;                 // name of phase
    sc_core::sc_time st_start;   // sim time start
    sc_core::sc_time st_end;     // sim time end
    clock_t rt_start;            // real time start
    clock_t rt_end;              // real time end
 
  } t_timing_rec;


  /// Data structure for timing information
  typedef std::map<unsigned int, t_timing_rec> t_timing_map;
  /// Iterator for timing information
  typedef t_timing_map::iterator t_timing_it;
  
  /// The map for tracking phases & time
  static t_timing_map timing_map;

  /// Create a new timing record and set starting time
  static void phase_start_timing(const unsigned int id, const char * name = "");
  /// Enter phase finishing time
  static void phase_end_timing(const unsigned int id);
  /// Return simulation time of phase id
  static sc_core::sc_time phase_systime(const unsigned int id);
  /// Return real time for processing phase id
  static double phase_realtime(const unsigned int id);
  /// Return name of phase id
  static const char * phase_get_name(const unsigned int id);
  /// Generate timing report for all phases at 'info' level
  static void report_timing();

};


#endif // TIMINGMONITOR_H
