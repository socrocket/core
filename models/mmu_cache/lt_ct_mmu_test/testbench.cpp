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

// constructor
SC_HAS_PROCESS(testbench);
testbench::testbench(sc_core::sc_module_name name) : sc_module(name), 
		     instruction_initiator_socket("instruction_initiator_socket"),
		     data_initiator_socket("data_initiator_socket")
{

  // register testbench thread
  SC_THREAD(initiator_thread);

}

// testbench initiator thread
void testbench::initiator_thread(void) {

  // test vars
  unsigned int data;

  while(1) {

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
    data=dread(0x0, 4, 2, 0, 0, 0);
    // [3:2] == 0b11; [1:0] = 0b11 -> dcache and icache enabled
    DUMP(name(),"cache_contr_reg: " << std::hex << data);
    assert(data==0xf);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read icache configuration register
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * 2. Read ICACHE CONFIGURATION REGISTER (ASI 0x2 - addr 8)   ");
    DUMP(name()," ********************************************************* ");    

    data=dread(0x8, 4, 2, 0, 0, 0);
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

    data=dread(0xc, 4, 2, 0, 0, 0);
    // [29:28] repl == 0b11, [26:24] sets == 0b100, [23:20] ssize == 0b0001 (1kb)
    // [18:16] lsize == 0b001 (1 word per line), [3] mmu present = 1
    //assert(data==0b0011 0 100 0001 0 001 00000000000000000);
    DUMP(name(),"dcache_config_reg: " << std:: hex << data);
    assert(data==0x34110008);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    sc_core::sc_stop();
  }
}

// issues an instruction read transaction and returns the result
unsigned int testbench::iread(unsigned int addr, unsigned int width, unsigned int flush, unsigned int flushl, unsigned int fline) {

  // locals
  sc_core::sc_time t;
  unsigned int data;
  tlm::tlm_generic_payload gp;
  icio_payload_extension * ext = new icio_payload_extension();

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
  
  // send
  instruction_initiator_socket->b_transport(gp,t);

  // suspend and burn the time
  wait(t);

  return(data);
}

// issues a data write transaction
void testbench::dwrite(unsigned int addr, unsigned int data, unsigned int width, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock) {

  // locals
  sc_core::sc_time t;
  tlm::tlm_generic_payload gp;
  dcio_payload_extension * ext = new dcio_payload_extension();

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

  // send
  data_initiator_socket->b_transport(gp,t);

  // suspend and burn the time
  wait(t);

}

// issues a data read transaction
unsigned int testbench::dread(unsigned int addr, unsigned int width, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock) {

  // locals
  sc_core::sc_time t;
  unsigned int data;
  tlm::tlm_generic_payload gp;
  dcio_payload_extension * ext = new dcio_payload_extension();
  

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

  // send
  data_initiator_socket->b_transport(gp,t);

  // suspend and burn the time
  wait(t);

  return(data);
}
