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
// Modified on $Date: 2010-10-07 19:25:02 +0200 (Thu, 07 Oct 2010) $
//          at $Revision: 166 $
//          by $Author: HWSWSIM $
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
#include "stimuli.h"

#include "../lib/mmu_cache_test_rtl.h"

class testbench : public mmu_cache_test {
    public:

        sc_out<bool> rst;
        sc_out<sc_logic> rst_scl;
	sc_out<sc_logic> signal_clk;

        sc_clock clock;

	void clock_gen_thread();
	void reset_gen_thread();

        virtual void instruction_initiator_thread();
	virtual void data_initiator_thread();

        SC_HAS_PROCESS(testbench);
        /// constructor
        testbench(sc_core::sc_module_name name) :
            mmu_cache_test(name), clock("clock", 10, 0.5, 0, true), rst("rst"), rst_scl("rst_scl") {

	    SC_THREAD(clock_gen_thread);
	    sensitive << clock;

	    SC_THREAD(reset_gen_thread);

            // register testbench thread
            //SC_THREAD(instruction_initiator_thread);
	    //sensitive << rst;
	    //dont_initialize();

	    //SC_THREAD(data_initiator_thread);
	    //sensitive << data_trigger;

	    vector_counter=0;
	    data_ready_done = false;

        }

     private:
	
	t_stim tester;
	unsigned int vector_counter;
	sc_event data_trigger;
	sc_event data_ready;
	bool data_ready_done;

};

#endif // __TESTBENCH_H__
