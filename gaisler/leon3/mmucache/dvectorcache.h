// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file dvectorcache.h
/// Class definition of a data cache. The cache can be configured direct mapped
/// or set associative. Set-size, line-size and replacement strategy can be
/// defined through constructor arguments.
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef __DVECTORCACHE_H__
#define __DVECTORCACHE_H__

#include "gaisler/leon3/mmucache/vectorcache.h"
#include "gaisler/leon3/mmucache/defines.h"

#include "core/common/base.h"
#include "core/common/systemc.h"

// Implementation of data cache memory and controller
// --------------------------------------------------
/// @brief Data cache implementation for TrapGen LEON3 simulator
class dvectorcache : public vectorcache {

 public:

  GC_HAS_CALLBACKS();

  /// Implement ccr check
  unsigned int check_mode();

  /// Implement cache type function
  t_cache_type get_cache_type();

  /// Automatically called at start of simulation
  void start_of_simulation();

  /// Calculate power/energy values from normalized input data
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

   /// Normalized static power input for dtag ram (dp)
  sr_param<double> sta_dtag_power_norm;

  /// Normalized static power input for ddata ram (sp)
  sr_param<double> sta_ddata_power_norm;

  /// Normalized internal power input for dtag ram (dp)
  sr_param<double> int_dtag_power_norm;

  /// Normalized internal power input for ddata ram (sp)
  sr_param<double> int_ddata_power_norm;

  /// Normalized read access energy for dtag ram (dp)
  sr_param<double> dyn_dtag_read_energy_norm;

  /// Normalized write access energy for dtag ram (dp)
  sr_param<double> dyn_dtag_write_energy_norm;

  /// Normalized read access energy for ddata ram (sp)
  sr_param<double> dyn_ddata_read_energy_norm;

  /// Normalized write access energy for ddata ram (sp)
  sr_param<double> dyn_ddata_write_energy_norm;

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

  /// Parameter array for power output of dtag ram
  gs::gs_param_array dtag;

  /// Dynamic energy per dtag read access
  sr_param<double> dyn_tag_read_energy;

  /// Dynamic energy per dtag write access
  sr_param<double> dyn_tag_write_energy;

  /// Parameter array for power output of ddata ram
  gs::gs_param_array ddata;

  /// Dynamic energy per ddata read access
  sr_param<double> dyn_data_read_energy;

  /// Dynamic energy per ddata write access
  sr_param<double> dyn_data_write_energy;

  // Constructor
  // args: sysc module name, pointer to AHB read/write methods (of parent), delay on read hit, delay on read miss (incr), number of sets, setsize in kb, linesize in b, replacement strategy
  /// @brief Constructor of data cache
  /// @param name                              SystemC module name
  /// @param mmu_cache                         Pointer to top-level class of cache subsystem (mmu_cache) for access to AHB bus interface
  /// @param tlb_adaptor                       Pointer to memory management unit
  /// @param hit_read_response_delay           Delay for a cache read hit
  /// @param miss_read_response_delay          Delay for a cache read miss
  /// @param write_response_delay              Delay for a cache write access (hit/miss)
  /// @param sets                              Number of cache sets
  /// @param setsize                           Size of a cache set (in kbytes)
  /// @param setlock                           Enable cache line locking
  /// @param linesize                          Size of a cache line (in bytes)
  /// @param repl                              Cache replacement strategy
  /// @param lram                              Local RAM configured
  /// @param lramstart                         The 8 MSBs of the local ram start address (16MB segment)
  /// @param lramsize                          Size of local ram (size in kbyte = 2^lramsize)
 dvectorcache(ModuleName name, mmu_cache_if * _mmu_cache,
              mem_if * _tlb_adaptor, unsigned int mmu_en,
              unsigned int sets, unsigned int setsize,
              unsigned int setlock, unsigned int linesize,
              unsigned int repl, unsigned int lram,
              unsigned int lramstart, unsigned int lramsize, bool pow_mon) :
  vectorcache(name, _mmu_cache,
              _tlb_adaptor,
              mmu_en,
              0, // burst fetch forbidden
	      1, // enable new linefetch mode for dcaches
              sets, setsize, setlock,
              linesize, repl, lram, lramstart, lramsize, pow_mon),
    sta_power_norm("power.mmu_cache.dcache.sta_power_norm", 1.35e+8, true), // norm. static power logic (controller)
    int_power_norm("power.mmu_cache.dcache.int_power_norm", 1.264e-8, true), // norm. internal power logic (controller)
    sta_dtag_power_norm("power.mmu_cache.dcache.dtag.sta_power_norm", 1726.5625, true), // norm. static power dtag ram
    sta_ddata_power_norm("power.mmu_cache.dcache.ddata.sta_power_norm", 1269.53125, true), // norm. static power ddata ram
    int_dtag_power_norm("power.mmu_cache.dcache.dtag.int_power_norm", 1.69544e-12, true), // norm internal power dtag ram
    int_ddata_power_norm("power.mmu_cache.dcache.ddata.int_power_norm", 1.61011e-12, true), // norm internal power ddata ram
    dyn_dtag_read_energy_norm("power.mmu_cache.dcache.dtag.dyn_read_energy_norm", 1.01493e-12, true), // norm. read energy dtag
    dyn_dtag_write_energy_norm("power.mmu_cache.dcache.dtag.dyn_write_energy_norm", 1.01493e-12, true), // norm. write energy dtag
    dyn_ddata_read_energy_norm("power.mmu_cache.dcache.ddata.dyn_read_energy_norm", 7.57408e-13, true), // norm. read energy ddata
    dyn_ddata_write_energy_norm("power.mmu_cache.dcache.ddata.dyn_write_energy_norm", 7.57408e-13, true), // norm. write enegy ddata
    power("power"), // parameter array for controller
    sta_power("sta_power", 0.0, power), // static power
    int_power("int_power", 0.0, power), // internal power
    swi_power("swi_power", 0.0, power), // switching power
    power_frame_starting_time("power_fram_starting_time", SC_ZERO_TIME, power),
    dtag("dtag", power), // parameter array for dtag ram (sub-array of power)
    dyn_tag_read_energy("dyn_read_energy", 0.0, dtag), // read energy dtag ram
    dyn_tag_write_energy("dyn_write_energy", 0.0, dtag), // write energy dtag ram
    ddata("ddata", power), // parameter array for ddata ram (sub-array of power)
    dyn_data_read_energy("dyn_read_energy", 0.0, ddata), // read energy of ddata ram
    dyn_data_write_energy("dyn_write_energy", 0.0, ddata) // write energy of ddata ram

      {
        // Register power callback functions
        if (pow_mon) {

          GC_REGISTER_TYPED_PARAM_CALLBACK(&sta_power, gs::cnf::pre_read, dvectorcache, sta_power_cb);
          GC_REGISTER_TYPED_PARAM_CALLBACK(&int_power, gs::cnf::pre_read, dvectorcache, int_power_cb);
          GC_REGISTER_TYPED_PARAM_CALLBACK(&swi_power, gs::cnf::pre_read, dvectorcache, swi_power_cb);

        }
      }

  ~dvectorcache() {

    GC_UNREGISTER_CALLBACKS();

  }
};

#endif // __DVECTORCACHE_H__

/// @}
