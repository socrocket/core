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

#ifndef __MMU_CACHE_BASE_H__
#define __MMU_CACHE_BASE_H__

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
#include "gaisler/leon3/mmucache/cache_if.h"
#include "gaisler/leon3/mmucache/ivectorcache.h"
#include "gaisler/leon3/mmucache/dvectorcache.h"
#include "gaisler/leon3/mmucache/nocache.h"
#include "gaisler/leon3/mmucache/mmu_cache_if.h"
#include "gaisler/leon3/mmucache/mmu.h"
#include "gaisler/leon3/mmucache/localram.h"

/// @addtogroup mmu_cache MMU_Cache
/// @{

/// Top-level class of the memory sub-system for the TrapGen LEON3 simulator
class mmu_cache_base :
  public AHBMaster<>,
  public mmu_cache_if,
  public CLKDevice {

 public:

  GC_HAS_CALLBACKS();
  SC_HAS_PROCESS(mmu_cache_base);
  SR_HAS_SIGNALS(mmu_cache_base);
  // TLM sockets
  // -----------

  // snooping port
  signal<t_snoop>::in snoop;

  // Signalkit IRQ output
  signal<std::pair<uint32_t, bool> >::out irq;

  /// @brief Constructor of the top-level class of the memory sub-system (caches and mmu).
  mmu_cache_base(
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
  ~mmu_cache_base();

  // Member functions
  // ----------------
  /// Instruction interface to functional part of the model
  virtual void exec_instr(
      const unsigned int &addr,
      unsigned char *ptr,
      unsigned int asi,
      unsigned int *debug,
      const unsigned int &flush,
      sc_core::sc_time& delay,
      bool is_dbg);

  /// Data interface to functional part of the model
  virtual void exec_data(
      const tlm::tlm_command cmd,
      const unsigned int &addr,
      unsigned char *ptr,
      unsigned int len,
      unsigned int asi,
      unsigned int *debug,
      unsigned int flush,
      unsigned int lock,
      sc_core::sc_time& delay,
      bool is_dbg,
      tlm::tlm_response_status &response);

  /// Called from AHB master to signal begin response
  virtual void response_callback(tlm::tlm_generic_payload * trans);

  /// MemIF implementation - writes data to AHB master
  virtual void mem_write(unsigned int addr, unsigned int asi, unsigned char * data,
                         unsigned int length, sc_core::sc_time * t,
                         unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock);
  /// MemIF implementation - reads data from AHB master
  virtual bool mem_read(unsigned int addr, unsigned int asi, unsigned char * data,
                        unsigned int length, sc_core::sc_time * t,
                        unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock);

  /// Send an interrupt over the central IRQ interface
  virtual void set_irq(uint32_t tt);

  /// Sent an exception to the CPU
  virtual void trigger_exception(unsigned int exception) = 0;

  /// Writes the cache control register
  void write_ccr(unsigned char * data, unsigned int len, sc_core::sc_time *delay, unsigned int * debug, bool is_dbg);
  /// Read the cache control register
  virtual unsigned int read_ccr(bool internal);

  /// Snooping function (For calling dcache->snoop_invalidate)
  void snoopingCallBack(const t_snoop& snoop, const sc_core::sc_time& delay);

  /// Automatically called at the beginning of the simulation
  void start_of_simulation();

  /// Calculate power/energy values from normalized input data
  void power_model();

  /// Static power callback
  gs::cnf::callback_return_type sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Dynamic/Internal power callback
  gs::cnf::callback_return_type int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Dynamic/Switching power callback
  gs::cnf::callback_return_type swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Called at end of simulation to print execution statistics
  void end_of_simulation();

  /// Reset function
  void dorst();

  /// Deal with clock changes
  void clkcng();

  /// Return clock period (for ahb interface)
  sc_core::sc_time get_clock();

  // data members
  // ------------

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

 protected:

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

  void mem_access();

  unsigned char write_buf[1024];
  unsigned int wb_pointer;

  sc_event bus_read_completed;

  tlm::tlm_fifo<tlm::tlm_generic_payload *> bus_in_fifo;

  /// Total number of successful transactions for execution statistics 
  sr_param<uint64_t> m_right_transactions;

  /// Total number of transactions for execution statistics
  sr_param<uint64_t> m_total_transactions;

  /// power monitoring enabled
  bool m_pow_mon;

  /// amba abstraction layer
  AbstractionLayer m_abstractionLayer;

  /// begin response signal for AT
  sc_event ahb_response_event;

  // ****************************************************
  // Power Modeling Parameters

  /// Normalized static power of controller
  sr_param<double> sta_power_norm;

  /// Normalized internal power of controller
  sr_param<double> int_power_norm;

  /// Normalized read access energy
  sr_param<double> dyn_read_energy_norm;

  /// Normalized write access energy
  sr_param<double> dyn_write_energy_norm;

  /// Controller static power
  sr_param<double> sta_power;

  /// Controller internal power
  sr_param<double> int_power;

  /// Controller switching power
  sr_param<double> swi_power;

  /// Power frame starting time
  sr_param<sc_core::sc_time> power_frame_starting_time;

  /// Dynamic energy per read access
  sr_param<double> dyn_read_energy;

  /// Dynamic energy per write access
  sr_param<double> dyn_write_energy;

  /// Number of reads from memory (read & reset by monitor)
  sr_param<uint64_t> dyn_reads;

  /// Number of writes to memory (read & reset by monitor)
  sr_param<uint64_t> dyn_writes;    

  uint64_t globl_count;
  
};

/// @}

#endif //__MMU_CACHE_H__
/// @}
