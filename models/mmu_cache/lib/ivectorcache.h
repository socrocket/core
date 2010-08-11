/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       ivectorcache.h - Class definition of an instruction     */
/*             cache. The cache can be configured direct mapped or     */
/*             set associative. Set-size, line-size and replacement    */
/*             strategy can be defined through constructor arguments.  */
/*                                                                     */
/* Modified on $Date$                                                  */
/*          at $Revision$                                              */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#ifndef __IVECTORCACHE_H__
#define __IVECTORCACHE_H__

#include "tlm.h"

#include <vector>
#include <iostream>
#include "math.h"

#include "defines.h"
#include "mmu_cache_if.h"
#include "mmu_if.h"

#include "icio_payload_extension.h"

// implementation of cache memory and controller
// ---------------------------------------------
/// @brief Instruction cache implementation for TrapGen LEON3 simulator 
class ivectorcache : public sc_core::sc_module {

 public:

  // external interface functions:
  // ----------------------------
  /// read from cache
  void read(unsigned int address, unsigned int * data, sc_core::sc_time * t, unsigned int * debug);
  /// flush cache
  void flush(sc_core::sc_time * t, unsigned int * debug);
  /// read instruction cache tags (ASI 0xc)
  void read_cache_tag(unsigned int address, unsigned int * data, sc_core::sc_time *t);
  /// write instruction cache tags (ASI 0xc)
  void write_cache_tag(unsigned int address, unsigned int * data, sc_core::sc_time *t);
  /// read instruction cache entries/data (ASI 0xd)
  void read_cache_entry(unsigned int address, unsigned int * data, sc_core::sc_time *t);
  /// write instruction cache entries/data (ASI 0xd)
  void write_cache_entry(unsigned int address, unsigned int * data, sc_core::sc_time *t);
  /// read configuration register (read-only)
  unsigned int read_config_reg(sc_core::sc_time *t);

  // internal behavioral functions
  // -----------------------------
  /// reads a cache line from a cache set
  t_cache_line * lookup(unsigned int set, unsigned int idx);
  /// returns number of set to be refilled - depending on replacement strategy
  unsigned int replacement_selector(unsigned int);

  // constructor
  // args: sysc module name, pointer to AHB read/write methods (of parent), delay on read hit, delay on read miss (incr), number of sets, setsize in kb, linesize in b, replacement strategy

  /// @brief Constructor of instruction cache
  /// @param name                               SystemC module name
  /// @param _parent                            Pointer to top-level class of cache subsystem (mmu_cache) for access to AHB bus interface
  /// @param _mmu                               Pointer to memory management unit
  /// @param icache_hit_read_response_delay     Delay for an instruction cache hit
  /// @param icache_miss_read_response_delay    Delay for an instruction cache miss
  /// @param sets                               Number of instruction cache sets
  /// @param setsize                            Size of an instruction cache set (in kbytes)
  /// @param linesize                           Size of an instruction cache line (in bytes)
  /// @param repl                               Instruction cache replacement strategy
  ivectorcache(sc_core::sc_module_name name, 
	       mmu_cache_if &_parent,
	       mmu_if * _mmu,
	       int mmu_en,
	       sc_core::sc_time icache_hit_read_response_delay, 
	       sc_core::sc_time icache_miss_read_response_delay, 
	       int sets, 
	       int setsize,
	       int linesize,
	       int repl);

  // destructor
  ~ivectorcache();

  // debug and helper functions
  // --------------------------
  // displays cache lines for debug
  void dbg_out(unsigned int line); 

 public:

  // data members
  // ------------
  /// pointer to the class with the amba interface
  mmu_cache_if * m_parent;

  /// pointer to the class with the mmu interface
  mmu_if * m_mmu;

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

  /// helper for cache handling
  t_cache_line m_default_cacheline;
  std::vector<t_cache_line*> m_current_cacheline;
  
  // cache parameters
  // ----------------
  /// number of cache sets
  unsigned int m_sets;
  /// size in kb
  unsigned int m_setsize;
  /// number of bytes per line
  unsigned int m_linesize;
  /// number of bits for addressing the line offset
  unsigned int m_offset_bits;
  /// number of lines in the cache
  unsigned int m_number_of_vectors;
  /// address-bits used for index
  unsigned int m_idx_bits;
  /// address-bits used for index
  unsigned int m_tagwidth;
  /// replacement strategy
  unsigned int m_repl;
  
  // other parameters
  // ----------------
  /// mmu enabled
  unsigned int m_mmu_en;

  // delay parameters
  // ----------------
  sc_core::sc_time m_icache_hit_read_response_delay;
  sc_core::sc_time m_icache_miss_read_response_delay;

};

#endif // __IVECTORCACHE_H__
  

  
