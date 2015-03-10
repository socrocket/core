// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file vectorcache.h
/// Class definition of a virtual cache model. The cache can be configured
/// direct mapped or set associative. Set-size, line-size and replacement
/// strategy can be defined through constructor arguments.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef __VECTORCACHE_H__
#define __VECTORCACHE_H__

#include <vector>
#include "core/common/base.h"
#include "core/common/systemc.h"
#include "core/common/gs_config.h"

#include "math.h"
#include "core/common/verbose.h"
#include "core/models/leon3/mmucache/defines.h"
#include "core/models/leon3/mmucache/cache_if.h"
#include "core/models/leon3/mmucache/mmu_cache_if.h"
#include "core/models/leon3/mmucache/tlb_adaptor.h"
#include "core/models/leon3/mmucache/mem_if.h"
#include "core/common/vendian.h"

// implementation of cache memory and controller
/// @brief virtual cache model, contain common functionality of instruction and data cache
class vectorcache : public DefaultBase, public cache_if {

 public:

  // Memory interface functions (mem_if)
  // -----------------------------------------------------------
  /// Read from cache
  virtual bool mem_read(unsigned int address, unsigned int asi, unsigned char * data,
                        unsigned int len, sc_core::sc_time * t,
                        unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock);
  /// Write through cache
  virtual void mem_write(unsigned int address, unsigned int asi, unsigned char * data,
                         unsigned int len, sc_core::sc_time * t,
                         unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock);
  /// Flush cache
  virtual void flush(sc_core::sc_time * t, unsigned int * debug, bool is_dbg);

  /// Read data cache tags (ASI 0xe)
  virtual void read_cache_tag(unsigned int address, unsigned int * data,
                              sc_core::sc_time *t);
  /// Write data cache tags (ASI 0xe)
  virtual void write_cache_tag(unsigned int address, unsigned int * data,
                               sc_core::sc_time *t);
  /// Read data cache entries/data (ASI 0xf)
  virtual void read_cache_entry(unsigned int address,
                                unsigned int * data, sc_core::sc_time *t);
  /// Write data cache entries/data (ASI 0xf)
  virtual void write_cache_entry(unsigned int address, unsigned int * data,
                                 sc_core::sc_time *t);
  /// Read cache configuration register (ASI 0x2)
  virtual unsigned int read_config_reg(sc_core::sc_time *t);

  /// Returns the mode bits of the cache
  virtual unsigned int check_mode()=0;

  /// Returns type of cache implementing this interface
  virtual t_cache_type get_cache_type()=0;

  /// Snooping function (invalidates cache line(s))
  virtual void snoop_invalidate(const t_snoop& snoop, const sc_core::sc_time& delay);

  // Debug and helper functions
  // --------------------------
  /// Display of cache lines for debug
  virtual void dbg_out(unsigned int line);

  /// Helper functions for definition of clock cycle
  void clkcng(sc_core::sc_time &clk);

  /// Transforms a cache-line offset into a valid mask
  inline unsigned int offset2valid(unsigned int offset, unsigned int len = 4);

  void end_of_simulation();

 protected:

  // internal behavioral functions
  // -----------------------------
  /// reads a cache line from a cache set
  inline t_cache_line * lookup(unsigned int set, unsigned int idx);
  /// returns number of the set to be refilled - depending on replacement strategy
  unsigned int replacement_selector(unsigned int);
  /// updates the lru counters for every cache hit
  void lru_update(unsigned int set_select);
  /// updates the lrr bits for every line replacement
  void lrr_update(unsigned int set_select);

 protected:
  // constructor
  // args: sysc module name, pointer to AHB read/write methods (of parent), delay on read hit, delay on read miss (incr), number of sets, setsize in kb, linesize in b, replacement strategy
  /// @brief Constructor of data cache
  /// @param name                              SystemC module name
  /// @param _mmu_cache                        Pointer to top-level class of cache subsystem (mmu_cache) for access to AHB bus interface
  /// @param _tlb_adaptor                      Pointer to memory management unit
  /// @param burst_en                          Allows the cache to be switched in burst fetch mode (by CCR setting)
  /// @param sets                              Number of cache sets
  /// @param setsize                           Size of a cache set (in kbytes)
  /// @param linesize                          Size of a cache line (in bytes)
  /// @param repl                              Cache replacement strategy
  /// @param lram                              Local RAM configured
  /// @param lramstart                         The 8 MSBs of the local ram start address (16MB segment)
  /// @param lramsize                          Size of local ram (size in kbyte = 2^lramsize)
  /// @param pow_mon                           Enables power monitoring
  vectorcache(ModuleName name, mmu_cache_if * _mmu_cache,
              mem_if * _tlb_adaptor, unsigned int mmu_en,
              unsigned int burst_en,
	      bool new_linefetch_en,
              unsigned int sets,
              unsigned int setsize, unsigned int setlock,
              unsigned int linesize, unsigned int repl,
              unsigned int lram, unsigned int lramstart,
              unsigned int lramsize,
              bool pow_mon);

  virtual ~vectorcache();

 protected:

  // data members
  // ------------
  /// pointer to the class with the amba interface
  mmu_cache_if * m_mmu_cache;

  /// pointer to the class with the mmu interface
  mem_if * m_tlb_adaptor;

  /// cache configuration register (ASI 0x2):
  /// [3]     MMU present - This bit is set to '1' if an MMU is present
  /// [11:4]  Local RAM start address - The 8 MSBs of the local ram start address
  /// [15:12] Local RAM size (LRSZ) - Size in Kbytes of the local scratch pad ram
  /// (local RAM size = 2^LRSZ)
  /// [18:16] Line size (LSIZE) - The size (words) of each cache line
  /// (line size = 2^LSIZE)
  /// [19]    Local RAM (LR) - Set if local scratch pad ram is implemented.
  /// [23:20] Set size (SSIZE) - Size in Kbytes of each cache set.
  /// (set size = 2^SSIZE)
  /// [26:24] Cache associativity (SETS) - Number of sets in the cache
  /// (000 - direct mapped, 001 - 2-way associative, 010 - 3-way associative, 011 - 4-way associative)
  /// [27]    Cache snooping (SN) - Set if snooping is implemented
  /// [29-28] Cache replacement policiy (REPL)
  /// (00 - no repl. (direct mapped), 01 - LRU, 10 - LRR, 11 - RANDOM)
  /// [31]    Cache locking (CL) - Set if cache locking is implemented
  unsigned int CACHE_CONFIG_REG;

  /// the actual cache memory
  std::vector<std::vector<t_cache_line>*> cache_mem;

  // helpers for cache handling
  t_cache_line m_default_cacheline;
  std::vector<t_cache_line*> m_current_cacheline;
  t_cache_line* m_snoop_cacheline;

  /// indicates whether the cache can be put in burst mode or not
  unsigned int m_burst_en;
  /// enables linefetch mode for "newer" as used in "newer" versions
  /// of dcache in grlib
  bool m_new_linefetch_en;
  /// pseudo random pointer
  unsigned int m_pseudo_rand;

  // cache parameters
  // ----------------
  /// number of cache sets (000 - direct mapped, 001 - 2x, 010 - 3x, 011 -4x
  unsigned int m_sets;
  /// indicates size of cacheset in kb = 2^m_setsize
  unsigned int m_setsize;
  /// cache line locking
  unsigned int m_setlock;
  /// indicates size of cacheline in words = 2^m_linesize
  unsigned int m_linesize;
  /// number of words per cacheline
  unsigned int m_wordsperline;
  /// number of bytes per cacheline
  unsigned int m_bytesperline;
  /// number of bits for addressing the line offset
  unsigned int m_offset_bits;
  /// number of lines in the cache
  unsigned int m_number_of_vectors;
  /// address-bits used for index
  unsigned int m_idx_bits;
  /// address-bits used for tag
  unsigned int m_tagwidth;
  /// replacement strategy
  unsigned int m_repl;

  // other parameters
  // ----------------
  /// mmu enabled
  unsigned int m_mmu_en;

  // lru counter maximum
  int m_max_lru;

  // !!! The actual localram is instantiated in class mmu_cache.
  // !!! Settings are only needed for configuration register.

  /// local ram present
  unsigned int m_lram;
  /// start address of localram (8 MSBs)
  unsigned int m_lramstart;
  /// size of localram
  unsigned int m_lramsize;

  // Statistics
  // ----------
  /// GreenControl API container
  gs::cnf::cnf_api *m_api;

  /// Open a namespace for performance counting in the greencontrol realm
  gs::gs_param_array m_performance_counters;

  /// Counter for read hits
  gs::gs_param<unsigned long long *> rhits;

  /// Counter for read misses
  gs::gs_config<uint64_t> rmisses;

  /// Counter for write hits
  gs::gs_param<unsigned long long *> whits;

  /// Counter for write misses
  gs::gs_config<uint64_t> wmisses;

  /// Counter for bypass operations
  gs::gs_config<uint64_t> bypassops;

  /// enable power monitoring
  bool m_pow_mon;

  /// *****************************************************
  /// Power Modeling Parameters

  /// Number of tag ram reads (monitor read & reset)
  gs::gs_config<uint64_t> dyn_tag_reads;

  /// Number of tag ram writes (monitor read & reset)
  gs::gs_config<uint64_t> dyn_tag_writes;

  /// Number of data ram reads (monitor read & reset)
  gs::gs_config<uint64_t> dyn_data_reads;

  /// Number of data ram writes (monitor read & reset)
  gs::gs_config<uint64_t> dyn_data_writes;

  // delay parameters
  // ----------------
  sc_core::sc_time m_hit_read_response_delay;
  sc_core::sc_time m_miss_read_response_delay;
  sc_core::sc_time m_write_response_delay;

  /// Clock cycle time
  sc_core::sc_time clockcycle;

};

#endif // __VECTORCACHE_H__
/// @}
