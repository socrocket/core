
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
  sc_core::sc_in_clk clk;

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
  void ico_demux();

  void mds_capture();
  
  // forward transport function for icio socket
  void icio_custom_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay);

  // forward transport function for dcio socket
  void dcio_custom_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay);

  // instruction port service thread
  void icio_service_machine();

  void instruction_gen();

  // data port service thread
  void dcio_service_machine();

  void dcache_removal();
  
  void iread(unsigned int address, unsigned int * instr, unsigned int flush);
  void dread(unsigned int address, unsigned int * data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock);
  void dwrite(unsigned int address, unsigned int * data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock);

  /// Constructor
  SC_HAS_PROCESS(cpu_lt_rtl_adapter);
  cpu_lt_rtl_adapter(sc_core::sc_module_name name) :
                                       sc_module(name),
				       rst("rst"),
    				       ici("ici"),
				       ico("ico"),
				       dci("dci"),
				       dco("dco"),
				       icio("icio"),
				       dcio("dcio") {

    // register forward transport functions for icio and dcio sockets
    icio.register_b_transport(this, &cpu_lt_rtl_adapter::icio_custom_b_transport);
    dcio.register_b_transport(this, &cpu_lt_rtl_adapter::dcio_custom_b_transport);

    SC_THREAD(ico_demux);
    sensitive << ico;

    SC_THREAD(mds_capture);
    sensitive << ico_mds;

    SC_THREAD(icio_service_machine);
    sensitive << clk.pos() << rst;

    SC_THREAD(instruction_gen);
    sensitive << next_instruction_event;
    dont_initialize();

    SC_THREAD(dcio_service_machine);
    dont_initialize();

    SC_THREAD(dcache_removal);
    dont_initialize();

    // initialize locals

    icio_address = 0;
    icio_hrdata = 0;

    dcio_address = 0;
    dcio_wdata = 0;
    dcio_asi = 0;
    dcio_lock = SC_LOGIC_0;
    dcio_flush = SC_LOGIC_0;
    dcio_flushl = SC_LOGIC_0;
    dcio_size = 0;
    dcio_wen = 0;

    ihrdata_reg = 0;
    dhrdata_reg = 0;

    ici_request_pending = 0;
    dci_read_request_pending = 0;
    dci_write_request_pending = 0;

    ico_hold = SC_LOGIC_0;
    ico_mds = SC_LOGIC_0;
    

  }
  
  sc_event next_instruction_event;
  sc_event dcache_removal_event;
  

  sc_signal<sc_logic> ico_hold;
  sc_signal<sc_logic> ico_mds;


  unsigned int icio_address;
  sc_logic icio_flush;
  unsigned int icio_hrdata;

  unsigned int dcio_address;
  unsigned int dcio_wdata;
  unsigned int dcio_asi;
  sc_logic dcio_lock;
  sc_logic dcio_flush;
  sc_logic dcio_flushl;
  unsigned int dcio_size;
  unsigned int dcio_wen;

  unsigned int ihrdata_reg;
  unsigned int dhrdata_reg;

  sc_event icache_ready;
  sc_event dcache_ready;

  sc_signal<bool> ici_request_pending;
  sc_signal<bool> dci_read_request_pending;
  sc_signal<bool> dci_write_request_pending;

};  

#endif // _CPU_LT_RTL_ADAPTER_H__
