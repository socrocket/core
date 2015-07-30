// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file localram.h
/// Class definition of scratchpad/localram. Can be attached to the icache and
/// dcache controllers. The localram enables fast 0-waitstate access to
/// instructions or data.
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef __LOCALRAM_H__
#define __LOCALRAM_H__

#include <stdint.h>
#include "core/common/base.h"
#include "core/common/sr_param.h"

#include "gaisler/leon3/mmucache/mem_if.h"
#include "gaisler/leon3/mmucache/defines.h"

// Local scratchpad ram can optionally be attached to both instruction and data cache controllers.
// The scratch pad ram provides fast 0-waitstates ram memories for instructions and data.
// The ram can be between 1 - 512 kbyte, and mapped to any 16 Mbyte block in the address space.
// Accesses the the scratchpad ram are not cached, and will not appear on the AHB bus.
// The scratch pads do not appear on the AHB bus and can only be read or written by the
// processor. The instruction scratchpad must be initialized by software (through store instr.)
// before it can be used. The default address for the instruction ram is 0x83000000,
// and for the data ram 0x8f000000.
// ! Local scratch pad ram can only be enabled when the MMU is disabled !
// ! Address decoding and checking is done in class mmu_cache !

/// @brief Local Scratchpad RAM
class localram : public DefaultBase, public mem_if {

 public:

  GC_HAS_CALLBACKS();

  // Memory interface functions (mem_if):
  // -----------------------------
  /// Read from scratchpad
  virtual bool mem_read(unsigned int address, unsigned int asi, unsigned char *data, unsigned int len,
                        sc_core::sc_time *t, unsigned int *debug, bool is_dbg, bool &cacheable);
  /// Write to scratchpad
  virtual void mem_write(unsigned int address, unsigned int asi, unsigned char *data, unsigned int len,
                         sc_core::sc_time *t, unsigned int *debug, bool is_dbg, bool &cacheable);

  /// Helper functions for definition of clock cycle
  void clkcng(sc_core::sc_time &clk);

  /// Automatically called at the beginning of the simulation
  void start_of_simulation();

  // Calculate power/energy values from normalized input data
  void power_model();

  /// Static power callback
  gs::cnf::callback_return_type sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Dynamic/Internal power callback
  gs::cnf::callback_return_type int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Dynamic/Switching power callback
  gs::cnf::callback_return_type swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Hook up for showing statistics
  void end_of_simulation();

  // Constructor
  // -----------
  /// @brief Constructor of scratchpad RAM implementation (localram)
  /// @param name    SystemC module name
  /// @param lrsize  Local ram size. Size in kbyte = 2^lrsize (like top-level template)
  /// @param lrstart Local ram start address. The 8 most significant bits of the address.
  localram(ModuleName name,
           unsigned int lrsize,
           unsigned int lrstart,
           bool pow_mon = false);

  /// Destructor
  ~localram();

  // Pointer to actual memory
  t_cache_data * scratchpad;

  // Helpers
  // -------
  t_cache_data m_default_entry;

  // Local RAM parameters
  // --------------------
  /// Size of the local ram in words
  unsigned int m_lrsize;

  /// Start address of the local ram
  unsigned int m_lrstart;

  /// Power monitoring enabled or not
  bool m_pow_mon;

  /// *****************************************************
  /// Performance Counters

  /// GreenControl API container
  gs::cnf::cnf_api *m_api;

  /// Open a namespace for performance counting in the greencontrol realm
  gs::gs_param_array m_performance_counters;

  /// Number of read accesses
  sr_param<uint64_t> sreads;

  /// Number of write accesses
  sr_param<uint64_t> swrites;

  /// Volume of total reads (bytes)
  sr_param<uint64_t> sreads_byte;

  /// Volume of total writes (bytes)
  sr_param<uint64_t> swrites_byte;

  /// *****************************************************
  /// Power Modeling Parameters

  /// Normalized static power input
  sr_param<double> sta_power_norm;

  /// Normalized internal power input (activation independent)
  sr_param<double> int_power_norm;

  /// Normalized read access energy
  sr_param<double> dyn_read_energy_norm;

  /// Normalized write access energy
  sr_param<double> dyn_write_energy_norm;

  /// Parameter array for power data output
  gs::gs_param_array power;

  /// Static power of module
  sr_param<double> sta_power;

  /// Internal power of module (activation independent)
  sr_param<double> int_power;

  /// Swiching power of module
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

  /// Clock cycle time
  sc_core::sc_time clockcycle;

};

#endif // __LOCALRAM_H__
/// @}
