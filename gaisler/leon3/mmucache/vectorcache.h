// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file vectorcache.h
/// Class definition of a virtual cache model. The cache can be configured
/// direct mapped or set associative. Way-size, line-size and replacement
/// strategy can be defined through constructor arguments.
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef __VECTORCACHE_H__
#define __VECTORCACHE_H__

#include <vector>
#include <stdio.h>
#include <string>
#include <sstream>
#include "core/common/base.h"
#include "core/common/systemc.h"
#include "core/common/sr_param.h"
#include "core/common/scireg.h"

#include "math.h"
#include "core/common/verbose.h"
#include "gaisler/leon3/mmucache/defines.h"
#include "gaisler/leon3/mmucache/cache_if.h"
#include "gaisler/leon3/mmucache/mmu_cache_if.h"
#include "gaisler/leon3/mmucache/tlb_adaptor.h"
#include "gaisler/leon3/mmucache/mem_if.h"
#include "core/common/vendian.h"

// implementation of cache memory and controller
/// @brief virtual cache model, contain common functionality of instruction and data cache
class vectorcache : public DefaultBase, public cache_if, public scireg_ns::scireg_region_if
{

  /// --------------------------------------------------------------------------
  /// @name Interface Data Methods
  /// @{

public:
  /// Read from cache
  virtual bool mem_read(unsigned int address, unsigned int asi, unsigned char * data,
                        unsigned int len, sc_core::sc_time * t,
                        unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock);
  /// Write through cache
  virtual void mem_write(unsigned int address, unsigned int asi, unsigned char * data,
                         unsigned int len, sc_core::sc_time * t,
                         unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock);

  /// @} Interface Data Methods
  /// --------------------------------------------------------------------------
  /// @name Interface Control Methods
  /// @{

  /// Read cache configuration register (ASI 0x2)
  virtual unsigned int read_config_reg(sc_core::sc_time *t);

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
  /// Flush cache
  virtual void flush(sc_core::sc_time * t, unsigned int * debug, bool is_dbg);

  /// Snooping function (invalidates cache line(s))
  virtual void snoop_invalidate(const t_snoop& snoop, const sc_core::sc_time& delay);

  virtual scireg_ns::scireg_response scireg_get_region_type(scireg_ns::scireg_region_type& t) const {
    t = scireg_ns::SCIREG_BANK;
    return scireg_ns::SCIREG_SUCCESS;
  }

  /// Get parent SystemC modules associated with this region
  virtual scireg_ns::scireg_response scireg_get_parent_modules(std::vector<sc_core::sc_module*>& v) const {
    v.push_back(&const_cast<vectorcache&>(*this));
    return scireg_ns::SCIREG_SUCCESS;
  }

  /// Get child regions mapped into this region, by returning a mapped region object representing each mapping.
  /// The size and offset parameters can be used to constrain the range of the search
  virtual scireg_ns::scireg_response scireg_get_child_regions(
      std::vector<scireg_ns::scireg_mapped_region>& mapped_regions,
      sc_dt::uint64 size=sc_dt::uint64(-1), sc_dt::uint64 offset=0) const {
   for (std::vector<scireg_ns::scireg_mapped_region*>::const_iterator i = this->mapped_regions->begin();
       i != this->mapped_regions->end(); ++i) {
     mapped_regions.push_back(**i);
   }
    return scireg_ns::SCIREG_SUCCESS;
  }

  /// @} Interface Control Methods
  /// --------------------------------------------------------------------------
  /// @name Internal Methods
  /// @{
protected:
  /// Address parsing one-liners
  inline unsigned get_tag(unsigned address)
    {return (address >> (m_idx_bits + m_offset_bits));}
  inline unsigned get_idx(unsigned address)
    {return ((address << m_tag_bits) >> (m_tag_bits + m_offset_bits));}
  inline unsigned get_offset(unsigned address)
    {return ((address << (m_tag_bits + m_idx_bits)) >> (m_tag_bits + m_idx_bits));}
  inline unsigned get_address(unsigned tag, unsigned idx, unsigned offset)
    {return (tag << (m_idx_bits + m_offset_bits)) | (idx << m_offset_bits) | offset;}

  /// Transforms a cache-line offset into a valid mask
  inline unsigned int offset2valid(unsigned int offset, unsigned int len = 4);

  /// Returns number of the way to be refilled - depending on replacement strategy
  unsigned int replacement_selector(unsigned int idx, unsigned int mode);

  /// Updates the lru counters for every cache hit
  void lru_update(unsigned int idx, unsigned int set_select);

  /// Updates the lrr bits for every line replacement
  void lrr_update(unsigned int idx, unsigned int set_select);

  /// Reads a cache line from a given cache way.
  /// Returns an iterator to found line (more useful than pointer for looping through ways of a given index).
  /// Returns null for incorrect parameters.
  inline std::vector<t_cache_line*>::iterator lookup_line(unsigned idx, unsigned way) {return (cache_mem->begin())+(idx*(m_sets+1)+way);}

  /// Searches for a cache tag in all cache ways. Updates power information for reading tags.
  /// Returns found way if tag matches and data is valid, otherwise -1.
  int locate_line(unsigned const tag, unsigned const idx, unsigned const offset, unsigned const len,
                  sc_core::sc_time * delay);

  /// Allocates a cache line in a given way. Updates timing and power information.
  /// Returns the allocated way (should be same as parameter), otherwise -1.
  int update_line(unsigned const tag, unsigned const idx, unsigned const offset,
                  unsigned const way, unsigned const len,
                  unsigned char* const data,
                  sc_core::sc_time * delay, unsigned* debug, bool& cacheable, bool is_dbg);

  /// Allocates a cache line in either an invalid way or a way found be replacement selection.
  /// Updates timing and power information.
  /// Returns the allocated way, otherwise -1.
  int allocate_line(unsigned const tag, unsigned const idx, unsigned const offset,
                    unsigned const len,
                    unsigned char* const data,
                    sc_core::sc_time * delay, unsigned* debug, bool& cacheable, bool is_dbg);

  /// @} Internal Methods
  /// --------------------------------------------------------------------------
  /// @name Diagnostic Methods
  /// @{

public:
  /// Returns the mode bits of the cache
  virtual unsigned int check_mode()=0;

  /// Returns type of cache implementing this interface
  virtual t_cache_type get_cache_type()=0;

  /// Display of cache lines for debug
  virtual void dbg_out(unsigned int line);

  /// Helper functions for definition of clock cycle
  void clkcng(sc_core::sc_time &clk);

  /// @} Diagnostic Methods
  /// --------------------------------------------------------------------------
  /// @name Constructors and Destructors
  /// @{

protected:
  // constructor
  // args: sysc module name, pointer to AHB read/write methods (of parent), delay on read hit, delay on read miss (incr), number of ways, waysize in kb, linesize in b, replacement strategy
  /// @brief Constructor of data cache
  vectorcache(ModuleName name,              ///> SystemC module name
              mmu_cache_if* _mmu_cache,     ///> Pointer to top-level class of cache subsystem (mmu_cache) for access to AHB bus interface
              mem_if* _tlb_adaptor,         ///> Pointer to memory management unit
              unsigned int mmu_en,          ///> Memory management unit enabled
              unsigned int burst_en,        ///> Allows the cache to be switched in burst fetch mode (by CCR setting)
              bool new_linefetch_en,        ///>
              unsigned int sets,            ///> Number of cache ways
              unsigned int setsize,         ///> Size of a cache way (in kbytes)
              unsigned int setlock,         ///>
              unsigned int linesize,        ///> Size of a cache line (in words)
              unsigned int repl,            ///> Cache replacement strategy
              unsigned int lram,            ///> Local RAM configured
              unsigned int lramstart,       ///> The 8 MSBs of the local ram start address (16MB segment)
              unsigned int lramsize,        ///> Size of local ram (size in kbyte = 2^lramsize)
              bool pow_mon);                ///> Enables power monitoring

  virtual ~vectorcache();

public:
  void end_of_simulation();

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

protected:
  /// Pointer to the class with the amba interface
  mmu_cache_if * m_mmu_cache;

  /// Pointer to the class with the mmu interface
  mem_if * m_tlb_adaptor;

  /// Cache configuration register (ASI 0x2):
  /// [3]     MMU present - This bit is set to '1' if an MMU is present
  /// [11:4]  Local RAM start address - The 8 MSBs of the local ram start address
  /// [15:12] Local RAM size (LRSZ) - Size in Kbytes of the local scratch pad ram
  /// (local RAM size = 2^LRSZ)
  /// [18:16] Line size (LSIZE) - The size (words) of each cache line
  /// (line size = 2^LSIZE)
  /// [19]    Local RAM (LR) - Set if local scratch pad ram is implemented.
  /// [23:20] Way size (WSIZE) - Size in Kbytes of each cache way.
  /// (way size = 2^WSIZE)
  /// [26:24] Cache associativity (WAYS) - Number of ways in the cache
  /// (000 - direct mapped, 001 - 2-way associative, 010 - 3-way associative, 011 - 4-way associative)
  /// [27]    Cache snooping (SN) - Set if snooping is implemented
  /// [29-28] Cache replacement policiy (REPL)
  /// (00 - no repl. (direct mapped), 01 - LRU, 10 - LRR, 11 - RANDOM)
  /// [31]    Cache locking (CL) - Set if cache locking is implemented
  unsigned int CACHE_CONFIG_REG;

  /// The actual cache memory
  std::vector<t_cache_line*> *cache_mem;

  /// The children scireg_reagion_ifs
  std::vector<scireg_ns::scireg_mapped_region*> *mapped_regions;

  /// Indicates whether the cache can be put in burst mode or not
  unsigned int m_burst_en;
  /// Enables linefetch mode for "newer" as used in "newer" versions
  /// of dcache in grlib
  bool m_new_linefetch_en;
  /// Pseudo-random pointer
  unsigned int m_pseudo_rand;

  /// @} Data
  /// --------------------------------------------------------------------------
  /// @name Parameters
  /// @{

protected:
  /// Number of cache ways (000: direct mapped, 001: 2x, 010: 3x, 011: 4x)
  unsigned int m_sets;
  /// Size of cache way in kB = 2^m_waysize
  unsigned int m_setsize;
  /// Cache line locking
  unsigned int m_setlock;
  /// Size of cache line in words = 2^m_linesize
  unsigned int m_linesize;
  /// Number of words per cache line
  unsigned int m_wordsperline;
  /// Number of bytes per cache line
  unsigned int m_bytesperline;
  /// Number of bits for addressing the line offset
  unsigned int m_offset_bits;
  /// Number of lines in the cache
  unsigned int m_number_of_vectors;
  /// Address bits used for index
  unsigned int m_idx_bits;
  /// Address bits used for tag
  unsigned int m_tag_bits;
  /// Replacement strategy
  unsigned int m_repl;

  /// MMU enabled
  unsigned int m_mmu_en;

  /// LRU counter maximum
  int m_max_lru;

  // !!! The actual localram is instantiated in class mmu_cache.
  // !!! Settings are only needed for configuration register.
  /// Local ram present
  unsigned int m_lram;
  /// Start address of localram (8 MSBs)
  unsigned int m_lramstart;
  /// Size of localram
  unsigned int m_lramsize;

  /// @} Parameters
  /// --------------------------------------------------------------------------
  /// @name Statistics
  /// @{

protected:
  /// GreenControl API container
  gs::cnf::cnf_api *m_api;

  /// Open a namespace for performance counting in the greencontrol realm
  gs::gs_param_array m_performance_counters;

  /// Counter for read hits
  gs::gs_param<unsigned long long *> rhits;

  /// Counter for read misses
  sr_param<uint64_t> rmisses;

  /// Counter for write hits
  gs::gs_param<unsigned long long *> whits;

  /// Counter for write misses
  sr_param<uint64_t> wmisses;

  /// Counter for bypass operations
  sr_param<uint64_t> bypassops;

  /// Enable power monitoring
  bool m_pow_mon;

  /// @} Statistics
  /// --------------------------------------------------------------------------
  /// @name Timing and Power Modeling
  /// @{

protected:
  /// Number of tag ram reads (monitor read & reset)
  sr_param<uint64_t> dyn_tag_reads;

  /// Number of tag ram writes (monitor read & reset)
  sr_param<uint64_t> dyn_tag_writes;

  /// Number of data ram reads (monitor read & reset)
  sr_param<uint64_t> dyn_data_reads;

  /// Number of data ram writes (monitor read & reset)
  sr_param<uint64_t> dyn_data_writes;

  /// Timing parameters
  sc_core::sc_time m_hit_read_response_delay;
  sc_core::sc_time m_miss_read_response_delay;
  sc_core::sc_time m_write_response_delay;

  /// Clock cycle time
  sc_core::sc_time clockcycle;

  /// @} Timing and Power Modeling
  /// --------------------------------------------------------------------------
};

#endif // __VECTORCACHE_H__
/// @}
