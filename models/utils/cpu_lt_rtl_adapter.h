
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

  // transactor thread
  void cache_transactor();

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

    // transactor thread
    SC_THREAD(cache_transactor);
    sensitive << clk.pos() << ival << dval << ico << dco << rst;

  }

 private:

  sc_signal<icache_in_type> ival;
  sc_signal<dcache_in_type> dval;

  sc_event cache_ready;
  
};  

#endif // _CPU_LT_RTL_ADAPTER_H__
