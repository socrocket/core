// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       testbench.h - Class definition of the                   *
// *             stimuli generator/monitor for the current testbench.    *
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision$                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************

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

