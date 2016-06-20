// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file mmu_cache.h
/// Class definition of LEON2/3 cache-subsystem consisting of instruction cache,
/// data cache, i/d localrams and memory management unit. The mmu_cache class
/// provides two TLM slave sockets for connecting the cpu and an AHB master
/// interface for connecting the processor bus.
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef __MMU_CACHE_H__
#define __MMU_CACHE_H__

#include "core/common/sr_param.h"
#include "core/common/systemc.h"
#include "core/common/amba.h"
//#include <tlm_1/tlm_req_rsp/tlm_channels/tlm_fifo/tlm_fifo.h>

#include <math.h>

#include "gaisler/leon3/mmucache/icio_payload_extension.h"
#include "gaisler/leon3/mmucache/dcio_payload_extension.h"

#include "core/common/socrocket.h"
#include "core/common/sr_signal.h"
#include "core/common/ahbmaster.h"
#include "core/common/clkdevice.h"

#include "core/common/verbose.h"
#include "gaisler/leon3/mmucache/mmu_cache_base.h"

/// Top-level class of the memory sub-system for the TrapGen LEON3 simulator
class mmu_cache :
  public mmu_cache_base {

 public:

  GC_HAS_CALLBACKS();
  SC_HAS_PROCESS(mmu_cache);
  SR_HAS_SIGNALS(mmu_cache);
  // TLM sockets
  // -----------

  // iu3 instruction cache in/out
  tlm_utils::simple_target_socket<mmu_cache> icio;

  // iu3 data cache in/out
  tlm_utils::simple_target_socket<mmu_cache> dcio;

  // snooping port
  signal<t_snoop>::in snoop;

  // Signalkit IRQ output
  signal<std::pair<uint32_t, bool> >::out irq;

  /// @brief Constructor of the top-level class of the memory sub-system (caches and mmu).
  mmu_cache(
      ModuleName name = "",  ///< SystemC module name
      bool icen = true,                   ///< instruction cache enable
      uint32_t irepl = 1,                 ///< instruction cache replacement strategy
      uint32_t isets = 4,                 ///< number of instruction cache sets
      uint32_t ilinesize = 8,             ///< instruction cache line size (in bytes)
      uint32_t isetsize = 8,              ///< size of an instruction cache set (in kbytes)
      uint32_t isetlock = true,           ///< enable instruction cache locking
      uint32_t dcen = true,               ///< data cache enable
      uint32_t drepl = 1,                 ///< data cache replacement strategy
      uint32_t dsets = 2,                 ///< number of data cache sets
      uint32_t dlinesize = 4,             ///< data cache line size (in bytes)
      uint32_t dsetsize = 8,              ///< size of a data cache set (in kbytes)
      bool dsetlock = true,               ///< enable data cache locking
      bool dsnoop = true,                 ///< enable data cache snooping
      bool ilram = false,                 ///< enable instruction scratch pad
      uint32_t ilramsize = 0x000,         ///< size of the instruction scratch pad (in kbytes)
      uint32_t ilramstart = 0x000,        ///< start address of the instruction scratch pad
      uint32_t dlram = false,             ///< enable data scratch pad
      uint32_t dlramsize = 0x000,         ///< size of the data scratch pad (in kbytes)
      uint32_t dlramstart = 0x000,        ///< start address of the data scratch pad
      uint32_t cached = 0,                ///< fixed cacheability mask
      bool mmu_en = true,                 ///< mmu enable
      uint32_t itlb_num = 8,              ///< number of instruction TLBs
      uint32_t dtlb_num = 8,              ///< number of data TLBs
      uint32_t tlb_type = 0,              ///< split or shared instruction and data TLBs
      uint32_t tlb_rep = 1,               ///< TLB replacement strategy
      uint32_t mmupgsz = 0,               ///< MMU page size
      uint32_t hindex = 0,                ///< ID of the bus master
      bool pow_mon = false,               ///< Enable power monitoring
      AbstractionLayer ambaLayer = amba::amba_LT);  ///< Select LT or AT abstraction

  // Destructor
  ~mmu_cache();

  // Member functions
  // ----------------
  /// Instruction interface to functional part of the model
  void exec_instr(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay, bool is_dbg);
  /// Data interface to functional part of the model
  void exec_data(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay, bool is_dbg);

  /// TLM blocking forward transport function for icio socket
  void icio_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay);
  /// TLM blocking forward transport function for dcio socket
  void dcio_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay);

  /// TLM non-blocking forward transport function for icio socket
  tlm::tlm_sync_enum icio_nb_transport_fw(tlm::tlm_generic_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &delay);
  /// TLM non-blocking forward transport function for dcio socket
  tlm::tlm_sync_enum dcio_nb_transport_fw(tlm::tlm_generic_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &delay);

  /// TLM instruction debug transport
  unsigned int icio_transport_dbg(tlm::tlm_generic_payload &trans);

  /// TLM data debug transport
  unsigned int dcio_transport_dbg(tlm::tlm_generic_payload &trans);

  /// Instruction service thread for AT
  void icio_service_thread();

  /// Data service thread for AT
  void dcio_service_thread();

  /// TLB flush complete
  //virtual void tlb_flush();

  /// TLB flush certain entry
  //virtual void tlb_flush(uint32_t vpn);

  // data members
  // ------------

  // icio payload event queue (for AT)
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> icio_PEQ;

  // dcio payload event queue (for AT)
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> dcio_PEQ;
};

#endif //__MMU_CACHE_H__
/// @}
