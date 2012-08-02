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
// Title:      dvectorcache.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Class definition of a data
//             cache. The cache can be configured direct mapped or
//             set associative. Set-size, line-size and replacement
//             strategy can be defined through constructor arguments.
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

#ifndef __DVECTORCACHE_H__
#define __DVECTORCACHE_H__

#include "vectorcache.h"
#include "defines.h"

#include "tlm.h"

// Implementation of data cache memory and controller
// --------------------------------------------------
/// @brief Data cache implementation for TrapGen LEON3 simulator
class dvectorcache : public vectorcache {

 public:

  // implement ccr check
  unsigned int check_mode();
  
  /// *****************************************************
  /// Power Modeling Parameters
  
  /// Normalized static power input for logic
  gs::gs_param<double> sta_power_norm;

  /// Normalized dynamic power input for logic (activation independent)
  gs::gs_param<double> dyn_power_norm;

   /// Normalized static power input for dtag ram (dp)
  gs::gs_param<double> sta_dtag_power_norm;

  /// Normalized static power input for ddata ram (sp)
  gs::gs_param<double> sta_ddata_power_norm;

  /// Normalized dynamic power input for dtag ram (dp)
  gs::gs_param<double> dyn_dtag_power_norm;

  /// Normalized dynamic power input for ddata ram (sp)
  gs::gs_param<double> dyn_ddata_power_norm;

  /// Normalized read access energy for dtag ram (dp)
  gs::gs_param<double> dyn_dtag_read_energy_norm;

  /// Normalized write access energy for dtag ram (dp)
  gs::gs_param<double> dyn_dtag_write_energy_norm;

  /// Normalized read access energy for ddata ram (sp)
  gs::gs_param<double> dyn_ddata_read_energy_norm;

  /// Normalized write access energy for ddata ram (sp)
  gs::gs_param<double> dyn_ddata_write_energy_norm;

  /// Parameter array for power output (dcache controller)
  gs::gs_param_array power;

  /// Static power of module
  gs::gs_param<double> sta_power;

  /// Dynamic power of module (activation independent)
  gs::gs_param<double> dyn_power;

  /// Parameter array for power output of dtag ram
  gs::gs_param_array dtag;

  /// Static power of dtag RAM
  gs::gs_param<double> sta_dtag_power;

  /// Dynamic power of dtag RAM (activation independent)
  gs::gs_param<double> dyn_dtag_power;

  /// Dynamic energy per dtag read access
  gs::gs_param<double> dyn_dtag_read_energy;

  /// Dynamic energy per dtag write access
  gs::gs_param<double> dyn_dtag_write_energy;

  /// Number of dtag reads (monitor read & reset)
  gs::gs_param<uint64_t> dyn_dtag_reads;

  /// Number of dtag writes (monitor read & reset)
  gs::gs_param<uint64_t> dyn_dtag_writes;

  /// Parameter array for power output of ddata ram
  gs::gs_param_array ddata;

  /// Static power of ddata RAM
  gs::gs_param<double> sta_ddata_power;

  /// Dynamic power of ddata RAM
  gs::gs_param<double> dyn_ddata_power;

  /// Dynamic energy per ddata read access
  gs::gs_param<double> dyn_ddata_read_energy;

  /// Dynamic energy per ddata write access
  gs::gs_param<double> dyn_ddata_write_energy;

  /// Number of ddata reads (monitor read & reset)
  gs::gs_param<uint64_t> dyn_ddata_reads;
  
  /// Number of ddata writes (monitor read & reset)
  gs::gs_param<uint64_t> dyn_ddata_writes;
  
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
 dvectorcache(sc_core::sc_module_name name, mmu_cache_if * _mmu_cache,
              mem_if * _tlb_adaptor, unsigned int mmu_en,
              unsigned int sets, unsigned int setsize,
              unsigned int setlock, unsigned int linesize,
              unsigned int repl, unsigned int lram,
              unsigned int lramstart, unsigned int lramsize, bool pow_mon) :
  vectorcache(name, _mmu_cache,
              _tlb_adaptor,
              mmu_en,
              0, // burst fetch forbidden
              sets, setsize, setlock,
              linesize, repl, lram, lramstart, lramsize, pow_mon),
    sta_power_norm("power.mmu_cache.dcache.sta_power_norm", 0.0, true),
    dyn_power_norm("power.mmu_cache.dcache.dyn_power_norm", 0.0, true),
    sta_dtag_power_norm("power.mmu_cache.dcache.dtag.sta_power_norm", 0.0, true),
    sta_ddata_power_norm("power.mmu_cache.dcache.ddata.sta_power_norm", 0.0, true),
    dyn_dtag_power_norm("power.mmu_cache.dcache.dtag.dyn_power_norm", 0.0, true),
    dyn_ddata_power_norm("power.mmu_cache.dcache.ddata.dyn_power_norm", 0.0, true),
    power("power"),
    sta_power("sta_power", 0.0, power),
    dyn_power("dyn_power", 0.0, power),
    dtag("dtag", power),
    sta_dtag_power("sta_power", 0.0, dtag),
    dyn_dtag_power("dyn_power", 0.0, dtag),
    dyn_dtag_read_energy("dyn_read_energy", 0.0, dtag),
    dyn_dtag_write_energy("dyn_write_energy", 0.0, dtag),
    dyn_dtag_reads("dyn_reads", 0.0, dtag),
    dyn_dtag_writes("dyn_writes", 0.0, dtag),
    ddata("ddata", power),
    sta_ddata_power("sta_power", 0.0, ddata),
    dyn_ddata_power("dyn_power", 0.0, ddata),
    dyn_ddata_read_energy("dyn_read_energy", 0.0, ddata),
    dyn_ddata_write_energy("dyn_write_energy", 0.0, ddata),
    dyn_ddata_reads("dyn_reads", 0.0, ddata),
    dyn_ddata_writes("dyn_writes", 0.0, ddata)
 
      {
        
        // nothing to do

      }

};

#endif // __DVECTORCACHE_H__

