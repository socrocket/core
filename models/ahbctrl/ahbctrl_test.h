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
// Title:      ahbctrl_test.h
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
#include "socrocket.h"

#include "verbose.h"
#include "ahbdevice.h"
#include "signalkit.h"

class ahbctrl_test : public sc_module, public AHBDevice, public signalkit::signal_module<ahbctrl_test> {

 public:

  /// AMBA master socket
  amba::amba_master_socket<32> ahb;

  /// Snooping port
  signal<t_snoop>::in snoop;

  // Member functions
  // ----------------

  /// Generates random write operations within haddr/hmask region
  void random_write(unsigned int length);

  /// Generates random read operations within haddr/hmask region
  void random_read(unsigned int length);

  /// Write operation / write data will be cached in local storage
  void check_write(unsigned int addr, unsigned char * data, unsigned int length);

  /// Read operation / results will be checked against data in local storage
  void check_read(unsigned int addr, unsigned char * data, unsigned int length);

  /// Callback for snooping
  void snoopingCallBack(const t_snoop & snoop, const sc_core::sc_time & delay);

  /// Write data to ahb
  void ahbwrite(unsigned int addr, unsigned char *data, unsigned int length, unsigned int burst_size);

  /// Read data from ahb
  void ahbread(unsigned int addr, unsigned char * data, unsigned int length, unsigned int burst_size);

  /// Thread for response processing
  void ResponseThread();

  /// Check transaction
  void checkTXN(tlm::tlm_generic_payload* trans);

  /// TLM non-blocking transport backward
  tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& gp, tlm::tlm_phase& phase, sc_core::sc_time& delay);

  SC_HAS_PROCESS(ahbctrl_test);

  /// Constructor
  ahbctrl_test(sc_core::sc_module_name name,
	       unsigned int haddr, // haddr for random instr. generation
	       unsigned int hmask, // hmask for random instr. generation
	       unsigned int master_id, // id of the bus master
	       sc_core::sc_time inter, // interval of random instructions (waiting period)
	       amba::amba_layer_ids abstractionLayer);

  // data members
  // ------------

  // System time for simulation accuracy measurement
  sc_core::sc_time phase_systime_start;
  sc_core::sc_time phase_systime_end;

  // Realtime clock for simulation performance measurement
  clock_t phase_realtime_start;
  clock_t phase_realtime_end;

  /// Typ for local cache entry
  typedef struct {

    unsigned int data;
    bool valid;

  } t_entry;

  /// Memory for keeping track of write operations
  std::map<unsigned int, t_entry> localcache;
  std::map<unsigned int, t_entry>::iterator it;

  // Address range for random instruction generation
  unsigned int m_haddr;
  unsigned int m_hmask;

  unsigned int m_addr_range_lower_bound;
  unsigned int m_addr_range_upper_bound;

  /// ID of the master socket
  unsigned int m_master_id;

  // Intervall between operations
  // (The time we wait after return of an transaction,
  // before generating the next one.)
  sc_core::sc_time m_inter;

  /// AMBA abstraction layer (LT/AT)
  amba::amba_layer_ids m_abstractionLayer;

 private:

  /// PEQ for response synchronization
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> mResponsePEQ;

  /// Events for phase notifications
  sc_event mEndRequestEvent;
  sc_event mBeginResponseEvent;
  sc_event mEndDataEvent;

};  

#endif // __AHBCTRL_TEST_H__
