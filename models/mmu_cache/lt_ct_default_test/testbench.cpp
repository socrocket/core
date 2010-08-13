// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       mmu_cache_test.cpp - Provides two TLM initiator sockets *
// *             and several helper functions to simplify the coding     *
// *             of testbenches for mmu_cache.                           *
// *                                                                     *
// * Modified on $Date$  *
// *          at $Revision$                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************

#include "testbench.h"

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

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * Phase 0: Read system registers (ASI 0x2) ");
    DUMP(name()," ************************************************************");

    // read cache control register
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 1. Read CACHE CONTROL REGISTER (ASI 0x2 - addr 0)    ");
    DUMP(name()," ********************************************************* ");
    
    // args: address, length, asi, flush, flushl, lock, debug 
    data=dread(0x0, 4, 2, 0, 0, 0, debug);
    // [3:2] == 0b11; [1:0] = 0b11 -> dcache and icache enabled
    DUMP(name(),"cache_contr_reg: " << std::hex << data);
    assert(data==0xf);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read icache configuration register
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 2. Read ICACHE CONFIGURATION REGISTER (ASI 0x2 - addr 8)   ");
    DUMP(name()," ********************************************************* ");    

    data=dread(0x8, 4, 2, 0, 0, 0, debug);
    // [29:28] repl == 0b11, [26:24] sets == 0b011, [23:20] ssize == 0b0000 (1kb)
    // [19] lram == 0, [18:16] lsize == 0b000 (1 word per line)
    // [15:12] lramsize == 0, [11:4] lramstart == 0x8e, [3] mmu == 0
    DUMP(name(),"icache_config_reg: " << std::hex << data);
    assert(data==0x330008e0);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read icache configuration register
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 3. Read DCACHE CONFIGURATION REGISTER (ASI 0x2 - addr 0xc)   ");
    DUMP(name()," ********************************************************* ");    

    data=dread(0xc, 4, 2, 0, 0, 0, debug);
    // [29:28] repl == 0b11, [26:24] sets == 0b100, [23:20] ssize == 0b0001 (1kb)
    // [19] lram == 0, [18:16], lsize == 0b001 (1 word per line)
    // [15:12] lramsize == 1, [11:4] lramstart == 0x8f, [3] mmu == 0
    DUMP(name(),"dcache_config_reg: " << std:: hex << data);
    assert(data==0x330008f0);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * Phase 1: Main memory is written through dcache. ");
    DUMP(name()," * The addresses are selected in a way that indices are equal");
    DUMP(name()," * but tags are different (different pages).");
    DUMP(name()," ************************************************************");

    // write some data to memory (cache write miss)
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 4. DCACHE write addr 0x64 (cache write miss)               ");
    DUMP(name()," ********************************************************* ");    

    data=0x04030201;
    // args: address, data, length, asi, flush, flushl, lock 
    dwrite(0x64,data, 4, 0x8, 0, 0, 0, debug);
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 5. DCACHE write addr 0x464 (cache write miss)              ");
    DUMP(name()," ********************************************************* ");    

    data=0x08070605;
    dwrite(0x464,data,4, 0x8, 0, 0, 0, debug);
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 6. DCACHE write addr 0x864 (cache write miss)              ");
    DUMP(name()," ********************************************************* ");    

    data=0x0c0b0a09;
    dwrite(0x864,data,4, 0x8, 0, 0, 0, debug);
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 7. DCACHE write addr 0xc64 (cache write miss)              ");
    DUMP(name()," ********************************************************* ");

    data=0x100f0e0d;
    dwrite(0xc64,data,4, 0x8, 0, 0, 0, debug);
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ********************************************************************  ");
    DUMP(name()," * Phase 2: The data which has been written in Phase 1 is read back ");
    DUMP(name()," * through the data interface. Because the address tags are different, ");
    DUMP(name()," * all four sets of the cache should be filled. The first part ");
    DUMP(name()," * provokes cache misses, the second part cache hits. ");
    DUMP(name()," ******************************************************************** ");
  
    // first read (cache miss)
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 8. DCACHE read addr 0x64 (cache miss)         "                    );
    DUMP(name()," ********************************************************* ");

    // args: address, length, asi, flush, flushl, lock
    data = dread(0x64, 4, 0x8, 0, 0, 0, debug);
    DUMP(name(), "DCACHE read from 0x64 returned " << std::hex << data);
    assert(data==0x04030201);
    DUMP(this->name(),"debug: " << std::hex << *debug);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read again (cache hit)
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 9. DCACHE read again from 0x64 (cache hit)           "             );
    DUMP(name()," ********************************************************* ");

    data = dread(0x64, 4, 0x8, 0, 0, 0, debug);
    DUMP(name(), "DCACHE read from 0x64 returned " << std::hex << data);
    assert(data==0x04030201);
    assert(CACHEREADHIT_CHECK(*debug));
 
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load next bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * 10. DCACHE read addr 0x464 / same idx, diff tag (cache miss) ");
    DUMP(name()," * should fill one of the empty banks (3 left)              ");
    DUMP(name()," ********************************************************** ");
    data = dread(0x464, 4, 0x8, 0, 0, 0, debug);
    DUMP(name(), "DCACHE read from 0x464 returned " << std::hex << data);
    assert(data==0x08070605);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load next bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * 11. DCACHE read addr 0x864 / same idx, diff tag (cache miss) ");
    DUMP(name()," * should fill one of the empty banks (2 left)              ");
    DUMP(name()," ********************************************************** ");

    data = dread(0x864, 4, 0x8, 0, 0, 0, debug);
    DUMP(name(), "DCACHE read from 0x864 returned " << std::hex << data);
    assert(data==0x0c0b0a09);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load the last free bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * 12. DCACHE read addr 0xc64 / same idx, diff tag (cache miss) ");
    DUMP(name()," * should fill the last empty bank                          ");
    DUMP(name()," ********************************************************** ");

    data = dread(0xc64, 4, 0x8, 0, 0, 0, debug);
    DUMP(name(), "DCACHE read from 0xc64 returned " << std::hex << data);
    assert(data==0x100f0e0d);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 13. DCACHE read from 0x464 (cache hit) "             );
    DUMP(name()," ************************************************* ");

    data = dread(0x464, 4, 0x8, 0, 0, 0, debug);
    DUMP(name(), "DCACHE read from 0x464 returned " << std::hex << data);
    assert(data==0x08070605);
    assert(CACHEREADHIT_CHECK(*debug));
    
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 14. DCACHE read from 0x864 (cache hit) ");
    DUMP(name()," ************************************************* ");

    data = dread(0x864, 4, 0x8, 0, 0, 0, debug);
    DUMP(name(), "DCACHE read from 0x864 returned " << std::hex << data);
    assert(data==0x0c0b0a09);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 15. DCACHE read from 0xc64 (cache hit) "             );
    DUMP(name()," ************************************************* ");

    data = dread(0xc64, 4, 0x8, 0, 0, 0, debug);
    DUMP(name(), "DCACHE read from 0xc64 returned " << std::hex << data);
    assert(data==0x100f0e0d);
    assert(CACHEREADHIT_CHECK(*debug));

    DUMP(name()," ******************************************************************** ");
    DUMP(name()," * Phase 3: Similar to Phase 2, the same data is now loaded through ");
    DUMP(name()," * the instruction interface. All sets of the icache should be ");
    DUMP(name()," * filled. The first part provokes misses the second hits. ");
    DUMP(name()," ******************************************************************** ");

    // first read (cache miss)
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 16. ICACHE read addr 0x64 (cache miss)         "                    );
    DUMP(name()," ********************************************************* ");

    // args: addr, length, flush, flushl, fline
    data = iread(0x64, 4, 0x8, 0, 0, debug);
    DUMP(name(), "ICACHE read from 0x64 returned " << std::hex << data);
    assert(data==0x04030201);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read again (cache hit)
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 17. ICACHE read again from 0x64 (cache hit)           "             );
    DUMP(name()," ********************************************************* ");

    data = iread(0x64, 4, 0x8, 0, 0, debug);
    DUMP(name(), "ICACHE read from 0x64 returned " << std::hex << data);
    assert(data==0x04030201);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load next bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * 18. ICACHE read addr 0x464 / same idx, diff tag (cache miss) ");
    DUMP(name()," * should fill one of the empty banks (3 left)              ");
    DUMP(name()," ********************************************************** ");
    data = iread(0x464, 4, 0x8, 0, 0, debug);
    DUMP(name(), "ICACHE read from 0x464 returned " << std::hex << data);
    assert(data==0x08070605);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load next bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * 19. ICACHE read addr 0x864 / same idx, diff tag (cache miss) ");
    DUMP(name()," * should fill one of the empty banks (2 left)              ");
    DUMP(name()," ********************************************************** ");

    data = iread(0x864, 4, 0x8, 0, 0, debug);
    DUMP(name(), "ICACHE read from 0x864 returned " << std::hex << data);
    assert(data==0x0c0b0a09);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load the last free bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * 20. ICACHE read addr 0xc64 / same idx, diff tag (cache miss) ");
    DUMP(name()," * should fill the last empty bank                          ");
    DUMP(name()," ********************************************************** ");

    data = iread(0xc64, 4, 0x8, 0, 0, debug);
    DUMP(name(), "ICACHE read from 0xc64 returned " << std::hex << data);
    assert(data==0x100f0e0d);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 21. ICACHE read from 0x464 (cache hit) "                    );
    DUMP(name()," ************************************************* ");

    data = iread(0x464, 4, 0x8, 0, 0, debug);
    DUMP(name(), "ICACHE read from 0x464 returned " << std::hex << data);
    assert(data==0x08070605);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 22. ICACHE read from 0x864 (cache hit) ");
    DUMP(name()," ************************************************* ");

    data = iread(0x864, 4, 0x8, 0, 0, debug);
    DUMP(name(), "ICACHE read from 0x864 returned " << std::hex << data);
    assert(data==0x0c0b0a09);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 23. ICACHE read from 0xc64 (cache hit) "                    );
    DUMP(name()," ************************************************* ");

    data = iread(0xc64, 4, 0x8, 0, 0, debug);
    DUMP(name(), "ICACHE read from 0xc64 returned " << std::hex << data);
    assert(data==0x100f0e0d);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************* ");
    DUMP(name()," * 24. Content of ICACHE line 25 (dbg_out)        * ");
    DUMP(name()," ************************************************* ");

    dwrite(0xfe,25,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************* ");
    DUMP(name()," * 25. Content of DCACHE line 25 (dbg_out)        * ");
    DUMP(name()," ************************************************* ");

    dwrite(0xff,25,4,2,0,0,0, debug);
   
    DUMP(name()," ******************************************************************** ");
    DUMP(name()," * Phase 4: Test diagnostic read/write of caches ");
    DUMP(name()," ******************************************************************** ");

    // dtag read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 26. DCACHE read TAG from set 0 line 25 (ASI 0xE) ");
    DUMP(name()," ************************************************* ");
    
    // TAG address = SET & LINE [12-5] & SUBBLOCK [4-2] & "00"
    //set 0b0 line 0b00011001 0b00000
    data = dread(0x320, 4, 0xe, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned TAG: " << std::hex << data);
    // [] = atag == 0, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000001);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dtag read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 27. DCACHE read TAG from set 1 line 25 (ASI 0xE) ");
    DUMP(name()," ************************************************* ");
    
    //set 0b1 line 0b00011001 0b00000
    data = dread(0x2320, 4, 0xe, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned TAG: " << std::hex << data);
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000401);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dtag read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 28. DCACHE read TAG from set 2 line 25 (ASI 0xE) ");
    DUMP(name()," ************************************************* ");
    
    //set 0b2 line 0b00011001 0b00000    
    data = dread(0x4320, 4, 0xe, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned TAG: " << std::hex << data);
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000801);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dtag read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 29. DCACHE read TAG from set 3 line 25 (ASI 0xE) ");
    DUMP(name()," ************************************************* ");
    
    //set 0b3 line 0b00011001 0b00000
    data = dread(0x6320, 4, 0xe, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned TAG: " << std::hex << data);
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000c01);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache entry read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 30. DCACHE read entry 0 from set 0 line 25 (ASI 0xF) ");
    DUMP(name()," ************************************************* ");

    // TAG address = SET & LINE [12-5] & SUBBLOCK [4-2] & "00"
    //set 0b0 line 0b00011001 subblock 0b000 00
    data = dread(0x320, 4, 0xf, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned DATA: " << std::hex << data);
    assert(data==0x04030201);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache entry read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 31. DCACHE read entry 0 from set 1 line 25 (ASI 0xF) ");
    DUMP(name()," ************************************************* ");

    //set 0b1 line 0b00011001 subblock 0b000 00
    data = dread(0x2320, 4, 0xf, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned DATA: " << std::hex << data);
    assert(data==0x08070605);

    wait(LOCAL_CLOCK,sc_core::SC_NS);    

    // dcache entry read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 32. DCACHE read entry 0 from set 2 line 25 (ASI 0xF) ");
    DUMP(name()," ************************************************* ");

    //set 0b2 line 0b00011001 subblock 0b000 00    
    data = dread(0x4320, 4, 0xf, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned DATA: " << std::hex << data);
    assert(data==0x0c0b0a09);

    // dcache entry read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 33. DCACHE read entry 0 from set 3 line 25 (ASI 0xF) ");
    DUMP(name()," ************************************************* ");

    //set 0b3 line 0b00011001 subblock 0b000 00
    data = dread(0x6320, 4, 0xf, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned DATA: " << std::hex << data);
    assert(data==0x100f0e0d);

    // itag read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 34. ICACHE read TAG from set 0 line 25 (ASI 0xC) ");
    DUMP(name()," ************************************************* ");

    // TAG address = SET & LINE [12-5] & SUBBLOCK [4-2] & "00"
    //set 0b0 line 0b00011001 0b00000
    data = dread(0x320, 4, 0xc, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned TAG: " << std::hex << data);
    // [] = atag == 0, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000001);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // itag read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 35. ICACHE read TAG from set 1 line 25 (ASI 0xC) ");
    DUMP(name()," ************************************************* ");
    
    //set 0b1 line 0b00011001 0b00000
    data = dread(0x2320, 4, 0xc, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned TAG: " << std::hex << data);
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000401);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // itag read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 36. ICACHE read TAG from set 2 line 25 (ASI 0xC) ");
    DUMP(name()," ************************************************* ");
    
    //set 0b2 line 0b00011001 0b00000    
    data = dread(0x4320, 4, 0xc, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned TAG: " << std::hex << data);
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000801);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // itag read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 37. ICACHE read TAG from set 3 line 25 (ASI 0xC) ");
    DUMP(name()," ************************************************* ");
    
    //set 0b3 line 0b00011001 0b00000
    data = dread(0x6320, 4, 0xc, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned TAG: " << std::hex << data);
    // [31:10] = atag == 1, [9] lrr = 0, [8] lock = 0, LSBs valid = 1
    assert(data==0x00000c01);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // dcache entry read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 38. ICACHE read entry 0 from set 0 line 25 (ASI 0xD) ");
    DUMP(name()," ************************************************* ");

    // TAG address = SET & LINE [12-5] & SUBBLOCK [4-2] & "00"
    //set 0b0 line 0b00011001 subblock 0b000 00
    data = dread(0x320, 4, 0xd, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned DATA: " << std::hex << data);
    assert(data==0x04030201);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // icache entry read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 39. ICACHE read entry 0 from set 1 line 25 (ASI 0xD) ");
    DUMP(name()," ************************************************* ");

    //set 0b1 line 0b00011001 subblock 0b000 00
    data = dread(0x2320, 4, 0xd, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned DATA: " << std::hex << data);
    assert(data==0x08070605);

    wait(LOCAL_CLOCK,sc_core::SC_NS);    

    // icache entry read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 40. ICACHE read entry 0 from set 2 line 25 (ASI 0xD) ");
    DUMP(name()," ************************************************* ");

    //set 0b2 line 0b00011001 subblock 0b000 00    
    data = dread(0x4320, 4, 0xd, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned DATA: " << std::hex << data);
    assert(data==0x0c0b0a09);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // icache entry read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 41. ICACHE read entry 0 from set 3 line 25 (ASI 0xD) ");
    DUMP(name()," ************************************************* ");

    //set 0b3 line 0b00011001 subblock 0b000 00
    data = dread(0x6320, 4, 0xd, 0, 0, 0, debug);
    DUMP(name(), "DCACHE returned DATA: " << std::hex << data);
    assert(data==0x100f0e0d);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // dcache tag write
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 42. DCACHE write TAG to set 0 line 26 (ASI 0xE) ");
    DUMP(name()," ************************************************* ");

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (0 << 10) | 1;
    
    DUMP(name(),"DCACHE write TAG: " << std::hex << data << " Address: 0x340");
    // set 0b0 line 0b00011010 subblock 0b000 00
    dwrite(0x340, data, 4, 0xe, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // dcache entry write
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 43. DCACHE write entry 0 to set 0 line 26 (ASI 0xF) ");
    DUMP(name()," ************************************************* ");

    // set 0b0 line 0b00011010 subblock 0b000 00
    dwrite(0x340, 0x12345678, 4, 0xf, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    DUMP(name()," ************************************************* ");
    DUMP(name()," * 44. Content of DCACHE line 26 (dbg_out)        * ");
    DUMP(name()," ************************************************* ");

    dwrite(0xff,26,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // dcache read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 45. DCACHE read from Address 0x68 (hit set 0)       ");
    DUMP(name()," ************************************************* ");

    data = dread(0x68, 4, 0x8, 0, 0, 0, debug);
    DUMP(name(),"DCACHE returned data: " << std::hex << data);
    assert(data==0x12345678);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // icache tag write
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 46. ICACHE write TAG to set 3 line 26 (ASI 0xC) ");
    DUMP(name()," ************************************************* ");

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (1 << 10) | 1;
    
    DUMP(name(),"ICACHE write TAG: " << std::hex << data << " Address: 0x6340");
    // set 0b11 line 0b00011010 subblock 0b000 00
    dwrite(0x6340, data, 4, 0xc, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // icache entry write
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 47. ICACHE write entry 0 to set 3 line 26 (ASI 0xD) ");
    DUMP(name()," ************************************************* ");

    // set 0b11 line 0b00011010 subblock 0b000 00
    dwrite(0x6340, 0x87654321, 4, 0xd, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    DUMP(name()," ************************************************* ");
    DUMP(name()," * 48. Content of ICACHE line 26 (dbg_out)        * ");
    DUMP(name()," ************************************************* ");

    dwrite(0xfe,26,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // icache read
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 49. ICACHE read from Address 0x468 (hit set 3)       ");
    DUMP(name()," ************************************************* ");

    data = iread(0x468, 4, 0, 0, 0, debug);
    DUMP(name(),"ICACHE returned data: " << std::hex << data);
    assert(data==0x87654321);
    assert(CACHEREADHIT_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS); 
    
    DUMP(name()," ******************************************************************** ");
    DUMP(name()," * Phase 5: Cache flushing           ");
    DUMP(name()," ******************************************************************** ");

    // flush icache
    DUMP(name()," ************************************************* ");
    DUMP(name()," * 50. Flush instruction cache (write with ASI 0x10)    ");
    DUMP(name()," ************************************************* ");

    // flushing the instruction cache will cause the diagnostic icache write
    // to be transferred to main memory (set 3 line 26 tag 401 address 0x468 data 0x87654321
    dwrite(0x0, 0x0, 4, 0x10, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // flush dcache
    DUMP(name(),"************************************************* ");
    DUMP(name()," * 51. Flush data cache (write with ASI 0x11)    ");
    DUMP(name()," ************************************************* ");

    // Flushing the data cache will cause the diagnostic dcache write
    // to be transferred to main memory (set 0 line 26 tag 1 address 0x68 data 0x12345678)
    dwrite(0x0, 0x0, 4, 0x11, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // To checker whether the flushes have worked correctly, some cache locations
    // are invalidated to provoke misses.

    // invalidate
    DUMP(name(),"************************************************** ");
    DUMP(name()," * 52. invalidate icache address 0x468                 ");
    DUMP(name()," ************************************************* ");

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (1 << 10) | 0;
    
    DUMP(name(),"ICACHE write TAG: " << std::hex << data << " Address: 0x6340");
    // set 0b11 line 0b00011010 subblock 0b000 00
    dwrite(0x6340, data, 4, 0xc, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);    

    // invalidate
    DUMP(name(),"************************************************** ");
    DUMP(name(),"* 53. invalidate dcache address 0x468                 ");
    DUMP(name(),"************************************************* ");

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (0 << 10) | 0;
    
    DUMP(name(),"DCACHE write TAG: " << std::hex << data << " Address: 0x340");
    // set 0b0 line 0b00011010 subblock 0b000 00
    dwrite(0x340, data, 4, 0xe, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // icache read
    DUMP(name(),"************************************************* ");
    DUMP(name(),"* 54. ICACHE read from Address 0x468 (miss)           ");
    DUMP(name(),"************************************************* ");

    data = iread(0x468, 4, 0, 0, 0, debug);
    DUMP(name(),"ICACHE returned data: " << std::hex << data);
    assert(data==0x87654321);
    assert(CACHEREADMISS_CHECK(*debug));

    // dcache read
    DUMP(name(),"************************************************* ");
    DUMP(name(),"* 55. DCACHE read from Address 0x68 (hit set 0)       ");
    DUMP(name(),"************************************************* ");

    data = dread(0x68, 4, 0x8, 0, 0, 0, debug);
    DUMP(name(),"DCACHE returned data: " << std::hex << data);
    assert(data==0x12345678);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // The caches can also be flushed by setting the FD/FI bits of
    // the cache control register.

    // Diagnostic writes are used to write data to the caches
    // which are not present in memory.

    // icache tag write
    DUMP(name(),"************************************************* ");
    DUMP(name(),"* 56. ICACHE write TAG to set 3 line 26 (ASI 0xC) ");
    DUMP(name(),"************************************************* ");

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (2 << 10) | 1;
    
    DUMP(name(),"ICACHE write TAG: " << std::hex << data << " Address: 0x6340");
    // set 0b11 line 0b00011010 subblock 0b000 00
    dwrite(0x6340, data, 4, 0xc, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // icache entry write
    DUMP(name(),"************************************************* ");
    DUMP(name(),"* 57. ICACHE write entry 0 to set 3 line 26 (ASI 0xD) ");
    DUMP(name(),"************************************************* ");

    // set 0b11 line 0b00011010 subblock 0b000 00
    dwrite(0x6340, 0x11111111, 4, 0xd, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

   // dcache tag write
    DUMP(name(),"************************************************* ");
    DUMP(name(),"* 58. DCACHE write TAG to set 0 line 26 (ASI 0xE) ");
    DUMP(name(),"************************************************* ");

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (3 << 10) | 1;
    
    DUMP(name(),"DCACHE write TAG: " << std::hex << data << " Address: 0x340");
    // set 0b0 line 0b00011010 subblock 0b000 00
    dwrite(0x340, data, 4, 0xe, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // dcache entry write
    DUMP(name(),"************************************************* ");
    DUMP(name(),"* 59. DCACHE write entry 0 to set 0 line 26 (ASI 0xF) ");
    DUMP(name(),"************************************************* ");

    // set 0b0 line 0b00011010 subblock 0b000 00
    dwrite(0x340, 0xffffffff, 4, 0xf, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // Now both caches are flushed (diagnostic data -> memory)
    DUMP(name(),"************************************************* ");
    DUMP(name(),"* 60. Flush instruction cache with CCR FD/FI (ASI 0x2)");
    DUMP(name(),"************************************************* ");

    // READ the CCR
    data=dread(0x0, 4, 2, 0, 0, 0, debug);

    // set FD and FI
    data |= (3 << 21);

    // write CCR and trigger both caches to flush
    dwrite(0x0, data, 4, 0x2, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // invalidate diagnostic cache data
    
    // invalidate
    DUMP(name(),"************************************************** ");
    DUMP(name(),"* 61. invalidate icache address 0x868                 ");
    DUMP(name(),"************************************************* ");

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (2 << 10) | 0;
    
    DUMP(name(),"ICACHE write TAG: " << std::hex << data << " Address: 0x6340");
    // set 0b11 line 0b00011010 subblock 0b000 00
    dwrite(0x6340, data, 4, 0xc, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);    

    // invalidate
    DUMP(name(),"************************************************** ");
    DUMP(name(),"* 62. invalidate dcache address 0xc68                 ");
    DUMP(name(),"************************************************* ");

    // atag starts from bit 10; entry 0 (bit 0) valid
    data = (3 << 10) | 0;
    
    DUMP(name(),"DCACHE write TAG: " << std::hex << data << " Address: 0x340");
    // set 0b0 line 0b00011010 subblock 0b000 00
    dwrite(0x340, data, 4, 0xe, 0, 0, 0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // Reading from the addresses of the 'diagnostic' data brings the 
    // data back to cache.

    DUMP(name(),"************************************************* ");
    DUMP(name(),"* 63. Content of DCACHE line 26 (dbg_out)         ");
    DUMP(name(),"************************************************* ");

    dwrite(0xff,26,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name(),"************************************************* ");
    DUMP(name(),"* 64. Content of ICACHE line 26 (dbg_out)         ");
    DUMP(name(),"************************************************* ");

    dwrite(0xfe,26,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // icache read
    DUMP(name(),"************************************************* ");
    DUMP(name(),"* 65.ICACHE read from Address 0x868 (miss)           ");
    DUMP(name(),"************************************************* ");

    data = iread(0x868, 4, 0, 0, 0, debug);
    DUMP(name(),"ICACHE returned data: " << std::hex << data);
    assert(data==0x11111111 );
    assert(CACHEREADMISS_CHECK(*debug));

    // dcache read
    DUMP(name(),"************************************************* ");
    DUMP(name(),"* 66. DCACHE read from Address 0xc68 (miss)            ");
    DUMP(name(),"************************************************* ");

    data = dread(0xc68, 4, 0x8, 0, 0, 0, debug);
    DUMP(name(),"DCACHE returned data: " << std::hex << data);
    assert(data==0xffffffff);
    assert(CACHEREADMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // *******************************************
    // * END OF TEST
    // *******************************************

    DUMP(name()," ************************************************* ");
    DUMP(name()," * 100. Content of DCACHE line 25 (dbg_out)        * ");
    DUMP(name()," ************************************************* ");

    dwrite(0xff,25,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************* ");
    DUMP(name()," * 101. Content of DCACHE line 26 (dbg_out)        * ");
    DUMP(name()," ************************************************* ");

    dwrite(0xff,26,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    DUMP(name()," ************************************************* ");
    DUMP(name()," * 102. Content of ICACHE line 25 (dbg_out)        * ");
    DUMP(name()," ************************************************* ");

    dwrite(0xfe,25,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************* ");
    DUMP(name()," * Content of ICACHE line 26 (dbg_out)        * ");
    DUMP(name()," ************************************************* ");

    dwrite(0xfe,26,4,2,0,0,0, debug);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    sc_core::sc_stop();
  }
}
