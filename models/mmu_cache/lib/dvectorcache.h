// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       dvectorcache.h - Class definition of a data             *
// *             cache. The cache can be configured direct mapped or     *
// *             set associative. Set-size, line-size and replacement    *
// *             strategy can be defined through constructor arguments.  *
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision$                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************

#ifndef __DVECTORCACHE_H__
#define __DVECTORCACHE_H__

#include "tlm.h"

#include <vector>
#include <iostream>
#include "math.h"

#include "defines.h"
#include "mmu_cache.h"
#include "vectorcache.h"
#include "mmu.h"
#include "mem_if.h"

// implementation of cache memory and controller
// ---------------------------------------------
/// @brief Data cache implementation for TrapGen LEON3 simulator
class dvectorcache : public vectorcache {

 public:

  // external interface functions (to be made public by childs):
  // -----------------------------------------------------------
  /// read from cache
  using vectorcache::read;
  /// write through cache
  using vectorcache::write; 
  /// flush cache
  using vectorcache::flush;
  /// read data cache tags (ASI 0xe)
  using vectorcache::read_cache_tag;
  /// write data cache tags (ASI 0xe)
  using vectorcache::write_cache_tag;
  /// read data cache entries/data (ASI 0xf)
  using vectorcache::read_cache_entry;
  /// write data cache entries/data (ASI 0xf)
  using vectorcache::write_cache_entry;
  /// read cache configuration register (ASI 0x2)
  using vectorcache::read_config_reg;
  /// debug output
  using vectorcache::dbg_out;

 public:

  // constructor
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
  /// @param linesize                          Size of a cache line (in bytes)
  /// @param repl                              Cache replacement strategy
  /// @param lram                              Local RAM configured
  /// @param lramstart                         The 8 MSBs of the local ram start address (16MB segment)
  /// @param lramsize                          Size of local ram (size in kbyte = 2^lramsize)
  dvectorcache(sc_core::sc_module_name name, 
 	       mmu_cache_if * _mmu_cache,
	       mem_if * _tlb_adaptor,
	       int mmu_en,
	       sc_core::sc_time dcache_hit_read_response_delay, 
	       sc_core::sc_time dcache_miss_read_response_delay, 
	       sc_core::sc_time dcache_write_response_delay,
	       int sets, 
	       int setsize, 
	       int linesize, 
	       int repl,
	       unsigned int lram,
	       unsigned int lramstart,
	       unsigned int lramsize) : vectorcache(name, 
						    _mmu_cache, 
						    _tlb_adaptor, 
						    mmu_en, 
						    dcache_hit_read_response_delay,
						    dcache_miss_read_response_delay,
						    sc_core::sc_time(0,sc_core::SC_NS),
						    sets,
						    setsize,
						    linesize,
						    repl,
						    lram,
						    lramstart,
						    lramsize) {}


 public:

  virtual unsigned int check_mode() {

    return(m_mmu_cache->read_ccr() & 0x3);

  }

};

#endif // __DVECTORCACHE_H__
  

  
