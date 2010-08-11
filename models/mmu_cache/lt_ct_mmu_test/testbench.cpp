// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       testbench.cpp - Implementation of the                   *
// *             stimuli generator/monitor for the current testbench.    *
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision$                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************

#include "testbench.h"

// constructor
SC_HAS_PROCESS(testbench);
testbench::testbench(sc_core::sc_module_name name) : sc_module(name), 
		     instruction_initiator_socket("instruction_initiator_socket"),
		     data_initiator_socket("data_initiator_socket")
{

  // register testbench thread
  SC_THREAD(initiator_thread);

}

/// testbench initiator thread
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
    
    // args: address, length, asi, flush, flushl, lock 
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
    // [29:28] repl == 0b11, [26:24] sets == 0b100, [23:20] ssize == 0b0001 (1kb)
    // [18:16] lsize == 0b001 (1 word per line), [3] mmu present = 1
    //assert(data==0b0011 0 100 0001 0 001 00000000000000000);
    DUMP(name(),"icache_config_reg: " << std::hex << data);
    assert(data==0x34110008);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read icache configuration register
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 3. Read DCACHE CONFIGURATION REGISTER (ASI 0x2 - addr 0xc)   ");
    DUMP(name()," ********************************************************* ");    

    data=dread(0xc, 4, 2, 0, 0, 0, debug);
    // [29:28] repl == 0b11, [26:24] sets == 0b100, [23:20] ssize == 0b0001 (1kb)
    // [18:16] lsize == 0b001 (1 word per line), [3] mmu present = 1
    //assert(data==0b0011 0 100 0001 0 001 00000000000000000);
    DUMP(name(),"dcache_config_reg: " << std:: hex << data);
    assert(data==0x34110008);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * Phase 1: Setup Demo Page Table ");
    DUMP(name()," ************************************************************");    

    // -----------------------
    // Level 1 - Table
    // -----------------------
    // 0x1000
    // 0x1004 0x00001401 (PTD: PTP 0x1400 >> 2, ET 0b01)
    // ...
    // 0x13ff
    // -----------------------
    // Level 2 - Table
    // -----------------------
    // 0x1400
    // 0x1404 0x00001501 (PTD: PTP 0x1500 >> 2, ET 0b01)
    // ...
    // 0x14ff
    // -----------------------
    // Level 3 - Table
    // -----------------------
    // 0x1500
    // 0x1504 0x00000402 (PPN: 0x4, ET 0x2 (Phy: 0x4000 - 0x5000)
    // 0x1508 0x00000502 (PPN: 0x5, ET 0x2)(Phy: 0x5000 - 0x5fff)
    // 0x150c 0x00000602 (PPN: 0x6, ET 0x2)(Phy: 0x6000 - 0x6fff)
    // 0x1510 0x00000702 (PPN: 0x7, ET 0x2)(Phy: 0x7000 - 0x7fff)
    // 0x1514 0x00000802 (PPN: 0x8, ET 0x2)(Phy: 0x8000 - 0x8fff)
    // 0x1518 0x00000902 (PPN: 0x9, ET 0x2)(Phy: 0x9000 - 0x9fff)
    // 0x151c 0x00000a02 (PPN: 0xa, ET 0x2)(Phy: 0xa000 - 0xafff)
    // 0x1520 0x00000b02 (PPN: 0xb, ET 0x2)(Phy: 0xb000 - 0xbfff)
    // ..
    // 0x15ff
    // -----------------------

    // !! PTP points to the base of the next-level page table.
    // !! The page tables themselve are indexed by the vpn indices (idx1,2,3)

    // demo entry to table 1 (PTD)
    dwrite(0x1004, 0x00001401, 4, 8, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEWRITEMISS_CHECK(*debug));

    // demo entry to table 2 (PTD)
    dwrite(0x1404, 0x00001501, 4, 8, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEWRITEMISS_CHECK(*debug));

    // demo entry to table 3 (PTE)
    dwrite(0x1504, 0x0000402, 4, 8, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEWRITEMISS_CHECK(*debug));

    // demo entry to table 3 (PTE)
    dwrite(0x1508, 0x0000502, 4, 8, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEWRITEMISS_CHECK(*debug));

    // demo entry to table 3 (PTE)
    dwrite(0x150c, 0x0000602, 4, 8, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEWRITEMISS_CHECK(*debug));

    // demo entry to table 3 (PTE)
    dwrite(0x1510, 0x0000702, 4, 8, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEWRITEMISS_CHECK(*debug));

    // demo entry to table 3 (PTE)
    dwrite(0x1514, 0x0000802, 4, 8, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEWRITEMISS_CHECK(*debug));

    // demo entry to table 3 (PTE)
    dwrite(0x1518, 0x0000902, 4, 8, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEWRITEMISS_CHECK(*debug));

    // demo entry to table 3 (PTE)
    dwrite(0x151c, 0x0000a02, 4, 8, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEWRITEMISS_CHECK(*debug));

    // demo entry to table 3 (PTE)
    dwrite(0x1520, 0x0000b02, 4, 8, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);
    assert(CACHEWRITEMISS_CHECK(*debug));

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * Phase 2: Activate MMU ");
    DUMP(name()," ************************************************************");

    // set MMU_CONTEXT_TABLE_POINTER to root of page table (ASI 0x19)
    dwrite(0x100, 0x1000, 4, 0x19, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // set MMU_CONTEXT_REGISTER to zero (ASI 0x19)
    dwrite(0x200, 0x0, 4, 0x19, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);   

    // The context register (process id) indexes
    // the context table pointer. Each process has its own page table.

    // read MMU_CONTROL_REGISTER
    data = dread(0x0, 4, 0x19, 0, 0, 0, debug); 

    // activate MMU by writing the control register
    data |= 0x1;
    dwrite(0x000, data, 4, 0x19, 0, 0, 0, debug);
    wait(LOCAL_CLOCK,sc_core::SC_NS);    

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * Phase 3: write data with virtual addressing ");
    DUMP(name()," ************************************************************");

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * DTLB Virtual Address TAG miss ");
    DUMP(name()," * Write to VADDR: 0x01041000, MMU will map to PADDR: 0x00004000" );
    DUMP(name()," ************************************************************ ");

    // vaddr 0x01041000 (idx1=1, idx2=1, idx3=1)
    dwrite(0x01041000, 0x00000001, 4, 8, 0, 0, 0, debug);
    assert(TLBMISS_CHECK(*debug));
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * DTLB hit ");
    DUMP(name()," * Write to VADDR: 0x01041000 - 0x01041ffc" );
    DUMP(name()," * -> fill physical page: 0x4000 - 0x4ffc" );
    DUMP(name()," ************************************************************ ");

    for(int i=4; i<4096; i+=4) {

      dwrite(0x1041000+i, i, 4, 8, 0, 0, 0, debug);
      assert(TLBHIT_CHECK(*debug));
      assert(CACHEWRITEMISS_CHECK(*debug));

      wait(LOCAL_CLOCK,sc_core::SC_NS);      

    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," * Write to 7 more memory pages ");
    DUMP(name()," * This will completely fill the 8 entry DTLB ");
    DUMP(name()," * Write to VADDR: 0x01042000 - 0x01048ffc" );
    DUMP(name()," * -> fill physical mem: 0x5000 - 0xbffc" );
    DUMP(name()," ************************************************************ ");

    dwrite(0x1042000, 4096, 4, 8, 0, 0, 0, debug);
    assert(TLBMISS_CHECK(*debug));
    assert(CACHEWRITEMISS_CHECK(*debug));

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    for(int i=4104; i<32768; i+=4) {

      dwrite(0x1041000+i, i, 4, 8, 0, 0, 0, debug);

      if ((i % 4096)==0) {
	assert(TLBMISS_CHECK(*debug));
	assert(CACHEWRITEMISS_CHECK(*debug));
      }
      else {
	assert(TLBHIT_CHECK(*debug));
	assert(CACHEWRITEMISS_CHECK(*debug));
      }
      
      wait(LOCAL_CLOCK,sc_core::SC_NS);      

    }

    DUMP(name()," ************************************************************ ");
    DUMP(name()," *  Diagnostic Lookup Data PDC (without causing AMBA reads)   ");
    DUMP(name()," *  (Check all the 8 Data TLBs)                               ");
    DUMP(name()," ************************************************************ ");    

    // VPN: 0x1041 -> PTE: 0x00000402 (PTN: 0x4, ET 0x2 (Phy: 0x4000 - 0x4fff)
    data = dread(0x1041003, 4, 6, 0, 0, 0, debug);
    DUMP(this->name(),"Diagnostic lookup DPDC with VPN: 0x1041 returned PTE: " << std::hex << data);
    assert(data==0x00000402);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // VPN: 0x1042 -> PTE: 0x00000502 (PTN: 0x5, ET 0x2 (Phy: 0x5000 - 0x5fff)
    data = dread(0x1042003, 4, 6, 0, 0, 0, debug);
    DUMP(this->name(),"Diagnostic lookup DPDC with VPN: 0x1042 returned PTE: " << std::hex << data);
    assert(data==0x00000502);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // VPN: 0x1043 -> PTE: 0x00000602 (PPN: 0x6, ET 0x2 (Phy: 0x6000 - 0x6fff)
    data = dread(0x1043003, 4, 6, 0, 0, 0, debug);
    DUMP(this->name(),"Diagnostic lookup DPDC with VPN: 0x1043 returned PTE: " << std::hex << data);
    assert(data==0x00000602);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // VPN: 0x1044 -> PTE: 0x00000702 (PPN: 0x7, ET 0x2 (Phy: 0x7000 - 0x7fff)
    data = dread(0x1044003, 4, 6, 0, 0, 0, debug);
    DUMP(this->name(),"Diagnostic lookup DPDC with VPN: 0x1044 returned PTE: " << std::hex << data);
    assert(data==0x00000702);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // VPN: 0x1045 -> PTE: 0x00000802 (PPN: 0x8, ET 0x2 (Phy: 0x8000 - 0x8fff)
    data = dread(0x1045003, 4, 6, 0, 0, 0, debug);
    DUMP(this->name(),"Diagnostic lookup DPDC with VPN: 0x1045 returned PTE: " << std::hex << data);
    assert(data==0x00000802);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // VPN: 0x1046 -> PTE: 0x00000902 (PPN: 0x9, ET 0x2 (Phy: 0x9000 - 0x9fff)
    data = dread(0x1046003, 4, 6, 0, 0, 0, debug);
    DUMP(this->name(),"Diagnostic lookup DPDC with VPN: 0x1046 returned PTE: " << std::hex << data);
    assert(data==0x00000902);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // VPN: 0x1047 -> PTE: 0x00000a02 (PPN: 0xa, ET 0x2 (Phy: 0xa000 - 0xafff)
    data = dread(0x1047003, 4, 6, 0, 0, 0, debug);
    DUMP(this->name(),"Diagnostic lookup DPDC with VPN: 0x1047 returned PTE: " << std::hex << data);
    assert(data==0x00000a02);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 

    // VPN: 0x1048 -> PTE: 0x00000b02 (PPN: 0xb, ET 0x2 (Phy: 0xb000 - 0xbfff)
    data = dread(0x1048003, 4, 6, 0, 0, 0, debug);
    DUMP(this->name(),"Diagnostic lookup DPDC with VPN: 0x1048 returned PTE: " << std::hex << data);
    assert(data==0x00000b02);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // VPN: 0x1049 -> Is not cached in PDC. Diagnostic read should return 0.
    data = dread(0x1049003, 4, 6, 0, 0, 0, debug);
    DUMP(this->name(),"Provoke miss on diagnostic lookup for VPN: 0x1049 - returned PTE: " << std::hex << data);
    assert(data==0);

    wait(LOCAL_CLOCK,sc_core::SC_NS); 
    sc_core::sc_stop();
  }
}

/// issues an instruction read transaction and returns the result
unsigned int testbench::iread(unsigned int addr, unsigned int width, unsigned int flush, unsigned int flushl, unsigned int fline, unsigned int * debug) {

  // locals
  sc_core::sc_time t;
  unsigned int data;
  tlm::tlm_generic_payload gp;
  icio_payload_extension * ext = new icio_payload_extension();

  // clear debug pointer for new transaction
  *debug = 0;

  // attache extension
  gp.set_extension(ext);

  // initialize
  gp.set_command(tlm::TLM_READ_COMMAND);
  gp.set_address(addr);
  gp.set_data_length(width);
  gp.set_streaming_width(4);
  gp.set_byte_enable_ptr(NULL);
  gp.set_data_ptr((unsigned char*)&data);
  gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  // attache extensions
  ext->flush  = flush;
  ext->flushl = flushl;
  ext->fline  = fline;
  ext->debug  = debug;
  
  // send
  instruction_initiator_socket->b_transport(gp,t);

  // suspend and burn the time
  wait(t);

  return(data);
}

/// issues a data write transaction
void testbench::dwrite(unsigned int addr, unsigned int data, unsigned int width, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock, unsigned int * debug) {

  // locals
  sc_core::sc_time t;
  tlm::tlm_generic_payload gp;
  dcio_payload_extension * ext = new dcio_payload_extension();

  // clear debug pointer for new transaction
  *debug = 0;

  // attache extension
  gp.set_extension(ext);

  // initialize
  gp.set_command(tlm::TLM_WRITE_COMMAND);
  gp.set_address(addr);
  gp.set_data_length(width);
  gp.set_streaming_width(4);
  gp.set_byte_enable_ptr(NULL);
  gp.set_data_ptr((unsigned char*)&data);

  ext->asi    = asi;
  ext->flush  = flush;
  ext->flushl = flushl;
  ext->lock   = lock;
  ext->debug  = debug;

  // send
  data_initiator_socket->b_transport(gp,t);

  // suspend and burn the time
  wait(t);

}

// issues a data read transaction
unsigned int testbench::dread(unsigned int addr, unsigned int width, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock, unsigned int * debug) {

  // locals
  sc_core::sc_time t;
  unsigned int data;
  tlm::tlm_generic_payload gp;
  dcio_payload_extension * ext = new dcio_payload_extension();
  
  // clear debug pointer for new transaction
  *debug = 0;

  // attache extension
  gp.set_extension(ext);

  // initialize
  gp.set_command(tlm::TLM_READ_COMMAND);
  gp.set_address(addr);
  gp.set_data_length(width);
  gp.set_streaming_width(4);
  gp.set_byte_enable_ptr(NULL);
  gp.set_data_ptr((unsigned char*)&data);
  gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  ext->asi    = asi;
  ext->flush  = flush;
  ext->flushl = flushl;
  ext->lock   = lock;
  ext->debug  = debug;

  // send
  data_initiator_socket->b_transport(gp,t);

  // suspend and burn the time
  wait(t);

  return(data);
}

