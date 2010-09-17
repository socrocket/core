// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       testbench.cpp - Implementation of the                   *
// *             stimuli generator/monitor for the current testbench.    *
// *                                                                     *
// * Modified on $Date: 2010-08-19 13:36:00 +0200 (Thu, 19 Aug 2010) $   *
// *          at $Revision: 49 $                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************

#include "testbench.h"
#include "verbose.h"

/// testbench initiator thread
void testbench::initiator_thread(void) {

  // test vars
  unsigned int data;
  unsigned int tmp;
  unsigned int seta, setb, setc;
  unsigned int * debug;

  while(1) {

    debug = &tmp;

    // *******************************************
    // * Test for icache configuration
    // * -----------------------------------------
    // * Number of cache sets 3
    // * Size of each cache set 8 kb
    // * Bytes per line 16 (offset bits: 4)
    // * Number of cache lines per set 512 (index bits: 9)
    // * Width of cache tag in bits 19
    // * Replacement strategy: 2 (LRR)
    // * Line Locking: 1
    // * MMU: 0
    // *******************************************

    // *******************************************
    // * Test for dcache configuration
    // * -----------------------------------------
    // * Number of cache sets 3
    // * Size of each cache set 8 kb
    // * Bytes per line 16 (offset bits: 4)
    // * Number of cache lines per set 512 (index bits: 9)
    // * Width of cache tag in bits 19
    // * Replacement strategy: 1 (LRU)
    // * Line Locking: 1
    // * MMU: 0
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
    // args: address, length, asi, flush, flushl, lock 
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
    // [29:28] repl == 0b10, [26:24] sets == 0b001, [23:20] ssize == 0b0011 (8kb)
    // [19] lram == 0, [18:16] lsize == 0b010 (4 word per line)
    // [15:12] lramsize == 0, [11:4] lramstart == 0x8e, [3] mmu present = 0
    v::info << name() << "icache_config_reg: " << std::hex << data << v::endl;
    assert(data==0x213208e0);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read icache configuration register
    v::info << name() << " ********************************************************* " << v::endl;
    v::info << name() << " * 3. Read DCACHE CONFIGURATION REGISTER (ASI 0x2 - addr 0xc)   " << v::endl;
    v::info << name() << " ********************************************************* " << v::endl;    

    data=dread(0xc, 4, 2, 0, 0, 0, debug);
    // [29:28] repl == 0b01, [26:24] sets == 0b010, [23:20] ssize == 0b0011 (8kb)
    // [19] lram == 0, [18:16], lsize == 0b010 (4 word per line)
    // [15:12] lramsize == 0, [11:4] lramstart == 0x8f, [3] mmu present = 0
    v::info << name() << "dcache_config_reg: " << std:: hex << data << v::endl;
    assert(data==0x123208f0);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * Phase 1: Test LRU on DCACHE " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 4. Initialize main memory " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;
    
    // | 31 - 13 (19 bit tag) | 12 - 4 (9 bit index) | 3 - 0 (4 bit offset) |

    // 0x00002000 (tag == 1)

    // init 10 lines
    for(unsigned int i = 0; i < 8*10; i++) {

      // args: address, data, length, asi, flush, flushl, lock
      dwrite((0x00002000 + (i<<2)), i, 4, 0x8, 0, 0, 0, debug);
      assert(CACHEWRITEMISS_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS);  
    }

    // 0x0004000 (tag == 2)

    // init 10 lines
    for(unsigned int i = 0; i < 8*10; i++) {

      // args: address, data, length, asi, flush, flushl, lock
      dwrite((0x00004000 + (i<<2)), i, 4, 0x8, 0, 0, 0, debug);
      assert(CACHEWRITEMISS_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS);  
    }

    // 0x0006000 (tag == 3)

    // init 10 lines
    for(unsigned int i = 0; i < 8*10; i++) {

      // args: address, data, length, asi, flush, flushl, lock
      dwrite((0x00006000 + (i<<2)), i, 4, 0x8, 0, 0, 0, debug);
      assert(CACHEWRITEMISS_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS);  
    }

    // 0x00008000 (tag == 4)

    // init 10 lines
    for(unsigned int i = 0; i < 8*10; i++) {

      // args: address, data, length, asi, flush, flushl, lock
      dwrite((0x00008000 + (i<<2)), i, 4, 0x8, 0, 0, 0, debug);
      assert(CACHEWRITEMISS_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS);  
    }

    // 0x0000a000 (tag == 5)

    // init 10 lines
    for(unsigned int i = 0; i < 8*10; i++) {

      // args: address, data, length, asi, flush, flushl, lock
      dwrite((0x0000a000 + (i<<2)), i, 4, 0x8, 0, 0, 0, debug);
      assert(CACHEWRITEMISS_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS);  
    }

    // 0x0000c000 (tag == 6)

    // init 10 lines
    for(unsigned int i = 0; i < 8*10; i++) {

      // args: address, data, length, asi, flush, flushl, lock
      dwrite((0x0000c000 + (i<<2)), i, 4, 0x8, 0, 0, 0, debug);
      assert(CACHEWRITEMISS_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS);  
    }

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 5. Fill line 3 offset 2 of set 1, 2, and 3 " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // load tag 1
    data = dread(0x00002032, 4, 8, 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // load tag 2
    data = dread(0x00004032, 4, 8, 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // load tag 3
    data = dread(0x00006032, 4, 8, 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 6. Check if the data was cached correctly " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;
 
    // load tag 1
    data = dread(0x00002032, 4, 8, 0, 0, 0, debug);
    assert(CACHEREADHIT_CHECK(*debug));
    seta = *debug & 0x3;
    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // load tag 2
    data = dread(0x00004032, 4, 8, 0, 0, 0, debug);
    assert(CACHEREADHIT_CHECK(*debug));
    setb = *debug & 0x3;
    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // load tag 3
    data = dread(0x00006032, 4, 8, 0, 0, 0, debug);
    assert(CACHEREADHIT_CHECK(*debug));
    setc = *debug & 0x3;
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 7. Check LRU - The sets should be replaced in the same " << v::endl;
    v::info << name() << " *    order they have been written in 6." << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // load tag 4 (supposed to go into seta)
    data = dread(0x00008032, 4, 8, 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    assert((*debug & 0x3) == seta);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    
    // load tag 5 (supposed to go into setb)
    data = dread(0x0000a032, 4, 8, 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    assert((*debug & 0x3) == setb);
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // load tag 6 (supposed to go into setb)
    data = dread(0x0000c032, 4, 8, 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    assert((*debug & 0x3) == setc);
    wait(LOCAL_CLOCK,sc_core::SC_NS);  
    
    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * Phase 2: Test LRR on ICACHE " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 8. Fill line 3 offset 2 of set 1 and 2 " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;    

    // load tag 1
    data = iread(0x00002032, 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // load tag 2
    data = iread(0x00004032, 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    wait(LOCAL_CLOCK,sc_core::SC_NS); 
    
    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 9. Check if the data was cached correctly " << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // load tag 1
    data = iread(0x00002032, 0, 0, 0, debug);
    assert(CACHEREADHIT_CHECK(*debug));
    seta = *debug & 0x3;
    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // load tag 2
    data = iread(0x00004032, 0, 0, 0, debug);
    assert(CACHEREADHIT_CHECK(*debug));
    setb = *debug & 0x3;
    wait(LOCAL_CLOCK,sc_core::SC_NS); 
 
    v::info << name() << " ************************************************************ " << v::endl;
    v::info << name() << " * 10. Check LRR - The sets should be replaced in the same " << v::endl;
    v::info << name() << " *     order they have been written in 6." << v::endl;
    v::info << name() << " ************************************************************" << v::endl;

    // load tag 3
    data = iread(0x00006032, 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    assert((*debug & 0x3)==seta);
    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // load tag 4
    data = iread(0x00008032, 0, 0, 0, debug);
    assert(CACHEREADMISS_CHECK(*debug));
    assert((*debug & 0x3)==setb);
    wait(LOCAL_CLOCK,sc_core::SC_NS);     

    wait(LOCAL_CLOCK,sc_core::SC_NS);
    sc_core::sc_stop();


  }
}
