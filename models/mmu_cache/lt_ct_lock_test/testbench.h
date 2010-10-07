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
// Title:      testbench.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Class definition of the
//             stimuli generator/monitor for the current testbench.
//
// Modified on $Date: 2010-08-13 20:04:30 +0200 (Fri, 13 Aug 2010) $
//          at $Revision: 45 $
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#ifndef __TESTBENCH_H__
#define __TESTBENCH_H__

#include "tlm.h"
#include "locals.h"

#include "../lib/mmu_cache_test.h"

class testbench : public mmu_cache_test {
 public:

  virtual void initiator_thread();

  SC_HAS_PROCESS(testbench);
  /// constructor
  testbench(sc_core::sc_module_name name) : mmu_cache_test(name) {

     // register testbench thread
     SC_THREAD(initiator_thread);
  }
};

#endif // __TESTBENCH_H__

