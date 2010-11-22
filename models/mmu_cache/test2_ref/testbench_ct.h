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

  sc_out<icache_out_type> ico_buf_out;

  sc_out<dcache_in_type> dci;
  sc_in<dcache_out_type> dco;

  // create systemc clock
  sc_clock clock;
  sc_clock clock_slow;

  void clock_gen_thread();
  void reset_gen_thread();
  void initiator_thread();
  void ico_monitor_thread();

  sc_signal<icache_out_type> ico_buf;

  void iread(unsigned int address, unsigned int * instr);
  void dread(unsigned int address, unsigned int * data, unsigned int length, unsigned int asi, unsigned int lock);
  void dwrite(unsigned int address, unsigned int data, unsigned int length, unsigned int asi, unsigned int lock);

  SC_HAS_PROCESS(testbench_ct);

  testbench_ct(sc_core::sc_module_name name) : sc_module(name), clock("clock",10,0.5,0,true), clock_slow("clock_slow",100,0.5,0,false), rst("rst"), rst_scl("rst_scl"), signal_clk("signal_clk"), ici("ici"), ico("ico"), dci("dci"), dco("dco"), ico_buf_out("ico_buf_out") {

    SC_THREAD(initiator_thread);
    sensitive << clock.posedge_event();
    dont_initialize();

    SC_THREAD(clock_gen_thread);
    sensitive << clock;

    SC_THREAD(ico_monitor_thread);
    sensitive << clock.posedge_event();

    SC_THREAD(reset_gen_thread);

  }

};  

#endif // __TESTBENCH_CT_H__
