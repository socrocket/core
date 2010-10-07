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
  unsigned int seta, setb, setc;
  unsigned int * debug;

  while(1) {

    debug = &tmp;

    // ************************************************************
    // * Test for cache configurations with muliple words per line
    // * ----------------------------------------------------------
    // * index  = 11  bit
    // * tag    = 16 bit
    // * offset = 5  bit
    // * sets   = 2
    // * random repl.
    // ************************************************************

    // suspend and wait for all components to be savely initialized
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // master activity
    // ===============

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * Phase 0: Read system registers (ASI 0x2) " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // read/write cache control register
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 1. ACTIVATE CACHES by writing the CONTROL REGISTER " << v::endl;
    v::info << name() << " * (ASI 0x2 - addr 0)    " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    // read cache control register !
    // args: address, length, asi, flush, flushl, lock, debug
    data=dread(0x0, 4, 2, 0, 0, 0, debug);
    // [3:2] == 0b11; [1:0] = 0b11 -> dcache and icache enabled
    v::info << name() << "cache_contr_reg: " << std::hex << data << v::endl;
    assert(data==0x0);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // activate caches:
    // CCR [3-2] = 11 enable dcache, CCR [1-0] enable icache
    dwrite(0x0, data |= 0xf, 4, 2, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read icache configuration register
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 2. Read ICACHE CONFIGURATION REGISTER (ASI 0x2 - addr 8)   " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    data=dread(0x8, 4, 2, 0, 0, 0, debug);
    // [29:28] repl == 0b11, [26:24] sets == 0b001, [23:20] ssize == 0b0110 (64kb)
    // [19] lram == 0, [18:16] lsize == 0b011 (8 words per line)
    // [15:12] lramsize == 0b0000 (1kb), [11:4] lramstart == 0x8e, [3] mmu = 0
    v::info << name() << "icache_config_reg: " << std::hex << data << v::endl;
    assert(data==0x316308e0);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read icache configuration register
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 3. Read DCACHE CONFIGURATION REGISTER (ASI 0x2 - addr 0xc)   " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    data=dread(0xc, 4, 2, 0, 0, 0, debug);
    // [29:28] repl == 0b11, [26:24] sets == 0b001, [23:20] ssize == 0b0110 (64kb)
    // [19] lram == 0, [18:16] lsize == 0b011 (8 word per line)
    // [15:12] lramsize == 0b0000 (1kb), [11:4] lramstart == 0x8f, [3] mmu = 0
    v::info << name() << "dcache_config_reg: " << std:: hex << data << v::endl;
    assert(data==0x316308f0);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * Phase 1: Test the dcache " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 4. Initialize main memory ((write miss) " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // | 31 - 16 (16 bit tag) | 15 - 5 (11 bit index) | 4 - 0 (5 bit offset) |

    // 0x00010000 (tag == 1)

    // init 10 lines
    for(unsigned int i = 0; i < 8*10; i++) {

      // args: address, data, length, asi, flush, flushl, lock
      dwrite((0x00010000 + (i<<2)), i, 4, 0x8, 0, 0, 0, debug);
      assert(CACHEWRITEMISS_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS);
    }

    // 0x00020000 (tag == 2)

    // init 10 lines
    for(unsigned int i = 0; i < 8*10; i++) {

      // args: address, data, length, asi, flush, flushl, lock
      dwrite((0x00020000 + (i<<2)), i, 4, 0x8, 0, 0, 0, debug);
      assert(CACHEWRITEMISS_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS);
    }

    // 0x00030000 (tag == 3)

    // init 10 lines
    for(unsigned int i = 0; i < 8*10; i++) {

      // args: address, data, length, asi, flush, flushl, lock
      dwrite((0x00030000 + (i<<2)), i, 4, 0x8, 0, 0, 0, debug);
      assert(CACHEWRITEMISS_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS);
    }

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 5. Fill line 0 of one of the sets (atag 1 - read miss) " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    for(unsigned int i = 0; i < 8; i++) {

      // args: address, length, asi, flush, flushl, lock
      data = dread((0x00010000 + (i<<2)), 4, 0x8, 0, 0, 0, debug);
      assert(CACHEREADMISS_CHECK(*debug));

    }

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 6. Read again with index 0 and atag 1 (atag 1 - read hit) " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    for(unsigned int i = 0; i < 8; i++) {

      data = dread((0x00010000 + (i<<2)), 4, 0x8, 0, 0, 0, debug);
      assert(CACHEREADHIT_CHECK(*debug));
      assert(data==i);
    }

    unsigned int set_under_test = (*debug & 0x11);
    v::info << name() << " The data has been buffered in set: " << set_under_test << v::endl;

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 7. Display the cache line " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // dbg out: dcache, line 0, length 4, asi 2, ..
    dwrite(0xff, 0, 4, 2, 0, 0, 0, debug);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 8. Diagnostic checks " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // TAG address = SET & LINE [15-5] & SUBBLOCK [4-2] & "00"

    // read dcache tag of the set under test
    data = dread((set_under_test << 16), 4, 0xe, 0, 0, 0, debug);
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, all entries valid = 0xff
    assert(data==0x4ff);

    // read dcache entries
    for(unsigned int i = 0; i < 8; i++) {

      data = dread(((set_under_test << 16)|(i<<2)), 4, 0xf, 0, 0, 0, debug);
      assert(data==i);

      wait(LOCAL_CLOCK,sc_core::SC_NS);
    }

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 9. Read line with tag 2 -> fill the second set (cache miss)" << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    for(unsigned int i = 0; i < 8; i++) {

      data = dread((0x00020000 + (i<<2)), 4, 0x8, 0, 0, 0, debug);
      assert(CACHEREADMISS_CHECK(*debug));
      assert(data==i);

      wait(LOCAL_CLOCK,sc_core::SC_NS);
    }

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 10. Invalidate set_under_test, line 0 subblock 3 " << v::endl;
    v::info << name() << " * (by diagnostic write)" << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // TAG address = SET & LINE [15-5] & SUBBLOCK [4-2] & "00"
    // DATA = ATAG [26..10], LRR [9], LOCK [9], VALID [7..0]
    dwrite((set_under_test << 16), ((1 << 10)|0xf7), 4, 0xe, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 11. Read tag 3, line 0, subblock 3  " << v::endl;
    v::info << name() << " * This should give line 0 in set_under_test a new tag  " << v::endl;
    v::info << name() << " * and invalidate the old data (all entries except subblock 3)." << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    data = dread((0x00030000 + (3<<2)), 4, 0x8, 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(data==3);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 12. Diagnostic check " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // TAG address = SET & LINE [15-5] & SUBBLOCK [4-2] & "00"

    // read dcache tag of the set under test
    data = dread((set_under_test << 16), 4, 0xe, 0, 0, 0, debug);
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, subblock 3 valid = 0x08;
    assert(data==0xc08);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 13. Display the cache line " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // dbg out: dcache, line 0, length 4, asi 2, ..
    dwrite(0xff, 0, 4, 2, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * Phase 2: Test the icache " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 14. Fill line 0 of one of the sets (atag 1 - read miss) " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    for(unsigned int i = 0; i < 8; i++) {

      // args: address, length, flush, flushl, lock
      data = iread((0x00010000 + (i<<2)), 0, 0, 0, debug);
      assert(CACHEREADMISS_CHECK(*debug));

    }

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 15. Read again with index 0 and atag 1 (atag 1 - read hit) " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    for(unsigned int i = 0; i < 8; i++) {

      data = iread((0x00010000 + (i<<2)), 0, 0, 0, debug);
      assert(CACHEREADHIT_CHECK(*debug));
      assert(data==i);
    }

    set_under_test = (*debug & 0x11);
    v::info << name() << " The data (instructions) has been buffered in set: " << set_under_test << v::endl;

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 16. Display the cache line " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // dbg out: icache, line 0, length 4, asi 2, ..
    dwrite(0xfe, 0, 4, 2, 0, 0, 0, debug);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 17. Diagnostic checks " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // TAG address = SET & LINE [15-5] & SUBBLOCK [4-2] & "00"

    // read icache tag of the set under test
    data = dread((set_under_test << 16), 4, 0xc, 0, 0, 0, debug);
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, all entries valid = 0xff
    assert(data==0x4ff);

    // read icache entries
    for(unsigned int i = 0; i < 8; i++) {

      data = dread(((set_under_test << 16)|(i<<2)), 4, 0xd, 0, 0, 0, debug);
      assert(data==i);

      wait(LOCAL_CLOCK,sc_core::SC_NS);
    }

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 18. Read line with tag 2 -> fill the second set (cache miss)" << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    for(unsigned int i = 0; i < 8; i++) {

      data = iread((0x00020000 + (i<<2)), 0, 0, 0, debug);
      assert(CACHEREADMISS_CHECK(*debug));
      assert(data==i);

      wait(LOCAL_CLOCK,sc_core::SC_NS);
    }

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 19. Invalidate set_under_test, line 0, subblock 3 " << v::endl;
    v::info << name() << " * (by diagnostic write)" << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // TAG address = SET & LINE [15-5] & SUBBLOCK [4-2] & "00"
    // DATA = ATAG [26..10], LRR [9], LOCK [9], VALID [7..0]
    dwrite((set_under_test << 16), ((1 << 10)|0xf7), 4, 0xc, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 20. Read instruction cache with tag 3, line 0, subblock 3  " << v::endl;
    v::info << name() << " * This should give line 0 in set_under_test a new tag  " << v::endl;
    v::info << name() << " * and invalidate the old data (all entries except subblock 3)." << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    data = iread((0x00030000 + (3<<2)), 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(data==3);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 21. Diagnostic check " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // TAG address = SET & LINE [15-5] & SUBBLOCK [4-2] & "00"

    // read icache tag of the set under test
    data = dread((set_under_test << 16), 4, 0xc, 0, 0, 0, debug);
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, subblock 3 valid = 0x08;
    assert(data==0xc08);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 22. Display the cache line " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // dbg out: icache, line 0, length 4, asi 2, ..
    dwrite(0xfe, 0, 4, 2, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * Phase 3: Test random replacement " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 23. Load data to line 5 of both cache sets (tag 1, tag2) " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // | 31 - 16 (16 bit tag) | 15 - 5 (11 bit index) | 4 - 0 (5 bit offset) |
    data = dread(0x000100a0, 4, 8, 0, 0, 0, debug);
    v::info << this->name() << "Data: " << std::hex << data << v::endl;
    assert(data==40);
    assert(CACHEREADMISS_CHECK(*debug));
    // number of set that contains the new data
    seta = (*debug & 0x3);

    data = dread(0x000200a0, 4, 8, 0, 0, 0, debug);
    assert(data==40);
    assert(CACHEREADMISS_CHECK(*debug));
    setb = (*debug & 0x3);
    // the other set shall be used to cache the data
    assert(seta != setb);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 24. Test line replacement " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // reading with tag 3 should replace seta or setb
    data = dread(0x000300a0, 4, 8, 0, 0, 0, debug);
    assert(data==40);
    assert(CACHEREADMISS_CHECK(*debug));
    setc = (*debug & 0x3);

    // reading again from the same address will produce a hit in the same set
    data = dread(0x000300a0, 4, 8, 0, 0, 0, debug);
    assert(data==40);
    assert(CACHEREADHIT_CHECK(*debug));
    assert(setc == (*debug & 0x3));

    // read the tag which has just been replaced (bring back to cache)
    if (setc == seta) {

      // tag 1 was removed from cache
      data = dread(0x000100a0, 4, 8, 0, 0, 0, debug);
      assert(data==40);
      assert(CACHEREADMISS_CHECK(*debug));

      data = dread(0x000100a0, 4, 8, 0, 0, 0, debug);
      assert(CACHEREADHIT_CHECK(*debug));

    } else {

      // tag 2 was removed from cache
      data = dread(0x000200a0, 4, 8, 0, 0, 0, debug);
      assert(data==40);
      assert(CACHEREADMISS_CHECK(*debug));

      data = dread(0x000200a0, 4, 8, 0, 0, 0, debug);
      assert(CACHEREADHIT_CHECK(*debug));

    }

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * Phase 4: Test instruction burst-fetch mode " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 23. Set cache control register to instr. burst fetch  " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // read cache control register
    data=dread(0x0, 4, 2, 0, 0, 0, debug);
    // [3:2] DCS == 0b11; [1:0] ICS = 0b11
    v::info << name() << "cache_contr_reg: " << std::hex << data << v::endl;
    assert(data==0xf);

    // switch on bit 16 - Instruction Burst Fetch (IB)
    dwrite(0x0, data |= (1 << 16), 4, 2, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // check new status of cache control
    data=dread(0x0, 4, 2, 0, 0, 0, debug);
    // [16] IB == 0b1, [3:2] DCS == 0b11; [1:0] ICS = 0b11
    v::info << name() << "cache_contr_reg: " << std::hex << data << v::endl;
    assert(data==0x1000f);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 24. Invalidate instruction cache set 0  " << v::endl;
    v::info << name() << " * by deleting cache tag and valid bits " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // TAG address = SET & LINE [15-5] & SUBBLOCK [4-2] & "00"
    // DATA = ATAG [26..10], LRR [9], LOCK [9], VALID [7..0]
    dwrite(0, 0, 4, 0xc, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 25. Instruction read with tag 1, line 0, offset 2.  " << v::endl;
    v::info << name() << " * This should cause the cache line to fill from subblock 2 " << v::endl;
    v::info << name() << " * until the end (subblock 8)" << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // The read from the cache line causes a read miss.
    data=iread(0x00010008, 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(data==2);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // If the cacheline has been properly filled by the burst
    // the following reads will be hits.
    for (unsigned int i=3; i < 8; i++) {

      data=iread((0x00010000 + (i<<2)), 0, 0, 0, debug);
      assert(CACHEREADHIT_CHECK(*debug));
      assert(data==i);

      wait(LOCAL_CLOCK,sc_core::SC_NS);
    }

    wait(LOCAL_CLOCK,sc_core::SC_NS);
    sc_core::sc_stop();
  }
}

