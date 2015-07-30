// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file ivectorcache.h
/// Class definition of an instruction cache. The cache can be configured direct
/// mapped or set associative. Set-size, line-size and replacement strategy can
/// be defined through constructor arguments.
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef __IVECTORCACHE_H__
#define __IVECTORCACHE_H__

#include "gaisler/leon3/mmucache/vectorcache.h"
#include "gaisler/leon3/mmucache/defines.h"

#include "tlm.h"

// implementation of instruction cache memory and controller
// ----------------------------------------------------------
/// @brief Instruction cache implementation for TrapGen LEON3 simulator
class ivectorcache : public vectorcache {

 public:

  GC_HAS_CALLBACKS();

  // Overwrite write function
  void mem_write(unsigned int address, unsigned char * data,
                 unsigned int len, sc_core::sc_time * t,
                 unsigned int * debug, bool is_dbg, bool &cacheable);

  // Implement ccr check
  unsigned int check_mode();

  // Implement cache type function
  t_cache_type get_cache_type();

  // Automatically called at start of simulation
  void start_of_simulation();

  // Calculate power/energy values from normalized input data
  void power_model();

  /// Static power callback
  gs::cnf::callback_return_type sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Dynamic/Internal power callback
  gs::cnf::callback_return_type int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Dynamic/Switching power callback
  gs::cnf::callback_return_type swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// *****************************************************
  /// Power Modeling Parameters

  /// Normalized static power input for logic
  sr_param<double> sta_power_norm;

  /// Normalized internal power input for logic (activation independent)
  sr_param<double> int_power_norm;

   /// Normalized static power input for itag ram (dp)
  sr_param<double> sta_itag_power_norm;

  /// Normalized static power input for idata ram (sp)
  sr_param<double> sta_idata_power_norm;

  /// Normalized internal power input for itag ram (dp)
  sr_param<double> int_itag_power_norm;

  /// Normalized internal power input for idata ram (sp)
  sr_param<double> int_idata_power_norm;

  /// Normalized read access energy for itag ram (dp)
  sr_param<double> dyn_itag_read_energy_norm;

  /// Normalized write access energy for itag ram (dp)
  sr_param<double> dyn_itag_write_energy_norm;

  /// Normalized read access energy for idata ram (sp)
  sr_param<double> dyn_idata_read_energy_norm;

  /// Normalized write access energy for idata ram (sp)
  sr_param<double> dyn_idata_write_energy_norm;

  /// Parameter array for power output (dcache controller)
  gs::gs_param_array power;

  /// Static power of module
  sr_param<double> sta_power;

  /// Internal dynamic power (activation independent)
  sr_param<double> int_power;

  /// Switching power
  sr_param<double> swi_power;

  /// Power frame starting time
  sr_param<sc_core::sc_time> power_frame_starting_time;

  /// Parameter array for power output of itag ram
  gs::gs_param_array itag;

  /// Dynamic energy per itag read access
  sr_param<double> dyn_tag_read_energy;

  /// Dynamic energy per itag write access
  sr_param<double> dyn_tag_write_energy;

  /// Parameter array for power output of idata ram
  gs::gs_param_array idata;

  /// Dynamic energy per idata read access
  sr_param<double> dyn_data_read_energy;

  /// Dynamic energy per idata write access
  sr_param<double> dyn_data_write_energy;

  // Constructor
  // args: sysc module name, pointer to AHB read/write methods (of parent), delay on read hit, delay on read miss (incr), number of sets, setsize in kb, linesize in b, replacement strategy
  /// @brief Constructor of data cache
  /// @param name                              SystemC module name
  /// @param mmu_cache                         Pointer to top-level class of cache subsystem (mmu_cache) for access to AHB bus interface
  /// @param tlb_adaptor                       Pointer to memory management unit
  /// @param sets                              Number of cache sets
  /// @param setsize                           Size of a cache set (in kbytes)
  /// @param linesize                          Size of a cache line (in bytes)
  /// @param repl                              Cache replacement strategy
  /// @param lram                              Local RAM configured
  /// @param lramstart                         The 8 MSBs of the local ram start address (16MB segment)
  /// @param lramsize                          Size of local ram (size in kbyte = 2^lramsize)
 ivectorcache(ModuleName name, mmu_cache_if * _mmu_cache,
              mem_if * _tlb_adaptor, unsigned int mmu_en,
              unsigned int sets, unsigned int setsize,
              unsigned int setlock, unsigned int linesize,
              unsigned int repl, unsigned int lram,
              unsigned int lramstart, unsigned int lramsize,
              bool pow_mon) :
  vectorcache(name, _mmu_cache, _tlb_adaptor,
              mmu_en,
              1, // burst fetch allowed
	      0, // new_line_fetch mode forbidden (only for dcaches)
              sets, setsize, setlock, linesize,
              repl, lram, lramstart, lramsize, pow_mon),
    sta_power_norm("power.mmu_cache.icache.sta_power_norm", 1.10e+8, true), // norm. static power logic (controller)
    int_power_norm("power.mmu_cache.icache.int_power_norm", 1.381e-8, true), // norm. internal power logic (controller)
    sta_itag_power_norm("power.mmu_cache.icache.itag.sta_power_norm", 1269.53125, true), // norm. static power itag ram
    sta_idata_power_norm("power.mmu_cache.icache.idata.sta_power_norm", 1269.53125, true), // norm. static power idata ram
    int_itag_power_norm("power.mmu_cache.icache.itag.int_power_norm", 1.61011e-12, true), // norm. internal power itag ram
    int_idata_power_norm("power.mmu_cache.icache.idata.int_power_norm", 1.61011e-12, true), // norm. internal power idata ram
    dyn_itag_read_energy_norm("power.mmu_cache.icache.itag.dyn_read_energy_norm", 7.57408e-13, true), // norm. read energy itag
    dyn_itag_write_energy_norm("power.mmu_cache.icache.itag.dyn_write_energy_norm", 7.57408e-13, true), // norm. write energy itag
    dyn_idata_read_energy_norm("power.mmu_cache.icache.idata.dyn_read_energy_norm", 7.57408e-13, true), // norm. read energy idata
    dyn_idata_write_energy_norm("power.mmu_cache.icache.idata.dyn_write_energy_norm", 7.57408e-13, true), // norm. write enegy idata
    power("power"), // parameter array for controller
    sta_power("sta_power", 0.0, power), // static power
    int_power("int_power", 0.0, power), // internal power
    swi_power("swi_power", 0.0, power), // switching power
    power_frame_starting_time("power_frame_starting_time", SC_ZERO_TIME, power),
    itag("itag", power), // parameter array for itag ram (sub-array of power)
    dyn_tag_read_energy("dyn_read_energy", 0.0, itag), // read energy itag ram
    dyn_tag_write_energy("dyn_write_energy", 0.0, itag), // write energy itag ram
    idata("idata", power), // parameter array for idata ram (sub-array of power)
    dyn_data_read_energy("dyn_read_energy", 0.0, idata), // read energy idata ram
    dyn_data_write_energy("dyn_write_energy", 0.0, idata) // write energy idata ram

      {
        // Register power callback functions
        if (pow_mon) {

          GC_REGISTER_TYPED_PARAM_CALLBACK(&sta_power, gs::cnf::pre_read, ivectorcache, sta_power_cb);
          GC_REGISTER_TYPED_PARAM_CALLBACK(&int_power, gs::cnf::pre_read, ivectorcache, int_power_cb);
          GC_REGISTER_TYPED_PARAM_CALLBACK(&swi_power, gs::cnf::pre_read, ivectorcache, swi_power_cb);

        }
      }

  ~ivectorcache() {

    GC_UNREGISTER_CALLBACKS();

  }

};

#endif // __IVECTORCACHE_H__

/// @}
