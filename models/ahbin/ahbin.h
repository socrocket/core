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
// Title:      ahbin.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Class definition for input device.
//             Provides frames of data samples in regual intervals.
//             CPU is notifed about new data via IRQ.
//             
//
// Modified on $Date: 2012-02-28 11:45:20 +0100 (Tue, 28 Feb 2012) $
//          at $Revision: 785 $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#ifndef __AHBIN_H__
#define __AHBIN_H__

// The TLM header (to be included in each TL model)
#include <tlm.h>

// AHB TLM master socket and protocol implementation 
#include "ahbmaster.h"
// Timing interface (specify clock period)
#include "clkdevice.h"

// Signal definitions for IRQ communication
#include "signalkit.h"

// Verbosity kit - for output formatting and filtering
#include "verbose.h"

// Provides methods for generating random data
#include <math.h>

/// @addtogroup AHBIn Input_Device
/// @{

/// Definition of class AHBIn
class AHBIn : public AHBMaster<>, public CLKDevice {

 public:
  SC_HAS_PROCESS(AHBIn);
  SK_HAS_SIGNALS(AHBIn);

  /// SignalKit interrupt output
  signal<bool>::out irq;

  /// Constructor
  AHBIn(sc_core::sc_module_name name,    // The SystemC name of the component
               unsigned int hindex,         // The master index for registering with the AHB 
               unsigned int hirq,               // The number of the IRQ raised for available data
               unsigned int framesize,          // The size of the data frame to be generated
               unsigned int frameaddr,          // The address the data is supposed to be copied to
               sc_core::sc_time interval,       // The interval between data frames
               bool pow_mon,                    // Enable power monitoring
               amba::amba_layer_ids ambaLayer); // TLM abstraction layer

	

   /// Thread for triggering gen_frame (generates new_frame event)
  void frame_trigger();

  /// Thread for generating the data frame
  void gen_frame();

  /// Reset function
  void dorst();
  
  /// Deal with clock changes
  void clkcng();

  sc_core::sc_time get_clock();

  // data members
  // ------------

 private:

  /// IRQ number
  const uint32_t m_irq;

  /// Frame size in bytes
  const uint32_t m_framesize;

  /// Target address for data frame
  const uint32_t m_frameaddr;

  /// Time interval for creating data frame
  const sc_core::sc_time m_interval;

  /// ID of the AHB master
  const uint32_t m_master_id;

  /// Event for activating the gen_frame Thread
  sc_event new_frame;

  /// Pointer to the data frame
  uint32_t * frame;

  /// Power monitoring enabled/disable
  bool m_pow_mon;

  /// amba abstraction layer
  amba::amba_layer_ids m_abstractionLayer;

};

/// @}

#endif //__AHBIN_H__
