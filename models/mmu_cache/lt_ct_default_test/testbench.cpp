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

    // ************************************************************
    // * Phase 1: Main memory is written through dcache.
    // * The addresses are selected in a way that indices are equal
    // * but tags are different (different pages).
    // ************************************************************


    // write some data to memory (cache write miss)
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * DCACHE write addr 0x64 (cache write miss)               ");
    DUMP(name()," ********************************************************* ");    

    data=0x04030201;
    // args: addr, data, length, asi, flush, flushl, lock 
    dwrite(0x64,data, 4, 0, 0, 0, 0);

    DUMP(name()," ********************************************************* ");
    DUMP(name()," * DCACHE write addr 0x464 (cache write miss)              ");
    DUMP(name()," ********************************************************* ");    

    data=0x08070605;
    dwrite(0x464,data,4, 0, 0, 0, 0);

    DUMP(name()," ********************************************************* ");
    DUMP(name()," * DCACHE write addr 0x864 (cache write miss)              ");
    DUMP(name()," ********************************************************* ");    

    data=0x0c0b0a09;
    dwrite(0x864,data,4, 0, 0, 0, 0);

    DUMP(name()," ********************************************************* ");
    DUMP(name()," * DCACHE write addr 0xc64 (cache write miss)              ");
    DUMP(name()," ********************************************************* ");

    data=0x100f0e0d;
    dwrite(0xc64,data,4, 0, 0, 0, 0);

    // ********************************************************************
    // * Phase 1: The data which has been written in Phase 1 is read back
    // * through the data interface. Because the address tags are different,
    // * all four sets of the cache should be filled. The first part
    // * provokes cache misses, the second part cache hits.
    // ********************************************************************
  
    // first read (cache miss)
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * DCACHE read addr 0x64 (cache miss)         "                    );
    DUMP(name()," ********************************************************* ");

    // args: address, length, asi, flush, flushl, lock
    data = dread(0x64, 4, 0, 0, 0, 0);
    DUMP(name(), "DCACHE read from 0x64 returned " << std::hex << data);
    assert(data==0x04030201);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read again (cache hit)
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * DCACHE read again from 0x64 (cache hit)           "             );
    DUMP(name()," ********************************************************* ");

    data = dread(0x64, 4, 0, 0, 0, 0);
    DUMP(name(), "DCACHE read from 0x64 returned " << std::hex << data);
    assert(data==0x04030201);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load next bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * DCACHE read addr 0x464 / same idx, diff tag (cache miss) ");
    DUMP(name()," * should fill one of the empty banks (3 left)              ");
    DUMP(name()," ********************************************************** ");
    data = dread(0x464, 4, 0, 0, 0, 0);
    DUMP(name(), "DCACHE read from 0x464 returned " << std::hex << data);
    assert(data==0x08070605);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load next bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * DCACHE read addr 0x864 / same idx, diff tag (cache miss) ");
    DUMP(name()," * should fill one of the empty banks (2 left)              ");
    DUMP(name()," ********************************************************** ");

    data = dread(0x864, 4, 0, 0, 0, 0);
    DUMP(name(), "DCACHE read from 0x864 returned " << std::hex << data);
    assert(data==0x0c0b0a09);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load the last free bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * DCACHE read addr 0xc64 / same idx, diff tag (cache miss) ");
    DUMP(name()," * should fill the last empty bank                          ");
    DUMP(name()," ********************************************************** ");

    data = dread(0xc64, 4, 0, 0, 0, 0);
    DUMP(name(), "DCACHE read from 0xc64 returned " << std::hex << data);
    assert(data==0x100f0e0d);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * DCACHE read from 0x464 (cache hit) "                    );
    DUMP(name()," ************************************************* ");

    data = dread(0x464, 4, 0, 0, 0, 0);
    DUMP(name(), "DCACHE read from 0x464 returned " << std::hex << data);
    assert(data==0x08070605);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * DCACHE read from 0x864 (cache hit) ");
    DUMP(name()," ************************************************* ");

    data = dread(0x864, 4, 0, 0, 0, 0);
    DUMP(name(), "DCACHE read from 0x864 returned " << std::hex << data);
    assert(data==0x0c0b0a09);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * DCACHE read from 0xc64 (cache hit) "                    );
    DUMP(name()," ************************************************* ");

    data = dread(0xc64, 4, 0, 0, 0, 0);
    DUMP(name(), "DCACHE read from 0xc64 returned " << std::hex << data);
    assert(data==0x100f0e0d);

    // ********************************************************************
    // * Phase 3: Similar to Phase 2, the same data is now loaded through
    // * the instruction interface. All sets of the icache should be
    // * filled. The first part provokes misses the second hits.
    // ********************************************************************

    // first read (cache miss)
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * ICACHE read addr 0x64 (cache miss)         "                    );
    DUMP(name()," ********************************************************* ");

    // args: addr, length, flush, flushl, fline
    data = iread(0x64, 4, 0, 0, 0);
    DUMP(name(), "ICACHE read from 0x64 returned " << std::hex << data);
    assert(data==0x04030201);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read again (cache hit)
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * ICACHE read again from 0x64 (cache hit)           "             );
    DUMP(name()," ********************************************************* ");

    data = iread(0x64, 4, 0, 0, 0);
    DUMP(name(), "ICACHE read from 0x64 returned " << std::hex << data);
    assert(data==0x04030201);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load next bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * ICACHE read addr 0x464 / same idx, diff tag (cache miss) ");
    DUMP(name()," * should fill one of the empty banks (3 left)              ");
    DUMP(name()," ********************************************************** ");
    data = iread(0x464, 4, 0, 0, 0);
    DUMP(name(), "ICACHE read from 0x464 returned " << std::hex << data);
    assert(data==0x08070605);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load next bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * ICACHE read addr 0x864 / same idx, diff tag (cache miss) ");
    DUMP(name()," * should fill one of the empty banks (2 left)              ");
    DUMP(name()," ********************************************************** ");

    data = iread(0x864, 4, 0, 0, 0);
    DUMP(name(), "ICACHE read from 0x864 returned " << std::hex << data);
    assert(data==0x0c0b0a09);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load the last free bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * ICACHE read addr 0xc64 / same idx, diff tag (cache miss) ");
    DUMP(name()," * should fill the last empty bank                          ");
    DUMP(name()," ********************************************************** ");

    data = iread(0xc64, 4, 0, 0, 0);
    DUMP(name(), "ICACHE read from 0xc64 returned " << std::hex << data);
    assert(data==0x100f0e0d);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * ICACHE read from 0x464 (cache hit) "                    );
    DUMP(name()," ************************************************* ");

    data = iread(0x464, 4, 0, 0, 0);
    DUMP(name(), "ICACHE read from 0x464 returned " << std::hex << data);
    assert(data==0x08070605);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * ICACHE read from 0x864 (cache hit) ");
    DUMP(name()," ************************************************* ");

    data = iread(0x864, 4, 0, 0, 0);
    DUMP(name(), "ICACHE read from 0x864 returned " << std::hex << data);
    assert(data==0x0c0b0a09);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * ICACHE read from 0xc64 (cache hit) "                    );
    DUMP(name()," ************************************************* ");

    data = iread(0xc64, 4, 0, 0, 0);
    DUMP(name(), "ICACHE read from 0xc64 returned " << std::hex << data);
    assert(data==0x100f0e0d);


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
