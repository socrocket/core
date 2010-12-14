#ifndef __TESTBENCH_CT_H__
#define __TESTBENCH_CT_H__

#include "tlm.h"
#include "stdio.h"
#include "locals.h"
#include "verbose.h"

#include "mmu_cache_wrapper.h"

class testbench_ct : public sc_module {

 public:

  sc_out<bool> rst;
  sc_out<sc_logic> rst_scl;
  sc_out<sc_logic> signal_clk;

  sc_out<icache_in_type> ici;
  sc_in<icache_out_type> ico;

  sc_out<dcache_in_type> dci;
  sc_in<dcache_out_type> dco;

  // create systemc clock
  sc_clock clock;

  // clock and reset control
  void clock_gen_thread();
  void reset_gen_thread();

  // main stimuli process
  void initiator_thread();

  // react on cache outputs
  void ico_listener();
  void dco_listener();

  // helper functions for writing the ici, dci output ports
  void instr_gen(unsigned int fpc, bool inull, bool su, bool flush); 
  void data_gen(unsigned int asi, unsigned int address, unsigned int data, unsigned int size, bool enaddr, bool eenaddr, bool nullify, bool lock, bool read, bool write,bool flush);

  SC_HAS_PROCESS(testbench_ct);

  testbench_ct(sc_core::sc_module_name name) : sc_module(name), clock("clock",10,0.5,0,true), rst("rst"), rst_scl("rst_scl"), signal_clk("signal_clk"), ici("ici"), ico("ico"), dci("dci"), dco("dco") {

    SC_THREAD(initiator_thread);
    sensitive << clock.posedge_event();
    dont_initialize();

    SC_THREAD(clock_gen_thread);
    sensitive << clock;

    SC_THREAD(reset_gen_thread);

    SC_THREAD(ico_listener);
    sensitive << ico;

    SC_THREAD(dco_listener);
    sensitive << dco;

  }

};  

#endif // __TESTBENCH_CT_H__
