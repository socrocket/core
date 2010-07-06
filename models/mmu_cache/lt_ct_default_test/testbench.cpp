/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       testbench.cpp - Implementation of the                   */
/*             stimuli generator/monitor for the current testbench.    */
/*                                                                     */
/* Modified on $Date$                                                  */
/*          at $Revision$                                              */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#include "testbench.h"

// constructor
SC_HAS_PROCESS(testbench);
testbench::testbench(sc_core::sc_module_name name) : sc_module(name), m_initiator_socket("initiator_socket") {

  // register testbench thread
  SC_THREAD(initiator_thread);

}

// testbench initiator thread
void testbench::initiator_thread(void) {

  unsigned int data;

  while(1) {

    // ****************************************
    // * Test for default icache configuration
    // * -------------------------------------
    // * index  = 8  bit
    // * tag    = 22 bit
    // * offset = 2  bit
    // * sets   = 4
    // * random repl.
    // ****************************************

    // suspend and wait for all components to be savely initialized
    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // master activity
    // ===============

    // first read (cache miss)
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * read addr 0x64 (cache miss)         "                    );
    DUMP(name()," ********************************************************* ");

    data = read(0x64, 4);
    DUMP(name(), "read from 0x64 returned " << std::hex << data);
    assert(data==0x04030201);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // read again (cache hit)
    DUMP(name()," ********************************************************* ");
    DUMP(name()," * read again from 0x64 (cache hit)           "             );
    DUMP(name()," ********************************************************* ");

    data = read(0x64, 4);
    DUMP(name(), "read from 0x64 returned " << std::hex << data);
    assert(data==0x04030201);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load next bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * read addr 0x464 / same index, different tag (cache miss) ");
    DUMP(name()," * should fill one of the empty banks (3 left)              ");
    DUMP(name()," ********************************************************** ");
    data = read(0x464, 4);
    DUMP(name(), "read from 0x464 returned " << std::hex << data);
    assert(data==0x08070605);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load next bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * read addr 0x864 / same index, different tag (cache miss) ");
    DUMP(name()," * should fill one of the empty banks (2 left)              ");
    DUMP(name()," ********************************************************** ");

    data = read(0x864, 4);
    DUMP(name(), "read from 0x864 returned " << std::hex << data);
    assert(data==0x0c0b0a09);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // same index new tag (cache miss - load the last free bank)
    DUMP(name()," ********************************************************** ");
    DUMP(name()," * read addr 0xc64 / same index, different tag (cache miss) ");
    DUMP(name()," * should fill the last empty bank                          ");
    DUMP(name()," ********************************************************** ");

    data = read(0xc64, 4);
    DUMP(name(), "read from 0xc64 returned " << std::hex << data);
    assert(data==0x100f0e0d);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * read from 0x464 (cache hit) "                    );
    DUMP(name()," ************************************************* ");

    data = read(0x464, 4);
    DUMP(name(), "read from 0x464 returned " << std::hex << data);
    assert(data==0x08070605);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * read from 0x864 (cache hit) "                    );
    DUMP(name()," ************************************************* ");

    data = read(0x864, 4);
    DUMP(name(), "read from 0x864 returned " << std::hex << data);
    assert(data==0x0c0b0a09);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    // cache hit
    DUMP(name()," ************************************************* ");
    DUMP(name()," * read from 0xc64 (cache hit) "                    );
    DUMP(name()," ************************************************* ");

    data = read(0xc64, 4);
    DUMP(name(), "read from 0xc64 returned " << std::hex << data);
    assert(data==0x100f0e0d);

    wait(LOCAL_CLOCK,sc_core::SC_NS);

    sc_core::sc_stop();
  }

}

inline void testbench::write(unsigned int addr, unsigned int data, unsigned int width) {

  sc_core::sc_time t;

  gp_ptr gp;

  // initialize
  gp->set_command(tlm::TLM_WRITE_COMMAND);
  gp->set_address(addr);
  gp->set_data_length(width);
  gp->set_streaming_width(4);
  gp->set_byte_enable_ptr(NULL);
  gp->set_data_ptr((unsigned char*)&data);

  // send
  m_initiator_socket->b_transport(*gp,t);

  // suspend and burn the time
  wait(t);

}

inline unsigned int testbench::read(unsigned int addr, unsigned int width) {

  sc_core::sc_time t;
  unsigned int data;

  gp_ptr gp;

  // initialize
  gp->set_command(tlm::TLM_READ_COMMAND);
  gp->set_address(addr);
  gp->set_data_length(width);
  gp->set_streaming_width(4);
  gp->set_byte_enable_ptr(NULL);
  gp->set_data_ptr((unsigned char*)&data);
  gp->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  
  // send
  m_initiator_socket->b_transport(*gp,t);

  // suspend and burn the time
  wait(t);

  return(data);
}
