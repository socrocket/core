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
// Title:      testbench.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implementation of the
//             stimuli generator/monitor for the current testbench.
//
// Method:
//
// Modified on $Date: 2010-10-07 19:25:02 +0200 (Thu, 07 Oct 2010) $
//          at $Revision: 166 $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#include "testbench.h"
#include "verbose.h"

t_stim stim[VECTORS] = { \
  { fetch, 0xc0000, 1, 0, 0, 0x01010101, store, 0,   0, 0x2, 1, 0, 0, 0,   0 }, \
  { fetch, 0xc0004, 0, 0, 0, 0x02020202, dnop,  0,   0, 0,   0, 0, 0, 0,   0 }, \
  { fetch, 0xc0008, 0, 0, 0, 0x03030303, store, 0x0, 4, 0x2, 0, 0, 0, 0xf, 0 }, \
  { fetch, 0xc0000, 0, 0, 0, 0x01010101, dnop,  0,   0, 0,   0, 0, 0, 0,   0 }, \
  { fetch, 0xc0004, 0, 0, 0, 0x02020202, dnop,  0,   0, 0,   0, 0, 0, 0,   0 }, \
  { fetch, 0xc0008, 0, 0, 0, 0x03030303, dnop,  0,   0, 0,   0, 0, 0, 0,   0 }, \
  { fetch, 0xc0000, 0, 0, 0, 0x01010101, dnop,  0,   0, 0,   0, 0, 0, 0,   0 }, \
  { fetch, 0xc0004, 0, 0, 0, 0x02020202, dnop,  0,   0, 0,   0, 0, 0, 0,   0 }, \
  { fetch, 0xc0008, 0, 0, 0, 0x03030303, dnop,  0,   0, 0,   0, 0, 0, 0,   0 }, \
  { fetch, 0xc0008, 0, 0, 0, 0x03030303, dnop,  0,   0, 0,   0, 0, 0, 0,   0 }, \
  { fetch, 0xc0008, 0, 0, 0, 0x03030303, dnop,  0,   0, 0,   0, 0, 0, 0,   0 }, \
  { fetch, 0xc0008, 0, 0, 0, 0x03030303, dnop,  0,   0, 0,   0, 0, 0, 0,   0 }, \
  { fetch, 0xc0008, 0, 0, 0, 0x03030303, dnop,  0,   0, 0,   0, 0, 0, 0,   0 }, \
  { fetch, 0xc0008, 0, 0, 0, 0x03030303, dnop,  0,   0, 0,   0, 0, 0, 0,   0 }, \
  { fetch, 0xc0008, 0, 0, 0, 0x03030303, dnop,  0,   0, 0,   0, 0, 0, 0,   0 } \
};

// testbench instruction initiator thread
void testbench::instruction_initiator_thread(void) {

    unsigned int tmp;
    unsigned int instr;
    unsigned int *debug;
 
    debug = &tmp;

    // wait for save reset
    wait(1, SC_NS);

    while (1) {

      if (rst.read() == 1) {

	if (vector_counter == VECTORS) {

	  // stop simulation after processing all vectors
	  wait(100, SC_NS);
	  sc_stop();

	} else if (vector_counter == 2) {

	  // wait for caches to be savely flushed
	  // (caches are in bypass for the time)
	  wait(41200, SC_NS);
	  
	}

	tester = stim[vector_counter++];

	std::cout << name() << sc_time_stamp() << " Next Vector" << std::endl;

	if (tester.dtype != dnop) {

	  // start data thread before we get blocked by iread
	  std::cout << name() << sc_time_stamp() << " Notify data thread" << std::endl;
	  data_trigger.notify();
  	  data_ready_done = false;

	}

	if (tester.itype == fetch) {

	  std::cout << name() << sc_time_stamp() << " call iread" << std::endl;
	  instr = iread(tester.iaddr, tester.iflush, tester.iflushl, tester.ifline, debug);
	  //assert(instr==tester.iinstr); 
 
	}

	// if data thread still busy
	if ((tester.dtype != dnop)&&(!data_ready_done)) {

	  std::cout << " Waiting for data thread to finish! " << std::endl;

	  // wait to finish (sync with instruction thread)
	  wait(data_ready);
	}
      }
    }
}

// testbench data initiator thread
void testbench::data_initiator_thread(void) {

    unsigned int tmp;
    unsigned int data;
    unsigned int * debug;

    debug = &tmp;

    while (1) {

      wait();

      if (rst.read() == 1) {

	if (tester.dtype == load) {

	  std::cout << name() << sc_time_stamp() << " call dread" << std::endl;
	  data = dread(tester.daddr, tester.dlength, tester.dasi, tester.dflush, tester.dflushl, tester.dlock, debug);
	  assert(data==tester.dhrdata);

	} else if (tester.dtype == store) {

	  std::cout << name() << sc_time_stamp() << " call dwrite" << std::endl;
	  dwrite(tester.daddr, tester.dwdata, tester.dlength, tester.dasi, tester.dflush, tester.dflushl, tester.dlock, debug);
	  std::cout << sc_time_stamp() << "return from dwrite" << std::endl;

	}

	data_ready.notify();
	data_ready_done = true;
      }
   }
}


// generates a sc_logic version of clock for the hdl modules
void testbench::clock_gen_thread() {

  while(1) {

    signal_clk.write(SC_LOGIC_0);
    wait();
    signal_clk.write(SC_LOGIC_1);
    wait();
  }
}

// generates a boolean and a sc_logic version of reset
void testbench::reset_gen_thread() {

  rst.write(0);
  rst_scl.write(SC_LOGIC_0);
  wait(95, sc_core::SC_NS);

  rst.write(1);
  rst_scl.write(SC_LOGIC_1);

}

