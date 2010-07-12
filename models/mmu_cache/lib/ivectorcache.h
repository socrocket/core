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

#include <vector>
#include <iostream>
#include "math.h"

#include "defines.h"
#include "mmu_cache_if.h"
#include "mmu_if.h"

// implementation of cache memory and controller
// ---------------------------------------------
class ivectorcache : public sc_core::sc_module {

 public:

  // external interface functions:
  // ----------------------------
  // call to read from cache
  void read(unsigned int address, unsigned int * data, sc_core::sc_time * t);
  // call to flush cache
  void flush(sc_core::sc_time * t);

  // internal behavioral functions
  // -----------------------------
  // reads a cache line from a cache set
  t_cache_line * lookup(unsigned int set, unsigned int idx);
  // returns number of set to be refilled - depending on replacement strategy
  unsigned int replacement_selector(unsigned int);

  // constructor
  // args: sysc module name, pointer to AHB read/write methods (of parent), delay on read hit, delay on read miss (incr), number of sets, setsize in kb, linesize in b, replacement strategy  
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
  // display data from dedicated cache set
  void dbg_read(unsigned int set, unsigned int start_idx, unsigned int number_of_entries); 

 public:

  // data members
  // ------------
  // the class with the amba interface
  mmu_cache_if * m_parent;

  // the class with the mmu interface
  mmu_if * m_mmu;

  // the actual cache memory
  std::vector<std::vector<t_cache_line>*> cache_mem;

  // helper for cache handling
  t_cache_line m_default_cacheline;
  std::vector<t_cache_line*> m_current_cacheline;
  
  // cache parameters
  // ----------------
  // number of cache sets
  unsigned int m_sets;
  // size in kb
  unsigned int m_setsize;
  // number of bytes per line
  unsigned int m_linesize;
  // number of bits for addressing the line offset
  unsigned int m_offset_bits;
  // number of lines in the cache
  unsigned int m_number_of_vectors;
  // address-bits used for index
  unsigned int m_idx_bits;
  // address-bits used for index
  unsigned int m_tagwidth;
  // replacement strategy
  unsigned int m_repl;
  
  // other parameters
  // ----------------
  // mmu enabled ?
  unsigned int m_mmu_en;

  // delay parameters
  // ----------------
  sc_core::sc_time m_icache_hit_read_response_delay;
  sc_core::sc_time m_icache_miss_read_response_delay;

};

#endif // __IVECTORCACHE_H__
  

  
