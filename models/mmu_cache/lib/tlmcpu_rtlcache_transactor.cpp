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
			      m_InstrFIFO(2),
			      m_DataFIFO(2),
			      instr_pipe("instr_pipe"),
			      data_pipe("data_pipe"),
			      i_execute_address("i_execute_address"),
			      i_execute_valid("i_execute_valid"),
			      i_memory_address("i_memory_address"),
			      i_memory_valid("i_memory_valid"),
			      i_done_valid("i_done_valid"),
			      d_execute_address("d_execute_address"),
			      d_execute_data("d_execute_data"),
			      d_execute_valid("d_execute_valid"),
			      d_execute_flush("d_execute_flush"),
			      d_execute_flushl("d_execute_flushl"),
			      d_execute_asi("d_execute_asi"),
			      d_memory_address("d_memory_address"),
			      d_memory_data("d_memory_data"),
			      d_memory_valid("d_memory_valid"),
			      d_memory_write("d_memory_write"),			      
			      d_memory_flushl("d_memory_flushl"),
			      d_memory_flush("d_memory_flush"),
			      d_memory_asi("d_memory_asi"),
			      d_done_address("d_done_address"),
			      d_done_data("d_done_data"),
			      d_done_valid("d_done_valid"),
			      d_done_write("d_done_write"),

 			      m_abstractionLayer(abstractionLayer),
			      clockcycle(10.0, sc_core::SC_NS) {

  if (m_abstractionLayer == amba::amba_LT) {

    // Register blocking transport functions for icio and dcio sockets
    icio.register_b_transport(this, &tlmcpu_rtlcache_transactor::icio_b_transport);
    dcio.register_b_transport(this, &tlmcpu_rtlcache_transactor::dcio_b_transport);

  } else if (m_abstractionLayer == amba::amba_AT) {

    // Register non-blocking forward transport functions for icio and dcio sockets
    icio.register_nb_transport_fw(this, &tlmcpu_rtlcache_transactor::icio_nb_transport_fw);
    dcio.register_nb_transport_fw(this, &tlmcpu_rtlcache_transactor::dcio_nb_transport_fw);

  }

  nop_trans = new tlm::tlm_generic_payload();
  dext      = new dcio_payload_extension();

  nop_trans->set_command(tlm::TLM_READ_COMMAND);
  nop_trans->set_address(0);
  nop_trans->set_data_ptr(nop_data);
  nop_trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  nop_trans->set_data_length(4);

  dext->asi    = 0xc;
  dext->flush  = false;
  dext->flushl = false;
  dext->lock   = false;
  dext->debug  = 0;

  nop_trans->set_extension(dext);

  // Initialize instruction and data pipeline (flush out all stages)
  instr_pipe.flush(nop_trans, tlm::BEGIN_REQ);
  data_pipe.flush(nop_trans, tlm::BEGIN_REQ);

  // Register instruction and data processor/transactor threads
  SC_THREAD(i_request);

  SC_THREAD(d_request);

  SC_THREAD(i_pipe);
  sensitive << ipipe_event;

  SC_THREAD(d_pipe);
  sensitive << dpipe_event;

  // Mux ici input signals
  SC_THREAD(ici_signal_mux);
  sensitive << i_execute_address << i_execute_valid << i_memory_address << i_memory_valid << i_done_address << i_done_valid << ico;

  // Mux dci input signals
  SC_THREAD(dci_signal_mux);
  sensitive << d_execute_address << d_execute_data << d_execute_valid << d_execute_flushl << d_execute_flush << d_execute_asi
	    << d_memory_address << d_memory_data << d_memory_valid << d_memory_write << d_memory_flush << d_memory_flushl << d_memory_asi
            << d_done_address << d_done_data << d_done_valid << d_done_write << dco;


}

// Sample cache outputs
void tlmcpu_rtlcache_transactor::ici_signal_mux() {

  icache_in_type tmp;

  while(1) {

    ihold = ico.read().hold;

    // Address phase of pipeline
    // =========================
    if ((ico.read().hold & i_execute_valid) == SC_LOGIC_1) {
      
      // If the pipeline is not stalled and the transaction
      // in the exectue stage is valid (no nop).
      
      tmp.dpc = i_execute_address;
      tmp.fpc = i_execute_address;

    }

    // Data phase of pipeline
    // ======================
    if ((ico.read().hold & i_memory_valid) == SC_LOGIC_1) {

      tmp.rpc = i_memory_address;

    }

    // Done phase of pipeline
    // ======================
    if ((ico.read().hold & i_done_valid) == SC_LOGIC_1) {

      // Nothing to do ??

    }

    // Generate inull
    if ((i_execute_valid | i_memory_valid | i_done_valid) == false) {

      tmp.inull = SC_LOGIC_1;

    } else {

      tmp.inull = SC_LOGIC_0;

    }


    tmp.rbranch  = 0;
    tmp.fbranch  = 0;
    tmp.su       = SC_LOGIC_1;
    tmp.flush    = false;
    tmp.flushl   = false;
    tmp.fline    = 0;
    tmp.pnull    = SC_LOGIC_0;

    // Write icache input port
    ici.write(tmp);

    wait();
  }
}

// Mux dci input signals
void tlmcpu_rtlcache_transactor::dci_signal_mux() {

  dcache_in_type tmp;

  while(1) {

    dhold = dco.read().hold;

    // Address phase of pipeline (CPU - EXECUTE)
    // =========================================

    if ((dco.read().hold & d_execute_valid) == SC_LOGIC_1) {

      // If the pipeline is not stalled and the transaction
      // in the execute stage is valid (no nop):
      // 1. Set a new eaddress
      tmp.eaddress = d_execute_address;
      // 2. Set execute address enable
      tmp.eenaddr  = SC_LOGIC_1;
      // 3. Set extensions
      tmp.flush    = d_execute_flush;
      tmp.flushl   = d_execute_flushl;
      tmp.asi      = d_execute_asi;
      tmp.edata    = d_execute_data;


    } else {

      tmp.eenaddr  = SC_LOGIC_0;
      tmp.flush    = SC_LOGIC_0;
      tmp.flushl   = SC_LOGIC_0;

    }

    // Data Phase of pipeline (CPU - D_MEMORY)
    // =====================================

    // Generate maddress
    if ((dco.read().hold & d_memory_valid)==SC_LOGIC_1) {
      
      if (d_memory_write==SC_LOGIC_1) {

	// For write operations:
	// 1. Set the d_memory stage address
	tmp.maddress = d_memory_address;
	// 2. Set d_memory address enable
	tmp.enaddr   = SC_LOGIC_1;
	// 3. Set write data
	tmp.edata = d_memory_data;
	// 4. Set transfer size (FIXME!!!)
	tmp.size = 0x2;

      } else {

	// For read operations:
	// 1. Set the d_memory stage address
	tmp.maddress = d_memory_address;
	// 2. Set d_memory address enable
	tmp.enaddr = SC_LOGIC_1;
	// 3. Set transfer size (FIXME!!!)
	tmp.size = 0x2;

      }	

    } else if ((dco.read().hold==SC_LOGIC_0)&&(d_memory_write==SC_LOGIC_1)) {

      // In case the write buffer is full, the hold signal
      // will go down. In that case the write data must be
      // written to the maddress output.
      tmp.maddress = d_done_data;
      tmp.enaddr   = SC_LOGIC_0;

    } else {

      // NOP in d_memory stage - disable address line
      tmp.enaddr   = SC_LOGIC_0;

    }

    // Generate read/write selector
    if ((d_memory_write | d_done_write) == SC_LOGIC_1) {

      tmp.read     = SC_LOGIC_0;
      tmp.write    = SC_LOGIC_1;
      
    } else {

      tmp.read     = SC_LOGIC_1;
      tmp.write    = SC_LOGIC_0;

    }

    tmp.nullify  = SC_LOGIC_0;
    tmp.lock     = SC_LOGIC_0;
    tmp.dsuen    = SC_LOGIC_0;
    tmp.msu      = SC_LOGIC_0;
    tmp.esu      = SC_LOGIC_0;
    tmp.intack   = SC_LOGIC_0;

    // Write dcache input port
    dci.write(tmp);

    wait();

  }
}

// TLM blocking transport function for icio socket
void tlmcpu_rtlcache_transactor::icio_b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {

  v::debug << name() << "icio_b_transport called" << v::endl;

  m_InstrFIFO.write(&trans);
  delay = SC_ZERO_TIME;

  // Wait for response to become ready
  wait(m_BeginInstrResponseEvent);

  trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

}

// TLM non-blocking forward transport function for icio socket
tlm::tlm_sync_enum tlmcpu_rtlcache_transactor::icio_nb_transport_fw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  v::debug << name() << "icio_nb_transport_fw received transaction " << hex << &trans << " with phase " << phase << v::endl;

  if (phase == tlm::BEGIN_REQ) {

    m_InstrFIFO.write(&trans);
    delay = SC_ZERO_TIME;

  } else if (phase == tlm::END_RESP) {

    m_EndInstrResponseEvent.notify();
    
  }
  
  return(tlm::TLM_ACCEPTED);
}

// TLM blocking transport function for dcio socket
void tlmcpu_rtlcache_transactor::dcio_b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {

  v::debug << name() << "dcio_b_transport called" << v::endl;

  m_DataFIFO.write(&trans);
  delay = SC_ZERO_TIME;

  // Wait for response to become ready
  wait(m_BeginDataResponseEvent);
  
  trans.set_response_status(tlm::TLM_OK_RESPONSE);

}

// TLM non-blocking transport function for dcio socket
tlm::tlm_sync_enum tlmcpu_rtlcache_transactor::dcio_nb_transport_fw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  v::debug << name() << "dcio_nb_transport_fw received transaction " << hex << &trans << " with phase " << phase << v::endl;

  if (phase == tlm::BEGIN_REQ) {

    m_DataFIFO.write(&trans);
    delay = SC_ZERO_TIME;

  } else if (phase == amba::BEGIN_DATA) {

    m_EndDataResponseEvent.notify();

  } else if (phase == tlm::END_RESP) {

    // nothing to do

  }

  return(tlm::TLM_ACCEPTED);

} 

void tlmcpu_rtlcache_transactor::i_request() {

  tlm::tlm_generic_payload *trans;

  while(1) {

    wait(clock.posedge_event());

    // Sample icache data output
    instr_reg = ico.read().data[ico.read().set.to_uint()].to_uint();
    
    v::debug << name() << "INSTR_REG: " << hex << instr_reg << v::endl;

    // Unless there is a stall (hold != 1),
    // fill a new transaction or a nop into the pipeline
    if (ico.read().hold == SC_LOGIC_1) {

      if (m_InstrFIFO.nb_read(trans)) {

	instr_pipe.enter(trans, tlm::BEGIN_REQ);

      } else {

	instr_pipe.enter(nop_trans, tlm::BEGIN_REQ);

      }

      v::debug << name() << "SHIFT IPIPE" << v::endl;

    } else {

      v::debug << name() << "DON'T SHIFT IPIPE" << v::endl;

    }

    // Trigger instr_pipe
    ipipe_event.notify();
  }
}

void tlmcpu_rtlcache_transactor::d_request() {

  tlm::tlm_generic_payload *trans;

  while(1) {

    wait(clock.posedge_event());

    // Sample dcache data output
    rdata_reg = dco.read().data[dco.read().set.to_uint()].to_uint();

    v::debug << name() << "RDATA_REG: " << hex << rdata_reg << v::endl;

    // Unless there is a stall (hold != 1),
    // fill a new transaction or a nop into
    // the pipeline
    if (dhold == SC_LOGIC_1) {

      if (m_DataFIFO.nb_read(trans)) {

	data_pipe.enter(trans, tlm::BEGIN_REQ);

      } else {

	data_pipe.enter(nop_trans, tlm::BEGIN_REQ);

      }

      v::debug << name() << "SHIFT DPIPE" << v::endl;

    } else {

      v::debug << name() << "DON'T SHIFT DPIPE" << v::endl;

    }

    // Trigger data_pipe
    dpipe_event.notify();
  }
}

void tlmcpu_rtlcache_transactor::i_pipe() {

  std::pair<tlm::tlm_generic_payload *, tlm::tlm_phase> *trans_addr_state;
  std::pair<tlm::tlm_generic_payload *, tlm::tlm_phase> *trans_data_state;
  std::pair<tlm::tlm_generic_payload *, tlm::tlm_phase> *trans_done_state;
  std::pair<tlm::tlm_generic_payload *, tlm::tlm_phase> *trans_unst_state;

  tlm::tlm_generic_payload * trans_addr;
  tlm::tlm_generic_payload * trans_data;
  tlm::tlm_generic_payload * trans_done;
  tlm::tlm_generic_payload * trans_unst;

  icio_payload_extension *iext;

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;
  sc_core::sc_time delay;

  while(1) {

    wait();

    // Read pipeline state
    trans_addr_state = instr_pipe.gettrans(ADDR);
    trans_data_state = instr_pipe.gettrans(DATA);
    trans_done_state = instr_pipe.gettrans(DONE);
    trans_unst_state = instr_pipe.gettrans(UNSTALL);    

    // Make sure transactions are valid (real transaction or nop_trans)
    assert((trans_addr_state != NULL)&&(trans_data_state != NULL)&&(trans_done_state != NULL)&&(trans_unst_state != NULL));   

    // Extract payload from state
    trans_addr = trans_addr_state->first;
    trans_data = trans_data_state->first;
    trans_done = trans_done_state->first;
    trans_unst = trans_unst_state->first;

    v::debug << name() << "IPIPE ADDR: " << hex << trans_addr << " CMD: " << trans_addr->get_command() << v::endl;
    v::debug << name() << "IPIPE DATA: " << hex << trans_data << " CMD: " << trans_data->get_command() << v::endl;
    v::debug << name() << "IPIPE DONE: " << hex << trans_done << " CMD: " << trans_done->get_command() << v::endl;
    v::debug << name() << "IPIPE UNST: " << hex << trans_unst << " CMD: " << trans_unst->get_command() << v::endl;

    // ADDRESS Phase of pipeline (CPU - EXECUTE)
    // =========================================

    if (trans_addr != nop_trans) {

      // Signal: Valid instrauction in address stage
      i_execute_valid.write(true);

      // Transaction address
      i_execute_address.write(trans_addr->get_address());

      if (trans_addr_state->second == tlm::BEGIN_REQ) {

	// Send END_REQUEST
	phase = tlm::END_REQ;
	delay = SC_ZERO_TIME;

	v::debug << name() << "Transaction " << hex << trans_addr << " call to nb_transport_bw with phase " << phase << v::endl;

	status = icio->nb_transport_bw(*trans_addr, phase, delay);

	assert(status==tlm::TLM_ACCEPTED);

	trans_addr_state->second = tlm::END_REQ;

      }

    } else {

      // Signal: Instruction not valid (is NOP)
      i_execute_valid.write(false);

    }

    // DATA Phase of pipeline
    // ======================

    if (trans_data != nop_trans) {

      // Signal: Valid instruction in data phase
      i_memory_valid.write(true);

      // Transaction address
      i_memory_address.write(trans_data->get_address());

      // Copy sampled instruction to payload data pointer
      *(unsigned int*)trans_data->get_data_ptr() = instr_reg;
      

    } else {

      // Signal:: Instruction not valid (is NOP)
      i_memory_valid.write(false);

    }

    // DONE Phase of pipeline
    // ======================
    
    if (trans_done != nop_trans) {

      // Signal: Valid instruction in done phase
      i_done_valid.write(true);

      if (trans_done_state->second != tlm::BEGIN_RESP) {

	// Send BEGIN_RESPONSE
	phase = tlm::BEGIN_RESP;
	delay = SC_ZERO_TIME;

	trans_done->set_response_status(tlm::TLM_OK_RESPONSE);

	v::debug << name() << "Transaction " << hex << trans_done << " call to nb_transport_bw with phase " << phase << v::endl;
	v::debug << name() << "INSTR AT BEGIN RESP: " << hex << *(unsigned int*)trans_done->get_data_ptr() << v::endl;

	status = icio->nb_transport_bw(*trans_done, phase, delay);

	assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_COMPLETED));

	trans_done_state->second = tlm::BEGIN_RESP;

      }
    }
  }      
}

void tlmcpu_rtlcache_transactor::d_pipe() {

  std::pair<tlm::tlm_generic_payload *, tlm::tlm_phase> *trans_addr_state;
  std::pair<tlm::tlm_generic_payload *, tlm::tlm_phase> *trans_data_state;
  std::pair<tlm::tlm_generic_payload *, tlm::tlm_phase> *trans_done_state;
  std::pair<tlm::tlm_generic_payload *, tlm::tlm_phase> *trans_unst_state;

  tlm::tlm_generic_payload * trans_addr;
  tlm::tlm_generic_payload * trans_data;
  tlm::tlm_generic_payload * trans_done;
  tlm::tlm_generic_payload * trans_unst;

  dcio_payload_extension *dext;

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;
  sc_core::sc_time delay;

  while(1) {

    wait();

    // Read pipeline state
    trans_addr_state = data_pipe.gettrans(ADDR);
    trans_data_state = data_pipe.gettrans(DATA);
    trans_done_state = data_pipe.gettrans(DONE);
    trans_unst_state = data_pipe.gettrans(UNSTALL);

    // Make sure transactions are valid (real transaction or nop_trans)
    assert((trans_addr_state != NULL)&&(trans_data_state != NULL)&&(trans_done_state != NULL)&&(trans_unst_state != NULL));

    // Extract payload from state
    trans_addr = trans_addr_state->first;
    trans_data = trans_data_state->first;
    trans_done = trans_done_state->first;
    trans_unst = trans_unst_state->first;

    v::debug << name() << "DPIPE ADDR: " << hex << trans_addr << " CMD: " << trans_addr->get_command() << v::endl;
    v::debug << name() << "DPIPE DATA: " << hex << trans_data << " CMD: " << trans_data->get_command() << v::endl;
    v::debug << name() << "DPIPE DONE: " << hex << trans_done << " CMD: " << trans_done->get_command() << v::endl;
    v::debug << name() << "DPIPE UNST: " << hex << trans_unst << " CMD: " << trans_unst->get_command() << v::endl;

    // ADDRESS Phase of pipeline (CPU - EXECUTE)
    // =========================================

    if (trans_addr != nop_trans) {

      // Signal: Valid instruction in address stage
      d_execute_valid.write(true);

      if (trans_addr_state->second == tlm::BEGIN_REQ) {

	// Send END_REQUEST (For reads & writes)
	phase = tlm::END_REQ;
	delay = SC_ZERO_TIME;

	v::debug << name() << "Transaction " << hex << trans_addr << " call to nb_transport_bw with phase " << phase << v::endl;

	status = dcio->nb_transport_bw(*trans_addr, phase, delay);

	assert(status==tlm::TLM_ACCEPTED);

	trans_addr_state->second = tlm::END_REQ;

      }

      // Execute stage signalling for write commands
      if (trans_addr->get_command() == tlm::TLM_WRITE_COMMAND) {

	// Transaction address
	d_execute_address.write(trans_addr->get_address());
	// Transaction data
	d_execute_data.write(*(unsigned int *)trans_addr->get_data_ptr());
	
	// Payload extensions for flushing
	trans_addr->get_extension(dext);
	d_execute_flush.write((bool)(dext->flush));
	d_execute_flushl.write((bool)(dext->flushl));
	d_execute_asi.write(dext->asi);

      } else {

	// Execute stage signalling for read commands
	d_execute_address.write(trans_addr->get_address());

	// Payload extensions for flushing
	trans_addr->get_extension(dext);
	d_execute_flush.write((bool)(dext->flush));
	d_execute_flushl.write((bool)(dext->flushl));

      }
      

    } else {

      // Signal: Instruction not valid (is NOP)
      d_execute_valid.write(false);

    }

    // DATA Phase of pipeline (CPU - MEMORY)
    // =====================================

    if (trans_data != nop_trans) {

      // Signal: Valid instruction in memory stage
      d_memory_valid.write(true);

      // Memory stage signalling for write commands
      if (trans_data->get_command() == tlm::TLM_WRITE_COMMAND) {

	// Transaction address
	d_memory_address.write(trans_data->get_address());
	// Transaction data
	d_memory_data.write(*(unsigned int *)trans_data->get_data_ptr());
	// Is write transcaction
	d_memory_write.write(1);
      
      } else {

	// D_Memory stage signalling for read commands
	
	// Transaction address
	d_memory_address.write(trans_data->get_address());
	// Is read transaction (not write)
	d_memory_write.write(0);

      }

    } else {

      // Signal: Instruction not valid (is NOP)
      d_memory_valid.write(false);
      d_memory_write.write(0);

    }

  
    // DONE Phase of pipeline (CPU stall)
    // ==================================

    if (trans_done != nop_trans) {

      // Signal: Valid instruction in done phase
      d_done_valid.write(true);

      // Done stage signalling for write commands
      if (trans_done->get_command() == tlm::TLM_WRITE_COMMAND) {

	// Transaction address
	d_done_address.write(trans_done->get_address());
	// Transaction data
	d_done_data.write(*(unsigned int *)trans_done->get_data_ptr());	
	// Is write transaction
	d_done_write.write(1);

      } else if (trans_done->get_command() == tlm::TLM_READ_COMMAND) {

	// Done stage signalling for read commands
	d_done_address.write(trans_done->get_address());
	
	// Copy sampled read data to payload data pointer
	*(unsigned int*)trans_done->get_data_ptr() = rdata_reg;

	// Is read transaction (no write)
	d_done_write.write(0);

      }

    } else {

      d_done_valid.write(false);
      d_done_write.write(false);

    }

    // UNSTALL Phase of pipeline
    // =========================
    if (trans_unst != nop_trans) {

      if (trans_unst->get_command() == tlm::TLM_WRITE_COMMAND) {

	if (trans_unst_state->second != amba::END_DATA) {

	  // Send END_DATA
	  phase = amba::END_DATA;
	  delay = SC_ZERO_TIME;

	  trans_unst->set_response_status(tlm::TLM_OK_RESPONSE);

	  v::debug << name() << "Transaction " << hex << trans_unst << " call to nb_transport_bw with phase " << phase << v::endl;

	  status = dcio->nb_transport_bw(*trans_unst, phase, delay);

	  assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_COMPLETED));

	  trans_unst_state->second = amba::END_DATA;

	  // In case the master deltes the transaction after reception of END_DATA,
	  // it must be removed from the pipeline to avoid unpredictable behavior.
	  data_pipe.settrans(nop_trans, UNSTALL, tlm::BEGIN_REQ);

	}

      } else if (trans_unst->get_command() == tlm::TLM_READ_COMMAND) {

	if (trans_unst_state->second != tlm::BEGIN_RESP) {

	  // Send BEGIN_RESPONSE
	  phase = tlm::BEGIN_RESP;
	  delay = SC_ZERO_TIME;

	  trans_unst->set_response_status(tlm::TLM_OK_RESPONSE);
	
	  v::debug << name() << "Transaction " << hex << trans_unst << " call to nb_transport_bw with phase " << phase << v::endl;
	  v::debug << name() << "DATA AT BEGIN RESP: " << hex << *(unsigned int*)trans_unst->get_data_ptr() << v::endl;

	  status = dcio->nb_transport_bw(*trans_unst, phase, delay);

	  assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_COMPLETED));

	  trans_unst_state->second = tlm::BEGIN_RESP;

	  // In case the master deletes the transaction after reception of BEGIN_REQ,
	  // it must be removed from the pipeline to avoid unpredictable behavior.
	  data_pipe.settrans(nop_trans, UNSTALL, tlm::BEGIN_REQ);
	  
	}
      }
    }
  }
}

void tlmcpu_rtlcache_transactor::start_of_simulation() {

  // Initialize signals
  // ==================
  // Instruction port
  i_execute_address = 0;
  i_execute_valid   = 0;
  i_memory_address  = 0;
  i_memory_valid    = 0;
  i_done_address    = 0;
 
  // Data port
  d_execute_address = 0;
  d_execute_flushl  = false;
  d_execute_flush   = false;
  d_execute_valid   = false;
  d_memory_address  = 0;
  d_memory_data     = 0;
  d_memory_valid    = false;
  d_memory_write    = 0;
  d_done_data       = 0;
  d_done_address    = 0;

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
