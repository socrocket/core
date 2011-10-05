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
			      m_DataFIFO(2),
			      cpu_pipe("cpu_pipe"),
			      ico_reg("ico_reg"),
			      dco_reg("dco_reg"),
			      execute_address("execute_address"),
			      memory_address("memory_address"),
			      memory_data("memory_data"),
			      execute_valid("execute_valid"),
			      memory_valid("memory_valid"),
			      done_valid("done_valid"),
			      memory_write("memory_write"),
			      done_write("done_write"),
			      execute_flush("execute_flush"),
			      execute_flushl("execute_flushl"),
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

  // Initialize pipeline (flush out all stages)
  cpu_pipe.enter(nop_trans, tlm::BEGIN_REQ);
  cpu_pipe.enter(nop_trans, tlm::BEGIN_REQ);
  cpu_pipe.enter(nop_trans, tlm::BEGIN_REQ);

  // Register instruction and data processor/transactor threads
  //SC_THREAD(instr_request);

  //SC_THREAD(instr_stage1);

  //SC_THREAD(instr_stage2);

  //SC_THREAD(instr_stage3);

  SC_THREAD(data_request);

  SC_THREAD(data_pipe);
  sensitive << pipe_event;

  // Mux ici input signals
  SC_THREAD(ici_signal_mux);
  sensitive << ico;

  // Mux dci input signals
  SC_THREAD(dci_signal_mux);
  sensitive << execute_address << memory_address << memory_data << execute_valid << memory_valid << memory_write 
	    << done_valid << done_write << execute_flush << execute_flushl << dco;

  // Sample cache outputs
  SC_THREAD(sample_inputs);
  sensitive << clock.pos();

}

// Sample cache outputs
void tlmcpu_rtlcache_transactor::sample_inputs() {

  while(1) {

    wait();

    ico_reg.write(ico.read());
    dco_reg.write(dco.read());

  }

}

// Mux dci input signals
void tlmcpu_rtlcache_transactor::dci_signal_mux() {

  dcache_in_type tmp;
  sc_logic hold;
  

  while(1) {

    hold = dco.read().hold;

    // Generate asi
    tmp.asi      = 0xb;

    // Generate maddress
    if ((hold & memory_valid)==SC_LOGIC_1) {
      
      tmp.maddress = memory_address;
      tmp.enaddr   = SC_LOGIC_1;

    } else {

      tmp.maddress = memory_data;
      tmp.enaddr   = SC_LOGIC_0;

    }

    if ((hold & execute_valid)==SC_LOGIC_1) {

      tmp.eaddress = execute_address;
      tmp.eenaddr  = SC_LOGIC_1;
      tmp.flush    = execute_flush;
      tmp.flushl   = execute_flushl;

    } else {

      tmp.eenaddr  = SC_LOGIC_0;
      tmp.flush    = SC_LOGIC_0;
      tmp.flushl   = SC_LOGIC_0;

    }

    tmp.edata    = memory_data;
    tmp.size     = 0x2;

    tmp.nullify  = SC_LOGIC_0;
    tmp.lock     = SC_LOGIC_0;

    if (((memory_write | done_write) & (memory_valid | done_valid))==SC_LOGIC_1) {

      tmp.read     = SC_LOGIC_0;
      tmp.write    = SC_LOGIC_1;
      
    } else {

      tmp.read     = SC_LOGIC_1;
      tmp.write    = SC_LOGIC_0;

    }

    tmp.dsuen    = SC_LOGIC_0;
    tmp.msu      = SC_LOGIC_0;
    tmp.esu      = SC_LOGIC_0;
    tmp.intack   = SC_LOGIC_0;

    // Write dcache input port
    dci.write(tmp);

    wait();

  }
}

// Mux ici input signals
void tlmcpu_rtlcache_transactor::ici_signal_mux() {

  icache_in_type tmp;

  while(1) {

    tmp.rpc      = 0;
    tmp.fpc      = 0;
    tmp.dpc      = 0;
    tmp.rbranch  = 0;
    tmp.fbranch  = 0;
    tmp.inull    = SC_LOGIC_1;
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

// TLM blocking transport function for icio socket
void tlmcpu_rtlcache_transactor::icio_b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {

  v::debug << name() << "icio_b_transport called" << v::endl;

  //m_InstrTransFIFO.write(&trans);
  delay = SC_ZERO_TIME;

  // Wait for response to become ready
  wait(m_BeginInstrResponseEvent);

  trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

}

// TLM non-blocking forward transport function for icio socket
tlm::tlm_sync_enum tlmcpu_rtlcache_transactor::icio_nb_transport_fw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  v::debug << name() << "icio_nb_transport_fw received transaction " << hex << &trans << " with phase " << phase << v::endl;

  if (phase == tlm::BEGIN_REQ) {

    //m_InstrTransFIFO.write(&trans);
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

    return(tlm::TLM_ACCEPTED);

  } else if (phase == amba::BEGIN_DATA) {

    m_EndDataResponseEvent.notify();

  }

  return(tlm::TLM_ACCEPTED);

} 

void tlmcpu_rtlcache_transactor::data_request() {

  tlm::tlm_generic_payload *trans;

  while(1) {

    wait(clock.posedge_event());

    if (dco.read().hold == SC_LOGIC_1) {

      if (m_DataFIFO.nb_read(trans)) {

	cpu_pipe.enter(trans, tlm::BEGIN_REQ);

      } else {

	cpu_pipe.enter(nop_trans, tlm::BEGIN_REQ);

      }

      // Shift the pipeline
      pipe_event.notify();
    }
  }
}

void tlmcpu_rtlcache_transactor::data_pipe() {

  std::pair<tlm::tlm_generic_payload *, tlm::tlm_phase> *trans_addr_state;
  std::pair<tlm::tlm_generic_payload *, tlm::tlm_phase> *trans_data_state;
  std::pair<tlm::tlm_generic_payload *, tlm::tlm_phase> *trans_done_state;

  tlm::tlm_generic_payload * trans_addr;
  tlm::tlm_generic_payload * trans_data;
  tlm::tlm_generic_payload * trans_done;

  dcio_payload_extension *dext;

  tlm::tlm_phase phase;
  tlm::tlm_sync_enum status;
  sc_core::sc_time delay;

  while(1) {

    wait();

    // Read pipeline state
    trans_addr_state = cpu_pipe.gettrans(ADDR);
    trans_data_state = cpu_pipe.gettrans(DATA);
    trans_done_state = cpu_pipe.gettrans(DONE);

    // Make sure transactions are valid (real transaction or nop_trans)
    assert((trans_addr_state != NULL)&&(trans_data_state != NULL)&&(trans_done_state != NULL));

    // Extract payload from state
    trans_addr = trans_addr_state->first;
    trans_data = trans_data_state->first;
    trans_done = trans_done_state->first;

    // ADDRESS Phase of pipeline (CPU - EXECUTE)
    // =========================================

    if (trans_addr != nop_trans) {

      execute_valid.write(true);

    } else {

      execute_valid.write(false);

    }

    execute_address.write(trans_addr->get_address());

    trans_addr->get_extension(dext);
    execute_flush.write((bool)(dext->flush));
    execute_flushl.write((bool)(dext->flushl));

    // DATA Phase of pipeline (CPU - MEMORY)
    // =====================================

    if (trans_data != nop_trans) {

      memory_valid.write(true);

    } else {

      memory_valid.write(false);

    }

    memory_address.write(trans_data->get_address());
    memory_data.write(*(unsigned int *)trans_data->get_data_ptr());
    
    if (trans_data->get_command() == tlm::TLM_WRITE_COMMAND) {

      memory_write.write(SC_LOGIC_1);

    } else {

      memory_write.write(SC_LOGIC_0);

    }

    if ((trans_data != nop_trans) && (trans_data_state->second == tlm::BEGIN_REQ)) {

      // Send END_DATA
      phase = amba::END_DATA;
      delay = SC_ZERO_TIME;

      trans_data->set_response_status(tlm::TLM_OK_RESPONSE);

      v::debug << name() << "Transaction " << hex << trans_data << " call to nb_transport_bw with phase " << phase << v::endl;

      status = dcio->nb_transport_bw(*trans_data, phase, delay);

      assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_COMPLETED));

      trans_data_state->second == amba::END_DATA;

    }

    // DONE Phase of pipeline (CPU stall)
    // ==================================

    if ((trans_done != nop_trans)&&(dco.read().hold == SC_LOGIC_1)) {

      done_valid.write(true);

    } else {

      done_valid.write(false);

    }

    if (trans_done->get_command() == tlm::TLM_WRITE_COMMAND) {

      done_write.write(true);

    } else {

      done_write.write(false);

    }
  }
}

void tlmcpu_rtlcache_transactor::start_of_simulation() {

  // Initialize signals
  execute_address = 0;
  memory_address  = 0;
  memory_data     = 0;
  execute_valid   = false;
  memory_valid    = false;
  memory_write    = SC_LOGIC_0;
  execute_flushl  = false;
  execute_flush   = false;

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
