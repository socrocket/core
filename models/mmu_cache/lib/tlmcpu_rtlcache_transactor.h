#ifndef __TLMCPU_RTLCACHE_TRANSACTOR_H__
#define __TLMCPU_RTLCACHE_TRANSACTOR_H__

#include <tlm.h>
#include <simple_target_socket.h>

#include "verbose.h"
#include "amba.h"
#include "mmu_cache_wrapper.h"
#include "ahbpipe.h"

#include "../lib/icio_payload_extension.h"
#include "../lib/dcio_payload_extension.h"

/// Adapter class for connecting TLM Leon simulator (or testbench) to RTL version of mmu_cache
class tlmcpu_rtlcache_transactor : public sc_module {

 public:

  // Clock and reset inputs
  sc_in<bool> rst;
  sc_in_clk clock;

  // Instruction ports to/from mmu_cache
  sc_out<icache_in_type> ici;
  sc_in<icache_out_type> ico;

  // Data ports to/from mmu_cache
  sc_out<dcache_in_type> dci;
  sc_in<dcache_out_type> dco;

  // CPU instruction cache socket
  tlm_utils::simple_target_socket<tlmcpu_rtlcache_transactor> icio;
  
  // CPU data cache socket
  tlm_utils::simple_target_socket<tlmcpu_rtlcache_transactor> dcio;

  // Member functions
  // ----------------
 
  // Blocking transport function for icio socket
  void icio_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay);

  // Non-blocking forward transport function for icio socket
  tlm::tlm_sync_enum icio_nb_transport_fw(tlm::tlm_generic_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &delay);

  // Blocking transport function for dcio socket
  void dcio_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay);

  // Non-blocking forward transport function for dcio socket
  tlm::tlm_sync_enum dcio_nb_transport_fw(tlm::tlm_generic_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &delay);

  // Threads for processing icio transactions
  void i_request();
  void i_pipe();

  // Threads for processing dcio transactions
  void d_request();
  void d_pipe();

  // Mux dci and ici input signals
  void dci_signal_mux();
  void ici_signal_mux();

  // Helper functions for definition of clock cycle
  void clk(sc_core::sc_clock &clk);
  void clk(sc_core::sc_time &period);
  void clk(double period, sc_core::sc_time_unit base);

  // Called at start of simulation for signal initialization
  void start_of_simulation();

  /// Constructor
  SC_HAS_PROCESS(tlmcpu_rtlcache_transactor);
  tlmcpu_rtlcache_transactor(sc_core::sc_module_name name, amba::amba_layer_ids abstractionLayer);

 private:

  sc_fifo<tlm::tlm_generic_payload *> m_InstrFIFO;
  sc_fifo<tlm::tlm_generic_payload *> m_DataFIFO;

  sc_event m_BeginDataResponseEvent;
  sc_event m_BeginInstrResponseEvent;
  sc_event m_EndInstrResponseEvent;
  sc_event m_EndDataResponseEvent;

  // For modeling nops
  unsigned char nop_data[4];
  tlm::tlm_generic_payload * nop_trans;
  dcio_payload_extension * dext;

  /// Transaction pipeline for instruction access
  ahbpipe instr_pipe;

  /// Transaction pipeline for data access
  ahbpipe data_pipe;

  /// Event for triggering instr_pipe from instr_request()
  sc_event ipipe_event;
  
  /// Event for triggering data_pipe from data_request()
  sc_event dpipe_event;

  // Signals for controlling icache I/Os
  sc_signal<sc_lv<32> >  i_execute_address;
  sc_signal<bool>        i_execute_valid;

  sc_signal<sc_lv<32> >  i_memory_address;
  sc_signal<bool>        i_memory_valid;
  
  sc_signal<sc_lv<32> >  i_done_address;
  sc_signal<bool>        i_done_valid;
  
  // Signals for controlling dcache I/Os
  sc_signal<sc_lv<32> >  d_execute_address;
  sc_signal<sc_lv<32> >  d_execute_data;
  sc_signal<bool>        d_execute_valid;
  sc_signal<bool>        d_execute_flushl;
  sc_signal<bool>        d_execute_flush;
  sc_signal<sc_lv<8> >   d_execute_asi;
  sc_signal<bool>        d_execute_write;

  sc_signal<sc_lv<32> >  d_memory_address;
  sc_signal<sc_lv<32> >  d_memory_data;
  sc_signal<bool>        d_memory_valid;
  sc_signal<bool>        d_memory_write;
  sc_signal<bool>        d_memory_flushl;
  sc_signal<bool>        d_memory_flush;
  sc_signal<sc_lv<8> >   d_memory_asi;  

  sc_signal<sc_lv<32> >  d_done_address;
  sc_signal<sc_lv<32> >  d_done_data;
  sc_signal<bool>        d_done_valid;
  sc_signal<bool>        d_done_write;

  unsigned int           rdata_reg;
  unsigned int           instr_reg;

  sc_signal<sc_logic>    dhold;
  sc_signal<sc_logic>    ihold;

  // TLM Abstraction Layer
  amba::amba_layer_ids m_abstractionLayer;

  // Clock cycle time
  sc_core::sc_time clockcycle;


};  

#endif // _TLMCPU_RTLCACHE_ADAPTER_H__
