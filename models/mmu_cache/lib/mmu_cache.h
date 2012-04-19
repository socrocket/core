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
// Title:      mmu_cache.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Class definition of a cache-subsystem.
//             The cache-subsystem envelopes an instruction cache,
//             a data cache and a memory management unit.
//             The mmu_cache class provides two TLM slave interfaces
//             for connecting the cpu to the caches and an AHB master
//             interface for connection to the main memory.
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#ifndef __MMU_CACHE_H__
#define __MMU_CACHE_H__

#include <greencontrol/config.h>
#include <amba.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

#include <math.h>

#include "icio_payload_extension.h"
#include "dcio_payload_extension.h"

#include "socrocket.h"
#include "signalkit.h"
#include "ahbdevice.h"
#include "clkdevice.h"
#include "power_monitor.h"

#include "verbose.h"
#include "cache_if.h"
#include "ivectorcache.h"
#include "dvectorcache.h"
#include "nocache.h"
#include "mmu_cache_if.h"
#include "mmu.h"
#include "localram.h"

/// @addtogroup mmu_cache MMU_Cache
/// @{

/// Top-level class of the memory sub-system for the TrapGen LEON3 simulator
class mmu_cache : public sc_core::sc_module, public mmu_cache_if, public AHBDevice, public CLKDevice {

 public:
  SC_HAS_PROCESS(mmu_cache);
  SK_HAS_SIGNALS(mmu_cache);
  // TLM sockets
  // -----------
  
  // iu3 instruction cache in/out
  tlm_utils::simple_target_socket<mmu_cache> icio;
  
  // iu3 data cache in/out
  tlm_utils::simple_target_socket<mmu_cache> dcio;

  // amba master socket
  amba::amba_master_socket<32> ahb;
  
  // snooping port
  signal<t_snoop>::in snoop;

  // Signalkit IRQ output
  signal<bool>::selector irq;

  /// @brief Constructor of the top-level class of the memory sub-system (caches and mmu).
  /// @icen          instruction cache enable
  /// @irepl         instruction cache replacement strategy
  /// @isets         number of instruction cache sets
  /// @ilinesize     instruction cache line size (in bytes)
  /// @isetsize      size of an instruction cache set (in kbytes)
  /// @isetlock      enable instruction cache locking
  /// @dcen          data cache enable
  /// @drepl         data cache replacement strategy
  /// @dsets         number of data cache sets
  /// @dlinesize     data cache line size (in bytes)
  /// @dsetsize      size of a data cache set (in kbytes)
  /// @dsetlock      enable data cache locking
  /// @dsnoop        enable data cache snooping
  /// @ilram         enable instruction scratch pad
  /// @ilramsize     size of the instruction scratch pad (in kbytes)
  /// @ilramstart    start address of the instruction scratch pad
  /// @dlram         enable data scratch pad
  /// @dlramsize     size of the data scratch pad (in kbytes)
  /// @dlramstart    start address of the data scratch pad
  /// @cached        fixed cacheability mask
  /// @mmu_en        mmu enable
  /// @itlb_num      number of instruction TLBs
  /// @dtlb_num      number of data TLBs
  /// @tlb_type      split or shared instruction and data TLBs
  /// @tlb_rep       TLB replacement strategy
  /// @mmupgsz       MMU page size
  /// @name                               SystemC module name
  /// @id                                 ID of the AHB master
  /// @abstractionLayer                   Select LT or AT abstraction
  /// @icache_hit_read_response_delay     Delay on an instruction cache hit
  /// @icache_miss_read_response_delay    Delay on an instruction cache miss
  /// @dcache_hit_read_response_delay     Delay on a data cache read hit
  /// @dcache_miss_read_response_delay    Delay on a data cache read miss
  /// @dcache_write_response_delay        Delay on a data cache write (hit/miss)
  /// @itlb_hit_response_delay            Delay on an instruction TLB hit
  /// @itlb_miss_response_delay           Delay on an instruction TLB miss
  /// @dtlb_hit_response_delay            Delay on a data TLB hit
  /// @dtlb_miss_response_delay           Delay on a data TLB miss
  mmu_cache(unsigned int icen, unsigned int irepl, unsigned int isets,
            unsigned int ilinesize, unsigned int isetsize,
            unsigned int isetlock, unsigned int dcen, unsigned int drepl,
            unsigned int dsets, unsigned int dlinesize,
            unsigned int dsetsize, unsigned int dsetlock,
            unsigned int dsnoop, unsigned int ilram,
            unsigned int ilramsize, unsigned int ilramstart,
            unsigned int dlram, unsigned int dlramsize,
            unsigned int dlramstart, unsigned int cached,
            unsigned int mmu_en, unsigned int itlb_num,
            unsigned int dtlb_num, unsigned int tlb_type,
            unsigned int tlb_rep, unsigned int mmupgsz,
            sc_core::sc_module_name name, unsigned int id,
            bool powmon,
            amba::amba_layer_ids ambaLayer);

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
	
  /// TLM non-blocking backward transport function for ahb master socket
  tlm::tlm_sync_enum ahb_nb_transport_bw(tlm::tlm_generic_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &delay);
  
  /// TLM instruction debug transport
  unsigned int icio_transport_dbg(tlm::tlm_generic_payload &trans);

  /// TLM data debug transport
  unsigned int dcio_transport_dbg(tlm::tlm_generic_payload &trans);
  
  /// Instruction service thread for AT
  void icio_service_thread();

  /// Data service thread for AT
  void dcio_service_thread();

  /// Thread for response synchronization (AT only: sync and send END_RESP)
  void ResponseThread();

  /// Thread for data phase processing in write operations (AT only: sends BEGIN_DATA)
  void DataThread();

  /// Delayed release of transactions (AT only)
  void cleanUP();

  /// MemIF implementation - writes data to AHB master
  virtual void mem_write(unsigned int addr, unsigned int asi, unsigned char * data,
                         unsigned int length, sc_core::sc_time * t,
                         unsigned int * debug, bool is_dbg);
  /// MemIF implementation - reads data from AHB master
  virtual bool mem_read(unsigned int addr, unsigned int asi, unsigned char * data,
                        unsigned int length, sc_core::sc_time * t,
                        unsigned int * debug, bool is_dbg);

  /// Send an interrupt over the central IRQ interface
  virtual void set_irq(uint32_t tt);

  /// Writes the cache control register
  void write_ccr(unsigned char * data, unsigned int len, sc_core::sc_time *delay, unsigned int * debug, bool is_dbg);
  /// Read the cache control register
  virtual unsigned int read_ccr(bool internal);

  /// Snooping function (For calling dcache->snoop_invalidate)
  void snoopingCallBack(const t_snoop& snoop, const sc_core::sc_time& delay);
  
  /// Called at end of simulation to print execution statistics
  void end_of_simulation();

  /// Reset function
  void dorst();

  /// Deal with clock changes
  void clkcng();
  
  // data members
  // ------------
	
  // icio payload event queue (for AT)
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> icio_PEQ;

  // dcio payload event queue (for AT)
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> dcio_PEQ;

  /// instruction cache pointer
  cache_if * icache;
  /// data cache pointer
  cache_if * dcache;
  /// mmu poiner
  mmu * m_mmu;
  /// instruction scratchpad pointer
  localram * ilocalram;
  /// data scratchpad pointer
  localram * dlocalram;

 private:

  // CACHE CONTROL REGISTER
  // ======================
  // [1:0] instruction cache state (ICS) - indicates the current instruction cache state
  // (X0 - disabled, 01 - frozen, 11 - enabled)
  // [3:2] data cache state (DCS) - indicates the current data cache state
  // (X0 - disabled, 01 - frozen, 11 - enabled)
  // [4] instruction cache freeze on interrupt (IF) - if set the instruction cache will automatically be frozen when an asynchronous interrupt is taken
  // [5] data cache freeze on interrupt (DF) - if set the data cache will automatically be frozen when an asynchronous interrupt is taken
  // [14] - data cache flush pending (DP) - This bit is set when an data cache flush operation is in progress
  // [15] - instruction cache flush pending (IP) - This bis is set when an instruction cache flush operation is in progress
  // [16] - instruction burst fetch (IB) - This bit enables burst fill during instruction fetch
  // [21] - Flush Instruction cache (FI) - If set, will flush the instruction cache. Always reads as zero.
  // [22] - Flush data cache (FD)        - If set, will flush the data cache. Always reads as zero.
  // [23] - Data cache snoop enable (DS) - If set, will enable data cache snooping.
  unsigned int CACHE_CONTROL_REG;

  /// icache enable
  unsigned int m_icen;
  /// dcache enabled
  unsigned int m_dcen;
  /// dcache snooping enabled
  unsigned int m_dsnoop;
  /// instruction scratchpad enabled
  unsigned int m_ilram;
  /// instruction scratchpad starting address
  unsigned int m_ilramstart;
  /// data scratchpad enabled
  unsigned int m_dlram;
  /// data scratchpad starting address
  unsigned int m_dlramstart;
  /// enables fixed cacheability mask
  unsigned int m_cached;
  /// mmu enabled
  unsigned int m_mmu_en;
  /// amba master id
  unsigned int m_master_id;

  /// GreenControl API container
  gs::cnf::cnf_api *m_api;
        
  gs::gs_param_array m_performance_counters;
  /// Total number of successful transactions for execution statistics 
  gs::gs_param<unsigned long long> m_right_transactions;
  
  /// Total number of transactions for execution statistics
  gs::gs_param<unsigned long long> m_total_transactions;

  /// power monitoring enabled
  bool m_pow_mon;

  /// amba abstraction layer
  amba::amba_layer_ids m_abstractionLayer;
  
  /// Event for unblocking mem_read and mem_write from ahb_nb_transport_bw (on END_REQ)
  sc_event  mEndRequestEvent;
  /// Event for unblocking mem_read from ahb_nb_transport_bw (on BEGIN_RESP)
  sc_event  mBeginResponseEvent;
  
  /// PEQ for transfer of payload from ahb_nb_transport_bw to ResponseThread (on BEGIN_RESP)
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> mResponsePEQ;
  /// PEQ for transfer of payload from ahb_nb_transport_bw or mem_write to DataThread (after END_REQ)
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> mDataPEQ;
  /// PEQ for transfer of expired payload to cleanUP thread.
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> mEndTransactionPEQ;
};

/// @}

#endif //__MMU_CACHE_H__
