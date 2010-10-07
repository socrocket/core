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
// Title:      mmu_cache_test.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Provides two TLM initiator sockets
//             and several helper functions to simplify the coding
//             of testbenches for mmu_cache.
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

  while(1) {

    debug = &tmp;

    // *******************************************
    // * Test for default i/d cache configuration
    // * -----------------------------------------
    // * index  = 8  bit
    // * tag    = 22 bit
    // * offset = 2  bit
    // * sets   = 4
    // * random repl.
    // *******************************************

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
    // CCR [3:2] == 0b00; [1:0] = 0b00 -> dcache and icache disabled
    v::info << name() << "cache_contr_reg: " << std::hex << data << v::endl;
    assert(data==0x0);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // activate caches:
    // CCR [3-2] = 0b11 enable dcache, CCR [1-0] = 0b11 enable icache
    dwrite(0x0, 0xf, 4, 2, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read icache configuration register
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 2. Read ICACHE CONFIGURATION REGISTER (ASI 0x2 - addr 8)   " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    data=dread(0x8, 4, 2, 0, 0, 0, debug);
    // [29:28] repl == 0b11, [26:24] sets == 0b011, [23:20] ssize == 0b0000 (1kb)
    // [19] lram == 0, [18:16] lsize == 0b000 (1 word per line)
    // [15:12] lramsize == 0, [11:4] lramstart == 0x8e, [3] mmu == 0
    v::info << name() << "icache_config_reg: " << std::hex << data << v::endl;
    assert(data==0x330008e0);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read icache configuration register
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 3. Read DCACHE CONFIGURATION REGISTER (ASI 0x2 - addr 0xc)   " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    data=dread(0xc, 4, 2, 0, 0, 0, debug);
    // [29:28] repl == 0b11, [26:24] sets == 0b100, [23:20] ssize == 0b0001 (1kb)
    // [19] lram == 0, [18:16], lsize == 0b001 (1 word per line)
    // [15:12] lramsize == 1, [11:4] lramstart == 0x8f, [3] mmu == 0
    v::info << name() << "dcache_config_reg: " << std:: hex << data << v::endl;
    assert(data==0x330008f0);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * Phase 1: Main memory is written through dcache. " << v::endl;
    v::info << name() << " * The addresses are selected in a way that indices are equal" << v::endl;
    v::info << name() << " * but tags are different (different pages)." << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // write some data to memory (cache write miss)
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 4. DCACHE write addr 0x64 (cache write miss)               " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    data=0x04030201;
    // args: address, data, length, asi, flush, flushl, lock
    dwrite(0x64,data, 4, 0x8, 0, 0, 0, debug);
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 5. DCACHE write addr 0x464 (cache write miss)              " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    data=0x08070605;
    dwrite(0x464,data,4, 0x8, 0, 0, 0, debug);
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 6. DCACHE write addr 0x864 (cache write miss)              " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    data=0x0c0b0a09;
    dwrite(0x864,data,4, 0x8, 0, 0, 0, debug);
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 7. DCACHE write addr 0xc64 (cache write miss)              " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    data=0x100f0e0d;
    dwrite(0xc64,data,4, 0x8, 0, 0, 0, debug);
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ********************************************************************  " << v::endl;
    v::info << name() << " * Phase 2: The data which has been written in Phase 1 is read back " << v::endl;
    v::info << name() << " * through the data interface. Because the address tags are different " << v::endl;
    v::info << name() << " * all four sets of the cache should be filled. The first part " << v::endl;
    v::info << name() << " * provokes cache misses, the second part cache hits. " << v::endl;
    v::info << name() << " ******************************************************************** " << v::endl;

    // first read (cache miss)
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 8. DCACHE read addr 0x64 (cache miss)         "                     << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    // args: address, length, asi, flush, flushl, lock
    data = dread(0x64, 4, 0x8, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE read from 0x64 returned " << std::hex << data << v::endl;
    assert(data==0x04030201);
    v::info << this->name() << "debug: " << std::hex << *debug << v::endl;
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read again (cache hit)
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 9. DCACHE read again from 0x64 (cache hit)           "              << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    data = dread(0x64, 4, 0x8, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE read from 0x64 returned " << std::hex << data << v::endl;
    assert(data==0x04030201);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load next bank)
    v::info << name() << " ********************************************************** " << v::endl;
    v::info << name() << " * 10. DCACHE read addr 0x464 / same idx diff tag (cache miss) " << v::endl;
    v::info << name() << " * should fill one of the empty banks (3 left)              " << v::endl;
    v::info << name() << " ********************************************************** " << v::endl;
    data = dread(0x464, 4, 0x8, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE read from 0x464 returned " << std::hex << data << v::endl;
    assert(data==0x08070605);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load next bank)
    v::info << name() << " ********************************************************** " << v::endl;
    v::info << name() << " * 11. DCACHE read addr 0x864 / same idx,  diff tag (cache miss) " << v::endl;
    v::info << name() << " * should fill one of the empty banks (2 left)              " << v::endl;
    v::info << name() << " ********************************************************** " << v::endl;

    data = dread(0x864, 4, 0x8, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE read from 0x864 returned " << std::hex << data << v::endl;
    assert(data==0x0c0b0a09);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load the last free bank)
    v::info << name() << " ********************************************************** " << v::endl;
    v::info << name() << " * 12. DCACHE read addr 0xc64 / same idx, diff tag (cache miss) " << v::endl;
    v::info << name() << " * should fill the last empty bank                          " << v::endl;
    v::info << name() << " ********************************************************** " << v::endl;

    data = dread(0xc64, 4, 0x8, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE read from 0xc64 returned " << std::hex << data << v::endl;
    assert(data==0x100f0e0d);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 13. DCACHE read from 0x464 (cache hit) "              << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    data = dread(0x464, 4, 0x8, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE read from 0x464 returned " << std::hex << data << v::endl;
    assert(data==0x08070605);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 14. DCACHE read from 0x864 (cache hit) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    data = dread(0x864, 4, 0x8, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE read from 0x864 returned " << std::hex << data << v::endl;
    assert(data==0x0c0b0a09);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 15. DCACHE read from 0xc64 (cache hit) "              << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    data = dread(0xc64, 4, 0x8, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE read from 0xc64 returned " << std::hex << data << v::endl;
    assert(data==0x100f0e0d);
    assert(CACHEREADHIT_CHECK(*debug));

    v::info << name() << " ******************************************************************** " << v::endl;
    v::info << name() << " * Phase 3: Similar to Phase 2, the same data is now loaded through " << v::endl;
    v::info << name() << " * the instruction interface. All sets of the icache should be " << v::endl;
    v::info << name() << " * filled. The first part provokes misses the second hits. " << v::endl;
    v::info << name() << " ******************************************************************** " << v::endl;

    // first read (cache miss)
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 16. ICACHE read addr 0x64 (cache miss)         "                     << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    // args: addr, flush, flushl, fline
    data = iread(0x64, 0x8, 0, 0, debug);
    v::info << name() <<  "ICACHE read from 0x64 returned " << std::hex << data << v::endl;
    assert(data==0x04030201);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read again (cache hit)
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 17. ICACHE read again from 0x64 (cache hit)           "              << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    data = iread(0x64, 0x8, 0, 0, debug);
    v::info << name() <<  "ICACHE read from 0x64 returned " << std::hex << data << v::endl;
    assert(data==0x04030201);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load next bank)
    v::info << name() << " ********************************************************** " << v::endl;
    v::info << name() << " * 18. ICACHE read addr 0x464 / same idx, diff tag (cache miss) " << v::endl;
    v::info << name() << " * should fill one of the empty banks (3 left)              " << v::endl;
    v::info << name() << " ********************************************************** " << v::endl;
    data = iread(0x464, 0x8, 0, 0, debug);
    v::info << name() <<  "ICACHE read from 0x464 returned " << std::hex << data << v::endl;
    assert(data==0x08070605);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load next bank)
    v::info << name() << " ********************************************************** " << v::endl;
    v::info << name() << " * 19. ICACHE read addr 0x864 / same idx, diff tag (cache miss) " << v::endl;
    v::info << name() << " * should fill one of the empty banks (2 left)              " << v::endl;
    v::info << name() << " ********************************************************** " << v::endl;

    data = iread(0x864, 0x8, 0, 0, debug);
    v::info << name() <<  "ICACHE read from 0x864 returned " << std::hex << data << v::endl;
    assert(data==0x0c0b0a09);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load the last free bank)
    v::info << name() << " ********************************************************** " << v::endl;
    v::info << name() << " * 20. ICACHE read addr 0xc64 / same idx, diff tag (cache miss) " << v::endl;
    v::info << name() << " * should fill the last empty bank                          " << v::endl;
    v::info << name() << " ********************************************************** " << v::endl;

    data = iread(0xc64, 0x8, 0, 0, debug);
    v::info << name() <<  "ICACHE read from 0xc64 returned " << std::hex << data << v::endl;
    assert(data==0x100f0e0d);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 21. ICACHE read from 0x464 (cache hit) "                     << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    data = iread(0x464, 0x8, 0, 0, debug);
    v::info << name() <<  "ICACHE read from 0x464 returned " << std::hex << data << v::endl;
    assert(data==0x08070605);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 22. ICACHE read from 0x864 (cache hit) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    data = iread(0x864, 0x8, 0, 0, debug);
    v::info << name() <<  "ICACHE read from 0x864 returned " << std::hex << data << v::endl;
    assert(data==0x0c0b0a09);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 23. ICACHE read from 0xc64 (cache hit) "                     << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    data = iread(0xc64, 0x8, 0, 0, debug);
    v::info << name() <<  "ICACHE read from 0xc64 returned " << std::hex << data << v::endl;
    assert(data==0x100f0e0d);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 24. Content of ICACHE line 25 (dbg_out)        * " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    dwrite(0xfe,25,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 25. Content of DCACHE line 25 (dbg_out)        * " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    dwrite(0xff,25,4,2,0,0,0, debug);

    v::info << name() << " ******************************************************************** " << v::endl;
    v::info << name() << " * Phase 4: Test diagnostic read/write of caches " << v::endl;
    v::info << name() << " ******************************************************************** " << v::endl;

    // dtag read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 26. DCACHE read TAG from set 0 line 25 (ASI 0xE) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    // TAG address = SET & LINE [12-5] & SUBBLOCK [4-2] & "00"
    //set 0b0 line 0b00011001 0b00000
    data = dread(0x320, 4, 0xe, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned TAG: " << std::hex << data << v::endl;
    // [] = atag == 0, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000001);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dtag read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 27. DCACHE read TAG from set 1 line 25 (ASI 0xE) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    //set 0b1 line 0b00011001 0b00000
    data = dread(0x2320, 4, 0xe, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned TAG: " << std::hex << data << v::endl;
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000401);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dtag read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 28. DCACHE read TAG from set 2 line 25 (ASI 0xE) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    //set 0b2 line 0b00011001 0b00000
    data = dread(0x4320, 4, 0xe, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned TAG: " << std::hex << data << v::endl;
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000801);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dtag read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 29. DCACHE read TAG from set 3 line 25 (ASI 0xE) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    //set 0b3 line 0b00011001 0b00000
    data = dread(0x6320, 4, 0xe, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned TAG: " << std::hex << data << v::endl;
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000c01);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache entry read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 30. DCACHE read entry 0 from set 0 line 25 (ASI 0xF) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    // TAG address = SET & LINE [12-5] & SUBBLOCK [4-2] & "00"
    //set 0b0 line 0b00011001 subblock 0b000 00
    data = dread(0x320, 4, 0xf, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned DATA: " << std::hex << data << v::endl;
    assert(data==0x04030201);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache entry read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 31. DCACHE read entry 0 from set 1 line 25 (ASI 0xF) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    //set 0b1 line 0b00011001 subblock 0b000 00
    data = dread(0x2320, 4, 0xf, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned DATA: " << std::hex << data << v::endl;
    assert(data==0x08070605);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache entry read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 32. DCACHE read entry 0 from set 2 line 25 (ASI 0xF) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    //set 0b2 line 0b00011001 subblock 0b000 00
    data = dread(0x4320, 4, 0xf, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned DATA: " << std::hex << data << v::endl;
    assert(data==0x0c0b0a09);

    // dcache entry read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 33. DCACHE read entry 0 from set 3 line 25 (ASI 0xF) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    //set 0b3 line 0b00011001 subblock 0b000 00
    data = dread(0x6320, 4, 0xf, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned DATA: " << std::hex << data << v::endl;
    assert(data==0x100f0e0d);

    // itag read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 34. ICACHE read TAG from set 0 line 25 (ASI 0xC) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    // TAG address = SET & LINE [12-5] & SUBBLOCK [4-2] & "00"
    //set 0b0 line 0b00011001 0b00000
    data = dread(0x320, 4, 0xc, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned TAG: " << std::hex << data << v::endl;
    // [] = atag == 0, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000001);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // itag read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 35. ICACHE read TAG from set 1 line 25 (ASI 0xC) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    //set 0b1 line 0b00011001 0b00000
    data = dread(0x2320, 4, 0xc, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned TAG: " << std::hex << data << v::endl;
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000401);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // itag read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 36. ICACHE read TAG from set 2 line 25 (ASI 0xC) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    //set 0b2 line 0b00011001 0b00000
    data = dread(0x4320, 4, 0xc, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned TAG: " << std::hex << data << v::endl;
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000801);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // itag read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 37. ICACHE read TAG from set 3 line 25 (ASI 0xC) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    //set 0b3 line 0b00011001 0b00000
    data = dread(0x6320, 4, 0xc, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned TAG: " << std::hex << data << v::endl;
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000c01);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache entry read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 38. ICACHE read entry 0 from set 0 line 25 (ASI 0xD) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    // TAG address = SET & LINE [12-5] & SUBBLOCK [4-2] & "00"
    //set 0b0 line 0b00011001 subblock 0b000 00
    data = dread(0x320, 4, 0xd, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned DATA: " << std::hex << data << v::endl;
    assert(data==0x04030201);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // icache entry read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 39. ICACHE read entry 0 from set 1 line 25 (ASI 0xD) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    //set 0b1 line 0b00011001 subblock 0b000 00
    data = dread(0x2320, 4, 0xd, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned DATA: " << std::hex << data << v::endl;
    assert(data==0x08070605);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // icache entry read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 40. ICACHE read entry 0 from set 2 line 25 (ASI 0xD) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    //set 0b2 line 0b00011001 subblock 0b000 00
    data = dread(0x4320, 4, 0xd, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned DATA: " << std::hex << data << v::endl;
    assert(data==0x0c0b0a09);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // icache entry read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 41. ICACHE read entry 0 from set 3 line 25 (ASI 0xD) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    //set 0b3 line 0b00011001 subblock 0b000 00
    data = dread(0x6320, 4, 0xd, 0, 0, 0, debug);
    v::info << name() <<  "DCACHE returned DATA: " << std::hex << data << v::endl;
    assert(data==0x100f0e0d);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache tag write
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 42. DCACHE write TAG to set 0 line 26 (ASI 0xE) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (0 << 10) | 1;

    v::info << name() << "DCACHE write TAG: " << std::hex << data << " Address: 0x340" << v::endl;
    // set 0b0 line 0b00011010 subblock 0b000 00
    dwrite(0x340, data, 4, 0xe, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache entry write
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 43. DCACHE write entry 0 to set 0 line 26 (ASI 0xF) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    // set 0b0 line 0b00011010 subblock 0b000 00
    dwrite(0x340, 0x12345678, 4, 0xf, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 44. Content of DCACHE line 26 (dbg_out)        * " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    dwrite(0xff,26,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 45. DCACHE read from Address 0x68 (hit set 0)       " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    data = dread(0x68, 4, 0x8, 0, 0, 0, debug);
    v::info << name() << "DCACHE returned data: " << std::hex << data << v::endl;
    assert(data==0x12345678);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // icache tag write
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 46. ICACHE write TAG to set 3 line 26 (ASI 0xC) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (1 << 10) | 1;

    v::info << name() << "ICACHE write TAG: " << std::hex << data << " Address: 0x6340" << v::endl;
    // set 0b11 line 0b00011010 subblock 0b000 00
    dwrite(0x6340, data, 4, 0xc, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // icache entry write
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 47. ICACHE write entry 0 to set 3 line 26 (ASI 0xD) " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    // set 0b11 line 0b00011010 subblock 0b000 00
    dwrite(0x6340, 0x87654321, 4, 0xd, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 48. Content of ICACHE line 26 (dbg_out)        * " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    dwrite(0xfe,26,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // icache read
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 49. ICACHE read from Address 0x468 (hit set 3)       " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    data = iread(0x468, 0, 0, 0, debug);
    v::info << name() << "ICACHE returned data: " << std::hex << data << v::endl;
    assert(data==0x87654321);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ******************************************************************** " << v::endl;
    v::info << name() << " * Phase 5: Cache flushing           " << v::endl;
    v::info << name() << " ******************************************************************** " << v::endl;

    // flush icache
    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * 50. Flush instruction cache (write with ASI 0x10)    " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    // flushing the instruction cache will cause the diagnostic icache write
    // to be transferred to main memory (set 3 line 26 tag 401 address 0x468 data 0x87654321
    dwrite(0x0, 0x0, 4, 0x10, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // flush dcache
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << " * 51. Flush data cache (write with ASI 0x11)    " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    // Flushing the data cache will cause the diagnostic dcache write
    // to be transferred to main memory (set 0 line 26 tag 1 address 0x68 data 0x12345678)
    dwrite(0x0, 0x0, 4, 0x11, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // To checker whether the flushes have worked correctly, some cache locations
    // are invalidated to provoke misses.

    // invalidate
    v::info << name() << "************************************************** " << v::endl;
    v::info << name() << " * 52. invalidate icache address 0x468                 " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (1 << 10) | 0;

    v::info << name() << "ICACHE write TAG: " << std::hex << data << " Address: 0x6340" << v::endl;
    // set 0b11 line 0b00011010 subblock 0b000 00
    dwrite(0x6340, data, 4, 0xc, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // invalidate
    v::info << name() << "************************************************** " << v::endl;
    v::info << name() << "* 53. invalidate dcache address 0x468                 " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (0 << 10) | 0;

    v::info << name() << "DCACHE write TAG: " << std::hex << data << " Address: 0x340" << v::endl;
    // set 0b0 line 0b00011010 subblock 0b000 00
    dwrite(0x340, data, 4, 0xe, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // icache read
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 54. ICACHE read from Address 0x468 (miss)           " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    data = iread(0x468, 0, 0, 0, debug);
    v::info << name() << "ICACHE returned data: " << std::hex << data << v::endl;
    assert(data==0x87654321);
    assert(CACHEREADMISS_CHECK(*debug));

    // dcache read
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 55. DCACHE read from Address 0x68 (hit set 0)       " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    data = dread(0x68, 4, 0x8, 0, 0, 0, debug);
    v::info << name() << "DCACHE returned data: " << std::hex << data << v::endl;
    assert(data==0x12345678);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // The caches can also be flushed by setting the FD/FI bits of
    // the cache control register.

    // Diagnostic writes are used to write data to the caches
    // which are not present in memory.

    // icache tag write
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 56. ICACHE write TAG to set 3 line 26 (ASI 0xC) " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (2 << 10) | 1;

    v::info << name() << "ICACHE write TAG: " << std::hex << data << " Address: 0x6340" << v::endl;
    // set 0b11 line 0b00011010 subblock 0b000 00
    dwrite(0x6340, data, 4, 0xc, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // icache entry write
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 57. ICACHE write entry 0 to set 3 line 26 (ASI 0xD) " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // set 0b11 line 0b00011010 subblock 0b000 00
    dwrite(0x6340, 0x11111111, 4, 0xd, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

   // dcache tag write
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 58. DCACHE write TAG to set 0 line 26 (ASI 0xE) " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (3 << 10) | 1;

    v::info << name() << "DCACHE write TAG: " << std::hex << data << " Address: 0x340" << v::endl;
    // set 0b0 line 0b00011010 subblock 0b000 00
    dwrite(0x340, data, 4, 0xe, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache entry write
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 59. DCACHE write entry 0 to set 0 line 26 (ASI 0xF) " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // set 0b0 line 0b00011010 subblock 0b000 00
    dwrite(0x340, 0xffffffff, 4, 0xf, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // Now both caches are flushed (diagnostic data -> memory)
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 60. Flush instruction cache with CCR FD/FI (ASI 0x2)" << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // READ the CCR
    data=dread(0x0, 4, 2, 0, 0, 0, debug);

    // set FD and FI
    data |= (3 << 21);

    // write CCR and trigger both caches to flush
    dwrite(0x0, data, 4, 0x2, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // invalidate diagnostic cache data

    // invalidate
    v::info << name() << "************************************************** " << v::endl;
    v::info << name() << "* 61. Invalidate icache address 0x868                 " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (2 << 10) | 0;

    v::info << name() << "ICACHE write TAG: " << std::hex << data << " Address: 0x6340" << v::endl;
    // set 0b11 line 0b00011010 subblock 0b000 00
    dwrite(0x6340, data, 4, 0xc, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // invalidate
    v::info << name() << "************************************************** " << v::endl;
    v::info << name() << "* 62. Invalidate dcache address 0xc68                 " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (3 << 10) | 0;

    v::info << name() << "DCACHE write TAG: " << std::hex << data << " Address: 0x340" << v::endl;
    // set 0b0 line 0b00011010 subblock 0b000 00
    dwrite(0x340, data, 4, 0xe, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // Reading from the addresses of the 'diagnostic' data brings the
    // data back to cache.

    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 63. Content of DCACHE line 26 (dbg_out)         " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    dwrite(0xff,26,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 64. Content of ICACHE line 26 (dbg_out)         " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    dwrite(0xfe,26,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // icache read
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 65.ICACHE read from Address 0x868 (miss)           " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    data = iread(0x868, 0, 0, 0, debug);
    v::info << name() << "ICACHE returned data: " << std::hex << data << v::endl;
    assert(data==0x11111111 );
    assert(CACHEREADMISS_CHECK(*debug));

    // dcache read
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 66. DCACHE read from Address 0xc68 (miss)            " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    data = dread(0xc68, 4, 0x8, 0, 0, 0, debug);
    v::info << name() << "DCACHE returned data: " << std::hex << data << v::endl;
    assert(data==0xffffffff);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // *******************************************
    // * END OF TEST
    // *******************************************

    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * Content of DCACHE line 25 (dbg_out)        * " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    dwrite(0xff,25,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * Content of DCACHE line 26 (dbg_out)        * " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    dwrite(0xff,26,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * Content of ICACHE line 25 (dbg_out)        * " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    dwrite(0xfe,25,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * Content of ICACHE line 26 (dbg_out)        * " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    dwrite(0xfe,26,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);


    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* Phase 6: Sub-word access            " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // dcache write
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 67. Store 4 bytes to tag 5 line 0 (miss)  " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // 1 -> 0x1400
    dwrite(0x1400,1,1,8,0,0,0,debug);
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // 2 -> 0x1401
    dwrite(0x1401,2,1,8,0,0,0,debug);
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // 3 -> 0x1402
    dwrite(0x1402,3,1,8,0,0,0,debug);
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // 4 -> 0x1403
    dwrite(0x1403,4,1,8,0,0,0,debug);
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache read
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 68. Read 1 word and check result (miss)  " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    data = dread(0x1400,4,8,0,0,0,debug);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(data=0x04030201);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache read
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 69. Read 2 shorts and check result (hit)   " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    data = dread(0x1400,2,8,0,0,0,debug);
    assert(CACHEREADHIT_CHECK(*debug));
    assert(data=0x0201);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    data = dread(0x1402,2,8,0,0,0,debug);
    assert(CACHEREADHIT_CHECK(*debug));
    assert(data=0x0403);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache read
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 70. Read 4 bytes and check result (hit)  " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    data = dread(0x1400,1,8,0,0,0,debug);
    assert(CACHEREADHIT_CHECK(*debug));
    assert(data=0x01);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    data = dread(0x1401,1,8,0,0,0,debug);
    assert(CACHEREADHIT_CHECK(*debug));
    assert(data=0x02);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    data = dread(0x1402,1,8,0,0,0,debug);
    assert(CACHEREADHIT_CHECK(*debug));
    assert(data=0x03);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    data = dread(0x1403,1,8,0,0,0,debug);
    assert(CACHEREADHIT_CHECK(*debug));
    assert(data=0x04);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache write
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 71. Store 2 shorts to tag 5 line 1 (miss)  " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // 0x8765 -> 0x1404
    dwrite(0x1404,0x8765,2,8,0,0,0,debug);
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // 0xcba9 -> 0x1406
    dwrite(0x1406,0xcba9,2,8,0,0,0,debug);
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache write
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 72. Read 1 byte from tag 5 line 1 (miss)  " << v::endl;
    v::info << name() << "* The miss on the byte should cause the complete " << v::endl;
    v::info << name() << "* word to be cached. " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    data = dread(0x1405,1,8,0,0,0,debug);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(data=0x87);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache write
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 73. Read the two shorts with one word access (hit)  " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    data = dread(0x1404,4,8,0,0,0,debug);
    assert(CACHEREADHIT_CHECK(*debug));
    assert(data=0xcba98765);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* Phase 7: Disable caches and test bypass mode!  " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // read/write cache control register
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 80. DEACTIVATE CACHES by writing the CONTROL REGISTER " << v::endl;
    v::info << name() << " * (ASI 0x2 - addr 0)    " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    // read cache control register !
    data=dread(0x0, 4, 2, 0, 0, 0, debug);
    // CCR [3:2] == 0b11; [1:0] = 0b11 -> dcache and icache enabled
    v::info << name() << "cache_contr_reg: " << std::hex << data << v::endl;
    assert(data==0xf);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dactivate caches:
    // CCR [3-2] = 0b00 disable dcache, CCR [1-0] = 0b00 disable icache
    dwrite(0x0, 0x0, 4, 2, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // bypass write word
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 81. Write word through data interface (bypass) " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // write word in bypass mode
    dwrite(0x0, 0xeeeeffff, 4, 8, 0, 0, 0, debug);
    assert(CACHEBYPASS_CHECK(*debug));

    // bypass write short
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 82. Write shorts through data interface (bypass) " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // write shorts in bypass mode
    dwrite(0x4, 0xaaaa, 2, 8, 0, 0, 0, debug);
    assert(CACHEBYPASS_CHECK(*debug));

    // write shorts in bypass mode
    dwrite(0x6, 0xbbbb, 2, 8, 0, 0, 0, debug);
    assert(CACHEBYPASS_CHECK(*debug));

    // bypass write byte
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 83. Write bytes through data interface (bypass) " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // write shorts in bypass mode
    dwrite(0x8, 0xcc, 1, 8, 0, 0, 0, debug);
    assert(CACHEBYPASS_CHECK(*debug));

    // write shorts in bypass mode
    dwrite(0x9, 0xdd, 1, 8, 0, 0, 0, debug);
    assert(CACHEBYPASS_CHECK(*debug));

    // write shorts in bypass mode
    dwrite(0xa, 0xee, 1, 8, 0, 0, 0, debug);
    assert(CACHEBYPASS_CHECK(*debug));

    // write shorts in bypass mode
    dwrite(0xb, 0xff, 1, 8, 0, 0, 0, debug);
    assert(CACHEBYPASS_CHECK(*debug));

     // bypass read word
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 84. Read word through data interface (bypass) " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    data = dread(0x8, 4, 8, 0, 0, 0, debug);
    assert(data==0xffeeddcc);
    assert(CACHEBYPASS_CHECK(*debug));

     // bypass read shorts
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 85. Read shorts through data interface (bypass) " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    data = dread(0x0,2,8,0,0,0,debug);
    assert(data==0xffff);
    assert(CACHEBYPASS_CHECK(*debug));


    data = dread(0x2,2,8,0,0,0,debug);
    assert(data==0xeeee);
    assert(CACHEBYPASS_CHECK(*debug));

     // bypass read bytes
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 86. Read shorts through data interface (bypass) " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    data = dread(0x4, 1, 8, 0, 0, 0, debug);
    assert(data==0xaa);
    assert(CACHEBYPASS_CHECK(*debug));

    data = dread(0x1, 1, 8, 0, 0, 0, debug);
    assert(data==0xff);
    assert(CACHEBYPASS_CHECK(*debug));

    data = dread(0x6, 1, 8, 0, 0, 0, debug);
    assert(data==0xbb);
    assert(CACHEBYPASS_CHECK(*debug));

    data = dread(0x3, 1, 8, 0, 0, 0, debug);
    assert(data==0xee);
    assert(CACHEBYPASS_CHECK(*debug));

     // bypass read word
    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 87. Read word through instr. interface (bypass) " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    data = iread(0x8, 0, 0, 0, debug);
    assert(data==0xffeeddcc);
    assert(CACHEBYPASS_CHECK(*debug));

    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* Phase 8: Test cache freeze  " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 88. Invalidate line 0 in all sets of i/d cache. " << v::endl;
    v::info << name() << "* The atag of set 0 - 3 is set to 0 - 3.  " << v::endl;
    v::info << name() << "* This will allow unvalid entries to be replaced " << v::endl;
    v::info << name() << "* by new data with the same tag in frozen mode." << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    // !! in write data atag always starts from bit 10 !!

    // write tag: icache set 0 line 0
    // addr: set 0b00 line 0b00000000 subblock 0b000 00
    dwrite(0x0000, 0 << 10, 4, 0xc, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // write tag: icache set 1 line 0
    // addr: set 0b01 line 0b00000000 subblock 0b000 00
    dwrite(0x2000, 1 << 10, 4, 0xc, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // write tag: icache set 2 line 0
    // addr: set 0b10 line 0b00000000 subblock 0b000 00
    dwrite(0x4000, 2 << 10, 4, 0xc, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // write tag: icache set 3 line 0
    // addr: set 0b11 line 0b00000000 subblock 0b000 00
    dwrite(0x6000, 3 << 10, 4, 0xc, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // write tag: dcache set 0 line 0
    // addr: set 0b00 line 0b00000000 subblock 0b000 00
    dwrite(0x0000, 0 << 10, 4, 0xe, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // write tag: dcache set 1 line 0
    // addr: set 0b01 line 0b00000000 subblock 0b000 00
    dwrite(0x2000, 1 << 10, 4, 0xe, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // write tag: dcache set 2 line 0
    // addr: set 0b10 line 0b00000000 subblock 0b000 00
    dwrite(0x4000, 2 << 10, 4, 0xe, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // write tag: dcache set 3 line 0
    // addr: set 0b11 line 0b00000000 subblock 0b000 00
    dwrite(0x6000, 3 << 10, 4, 0xe, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << "************************************************* " << v::endl;
    v::info << name() << "* 89. Fill mem with data for tag 0 - 5 (line 0)  " << v::endl;
    v::info << name() << "************************************************* " << v::endl;

    dwrite(0x0000,0x11223344,4,8,0,0,0,debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEBYPASS_CHECK(*debug));

    dwrite(0x0400,0x55667788,4,8,0,0,0,debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEBYPASS_CHECK(*debug));

    dwrite(0x0800,0x99aabbcc,4,8,0,0,0,debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEBYPASS_CHECK(*debug));

    dwrite(0x0c00,0xddeeff11,4,8,0,0,0,debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEBYPASS_CHECK(*debug));

    dwrite(0x1000,0x22334455,4,8,0,0,0,debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEBYPASS_CHECK(*debug));

    // read/write cache control register
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 90. ACTIVATE CACHES by writing the CONTROL REGISTER " << v::endl;
    v::info << name() << " * (ASI 0x2 - addr 0)    " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    // read cache control register !
    data=dread(0x0, 4, 2, 0, 0, 0, debug);
    // CCR [3:2] == 0b00; [1:0] = 0b00 -> dcache and icache disabled
    v::info << name() << "cache_contr_reg: " << std::hex << data << v::endl;
    assert(data==0x0);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // activate caches:
    // CCR [3-2] = 0b11 enable dcache, CCR [1-0] = 0b11 enable icache
    dwrite(0x0, 0xf, 4, 2, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read miss
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 91. Fill one of the four sets with data (tag 0, line 0) " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    data=dread(0x0, 4, 8, 0, 0, 0, debug);
    assert(data==0x11223344);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(!(FROZENMISS_CHECK(*debug)));

    data=iread(0x0, 0, 0, 0, debug);
    assert(data==0x11223344);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(!(FROZENMISS_CHECK(*debug)));

    // read/write cache control register
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 92. FREEZE CACHES by writing the CONTROL REGISTER " << v::endl;
    v::info << name() << " * (ASI 0x2 - addr 0)    " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

     // read cache control register !
    data=dread(0x0, 4, 2, 0, 0, 0, debug);
    // CCR [3:2] == 0b11; [1:0] = 0b11 -> dcache and icache enabled
    v::info << name() << "cache_contr_reg: " << std::hex << data << v::endl;
    assert(data==0xf);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // freeze caches:
    // CCR [3-2] = 0b01  freeze dcache, CCR [1-0] = 0b01 freeze icache
    dwrite(0x0, 0x5, 4, 2, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read miss - freeze no effect
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() <<" * 93. Read from tag 1, 2 and 3. This should fill line 0 " << v::endl;
    v::info << name() << " * of the remaining 3 sets " << v::endl;
    v::info << name() << " * The data is cached despite the fact that the cache is" << v::endl;
    v::info << name() << " * frozen, because no preexisting valid data is destroyed." << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    data=dread(0x0400, 4, 8, 0, 0, 0, debug);
    assert(data==0x55667788);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(!(FROZENMISS_CHECK(*debug)));

    data=dread(0x0800, 4, 8, 0, 0, 0, debug);
    assert(data==0x99aabbcc);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(!(FROZENMISS_CHECK(*debug)));

    data=dread(0x0c00, 4, 8, 0, 0, 0, debug);
    assert(data==0xddeeff11);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(!(FROZENMISS_CHECK(*debug)));

    data=iread(0x0400, 0, 0, 0, debug);
    assert(data==0x55667788);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(!(FROZENMISS_CHECK(*debug)));

    data=iread(0x0800, 0, 0, 0, debug);
    assert(data==0x99aabbcc);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(!(FROZENMISS_CHECK(*debug)));

    data=iread(0x0c00, 0, 0, 0, debug);
    assert(data==0xddeeff11);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(!(FROZENMISS_CHECK(*debug)));

    // read miss - frozen data !!
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 94. Line 0 has been filled with valid data, in all four" << v::endl;
    v::info << name() << " * cache sets. Reading again from line 0 (with another tag)" << v::endl;
    v::info << name() << " * prevents the new data from being cached due to the freeze." << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    data=dread(0x1000, 4, 8, 0, 0, 0, debug);
    assert(data==0x22334455);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(FROZENMISS_CHECK(*debug));

    data=iread(0x1000, 0, 0, 0, debug);
    assert(data==0x22334455);
    assert(CACHEREADMISS_CHECK(*debug));
    assert(FROZENMISS_CHECK(*debug));

    // read hit
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 95. Check whether the four original tags are still " << v::endl;
    v::info << name() << " * cached and valid (read hit) " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    data=dread(0x0000, 4, 8, 0, 0, 0, debug);
    assert(data==0x11223344);
    assert(CACHEREADHIT_CHECK(*debug));

    data=dread(0x0400, 4, 8, 0, 0, 0, debug);
    assert(data==0x55667788);
    assert(CACHEREADHIT_CHECK(*debug));

    data=dread(0x0800, 4, 8, 0, 0, 0, debug);
    assert(data==0x99aabbcc);
    assert(CACHEREADHIT_CHECK(*debug));

    data=dread(0x0c00, 4, 8, 0, 0, 0, debug);
    assert(data==0xddeeff11);
    assert(CACHEREADHIT_CHECK(*debug));

    data=iread(0x0000, 0, 0, 0, debug);
    assert(data==0x11223344);
    assert(CACHEREADHIT_CHECK(*debug));

    data=iread(0x0400, 0, 0, 0, debug);
    assert(data==0x55667788);
    assert(CACHEREADHIT_CHECK(*debug));

    data=iread(0x0800, 0, 0, 0, debug);
    assert(data==0x99aabbcc);
    assert(CACHEREADHIT_CHECK(*debug));

    data=iread(0x0c00, 0, 0, 0, debug);
    assert(data==0xddeeff11);
    assert(CACHEREADHIT_CHECK(*debug));

    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * Content of DCACHE line 0 (dbg_out)        * " << v::endl;
    v::info << name() << " ************************************************ " << v::endl;

    dwrite(0xff,0,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * Content of ICACHE line 0 (dbg_out)        * " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    dwrite(0xfe,0,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read/write cache control register
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 96. ACTIVATE CACHES by writing the CONTROL REGISTER " << v::endl;
    v::info << name() << " * (ASI 0x2 - addr 0)    " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;

    // read cache control register !
    data=dread(0x0, 4, 2, 0, 0, 0, debug);
    // CCR [3:2] == 0b01; [1:0] = 0b01 -> dcache and icache frozen
    v::info << name() << "cache_contr_reg: " << std::hex << data << v::endl;
    assert(data==0x5);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // freeze caches:
    // CCR [3-2] = 0b11 enable dcache, CCR [1-0] = 0b11 enable icache
    dwrite(0x0, 0xf, 4, 2, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************* " << v::endl;
    v::info << name() << " * End of test      " << v::endl;
    v::info << name() << " ************************************************* " << v::endl;

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    sc_core::sc_stop();
  }
}
