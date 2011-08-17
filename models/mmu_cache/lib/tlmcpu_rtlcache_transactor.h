
#ifndef __TLMCPU_RTLCACHE_TRANSACTOR_H__
#define __TLMCPU_RTLCACHE_TRANSACTOR_H__

#include <tlm.h>
#include <simple_target_socket.h>

#include "verbose.h"
#include "amba.h"
#include "mmu_cache_wrapper.h"

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
  void instrRequestThread();
  void instrResponseThread();

  // Threads for processing dcio transactions
  void dataRequestThread();
  void dataSampleThread();
  void dataResponseThread();

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

  tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_InstrTransPEQ;
  std::deque<tlm::tlm_generic_payload *> m_DataTransFIFO;

  sc_fifo<tlm::tlm_generic_payload *> m_SampleDataTransFIFO;
  sc_fifo<tlm::tlm_generic_payload *> m_ResponseDataTransFIFO;

  //std::deque<tlm::tlm_generic_payload *> m_SampleDataTransFIFO;
  std::deque<tlm::tlm_generic_payload *> m_PostInstrTransFIFO;
  //std::deque<tlm::tlm_generic_payload *> m_ResponseDataTransFIFO;

  sc_event m_BeginDataResponseEvent;
  sc_event m_BeginInstrResponseEvent;
  sc_event m_EndInstrResponseEvent;
  sc_event m_EndDataResponseEvent;

  amba::amba_layer_ids m_abstractionLayer;

  // Signals for ici port connections
  sc_signal<sc_lv<32> >  i_rpc;
  sc_signal<sc_lv<32> >  i_fpc;
  sc_signal<sc_lv<32> >  i_dpc;
  sc_signal<sc_logic>    i_rbranch;
  sc_signal<sc_logic>    i_fbranch;
  sc_signal<sc_logic>    i_inull;
  sc_signal<sc_logic>    i_su;
  sc_signal<bool>        i_flush;
  sc_signal<sc_logic>    i_flushl;
  sc_signal<sc_lv<29> >  i_fline;
  sc_signal<sc_logic>    i_pnull; 

  // Signals for dci port connections
  sc_signal<sc_lv<8> >   d_asi;
  sc_signal<sc_lv<32> >  d_maddress;
  sc_signal<sc_lv<32> >  d_eaddress;
  sc_signal<sc_lv<32> >  d_edata;
  sc_signal<sc_lv<2> >   d_size;
  sc_signal<sc_logic>    d_enaddr;
  sc_signal<sc_logic>    d_eenaddr;
  sc_signal<sc_logic>    d_nullify;
  sc_signal<bool>        d_lock;
  sc_signal<sc_logic>    d_read;
  sc_signal<sc_logic>    d_write;
  sc_signal<bool>        d_flush;
  sc_signal<bool>        d_flushl;
  sc_signal<sc_logic>    d_dsuen;
  sc_signal<sc_logic>    d_msu;
  sc_signal<sc_logic>    d_esu;
  sc_signal<sc_logic>    d_intack;

  // Clock cycle time
  sc_core::sc_time clockcycle;
  
};  

#endif // _TLMCPU_RTLCACHE_ADAPTER_H__
