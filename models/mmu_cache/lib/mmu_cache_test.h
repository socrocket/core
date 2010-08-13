// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       mmu_cache_test.h - Provides two TLM initiator sockets   *
// *             and several helper functions to simplify the coding     *
// *             of testbenches for mmu_cache.                           *
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision$                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************

#ifndef __MMU_CACHE_TEST_H__
#define __MMU_CACHE_TEST_H__

#include <tlm.h>
#include <ostream>
#include <tlm_utils/simple_initiator_socket.h>

#include "defines.h"

#include "icio_payload_extension.h"
#include "dcio_payload_extension.h"

/// helper class for building testbenchs for mmu_cache
class mmu_cache_test : public sc_core::sc_module {

 public:
  // variables and object declaration

  // TLM2.0 initiator sockets for instructions and data
  tlm_utils::simple_initiator_socket<mmu_cache_test> instruction_initiator_socket; 
  tlm_utils::simple_initiator_socket<mmu_cache_test> data_initiator_socket;

  tlm::tlm_response_status gp_status;

  // Constructor
  mmu_cache_test(sc_core::sc_module_name name) : sc_module(name), 
		     instruction_initiator_socket("instruction_initiator_socket"),
                     data_initiator_socket("data_initiator_socket") { };

  // member functions
  // ----------------
  /// The main testbench thread (plain virtual)
  virtual void initiator_thread(void) = 0;

  /// issues an instruction read transaction and returns the result
  unsigned int iread(unsigned int addr, unsigned int width, unsigned int flush, unsigned int flushl, unsigned int fline, unsigned int * debug) {

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

  /// issues a data read transaction
  unsigned int dread(unsigned int addr, unsigned int width, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock, unsigned int * debug) {

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

  /// issues a data write transaction
  void dwrite(unsigned int addr, unsigned int data, unsigned int width, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock, unsigned int * debug) {

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
  
};

#endif // __MMU_CACHE_TEST_H__
