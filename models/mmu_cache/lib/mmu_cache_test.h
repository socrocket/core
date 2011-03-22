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
// Title:      mmu_cache_test.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Provides two TLM initiator sockets
//             and several helper functions to simplify the coding
//             of testbenches for mmu_cache.
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

#ifndef __MMU_CACHE_TEST_H__
#define __MMU_CACHE_TEST_H__

#include <tlm.h>
#include <ostream>
#include <ctime>

#if defined(MTI_SYSTEMC) || defined(NO_INCLUDE_PATHS)
#include "simple_initiator_socket.h"
#else
#include "tlm_utils/simple_initiator_socket.h"
#endif

#include "verbose.h"
#include "defines.h"

#include "amba.h"
#include "icio_payload_extension.h"
#include "dcio_payload_extension.h"

/// helper class for building testbenchs for mmu_cache
class mmu_cache_test : public sc_core::sc_module {

    public:

        // TLM2.0 initiator sockets for instructions and data
        tlm_utils::simple_initiator_socket<mmu_cache_test> instruction_initiator_socket;
        tlm_utils::simple_initiator_socket<mmu_cache_test> data_initiator_socket;

        tlm::tlm_response_status gp_status;

        // Constructor
        mmu_cache_test(sc_core::sc_module_name name, amba::amba_layer_ids abstractionLevel) :
            sc_module(name), instruction_initiator_socket("instruction_initiator_socket"), data_initiator_socket(
                    "data_initiator_socket") {

	  if (abstractionLevel==amba::amba_AT) {

	    instruction_initiator_socket.register_nb_transport_bw(this, &mmu_cache_test::instruction_nb_transport_bw);
	    data_initiator_socket.register_nb_transport_bw(this, &mmu_cache_test::data_nb_transport_bw);

	  }
	  
        }

	// locals
	// ------
	
	// system time for simulation accuracy measurement
	sc_core::sc_time phase_systime_start;
	sc_core::sc_time phase_systime_end;

	// realtime clock for simulation performance measurement
	clock_t phase_realtime_start;
	clock_t phase_realtime_end;

	// events for notifification of the testbench threads on completed transactions
	sc_event icio_completed;
	sc_event dcio_completed;

        // member functions
        // ----------------

	/// TLM non-blocking backward transport function for instruction initiator socket
	tlm::tlm_sync_enum instruction_nb_transport_bw(tlm::tlm_generic_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

	  tlm::tlm_sync_enum status = tlm::TLM_COMPLETED;

	  v::debug << name() << " instruction_nb_transport_bw @phase: " << phase << v::endl;

	  switch(phase) {

	    case tlm::BEGIN_RESP:

	      status = tlm::TLM_ACCEPTED;
	      icio_completed.notify();
	      break;

	    default:

	      v::error << name() << " TLM phase received on backward path not valid (2-Phase protocol expected)" << v::endl;
	      assert(0);
	      break;
	  }

	  return(status);

	}

	/// TLM non-blocking backward transport function for data initiator socket
	tlm::tlm_sync_enum data_nb_transport_bw(tlm::tlm_generic_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

	  tlm::tlm_sync_enum status = tlm::TLM_COMPLETED;

	  v::debug << name() << " data_nb_transport_bw @phase: " << phase << " delay: " << delay << v::endl;

	  switch(phase) {

	    case tlm::BEGIN_RESP:

	      status = tlm::TLM_ACCEPTED;
	      dcio_completed.notify();
	      break;

	    default:

	      v::error << name() << " TLM phase received on backward path not valid (2-Phase protocol expected)" << v::endl;
	      break;
	  }
	      
	  return(status);

	}

        /// issues a blocking instruction read transaction and returns the result
        unsigned int iread(unsigned int addr, unsigned int flush,
                           unsigned int flushl, unsigned int fline,
                           unsigned int * debug) {

            // locals
            sc_core::sc_time t = SC_ZERO_TIME;
            unsigned int data = 0;
            tlm::tlm_generic_payload gp;
            icio_payload_extension * ext = new icio_payload_extension();

            // clear debug pointer for new transaction
            *debug = 0;

            // attache extension
            gp.set_extension(ext);

            // initialize
            gp.set_command(tlm::TLM_READ_COMMAND);
            gp.set_address(addr);
            gp.set_data_length(4); // data length always 4 byte for instr. interface
            gp.set_streaming_width(4);
            gp.set_byte_enable_ptr(NULL);
            gp.set_data_ptr((unsigned char*)&data);
            gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

            // attache extensions
            ext->flush = flush;
            ext->flushl = flushl;
            ext->fline = fline;
            ext->debug = debug;

            // send
            instruction_initiator_socket->b_transport(gp, t);

	    // check response
	    assert(gp.get_response_status()==tlm::TLM_OK_RESPONSE);

            // suspend and burn the time
            wait(t);

            return (data);
        }

        /// issues a non-blocking instruction read transaction and returns the result
        unsigned int iread_nb(unsigned int addr, unsigned int flush,
                              unsigned int flushl, unsigned int fline,
                              unsigned int * debug) {

	    // initiale phase
	    tlm::tlm_phase phase = tlm::BEGIN_REQ;

	    // return status for nb_transport
	    tlm::tlm_sync_enum status;

            // locals
            sc_core::sc_time t = SC_ZERO_TIME;
            unsigned int data = 0;
            tlm::tlm_generic_payload gp;
            icio_payload_extension * ext = new icio_payload_extension();

            // clear debug pointer for new transaction
            *debug = 0;

            // attache extension
            gp.set_extension(ext);

            // initialize
            gp.set_command(tlm::TLM_READ_COMMAND);
            gp.set_address(addr);
            gp.set_data_length(4); // data length always 4 byte for instr. interface
            gp.set_streaming_width(4);
            gp.set_byte_enable_ptr(NULL);
            gp.set_data_ptr((unsigned char*)&data);
            gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

            // attache extensions
            ext->flush = flush;
            ext->flushl = flushl;
            ext->fline = fline;
            ext->debug = debug;

	    v::debug << name() << " Call nb_transport_fw with phase BEGIN_REQ" << v::endl;

            // send BEGIN_REQ
            status = instruction_initiator_socket->nb_transport_fw(gp, phase, t);

	    // check status
	    switch(status) {

	      case tlm::TLM_UPDATED:
		
		wait(t);
		break;

	      default:
	      
		// MMU_Cache implements 2-Phase AT protocol and should always send TLM_UPDATED after BEGIN_REG
		v::error << name() << " TLM return status undefined or not valid!! " << v::endl;
		// exit
		assert(0);
	    }

	    v::debug << name() << " Waiting for icio_completed" << v::endl;

            // wait for transaction to complete (from backward path)
            wait(icio_completed);

	    v::debug << name() << " Transaction received - set up end of response." << v::endl;

	    // let the target know we are done
	    if (gp.get_response_status()==tlm::TLM_OK_RESPONSE) {
	      
	      // no response accept delay
	      phase = tlm::END_RESP;
	      t = SC_ZERO_TIME;

	      v::debug << name() << " Call nb_transport_fw with phase END_RESP" << v::endl;

	      // send END_RESP
	      status = instruction_initiator_socket->nb_transport_fw(gp, phase, t);

	      // check status
	      switch(status) {

	        case tlm::TLM_COMPLETED:

		  wait(t);
		  break;

	        default:

		  // Unexpected phase
		  v::error << name() << " TLM return status undefined or not valid!! " << v::endl;
                  // exit
                  assert(0);
	      }
		 
	    } else {

	      // error
	      v::error << name() << " Error in payload repsonse status (not TLM_OK_RESPONSE)!! " << v::endl;
	      // exit
	      assert(0);
	    }
	    
            return (data);
        }
	

        /// issues a blocking data read transaction
        unsigned int dread(unsigned int addr, unsigned int length,
                           unsigned int asi, unsigned int flush,
                           unsigned int flushl, unsigned int lock,
                           unsigned int * debug) {

            // locals
            sc_core::sc_time t = SC_ZERO_TIME;
            unsigned int data = 0;
            tlm::tlm_generic_payload gp;
            dcio_payload_extension * ext = new dcio_payload_extension();

            // clear debug pointer for new transaction
            *debug = 0;

            // attache extension
            gp.set_extension(ext);

            // initialize
            gp.set_command(tlm::TLM_READ_COMMAND);
            gp.set_address(addr);
            gp.set_data_length(length);
            gp.set_streaming_width(4);
            gp.set_byte_enable_ptr(NULL);
            gp.set_data_ptr((unsigned char *)&data);
            gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

            ext->asi = asi;
            ext->flush = flush;
            ext->flushl = flushl;
            ext->lock = lock;
            ext->debug = debug;

            // send
            data_initiator_socket->b_transport(gp, t);

	    // check response
	    assert(gp.get_response_status()==tlm::TLM_OK_RESPONSE);

            // suspend and burn the time
            wait(t);

            return (data);
        }

        /// issues a non-blocking data read transaction
        unsigned int dread_nb(unsigned int addr, unsigned int length,
                              unsigned int asi, unsigned int flush,
                              unsigned int flushl, unsigned int lock,
                              unsigned int * debug) {

	    // initiale phase
	    tlm::tlm_phase phase = tlm::BEGIN_REQ;

	    // return status for nb_transport
	    tlm::tlm_sync_enum status;

            // locals
            sc_core::sc_time t = SC_ZERO_TIME;
            unsigned int data = 0;
            tlm::tlm_generic_payload gp;
            dcio_payload_extension * ext = new dcio_payload_extension();

            // clear debug pointer for new transaction
            *debug = 0;

            // attache extension
            gp.set_extension(ext);

            // initialize
            gp.set_command(tlm::TLM_READ_COMMAND);
            gp.set_address(addr);
            gp.set_data_length(length);
            gp.set_streaming_width(4);
            gp.set_byte_enable_ptr(NULL);
            gp.set_data_ptr((unsigned char *)&data);
            gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

            ext->asi = asi;
            ext->flush = flush;
            ext->flushl = flushl;
            ext->lock = lock;
            ext->debug = debug;

            // send
            status = data_initiator_socket->nb_transport_fw(gp, phase, t);

	    // check status
	    switch(status) {

	      case tlm::TLM_UPDATED:
		
		wait(t);
		break;

	      default:
	      
		// MMU_Cache implements 2-Phase AT protocol and should always send TLM_UPDATED after BEGIN_REG
		v::error << name() << " TLM return status undefined or not valid!! " << v::endl;
		// exit
		assert(0);
	    }
    
            // wait for transaction to complete
            wait(dcio_completed);

	    // let the target know we are done
	    if (gp.get_response_status()==tlm::TLM_OK_RESPONSE) {

	      // no response accept delay
	      phase = tlm::END_RESP;
	      t = SC_ZERO_TIME;

	      // send END_RESP
	      status = data_initiator_socket->nb_transport_fw(gp, phase, t);

	      // check status
	      switch(status) {

	        case tlm::TLM_COMPLETED:

		  wait(t);
		  break;

	        default:

		  // Unexpected phase
		  v::error << name() << " TLM return status undefined or not valid!! " << v::endl;
                  // exit
                  assert(0);
	      }
		 
	    } else {

	      // error
	      v::error << name() << " Error in payload response status (not TLM_OK_RESPONSE)!! " << v::endl;
	      // exit
	      assert(0);
	    }

            return (data);
        }

        /// issues a blocking data write transaction
        void dwrite(unsigned int addr, unsigned int data, unsigned int length,
                       unsigned int asi, unsigned int flush, unsigned int flushl,
                       unsigned int lock, unsigned int * debug) {

            // locals
            sc_core::sc_time t = SC_ZERO_TIME;
            tlm::tlm_generic_payload gp;
            dcio_payload_extension * ext = new dcio_payload_extension();

            // clear debug pointer for new transaction
            *debug = 0;

            // attache extension
            gp.set_extension(ext);

            // initialize
            gp.set_command(tlm::TLM_WRITE_COMMAND);
            gp.set_address(addr);
            gp.set_data_length(length);
            gp.set_streaming_width(4);
            gp.set_byte_enable_ptr(NULL);
            gp.set_data_ptr((unsigned char*)&data);

            ext->asi = asi;
            ext->flush = flush;
            ext->flushl = flushl;
            ext->lock = lock;
            ext->debug = debug;

            // send
            data_initiator_socket->b_transport(gp, t);

	    // check response
	    assert(gp.get_response_status()==tlm::TLM_OK_RESPONSE);

            // suspend and burn the time
            wait(t);

        }

        /// issues a non-blocking data write transaction
        void dwrite_nb(unsigned int addr, unsigned int data, unsigned int length,
                       unsigned int asi, unsigned int flush, unsigned int flushl,
                       unsigned int lock, unsigned int * debug) {

	    // initiale phase
	    tlm::tlm_phase phase = tlm::BEGIN_REQ;

	    // return status for nb_transport
	    tlm::tlm_sync_enum status;

            // locals
            sc_core::sc_time t = SC_ZERO_TIME;
            tlm::tlm_generic_payload gp;
            dcio_payload_extension * ext = new dcio_payload_extension();

            // clear debug pointer for new transaction
            *debug = 0;

            // attache extension
            gp.set_extension(ext);

            // initialize
            gp.set_command(tlm::TLM_WRITE_COMMAND);
            gp.set_address(addr);
            gp.set_data_length(length);
            gp.set_streaming_width(4);
            gp.set_byte_enable_ptr(NULL);
            gp.set_data_ptr((unsigned char*)&data);

            ext->asi = asi;
            ext->flush = flush;
            ext->flushl = flushl;
            ext->lock = lock;
            ext->debug = debug;

            // send BEGIN_REQ
            status = data_initiator_socket->nb_transport_fw(gp, phase, t);

	    // check status
	    switch(status) {

	      case tlm::TLM_UPDATED:
		
		wait(t);
		break;

	      default:
	      
		// MMU_Cache implements 2-Phase AT protocol and should always send TLM_UPDATED after BEGIN_REG
		v::error << name() << " TLM return status undefined or not valid!! " << v::endl;
		// exit
		assert(0);
	    }	    

            // wait for transaction to complete (from backward path)
	    wait(dcio_completed);

	    // let the target know we are done
	    if (gp.get_response_status()==tlm::TLM_OK_RESPONSE) {
	      
	      // no response accept delay
	      phase = tlm::END_RESP;
	      t = SC_ZERO_TIME;

	      // send END_RESP
	      status = data_initiator_socket->nb_transport_fw(gp, phase, t);

	      // check status
	      switch(status) {

	        case tlm::TLM_COMPLETED:

		  wait(t);
		  break;

	        default:

		  // Unexpected phase
		  v::error << name() << " TLM return status undefined or not valid!! " << v::endl;
                  // exit
                  assert(0);
	      }
		 
	    } else {

	      // error
	      v::error << name() << " Error in payload repsonse status (not TLM_OK_RESPONSE)!! " << v::endl;
	      // exit
	      assert(0);
	    }

        }

	/// Use this function to record system time and realtime at the beginning of a test phase
	void phase_start_timing() {

	  phase_systime_start = sc_core::sc_time_stamp();
	  phase_realtime_start = std::clock();

	}

	// Use this function to record system time and realtime at the end of a test phase
	void phase_end_timing() {

	  phase_systime_end = sc_core::sc_time_stamp();
	  phase_realtime_end = std::clock();
	
        }

	// Returns the difference between phase_systime_end and phase_systime_start
	sc_core::sc_time phase_systime() {

	  return(phase_systime_end - phase_systime_start);

	}

	// Returns the difference between phase_realtime_end and phase_realtime_start in seconds.
	double phase_realtime() {

	  return((phase_realtime_end - phase_realtime_start)/(double)CLOCKS_PER_SEC);
	
        }

};

#endif // __MMU_CACHE_TEST_H__
