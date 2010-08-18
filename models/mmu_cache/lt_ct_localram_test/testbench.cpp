/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       testbench.cpp - Implementation of the                   */
/*             stimuli generator/monitor for the current testbench.    */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

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
    // [29:28] repl == 0b11, [26:24] sets == 0b001, [23:20] ssize == 0b1000 (256kb)
    // [19] lram == 1, [18:16] lsize == 0b000 (1 word per line)
    // [15:12] lramsize == 0b1001 (512kb), [11:4] lramstart == 0x8e, [3] mmu = 0
    DUMP(name(),"icache_config_reg: " << std::hex << data);
    assert(data==0x318898e0);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read icache configuration register
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 3. Read DCACHE CONFIGURATION REGISTER (ASI 0x2 - addr 0xc)   ");
    DUMP(name()," ********************************************************* ");    

    data=dread(0xc, 4, 2, 0, 0, 0, debug);
    // [29:28] repl == 0b11, [26:24] sets == 0b001, [23:20] ssize == 0b1000 (256kb)
    // [19] lram == 1, [18:16] lsize == 0b000 (1 word per line)
    // [15:12] lramsize == 0b1001 (512kb), [11:4] lramstart == 0x8e, [3] mmu = 0
    DUMP(name(),"dcache_config_reg: " << std:: hex << data);
    assert(data==0x318898f0);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * Phase 1: Test the Cache ");
    DUMP(name()," ************************************************************");

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 4. Initialize 768kB of main memory (3x cache set size) ");
    DUMP(name()," ************************************************************");

    for (int i = 0; i < 0xc0000; i+=4) {

      // write only beginning and end of cache page
      if(((i&0x3ffff)<0x100)||((i&0x3ffff)>0x3ff00)) {

	dwrite(i,i,4,8,0,0,0,debug);
	assert(CACHEWRITEMISS_CHECK(*debug));

	wait(LOCAL_CLOCK,sc_core::SC_NS);
      }
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 5. Read 512 kB of memory (cache misses) ");
    DUMP(name()," * This should completely fill the two cache sets (2x 256kB) ");
    DUMP(name()," ************************************************************");

    // tags 0 and 1
    for (unsigned int i = 0; i < 0x80000; i+=4) {

      // read only beginning and end
      if((i<0x100)||(i>0x7ff00)) {
	data=dread(i, 4, 8, 0, 0, 0, debug);
	assert(data==i);
	assert(CACHEREADMISS_CHECK(*debug));
 
	wait(LOCAL_CLOCK,sc_core::SC_NS);   
      }
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 6. Read the same data again (cache hits) ");
    DUMP(name()," ************************************************************");   

    // tags 0 and 1
    for (unsigned int i = 0; i < 0x80000; i+=4) {

      // read only beginning and end
      if((i<0x100)||(i>0x7ff00)) {
	data=dread(i, 4, 8, 0, 0, 0, debug);
	assert(data==i);
	assert(CACHEREADHIT_CHECK(*debug));
 
	wait(LOCAL_CLOCK,sc_core::SC_NS);   
      }
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 7. Read another 256 kB from memory (cache miss) ");
    DUMP(name()," ************************************************************");

    // tag 2
    for (unsigned int i = 0x80000; i < 0xc0000; i+=4) {

      // read only beginning and end
      if((i<0x80100)||(i>0xbff00)) {
	data=dread(i, 4, 8, 0, 0, 0, debug);
	assert(data==i);
	assert(CACHEREADMISS_CHECK(*debug));
 
	wait(LOCAL_CLOCK,sc_core::SC_NS);   
      }
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 8. Read again the new 256 kB (cache miss) ");
    DUMP(name()," ************************************************************");

    // tag 2
    for (unsigned int i = 0x80000; i < 0xc0000; i+=4) {

      // read only beginning and end
      if((i<0x80100)||(i>0xbff00)) {
	data=dread(i, 4, 8, 0, 0, 0, debug);
	assert(data==i);
	assert(CACHEREADHIT_CHECK(*debug));
 
	wait(LOCAL_CLOCK,sc_core::SC_NS);   
      }
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * Phase 2: Test instruction scratchpad ");
    DUMP(name()," ************************************************************");   

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 9. Fill the instruction scratchpad with data ");
    DUMP(name()," ************************************************************");    

    // start of scratchpad segment
    for (unsigned int i = 0x8e000000; i < 0x8e080000; i+=4) {

      // write only beginning and end
      if((i<0x8e000100)||(i>0x8e07ff00)) {
	dwrite(i, i, 4, 8, 0, 0, 0, debug);
	assert(SCRATCHPAD_CHECK(*debug));

	wait(LOCAL_CLOCK,sc_core::SC_NS);  
      }
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 10. Read the instruction scratchpad");
    DUMP(name()," ************************************************************");       

    // start of scratchpad segment
    for (unsigned int i = 0x8e000000; i < 0x8e080000; i+=4) {

      // read only beginning and end
      if((i<0x8e000100)||(i>0x8e07ff00)) {

	data=iread(i, 0, 0, 0, debug);
	assert(data==i);
	assert(SCRATCHPAD_CHECK(*debug));

	wait(LOCAL_CLOCK,sc_core::SC_NS);
      }
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 11. Fill the data scratchpad with data");
    DUMP(name()," ************************************************************");    

    for (unsigned int i = 0x8f000000; i < 0x8f080000; i+=4) {

      // write only beginning and end
      if((i<0x8f000100)||(i>0x8f07ff00)) {
	dwrite(i, i, 4, 8, 0, 0, 0, debug);
	assert(SCRATCHPAD_CHECK(*debug));

	wait(LOCAL_CLOCK,sc_core::SC_NS);  
      }
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 12. Read the data scratchpad");
    DUMP(name()," ************************************************************"); 

    for (unsigned int i = 0x8f000000; i < 0x8f080000; i+=4) {

      // read only beginning and end
      if((i<0x8f000100)||(i>0x8f07ff00)) {
	data=dread(i, 4, 8, 0, 0, 0, debug);
	assert(SCRATCHPAD_CHECK(*debug));

	wait(LOCAL_CLOCK,sc_core::SC_NS);  
      }
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 13. Test byte store for data scratchpad ");
    DUMP(name()," ************************************************************");    

    for (unsigned int i = 0; i < 4; i++) {

      dwrite(0x8f000000+i, i+1, 1, 8, 0, 0, 0, debug);
      assert(SCRATCHPAD_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS); 
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 14. Test byte load for data scratchpad ");
    DUMP(name()," ************************************************************");     

    for (unsigned int i = 0; i < 4; i++) {

      data=dread(0x8f000000+i, 1, 8, 0, 0, 0, debug);
      assert(data==(i+1));
      assert(SCRATCHPAD_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS); 
    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 15. Test half-word read for data scratchpad ");
    DUMP(name()," ************************************************************");     

    data=dread(0x8f000000, 2, 8, 0, 0, 0, debug);
    assert(data==0x0201);
    assert(SCRATCHPAD_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    data=dread(0x8f000002, 2, 8, 0, 0, 0, debug);
    assert(data==0x0403);
    assert(SCRATCHPAD_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * 16. Test half-word write for data scratchpad ");
    DUMP(name()," ************************************************************");   

    dwrite(0x8f000004, 0x0b0a, 2, 8, 0, 0, 0, debug);
    assert(SCRATCHPAD_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    dwrite(0x8f000006, 0x0d0c, 2, 8, 0, 0, 0, debug);
    assert(SCRATCHPAD_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read back and check
    data=dread(0x8f000004, 4, 8, 0, 0, 0, debug);
    assert(data==0x0d0c0b0a);
    assert(SCRATCHPAD_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    sc_core::sc_stop();
  }
}

