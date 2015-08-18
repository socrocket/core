// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file mmu.h
/// Class definition of a memory management unit. The mmu can be configured to
/// have split or combined TLBs for instructions and data. The TLB size can be
/// configured as well. The memory page size is currently currently fixed to
/// 4kB.
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef __MMU_H__
#define __MMU_H__

#include <map>
#include <math.h>
#include "core/common/base.h"
#include "core/common/systemc.h"
#include "core/common/sr_param.h"

#include "gaisler/leon3/mmucache/mmu_if.h"
#include "gaisler/leon3/mmucache/tlb_adaptor.h"
#include "gaisler/leon3/mmucache/mmu_cache_if.h"

#include "core/common/vendian.h"
#include "gaisler/leon3/mmucache/defines.h"

// implementation of a memory management unit
// ------------------------------------------

/// @brief Memory Management Unit (MMU) for TrapGen LEON3 simulator
class mmu : public DefaultBase, public mmu_if {

 private:
  signed get_physical_address( uint64_t * paddr, signed * prot, unsigned * access_index,
                                  uint64_t vaddr, int asi, uint64_t * page_size,
                                  unsigned * debug, bool is_dbg, sc_core::sc_time * t, unsigned is_write, unsigned * pde_REMOVE );

 public:

  GC_HAS_CALLBACKS();

  /// @brief Constructor of the Memory Management Unit
  /// @param name                       SystemC module name,
  /// @param itlbnum                    Number of instruction TLBs
  /// @param dtlbnum                    Number of data TLBs
  /// @param tlb_type                   Type of TLB (shared or distinct)
  /// @param tlb_rep                    TLB replacement strategy
  /// @param mmupgsz                    MMU page size (default 4kB)
  mmu(ModuleName name, mmu_cache_if * _mmu_cache,
      unsigned int itlbnum,
      unsigned int dtlbnum, unsigned int tlb_type, unsigned int tlb_rep,
      unsigned int mmupgsz,
      bool pow_mon = false);

  /// Destructor
  ~mmu();

  // Member functions
  // ----------------
  /// Page descriptor cache (PDC) lookup
  signed tlb_lookup(unsigned int addr, unsigned asi,
                             std::map<t_VAT, t_PTE_context> * tlb,
                             unsigned int tlb_size, sc_core::sc_time * t,
                             unsigned int * debug, bool is_dbg, bool &cacheable,
                             unsigned is_write /* LOAD / STORE? */, uint64_t * paddr );
  /// Read mmu control register (ASI 0x19)
  unsigned int read_mcr();
  /// Read mmu context pointer register (ASI 0x19)
  unsigned int read_mctpr();
  /// Read mmu context register (ASI 0x19)
  unsigned int read_mctxr();
  /// Read mmu fault status register (ASI 0x19)
  unsigned int read_mfsr();
  /// Read mmu fault address register (ASI 0x19)
  unsigned int read_mfar();

  /// Write mmu control register (ASI 0x19)
  void write_mcr(unsigned int * data);
  /// Write mmu context pointer register (ASI 0x19)
  void write_mctpr(unsigned int * data);
  /// Write mmu context register (ASI 0x19)
  void write_mctxr(unsigned int * data);

  /// Diagnostic read of instruction PDC (ASI 0x5)
  void diag_read_itlb(unsigned int addr, unsigned int * data);
  /// Diagnostic write of instruction PDC (ASI 0x5)
  void diag_write_itlb(unsigned int addr, unsigned int * data);
  /// Diagnostic read of data PDC or shared instruction and data PDC (ASI 0x6)
  void diag_read_dctlb(unsigned int addr, unsigned int * data);
  /// Diagnostic write of data PDC or shared instruction and data PDC (ASI 0x6)
  void diag_write_dctlb(unsigned int addr, unsigned int * data);

  /// Selects a TLB entry for replacement (LRU or RANDOM replacement).
  /// Removes the selected entry from the TLB map and returns the 'number'
  /// of the TLB (which is now free).
  unsigned int tlb_remove(std::map<t_VAT, t_PTE_context> * tlb, unsigned int tlb_size);

  /// LRU replacement history updater
  void lru_update(t_VAT vpn, std::map<t_VAT, t_PTE_context> * tlb, unsigned int tlb_size);

  /// Return pointer to tlb instruction interface
  tlb_adaptor * get_itlb_if();
  /// Return pointer to tlb data interface
  tlb_adaptor * get_dtlb_if();

  /// Automatically started at beginning of simulation
  void start_of_simulation();

  /// Calculate power/energy values from normalized input data
  void power_model();
  
  /// TLB flush complete
  void tlb_flush();

  /// TLB flush certain entry
  void tlb_flush(uint32_t vpn);

  /// Static power callback
  gs::cnf::callback_return_type sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Dynamic/Internal power callback
  gs::cnf::callback_return_type int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Dynamic/Switching power callback
  gs::cnf::callback_return_type swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Displays execution statistics at the end of the simulation
  void end_of_simulation();

  /// Helper functions for definition of clock cycle
  void clkcng(sc_core::sc_time &clk);

 public:

  // Data members
  // ------------
  /// pointer to mmu_cache module (ahb interface functions)
  mmu_cache_if * m_mmu_cache;

  /// pointer to instruction tlb adaptor
  tlb_adaptor * itlb_adaptor;

  /// pointer to data tlb adaptor
  tlb_adaptor * dtlb_adaptor;

  // instruction and data tlb pointers
  // (depending on configuration may point to a shared tlb implementation)
  /// associative memory for instruction TLB (eventually also data tlb in shared mode)
  std::map<t_VAT, t_PTE_context> * itlb;
  /// associative memory for data TLB (not used in shared mode)
  std::map<t_VAT, t_PTE_context> * dtlb;

  /// iterator for PDC lookup
  std::map<t_VAT, t_PTE_context>::iterator pdciter;

  /// helper for tlb handling
  t_PTE_context * m_current_PTE_context;

  // mmu internal registers
  // ----------------------

  /// MMU Control Register (Page 253 Sparc Ref Manual):<br>
  /// -------------------------------------------------<br>
  /// [31-28] IMPL - Identifies the specific implementation of the MMU.
  /// It is hardwired into the implementation and is read only (0000).<br>
  /// [27-24] VER  - Version of the MMU implementation (0001)(read-only)<br>
  /// [32-21] ITLB - Number of ITLB entries. The number of ITLB entries is <br>
  /// calculated as 2^ITLB. If the TLB is shared between instructions and data, <br>
  /// this field indicates the total number of TLBs.
  /// [20-18] DTLB - Number of DTLB entries. The number of DTLB entries is <br>
  /// calculated as 2^DTLB. If the TLB is shared between instructions and data, <br>
  /// this field is zero.
  /// [17-16] Page size.The size of the smallest MMU page: 0 - 4kbyte, 1 - 8kbyte, <br>
  /// 2 - 16kbyte, 3 - 32kbyte.
  /// [15] TLB disable. When set to 1, the TLB will be disabled and each data <br>
  /// access will generate an MMU page table walk. <br>
  /// [14] Separate TLB. This bit is set to 1 if separate instructions <br>
  /// and data TLM are implemented. <br>
  /// [13-2] reserved <br>
  /// [1] NF  - The "No Fault" bit. When NF=0, any fault detected by the MMU
  /// causes FSR and FAR to be updated and causes a fault to be generated to
  /// the processor. When NF=1, a fault on an access to ASI 9 is handled as
  /// when NF=0; a fault on an access to any other ASI causes FSR and FAR
  /// to be updated but no fault is generated to the processor. If a fault on
  /// access to an ASI other than 9 occurs while NF=1, subsequently resetting
  /// NF from 1 to 0 does not cause a fault to the processor
  /// (even though FSR.FT != 0 at that time). A change in value of the NF bit
  /// takes effect as soon as the bit is written; a subsequent access to ASI 9
  /// will be evaluated according to the new value of the NF bit.<br>
  /// [0] E - The Enable bit enables (1) or disables (0) the MMU.
  /// When the MMU is disabled:<br>
  /// - All virtual addresses pass through the MMU untranslated and appear as
  /// physical addresses.<br>
  /// - The upper 4 of the 36 bits of the physical address are zero.<br>
  /// - The MMU indicates that all virtual addresses are non-cacheable<br>
  unsigned int MMU_CONTROL_REG;

  /// MMU Context Table Pointer (Page 254 Sparc Ref Manual):<br>
  /// ------------------------------------------------------<br>
  /// The Context Table Pointer points to the Context Table in physical memory.
  /// The table is indexed by the contents of the Context Register. The Context
  /// Table Pointer appears on bits 35 through 6 of the physical address bus during
  /// the first fetch occurring during miss processing. The context table pointed
  /// to by the Context Table Pointer must be aligned on a boundary equal to the
  /// size of the table.<br>
  /// [31-2] Context Table Pointer<br>
  unsigned int MMU_CONTEXT_TABLE_POINTER_REG;

  /// MMU Context Number (Page 255 Sparc Ref Manual):<br>
  /// -----------------------------------------------<br>
  /// The Context Register defines which of the possible process virtual address
  /// spaces is considered the current address space. Subsequently accesses to
  /// memory through the MMU are translated for the current address space, until
  /// the Context Register is changed. Each MMU implementation may specify a maximum
  /// context number, which must be one less than a power of 2.<br>
  /// [31-0] Context Number<br>
  unsigned int MMU_CONTEXT_REG;

  /// MMU Fault Status Register (Page 256 Sparc Ref Manual):<br>
  /// ------------------------------------------------------<br>
  /// The Fault Status Register provides information on exceptions (faults) issued
  /// by the MMU. Since the CPU is pipelined, several faults may occur before a
  /// trap is taken. The faults are grouped into three classes:<br>
  /// - instruction access faults<br>
  /// - data access faults<br>
  /// - translation table access faults<br>
  /// If another instruction access fault occurs before the fault status of a
  /// previous instruction access fault has been read by the CPU, the MMU
  /// writes the status of the latest fault into the Fault Status Register,writes
  /// the faulting address into the Fault Address Register, and sets the OW bit
  /// to indicate that the previous fault status has been lost. The MMU and CPU
  /// must ensure that if multiple data access faults can occur, only the status
  /// of the one taken by the CPU is latched into the Fault Status Register.
  /// If data fault status overwrites previous instruciton fault status, the
  /// overwrite bit (OW) is cleared, since the fault status is represented correctly.
  /// An instruciton access fault may not overwrite a data access fault.
  /// A translation table access fault occurs if an MMU page table access causes an
  /// external system error. If a translation table access fault overwrites a previous
  /// instruction or data access fault, the OW bit is cleared. An instruction or
  /// data access fault may not overwrite a translation table access fault.<br>
  /// [17-10] EBE - External Bus Error field bits are set when a system error occurs
  /// during memory access. The meaning of the individual bits are implementation-dependent.<br>
  /// External Bus Errors are time-outs, parity errors e.g.. Not supported by TLM model. <br>
  /// [9-8] L - The Level field is set to the page table level of the entry which caused the
  /// fault (0 - Context Table, 1 - Level 1 Page Table, 2 - Level 2, 3 - Level 3).<br>
  /// [7-5] AT - The Access Type field defines the taype of access which cause the fault.
  /// (see Page 257 of Sparc Ref Manual)<br>
  /// [4-2] FT - Defines the Fault Type of the current fault. (see Page 257 of Sparc Ref Man.)<br>
  /// [1] FAV - The Fault Address Valid bit is set to one if the contents of the Fault
  /// Address Register are valid.<br>
  /// [0] OW - The Overwrite bit is set to one if the Fault Status Register has been
  /// written more than once by faults of the same class since the last time it was read.<br>
  unsigned int MMU_FAULT_STATUS_REG;

  /// MMU Fault Address Register (Page 258 Sparc Ref Manual):<br>
  /// -------------------------------------------------------<br>
  /// The Fault Address Register contains the virtual memory address of the fault
  /// recorded in the Fault Status Register. Fault addresses are overwritten
  /// according to the same priority used for the Fault Status Register. Writes to the
  /// Fault Address Register are ignored.<br>
  /// [31-0] Fault Address<br>

  unsigned int MMU_FAULT_ADDRESS_REG;

  // mmu parameters
  // --------------
  /// number of instruction tlbs
  unsigned int m_itlbnum;
  /// number of data tlbs
  unsigned int m_dtlbnum;
  /// log2 version of itlbnum
  unsigned int m_itlblog2;
  /// log2 version of dtlbnum
  unsigned int m_dtlblog2;
  /// tlb type (bit0 - split/combined, bit1 - standard/fast write buffer)
  unsigned int m_tlb_type;
  /// tlb replacment strategy (no inform. found yet - tmp use random)
  unsigned int m_tlb_rep;
  /// mmu page size
  unsigned int m_mmupgsz;

  // page size indices (helpers)
  // ---------------------------
  /// width of vtag index 1
  unsigned int m_idx1;
  /// width of vtag index 2
  unsigned int m_idx2;
  /// width of vtag index 3
  unsigned int m_idx3;
  /// total width of vtag
  unsigned int m_vtag_width;

  /// Pseudo random counter for LRU
  uint32_t m_pseudo_rand;

  /// Power Monitoring enabled?
  bool m_pow_mon;

  unsigned access_table[8][8];

  // *****************************************************
  // Performance Counters

  /// GreenControl API container
  gs::cnf::cnf_api *m_api;

  /// Open a namespace for performance counting in the greencontrol realm
  gs::gs_param_array m_performance_counters;

  /// Number of TLB hits
  gs::gs_param<unsigned long long *> tihits;
  gs::gs_param<unsigned long long *> tdhits;

  /// Number of TLB misses
  sr_param<uint64_t> timisses;
  sr_param<uint64_t> tdmisses;


  // *****************************************************
  // Power Modeling Parameters

  /// Normalized static power of mmu
  sr_param<double> sta_power_norm;

  /// Normalized internal power of mmu (switching independent)
  sr_param<double> int_power_norm;

  /// Normalized tlb static power input
  sr_param<double> sta_tlb_power_norm;

  /// Normalized internal power of tlb
  sr_param<double> int_tlb_power_norm;

  /// Normalized tlb read energy
  sr_param<double> dyn_tlb_read_energy_norm;

  /// Normalized tlb write energy
  sr_param<double> dyn_tlb_write_energy_norm;

  /// Parameter array for power data output
  gs::gs_param_array power;

  /// MMU static power
  sr_param<double> sta_power;

  /// MMU internal power
  sr_param<double> int_power;

  /// MMU switching power
  sr_param<double> swi_power;

  /// Power frame starting time
  sr_param<sc_core::sc_time> power_frame_starting_time;

  /// Parameter array for power output of itlb
  gs::gs_param_array itlbram;

  /// Dynamic energy itlb read
  sr_param<double> dyn_itlb_read_energy;

  /// Dynamic energy itlb write
  sr_param<double> dyn_itlb_write_energy;

  /// Number of itlb reads
  sr_param<uint64_t> dyn_itlb_reads;

  /// Number of itlb writes
  sr_param<uint64_t> dyn_itlb_writes;

  /// Parameter array for power output of dtlb
  gs::gs_param_array dtlbram;

  /// Dynamic energy dtlb read
  sr_param<double> dyn_dtlb_read_energy;

  /// Dynamic energy dtlb write
  sr_param<double> dyn_dtlb_write_energy;

  /// Number of dtlb reads
  sr_param<uint64_t> dyn_dtlb_reads;

  /// Number of dtlb writes
  sr_param<uint64_t> dyn_dtlb_writes;

  /// Clock cycle time
  sc_core::sc_time clockcycle;

};

#endif // __MMU_H__
/// @}
