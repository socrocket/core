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
// Title:      ahbctrl.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    This class contains functions common to all AHBCTRL
//             tests. It implements the blocking and non-blocking
//             TLM-AHB master interface and respective read/write 
//             routines.
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

#ifndef __AHBCTRL_TEST_H__
#define __AHBCTRL_TEST_H__

#include <tlm.h>
#include <ctime>
#include "amba.h"

#include "verbose.h"
#include "ahbdevice.h"

class ahbctrl_test : public sc_module, public AHBDevice {

 public:

  /// AMBA master socket
  amba::amba_master_socket<32, 0> ahb;

  // Member functions
  // ----------------

  /// Write data to ahb
  void ahbwrite(unsigned int addr, unsigned char *data, unsigned int length, unsigned int burst_size);

  /// Read data from ahb
  void ahbread(unsigned int addr, unsigned char *data, unsigned int length, unsigned int burst_size);

  /// Thread for response processing
  void ResponseThread();

  /// Transaction processor
  void processTXN(tlm::tlm_generic_payload* trans);

  /// TLM non-blocking transport backward
  tlm::tlm_sync_enum nb_transport_bw(unsigned int id, tlm::tlm_generic_payload& gp, tlm::tlm_phase& phase, sc_core::sc_time& delay);

  // helpers for timing measurement
  void phase_start_timing();
  void phase_end_timing();
  sc_core::sc_time phase_systime();
  double phase_realtime();

  SC_HAS_PROCESS(ahbctrl_test);

  /// Constructor
  ahbctrl_test(sc_core::sc_module_name, amba::amba_layer_ids abstractionLayer, unsigned int master_id);

  // data members
  // ------------

  // System time for simulation accuracy measurement
  sc_core::sc_time phase_systime_start;
  sc_core::sc_time phase_systime_end;

  // Realtime clock for simulation performance measurement
  clock_t phase_realtime_start;
  clock_t phase_realtime_end;

  /// AMBA abstraction layer (LT/AT)
  amba::amba_layer_ids m_abstractionLayer;

  /// ID of the master socket
  unsigned int m_master_id;

 private:

  /// PEQ for response synchronization
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> mResponsePEQ;

  /// Event triggered by response thread - notifies processTXN about completion of END_RESP
  sc_event mEndResponseEvent;
  /// Event triggered by transport_bw - notifies processTXN about completion of END_REQ
  sc_event mEndRequestEvent;

};  

#endif // __AHBCTRL_TEST_H__
