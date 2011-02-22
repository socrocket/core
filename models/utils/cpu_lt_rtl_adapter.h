
#ifndef __CPU_LT_RTL_ADAPTER_H__
#define __CPU_LT_RTL_ADAPTER_H__

#include <tlm.h>
#include <simple_target_socket.h>

#include "stdio.h"
#include "verbose.h"
#include "mmu_cache_wrapper.h"

#include "../lib/icio_payload_extension.h"
#include "../lib/dcio_payload_extension.h"


class cpu_lt_rtl_adapter : public sc_module {

 public:

  // clock and reset inputs
  sc_in<bool> rst;
  sc_in_clk clk;

  // instruction ports to/from mmu_cache
  sc_out<icache_in_type> ici;
  sc_in<icache_out_type> ico;

  // data ports to/from mmu_cache
  sc_out<dcache_in_type> dci;
  sc_in<dcache_out_type> dco;

  // cpu instruction cache socket
  tlm_utils::simple_target_socket<cpu_lt_rtl_adapter> icio;
  
  // cpu data cache socket
  tlm_utils::simple_target_socket<cpu_lt_rtl_adapter> dcio;

  // member functions
  // ----------------
 
  // forward transport function for icio socket
  void icio_custom_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay);

  // forward transport function for dcio socket
  void dcio_custom_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay);

  // data to signal functions
  void iread(unsigned int address, unsigned int * instr, unsigned int flush);
  void dread(unsigned int address, unsigned int * data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock);
  void dwrite(unsigned int address, unsigned int * data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock);

  void start_of_simulation();

  // latch output of caches
  void data_latch();

  // state machine threads
  void fsm_clock_tick();
  void fsm_next_state();
  void fsm_do_state();

  /// Constructor
  SC_HAS_PROCESS(cpu_lt_rtl_adapter);
  cpu_lt_rtl_adapter(sc_core::sc_module_name name) :
                                       sc_module(name),
				       rst("rst"),
                                       clk("clk"),
                                       ici("ici"),
				       ico("ico"),
				       dci("dci"),
				       dco("dco"),
				       icio("icio"),
				       dcio("dcio"),
                                       ival("ival"),
                                       dval("dval")
    {

    // register forward transport functions for icio and dcio sockets
    icio.register_b_transport(this, &cpu_lt_rtl_adapter::icio_custom_b_transport);
    dcio.register_b_transport(this, &cpu_lt_rtl_adapter::dcio_custom_b_transport);

    // 3x threads for state machine implementation
    // ===========================================
    
    // 1. state transition at clock tick
    SC_THREAD(fsm_clock_tick);
    sensitive << clk.pos();

    // 2. assign signals and determine next state (async)
    SC_THREAD(fsm_next_state);
    sensitive << state << iread_pending << dread_pending << dwrite_pending << ico << dco << ival << dval;

  }

 private:

  enum adapter_state { idle, ireadaddr, dreadaddr, dwriteaddr, dwritemiss, ireadmiss, dreadmiss };

  sc_signal<adapter_state> state;
  sc_signal<adapter_state> nextstate;

  sc_signal<icache_in_type> ival;
  sc_signal<dcache_in_type> dval;

  sc_signal<sc_logic> ico_mds;
  sc_signal<sc_logic> dco_mds;
  sc_signal<sc_logic> ico_hold;
  sc_signal<sc_logic> dco_hold;

  sc_signal<bool> dwrite_pending;
  sc_signal<bool> dread_pending;
  sc_signal<bool> iread_pending;

  sc_event dwrite_done;
  sc_event dread_done;
  sc_event iread_done;

  unsigned int hdl_data;
  unsigned int hdl_instr;

  // buffer cache outputs (data and instruction) @clock
  unsigned int ico_data_reg;
  unsigned int dco_data_reg;

  unsigned int transactor_state;
  
};  

#endif // _CPU_LT_RTL_ADAPTER_H__
