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
// Title:      cpu_lt_rtl_adapter.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implementation of cpu_lt_rtl_adapter.
//             
//
// Method:
//
// Modified on $Date: 2010-10-07 19:25:02 +0200 (Thu, 07 Oct 2010) $
//          at $Revision $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#include "tlmcpu_rtlcache_transactor.h"

tlmcpu_rtlcache_transactor::tlmcpu_rtlcache_transactor(sc_core::sc_module_name name, amba::amba_layer_ids abstractionLayer) 
                            : sc_module(name),
			      rst("rst"),
                              clock("clock"),
                              ici("ici"),
			      ico("ico"),
			      dci("dci"),
			      dco("dco"),
			      icio("icio"),
			      dcio("dcio"),
 			      m_abstractionLayer(abstractionLayer),
			      m_InstrTransPEQ("InstrTransPEQ"),
			      clockcycle(10.0, sc_core::SC_NS),
                              m_SampleDataTransFIFO(2), 
                              m_ResponseDataTransFIFO(2) {

  if (m_abstractionLayer == amba::amba_LT) {

    // Register blocking transport functions for icio and dcio sockets
    icio.register_b_transport(this, &tlmcpu_rtlcache_transactor::icio_b_transport);
    dcio.register_b_transport(this, &tlmcpu_rtlcache_transactor::dcio_b_transport);

  } else if (m_abstractionLayer == amba::amba_AT) {

    // Register non-blocking forward transport functions for icio and dcio sockets
    icio.register_nb_transport_fw(this, &tlmcpu_rtlcache_transactor::icio_nb_transport_fw);
    dcio.register_nb_transport_fw(this, &tlmcpu_rtlcache_transactor::dcio_nb_transport_fw);

  }

  // Register instruction and data processor/transactor threads
  SC_THREAD(instrRequestThread);

  SC_THREAD(instrResponseThread);

  SC_THREAD(dataRequestThread);

  SC_THREAD(dataSampleThread);
  
  SC_THREAD(dataResponseThread);

  // Mux ici input signals
  SC_THREAD(ici_signal_mux);
  sensitive << i_rpc << i_fpc << i_dpc << i_rbranch << i_fbranch 
	    << i_inull << i_su << i_flush << i_flushl << i_fline << i_pnull;

  // Mux dci input signals
  SC_THREAD(dci_signal_mux);
  sensitive << d_asi << d_maddress << d_eaddress << d_edata << d_size << d_enaddr
	    << d_eenaddr << d_nullify << d_lock << d_read << d_write << d_flush
	    << d_flushl << d_dsuen << d_msu << d_esu << d_intack;


}

// Mux dci input signals
void tlmcpu_rtlcache_transactor::dci_signal_mux() {

  dcache_in_type tmp;

  while(1) {

    tmp.asi      = d_asi;
    tmp.maddress = d_maddress;
    tmp.eaddress = d_eaddress;
    tmp.edata    = d_edata;
    tmp.size     = d_size;
    tmp.enaddr   = d_enaddr;
    tmp.eenaddr  = d_eenaddr;
    tmp.nullify  = d_nullify;
    tmp.lock     = d_lock;
    tmp.read     = d_read;
    tmp.write    = d_write;
    tmp.flush    = d_flush;
    tmp.flushl   = d_flushl;
    tmp.dsuen    = d_dsuen;
    tmp.msu      = d_msu;
    tmp.esu      = d_esu;
    tmp.intack   = d_intack;

    // Write dcache input port
    dci.write(tmp);

    wait();

  }
}

// Mux ici input signals
void tlmcpu_rtlcache_transactor::ici_signal_mux() {

  icache_in_type tmp;

  while(1) {

    tmp.rpc      = i_rpc;
    tmp.fpc      = i_fpc;
    tmp.dpc      = i_dpc;
    tmp.rbranch  = i_rbranch;
    tmp.fbranch  = i_fbranch;
    tmp.inull    = i_inull;
    tmp.su       = i_su;
    tmp.flush    = i_flush;
    tmp.flushl   = i_flushl;
    tmp.fline    = i_fline;
    tmp.pnull    = i_pnull;

    // Write icache input port
    ici.write(tmp);

    wait();
  }
}

// TLM blocking transport function for icio socket
void tlmcpu_rtlcache_transactor::icio_b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {

  v::debug << name() << "icio_b_transport called" << v::endl;

  m_InstrTransPEQ.notify(trans, delay);
  delay = SC_ZERO_TIME;

  trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

}

// TLM non-blocking forward transport function for icio socket
tlm::tlm_sync_enum tlmcpu_rtlcache_transactor::icio_nb_transport_fw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  v::debug << name() << "icio_nb_transport_fw received transaction " << hex << &trans << " with phase " << phase << v::endl;

  if (phase == tlm::BEGIN_REQ) {

    m_InstrTransPEQ.notify(trans, delay);
    delay = SC_ZERO_TIME;

    return(tlm::TLM_ACCEPTED);

  } else if (phase == tlm::END_RESP) {

    m_EndInstrResponseEvent.notify();
    
  }
  
  return(tlm::TLM_ACCEPTED);
}

// TLM blocking transport function for dcio socket
void tlmcpu_rtlcache_transactor::dcio_b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {

  v::debug << name() << "dcio_b_transport called" << v::endl;

  m_DataTransFIFO.push_front(&trans);
  delay = SC_ZERO_TIME;

  // Wait for Response to be ready
  wait(m_BeginDataResponseEvent);
  
  trans.set_response_status(tlm::TLM_OK_RESPONSE);

}

// TLM non-blocking transport function for dcio socket
tlm::tlm_sync_enum tlmcpu_rtlcache_transactor::dcio_nb_transport_fw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  v::debug << name() << "dcio_nb_transport_fw received transaction " << hex << &trans << " with phase " << phase << v::endl;

  if (phase == tlm::BEGIN_REQ) {

    m_DataTransFIFO.push_front(&trans);
    delay = SC_ZERO_TIME;

    return(tlm::TLM_ACCEPTED);

  } else if (phase == tlm::END_RESP) {

    m_EndDataResponseEvent.notify();

  }

  return(tlm::TLM_ACCEPTED);

} 

// Instruction request thread
void tlmcpu_rtlcache_transactor::instrRequestThread() {

  unsigned int address;

  icio_payload_extension * iext;
  tlm::tlm_generic_payload * trans;

  while(1) {

    wait(m_InstrTransPEQ.get_event());

    // Get transaction from PEQ
    trans = m_InstrTransPEQ.get_next_transaction();

    v::debug << name() << "Instruction processor received new transaction " << hex << trans << " (setting up signals now!)" << v::endl;

    // Extract payload
    address = trans->get_address();
    trans->get_extension(iext);

    // Start signaling
    i_rpc = address;
    i_fpc = address;
    i_dpc = address;
    i_rbranch = SC_LOGIC_0;
    i_fbranch = SC_LOGIC_0;
    i_inull = SC_LOGIC_0;
    i_su = SC_LOGIC_1;
    i_flush = (bool)iext->flush;
    i_fline = 0;
    i_pnull = SC_LOGIC_0;

    // Transaction ready for post processing
    m_PostInstrTransFIFO.push_back(trans);

  }
}

// Instruction response thread
void tlmcpu_rtlcache_transactor::instrResponseThread() {

  while(1) {

    wait(clock.posedge_event());

  }

}

// Data request thread
void tlmcpu_rtlcache_transactor::dataRequestThread() {

  unsigned int address;
  unsigned int * data;
  unsigned int length;
  
  dcio_payload_extension *dext;
  tlm::tlm_generic_payload * trans;

  while(1) {

    // At this clock edge it is decided whether to
    // process a transaction or to assign default signals to dci.
    wait(clock.negedge_event());

    // Check if there's a transaction for post processing
    if (m_DataTransFIFO.size() != 0) {

      // Read transaction from FIFO
      trans = m_DataTransFIFO.front();
      m_DataTransFIFO.pop_front();

      v::debug << name() << "Data processor received new transaction " << hex << trans << v::endl;

      // Extract payload
      address = trans->get_address();
      data    = (unsigned int*)trans->get_data_ptr();
      length  = trans->get_data_length();
      trans->get_extension(dext);

      // Start read signaling
      if (trans->get_command()==tlm::TLM_READ_COMMAND) {

	v::debug << name() << "Data processor set up signals for read operation " << hex << trans << v::endl;

	d_asi = dext->asi;
	d_maddress = address;
	d_eaddress = address;
	d_edata = 0;

	// Transform byte length to transport size
	switch (length) {
	    
          case 1: d_size = 0; // byte
	          break;
	  case 2: d_size = 1; // half
	          break;
	  case 4: d_size = 2; // word
	          break;
	  case 8: d_size = 3; // dword
	          break;
	 default: d_size = 2;
	          v::warn << name() << "Invalid access size " << length << v::endl;
	}

	d_enaddr = SC_LOGIC_1;
	d_eenaddr = SC_LOGIC_1;
	d_nullify = SC_LOGIC_0;
	d_lock = (bool)dext->lock;
	d_read = SC_LOGIC_1;
	d_write = SC_LOGIC_0;
	d_flush = (bool)dext->flush;
	d_flushl = (bool)dext->flushl;
	d_dsuen = SC_LOGIC_0;
	d_msu = SC_LOGIC_0;
	d_esu = SC_LOGIC_0;
	d_intack = SC_LOGIC_0;

	// Start write signalling
      } else {

	v::debug << name() << "Data processor set up signals for write operation " << hex << trans << v::endl;

	d_asi = dext->asi;
	d_maddress = address;
	d_eaddress = address;
	d_edata = *data;

	// Transform byte length to transport size
	switch (length) {
	    
          case 1: d_size = 0; // byte
                  break;
          case 2: d_size = 1; // half
                  break;
          case 4: d_size = 2; // word
                  break;
          case 8: d_size = 3; // dword
                  break;
         default: d_size = 2;
                  v::warn << name() << "Invalid access size " << length << v::endl;
        }

        d_enaddr = SC_LOGIC_1;
        d_eenaddr = SC_LOGIC_1;
        d_nullify = SC_LOGIC_0;
        d_lock = (bool)dext->lock;
        d_read = SC_LOGIC_0;
        d_write = SC_LOGIC_1;
        d_flush = (bool)dext->flush;
        d_flushl = (bool)dext->flushl;
        d_dsuen = SC_LOGIC_0;
        d_msu = SC_LOGIC_0;
        d_esu = SC_LOGIC_0;
        d_intack = SC_LOGIC_0;

      }

      // Send to dataSampleThread
      m_SampleDataTransFIFO.write(trans);

    } else {

      d_asi = 0xb;
      d_maddress = 0;
      d_eaddress = 0;
      d_edata = 0;
      d_size = 2;
      d_enaddr = SC_LOGIC_0;
      d_eenaddr = SC_LOGIC_0;
      d_nullify = SC_LOGIC_0;
      d_lock = 0;
      d_read = SC_LOGIC_1;
      d_write = SC_LOGIC_0;
      d_flush = 0;
      d_flushl = 0;
      d_dsuen = SC_LOGIC_0;
      d_msu = SC_LOGIC_0;
      d_esu = SC_LOGIC_0;
      d_intack = SC_LOGIC_0;

    }
  }
}

void tlmcpu_rtlcache_transactor::dataSampleThread() {

  tlm::tlm_generic_payload * trans;

  while(1) {

    // At this clock edge the cache assigns the ahb signals
    wait(clock.posedge_event());

    // Check if there's a transaction in the fifo
    if (m_SampleDataTransFIFO.nb_read(trans) != 0) {

      v::debug << name() << "DataSampleThread received transaction " << hex << trans << v::endl;

      // Send to dataResponseThread
      m_ResponseDataTransFIFO.write(trans);

    }
  }
}

// Data response thread
void tlmcpu_rtlcache_transactor::dataResponseThread() {

  tlm::tlm_generic_payload * trans;
  unsigned int * data;

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;
  sc_core::sc_time delay;

  while(1) {

    // Data is sampled at this clock edge.
    // (Assumed data is ready)
    wait(clock.posedge_event());

    v::debug << name() << "Data post-processor clock" << v::endl;

    // Check if there's a transaction for post processing
    if (m_ResponseDataTransFIFO.nb_read(trans)) {

      v::debug << name() << "Data post-processor received new transaction " << hex << trans << v::endl;

      // Ready - continue
      if (dco.read().hold == SC_LOGIC_1) {

        if (m_abstractionLayer == amba::amba_LT) {

	  v::debug << name() << "LT - Begin Response Event" << v::endl;

	  m_BeginDataResponseEvent.notify();

	} else {

	  // Send BEGIN_RESP to master
	  phase = tlm::BEGIN_RESP;
	  delay = SC_ZERO_TIME;

	  v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << phase << v::endl;

	  status = dcio->nb_transport_bw(*trans, phase, delay);

	  switch(status) {

	  case tlm::TLM_ACCEPTED:
	  case tlm::TLM_UPDATED:

	    if (phase == tlm::BEGIN_RESP) {

	      wait(m_EndDataResponseEvent);

	    } else {

	      wait(delay);
	    
	    }
	    
	    break;

	  case tlm::TLM_COMPLETED:

	    wait(delay);

	  }
	}

      } else {
	
	if (trans->get_command()==tlm::TLM_READ_COMMAND) {

	  if ((dco.read().hold | dco.read().mds) == SC_LOGIC_0) {

	    // Obtain payload data pointer
	    data = (unsigned int *)trans->get_data_ptr();

	    // Sample data (to do: Alignment!!)
	    *data = dco.read().data[dco.read().set.to_uint()].to_uint();

	    v::debug << name() << "Data post processor sample data" << hex << *data << v::endl;

	  }

	  v::debug << name() << "ResponseDataTransFIFO push_back: " << hex << trans << v::endl;

	  // Put transaction back into fifo
	  m_ResponseDataTransFIFO.write(trans);

	  v::debug << name() << "After push_back!" << v::endl;

	} else {

	  //v::debug << name() << "write" << v::endl;

	}
      } 
    }
  }
}

void tlmcpu_rtlcache_transactor::start_of_simulation() {

  // Initialize signals
  i_rpc = 0;
  i_fpc = 0;
  i_dpc = 0;
  i_rbranch = SC_LOGIC_0;
  i_fbranch = SC_LOGIC_0;
  i_inull = SC_LOGIC_1;
  i_su = SC_LOGIC_1;
  i_flush = 0;
  i_fline = 0;
  i_pnull = SC_LOGIC_0;

  d_asi = 0xb;
  d_maddress = 0;
  d_eaddress = 0;
  d_edata = 0;
  d_size = 2;
  d_enaddr = SC_LOGIC_0;
  d_eenaddr = SC_LOGIC_0;
  d_nullify = SC_LOGIC_0;
  d_lock = 0;
  d_read = SC_LOGIC_1;
  d_write = SC_LOGIC_0;
  d_flush = 0;
  d_flushl = 0;
  d_dsuen = SC_LOGIC_0;
  d_msu = SC_LOGIC_0;
  d_esu = SC_LOGIC_0;
  d_intack = SC_LOGIC_0;

}

// Helper for setting clock cycle latency using sc_clock argument
void tlmcpu_rtlcache_transactor::clk(sc_core::sc_clock &clk) {

  clockcycle = clk.period();

}

// Helper for setting clock cycle latency using sc_time argument
void tlmcpu_rtlcache_transactor::clk(sc_core::sc_time &period) {

  clockcycle = period;

}

// Helper for setting clock cycle latency using a value-time_unit pair
void tlmcpu_rtlcache_transactor::clk(double period, sc_core::sc_time_unit base) {

  clockcycle = sc_core::sc_time(period, base);

}
