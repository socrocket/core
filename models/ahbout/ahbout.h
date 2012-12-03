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
// Title:      reliabilitymanager.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
// ********************************************************************

#ifndef AHBOUT_H
#define AHBOUT_H

#include <tlm.h>
#include <amba.h>
#include <map>
#include <fstream>

#include "ahbslave.h"
#include "clkdevice.h"
#include "msclogger.h"

class AHBOut : public AHBSlave<>, public CLKDevice {
 public:
  SC_HAS_PROCESS(AHBOut);
  
  /// Constructor
  /// @brief Constructor for the test bench memory class
  /// @param haddr AHB address of the AHB slave socket (12 bit)
  /// @param hmask AHB address mask (12 bit)
  /// @param ambaLayer Abstraction layer used (AT/LT)
  /// @param slave_id AHB Slave id
  /// @param outfile File name of a text file to initialize the memory from
  AHBOut(const sc_core::sc_module_name nm, 
         uint16_t haddr_, 
         uint16_t hmask_ = 0, 
         amba::amba_layer_ids ambaLayer = amba::amba_LT, 
         uint32_t slave_id = 0,
         char *outfile = NULL);

  /// Destructor
  ~AHBOut();

  uint32_t exec_func(tlm::tlm_generic_payload &gp, sc_time &delay, bool debug = false);

  sc_core::sc_time get_clock();

 private:
  /// 12 bit MSB address and mask (constructor parameters)
  const uint32_t mhaddr;
  const uint32_t mhmask;
  
  ofstream outfile;
};

#endif
