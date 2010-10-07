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
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#include "testbench.h"
#include "verbose.h"

// testbench initiator thread
void testbench::initiator_thread(void) {

    // test vars
    unsigned int data;
    unsigned int tmp;
    //unsigned int last;
    unsigned int * debug;

    while (1) {

        debug = &tmp;

        // ******************************************************
        // * Test for mmu_cache with ilram and dlram scratchpads
        // * ----------------------------------------------------
        // * index  = 16  bit
        // * tag    = 14 bit
        // * offset = 2  bit
        // * sets   = 2
        // * random repl.
        // ******************************************************

        // suspend and wait for all components to be savely initialized
        wait(LOCAL_CLOCK, sc_core::SC_NS);

        // master activity
        // ===============

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name() << " * Phase 0: Read system registers (ASI 0x2) "
                << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        // read/write cache control register
        v::info << name()
                << " ********************************************************* "
                << v::endl;
        v::info << name()
                << " * 1. ACTIVATE CACHES by writing the CONTROL REGISTER "
                << v::endl;
        v::info << name() << " * (ASI 0x2 - addr 0)    " << v::endl;
        v::info << name()
                << " ********************************************************* "
                << v::endl;

        // read cache control register !
        // args: address, length, asi, flush, flushl, lock, debug
        data = dread(0x0, 4, 2, 0, 0, 0, debug);
        // [3:2] == 0b11; [1:0] = 0b11 -> dcache and icache enabled
        v::info << name() << "cache_contr_reg: " << std::hex << data << v::endl;
        assert(data == 0x0);

        wait(LOCAL_CLOCK, sc_core::SC_NS);

        // activate caches:
        // CCR [3-2] = 11 enable dcache, CCR [1-0] enable icache
        dwrite(0x0, data |= 0xf, 4, 2, 0, 0, 0, debug);

        wait(LOCAL_CLOCK, sc_core::SC_NS);

        // read icache configuration register
        v::info << name()
                << " ********************************************************* "
                << v::endl;
        v::info << name()
                << " * 2. Read ICACHE CONFIGURATION REGISTER (ASI 0x2 - addr 8)   "
                << v::endl;
        v::info << name()
                << " ********************************************************* "
                << v::endl;

        data = dread(0x8, 4, 2, 0, 0, 0, debug);
        // [29:28] repl == 0b11, [26:24] sets == 0b001, [23:20] ssize == 0b1000 (256kb)
        // [19] lram == 1, [18:16] lsize == 0b000 (1 word per line)
        // [15:12] lramsize == 0b1001 (512kb), [11:4] lramstart == 0x8e, [3] mmu = 0
        v::info << name() << "icache_config_reg: " << std::hex << data
                << v::endl;
        assert(data == 0x318898e0);

        wait(LOCAL_CLOCK, sc_core::SC_NS);

        // read icache configuration register
        v::info << name()
                << " ********************************************************* "
                << v::endl;
        v::info << name()
                << " * 3. Read DCACHE CONFIGURATION REGISTER (ASI 0x2 - addr 0xc)   "
                << v::endl;
        v::info << name()
                << " ********************************************************* "
                << v::endl;

        data = dread(0xc, 4, 2, 0, 0, 0, debug);
        // [29:28] repl == 0b11, [26:24] sets == 0b001, [23:20] ssize == 0b1000 (256kb)
        // [19] lram == 1, [18:16] lsize == 0b000 (1 word per line)
        // [15:12] lramsize == 0b1001 (512kb), [11:4] lramstart == 0x8e, [3] mmu = 0
        v::info << name() << "dcache_config_reg: " << std::hex << data
                << v::endl;
        assert(data == 0x318898f0);

        wait(LOCAL_CLOCK, sc_core::SC_NS);

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name() << " * Phase 1: Test the Cache " << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name()
                << " * 4. Initialize 768kB of main memory (3x cache set size) "
                << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        for (int i = 0; i < 0xc0000; i += 4) {

            // write only beginning and end of cache page
            if (((i & 0x3ffff) < 0x100) || ((i & 0x3ffff) > 0x3ff00)) {

                dwrite(i, i, 4, 8, 0, 0, 0, debug);
                assert(CACHEWRITEMISS_CHECK(*debug));

                wait(LOCAL_CLOCK, sc_core::SC_NS);
            }
        }

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name() << " * 5. Read 512 kB of memory (cache misses) "
                << v::endl;
        v::info << name()
                << " * This should completely fill the two cache sets (2x 256kB) "
                << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        // tags 0 and 1
        for (unsigned int i = 0; i < 0x80000; i += 4) {

            // read only beginning and end
            if ((i < 0x100) || (i > 0x7ff00)) {
                data = dread(i, 4, 8, 0, 0, 0, debug);
                assert(data == i);
                assert(CACHEREADMISS_CHECK(*debug));

                wait(LOCAL_CLOCK, sc_core::SC_NS);
            }
        }

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name() << " * 6. Read the same data again (cache hits) "
                << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        // tags 0 and 1
        for (unsigned int i = 0; i < 0x80000; i += 4) {

            // read only beginning and end
            if ((i < 0x100) || (i > 0x7ff00)) {
                data = dread(i, 4, 8, 0, 0, 0, debug);
                assert(data == i);
                assert(CACHEREADHIT_CHECK(*debug));

                wait(LOCAL_CLOCK, sc_core::SC_NS);
            }
        }

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name()
                << " * 7. Read another 256 kB from memory (cache miss) "
                << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        // tag 2
        for (unsigned int i = 0x80000; i < 0xc0000; i += 4) {

            // read only beginning and end
            if ((i < 0x80100) || (i > 0xbff00)) {
                data = dread(i, 4, 8, 0, 0, 0, debug);
                assert(data == i);
                assert(CACHEREADMISS_CHECK(*debug));

                wait(LOCAL_CLOCK, sc_core::SC_NS);
            }
        }

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name() << " * 8. Read again the new 256 kB (cache miss) "
                << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        // tag 2
        for (unsigned int i = 0x80000; i < 0xc0000; i += 4) {

            // read only beginning and end
            if ((i < 0x80100) || (i > 0xbff00)) {
                data = dread(i, 4, 8, 0, 0, 0, debug);
                assert(data == i);
                assert(CACHEREADHIT_CHECK(*debug));

                wait(LOCAL_CLOCK, sc_core::SC_NS);
            }
        }

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name() << " * Phase 2: Test instruction scratchpad "
                << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name() << " * 9. Fill the instruction scratchpad with data "
                << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        // start of scratchpad segment
        for (unsigned int i = 0x8e000000; i < 0x8e080000; i += 4) {

            // write only beginning and end
            if ((i < 0x8e000100) || (i > 0x8e07ff00)) {
                dwrite(i, i, 4, 8, 0, 0, 0, debug);
                assert(SCRATCHPAD_CHECK(*debug));

                wait(LOCAL_CLOCK, sc_core::SC_NS);
            }
        }

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name() << " * 10. Read the instruction scratchpad"
                << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        // start of scratchpad segment
        for (unsigned int i = 0x8e000000; i < 0x8e080000; i += 4) {

            // read only beginning and end
            if ((i < 0x8e000100) || (i > 0x8e07ff00)) {

                data = iread(i, 0, 0, 0, debug);
                assert(data == i);
                assert(SCRATCHPAD_CHECK(*debug));

                wait(LOCAL_CLOCK, sc_core::SC_NS);
            }
        }

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name() << " * 11. Fill the data scratchpad with data"
                << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        for (unsigned int i = 0x8f000000; i < 0x8f080000; i += 4) {

            // write only beginning and end
            if ((i < 0x8f000100) || (i > 0x8f07ff00)) {
                dwrite(i, i, 4, 8, 0, 0, 0, debug);
                assert(SCRATCHPAD_CHECK(*debug));

                wait(LOCAL_CLOCK, sc_core::SC_NS);
            }
        }

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name() << " * 12. Read the data scratchpad" << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        for (unsigned int i = 0x8f000000; i < 0x8f080000; i += 4) {

            // read only beginning and end
            if ((i < 0x8f000100) || (i > 0x8f07ff00)) {
                data = dread(i, 4, 8, 0, 0, 0, debug);
                assert(SCRATCHPAD_CHECK(*debug));

                wait(LOCAL_CLOCK, sc_core::SC_NS);
            }
        }

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name() << " * 13. Test byte store for data scratchpad "
                << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        for (unsigned int i = 0; i < 4; i++) {

            dwrite(0x8f000000 + i, i + 1, 1, 8, 0, 0, 0, debug);
            assert(SCRATCHPAD_CHECK(*debug));

            wait(LOCAL_CLOCK, sc_core::SC_NS);
        }

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name() << " * 14. Test byte load for data scratchpad "
                << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        for (unsigned int i = 0; i < 4; i++) {

            data = dread(0x8f000000 + i, 1, 8, 0, 0, 0, debug);
            assert(data == (i + 1));
            assert(SCRATCHPAD_CHECK(*debug));

            wait(LOCAL_CLOCK, sc_core::SC_NS);
        }

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name() << " * 15. Test half-word read for data scratchpad "
                << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        data = dread(0x8f000000, 2, 8, 0, 0, 0, debug);
        assert(data == 0x0201);
        assert(SCRATCHPAD_CHECK(*debug));

        wait(LOCAL_CLOCK, sc_core::SC_NS);

        data = dread(0x8f000002, 2, 8, 0, 0, 0, debug);
        assert(data == 0x0403);
        assert(SCRATCHPAD_CHECK(*debug));

        wait(LOCAL_CLOCK, sc_core::SC_NS);

        v::info << name()
                << " ************************************************************ "
                << v::endl;
        v::info << name() << " * 16. Test half-word write for data scratchpad "
                << v::endl;
        v::info << name()
                << " ************************************************************"
                << v::endl;

        dwrite(0x8f000004, 0x0b0a, 2, 8, 0, 0, 0, debug);
        assert(SCRATCHPAD_CHECK(*debug));

        wait(LOCAL_CLOCK, sc_core::SC_NS);

        dwrite(0x8f000006, 0x0d0c, 2, 8, 0, 0, 0, debug);
        assert(SCRATCHPAD_CHECK(*debug));

        wait(LOCAL_CLOCK, sc_core::SC_NS);

        // read back and check
        data = dread(0x8f000004, 4, 8, 0, 0, 0, debug);
        assert(data == 0x0d0c0b0a);
        assert(SCRATCHPAD_CHECK(*debug));

        wait(LOCAL_CLOCK, sc_core::SC_NS);

        sc_core::sc_stop();
    }
}

