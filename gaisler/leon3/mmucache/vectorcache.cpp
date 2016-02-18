// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file vectorcache.cpp
/// Class definition of a virtual cache model. The cache can be configured
/// direct mapped or set associative. Way-size, line-size and replacement
/// strategy can be defined through constructor arguments.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include "gaisler/leon3/mmucache/vectorcache.h"


/*******************************************************************************
*  Generic Cache Memory and Controller Module
*******************************************************************************/
/// @name Constructors and Destructors
/// @{
/// Constructor
// args: sysc module name, pointer to AHB read/write methods (of parent), delay on read hit, delay on read miss (incr), number of ways, waysize in kb, linesize in words, replacement strategy
vectorcache::vectorcache(ModuleName name,
                         mmu_cache_if * _mmu_cache, mem_if *_tlb_adaptor,
                         unsigned int mmu_en, unsigned int burst_en,
                         bool new_linefetch_en, unsigned int sets,
                         unsigned int setsize, unsigned int setlock,
                         unsigned int linesize, unsigned int repl,
                         unsigned int lram, unsigned int lramstart,
                         unsigned int lramsize, bool pow_mon) :
    sc_module(name),
    m_mmu_cache(_mmu_cache),
    m_tlb_adaptor(_tlb_adaptor),
    m_burst_en(burst_en),
    m_new_linefetch_en(new_linefetch_en),
    m_pseudo_rand(0),
    m_sets(sets-1),
    m_setsize((unsigned)log2((double)setsize)),
    m_setlock(setlock),
    m_linesize((unsigned)log2((double)linesize)), // linesize [word]
    m_wordsperline(linesize),
    m_bytesperline(m_wordsperline << 2),
    m_offset_bits((unsigned)log2((double)m_bytesperline)),
    m_number_of_vectors(setsize*256/linesize), // sets = lines per way; waysize[kB]/linesize[word]*[256word]/[kB]
    m_idx_bits(m_setsize + 8 - m_linesize), // set index = log2 (sets)
    m_tag_bits(32 - m_idx_bits - m_offset_bits),
    m_repl(repl),
    m_mmu_en(mmu_en),
    m_lram(lram),
    m_lramstart(lramstart),
    m_lramsize((unsigned)log2((double)lramsize)),
    m_performance_counters("performance_counters"),
    rhits("read_hits", sets, m_performance_counters),
    rmisses("read_misses", 0ull, m_performance_counters),
    whits("write_hits", sets, m_performance_counters),
    wmisses("write_misses", 0ull, m_performance_counters),
    bypassops("bypass_operations", 0ull, m_performance_counters),
    m_pow_mon(pow_mon),
    dyn_tag_reads("dyn_tag_reads", 0ull), // number of itag reads
    dyn_tag_writes("dyn_tag_writes", 0ull), // number of itag writes
    dyn_data_reads("dyn_data_reads", 0ull), // number of idata reads
    dyn_data_writes("dyn_data_writes", 0ull), // number of idata writes
    clockcycle(10, sc_core::SC_NS)

{

    m_api = gs::cnf::GCnf_Api::getApiInstance(this);

    // Cache may have 1 to 4 ways
    if ((m_sets < 0)||(m_sets>3)) {
      srError()("ways", m_sets+1)("Cache may have 1-4 ways");
      assert(0);
    }

    // Linesize may be 4 or 8 words
    if ((linesize!=4)&&(linesize!=8)) {
      srError()("linesize", linesize)("Cache line size may be 4 or 8 words");
      assert(0);
    }

    // Way size may be 1 to 256 kb
    if ((setsize < 1)||(setsize > 256)) {
      srError()("waysize", setsize)("Size of cache way must be between 1 and 225 kb");
      assert(0);
    }

    // LRR replacement may only be selected for two-way caches
    if ((m_repl==2) && (m_sets!=1)) {
      srError()("LRR replacement may only be selected for two-way associative caches");
      assert(0);
    }

    // Direct mapped cache (replacement) only allowed for one-way caches
    if ((m_sets > 0) && (m_repl == 0)) {
      srError()("Invalid replacement policy. Direct mapped only allowed for one-way caches");
      assert(0);
    }

    // Create the cache sets
    srDebug()("Creating cache memory");
    //cache_mem = new std::vector<t_cache_line>(m_number_of_vectors*sets);
    cache_mem = new std::vector<t_cache_line*>();
    mapped_regions = new std::vector<scireg_ns::scireg_mapped_region*>();
    for (uint32_t i = 0; i < m_number_of_vectors*sets; i++) {
      char *buffer = new char[11];
      snprintf(buffer, 11, "line_%d", i);

      t_cache_line *line = new t_cache_line(*this, buffer, linesize);
//      srInfo()("name", buffer)("line created");
      cache_mem->push_back(line);

      scireg_ns::scireg_mapped_region *mapped_region = new scireg_ns::scireg_mapped_region();
      mapped_region->region = line;
      mapped_region->offset = i*32;
      mapped_region->name = buffer;
      mapped_regions->push_back(mapped_region);
    }


    // Configuration report
    v::info << this->name() << " ******************************************************************************* " << v::endl;
    v::info << this->name() << " * Created cache memory with following parameters:                               " << v::endl;
    v::info << this->name() << " * ----------------------------------------------- " << v::endl;
    v::info << this->name() << " * mmu_en: " << mmu_en << v::endl;
    v::info << this->name() << " * burst_en: " << burst_en << v::endl;
    v::info << this->name() << " * ways: " << sets << v::endl;
    v::info << this->name() << " * waysize: " << setsize << v::endl;
    v::info << this->name() << " * waylock: " << m_setlock << v::endl;
    v::info << this->name() << " * linesize: " << linesize << v::endl;
    v::info << this->name() << " * repl (0-Direct Mapped, 1-LRU, 2-LRR, 3-Random): " << m_repl << v::endl;
    v::info << this->name() << " * ---------------------------------------------------------- " << v::endl;
    v::info << this->name() << " * Size of each cache way " << (unsigned)pow(2, (double)m_setsize) << " kb" << v::endl;
    v::info << this->name() << " * Bytes per line " << m_bytesperline << " (offset bits: " << m_offset_bits << ")" << v::endl;
    v::info << this->name() << " * Number of sets (cache lines per way) " << m_number_of_vectors << " (index bits: " << m_idx_bits <<" )" << v::endl;
    v::info << this->name() << " * Width of cache tag in bits " << m_tag_bits << v::endl;
    v::info << this->name() << " ******************************************************************************* "  << v::endl;

    // lru counter saturation
    m_max_lru = (1 << m_sets) - 1;

    // set up configuration register
    // =============================
    CACHE_CONFIG_REG = (m_mmu_en << 3);
    // config register contains linesize in words
    CACHE_CONFIG_REG |= ((m_lramstart & 0xff) << 4);

    // lramsize must be between 1 and 512 kbyte
    assert((m_lramsize>=0)&&(m_lramsize<=9));
    // enter lramsize
    CACHE_CONFIG_REG |= ((m_lramsize & 0xf) << 12);

    // linesize must be 4 (m_linesize = 2) or 8 words (m_linesize = 3)
    assert((m_linesize==2)||(m_linesize==3));
    // enter linesize
    CACHE_CONFIG_REG |= ((m_linesize & 0x7) << 16);
    CACHE_CONFIG_REG |= ((m_lram & 0x1) << 19);

    // waysize must be between 1 and 256 kb (m_waysize 1 - 8)
    assert((m_setsize>=0)&&(m_setsize<=8));
    // enter waysize
    CACHE_CONFIG_REG |= ((m_setsize & 0xf) << 20);

    CACHE_CONFIG_REG |= ((m_sets & 0x7) << 24);

    // REPL: 00 - direct mapped, 01 - LRU, 10 - LRR, 11 - RANDOM
    if (m_sets == 0) {
      // if direct mapped cache set 0
      m_repl = 0;
    }

    // check repl range
    assert((m_repl>=0)&&(m_repl<=3));
    // enter replacement strategy
    CACHE_CONFIG_REG |= ((m_repl & 0x3) << 28);

    // Reset statistics
    for (uint32_t i = 0; i < sets; i++) {

      rhits[i] = 0;
      whits[i] = 0;

    }

    rmisses = 0;
    wmisses = 0;
    bypassops = 0;

} // vectorcache::vectorcache()

/// Destructor
vectorcache::~vectorcache() {
  for (std::vector<t_cache_line*>::iterator it = cache_mem->begin() ; it != cache_mem->end(); ++it) {
    delete (*it);
  }
  for (std::vector<scireg_ns::scireg_mapped_region*>::iterator it = mapped_regions->begin() ; it != mapped_regions->end(); ++it) {
    delete (*it);
  }
  delete cache_mem;
  delete mapped_regions;

} // vectorcache::~vectorcache()

/// @} Constructors and Destructors
/// ****************************************************************************
/// @name Interface Data Methods
/// @{

/// mem_if::Read from cache
/** @details
*   The behavior is as follows:
*   (!bypass-mmu && (enabled || frozen))
*   ? read_cache
*     (forced-miss || !found)
*     ? read_mem
*       found? update_cache
*            : (!frozen && cacheable)? allocate_cache
*   : bypass-mmu
*     ? read_mem(phys)
*     : disabled? read_mem
*/
bool vectorcache::mem_read(unsigned int address, unsigned int asi, unsigned char *data,
                           unsigned int len, sc_core::sc_time *delay,
                           unsigned int * debug, bool is_dbg, bool &cacheable,
                           bool is_lock) {

  unsigned tag = get_tag(address);
  unsigned idx = get_idx(address);
  unsigned offset = get_offset(address);
  unsigned byt = (address & 0x3);
  int cache_hit = -1;
  bool cacheable_local;

  unsigned ahb_address = 0;
  unsigned ahb_len = 0;
  // Data for refilling a cache line of maximum size
  unsigned char ahb_data[32];

  /// --------------------------------------------------------------------------
  /// !Bypass MMU && (Enabled || Frozen): Search cache

  if (!is_dbg && (asi != 0x1c) /* not bypass MMU */
  && (check_mode() & 0x1) /* enabled (0b11) or frozen (0b01) */) {

    srAnalyse()("addr", address)("Cache READ ACCESS");

    cache_hit = locate_line(tag, idx, offset, len, delay); // if hit, returns way

    // Update power information
    // Read all cache data lines in parallel
    /// NOTE: This should possibly go to locate_line. locate_line is also used
    /// by mem_write, however. It is unclear whether a cache lookup in mem_write
    /// also (unnecessarily) reads the data lines or just the tag lines. I've
    /// assumed the latter.
    if (m_pow_mon) dyn_data_reads += m_sets + 1;

    /// !Forced miss && In cache: Read from cache
    if (cache_hit != -1 && asi > 3 /* not forced cache miss */) {
      srAnalyse()("addr", address)("Cache READ HIT");

      // Read data from cache line
      srDebug()
        ("len", len)
        ("offset", offset)
        ("byt", byt)
        ("read");
      (*lookup_line(idx, cache_hit))->entry.copy_from(data, len, offset >> 2, byt);


      // Update flags
      if (m_repl == 1) lru_update(idx, cache_hit);

      // Increment time
      // One 32-bit load/store can be served per cycle (GRLIB IP 71.3.1).
      // If len > 4B, 1 cycle per 4B is added.
      *delay += ((len - 1) >> 2) * clockcycle;

      // If len > 4B, 1 data read per 4B is added.
      if (m_pow_mon) dyn_data_reads += (len - 1) >> 2;

      // Update debug information
      rhits[cache_hit]++;
      CACHEREADHIT_SET(*debug, cache_hit);

    /// ------------------------------------------------------------------------
    /// Forced miss || !In cache: Read from memory
    } else {

      if (asi <= 3) {
        srAnalyse()("addr", address)("Cache forced ASI READ MISS");
      } else {
        srAnalyse()("addr", address)("Cache READ MISS");
      }

      // Read data from mem: Calculate transfer parameters.
      // Line fetch: Fill whole cache line.
      if (m_new_linefetch_en) {
        // m_linesize = log2(words_per_line)         = log2(bytes_per_line/4)
        //            = log2(bytes_per_line)-log2(4) = log2(bytes_per_line)-2
        // => m_linesize+2 = log2(bytes_per_line)
        ahb_address = ((address >> (m_linesize+2)) << (m_linesize+2));
        ahb_len = m_bytesperline;

      // Burst fetch: Fill cache line from the beginning of the missed word
      // until the end of the line.
      } else if (m_burst_en && (m_mmu_cache->read_ccr(true) & 0x10000)) {
        ahb_address = ((address >> 2) << 2);
        ahb_len = m_bytesperline - ((offset >> 2) << 2);

      // Word fetch: Fill only missed word (for byte, half or word reads) or
      // double-word.
      } else {
        ahb_address = ((address >> 2) << 2);
        ahb_len = (len == 8) ? 8 /* len == 64bit */ : 4 /* len <= 32bit */;
      }

      srDebug()("addr", address)("burst address", ahb_address)("burst length", ahb_len)("Cache read miss will issue memory read");

      // Read data from mem: Returns true if data is cacheable.
      cacheable_local = m_tlb_adaptor->mem_read(ahb_address, asi, ahb_data, ahb_len,
      delay, debug, is_dbg, cacheable, is_lock);

      /// In cache (&& Forced miss): Update cache
      if (cache_hit != -1) {

        srDebug()("addr", address)("Cache read miss will update cache line");

        cache_hit = update_line(get_tag(ahb_address), get_idx(ahb_address),
                                get_offset(ahb_address), cache_hit, ahb_len,
                                ahb_data, delay, debug, cacheable, is_dbg);

      } // Update cache line

      /// !In cache && !Frozen && Cacheable: Allocate cache line
      // If cache is not frozen, we can use whichever way we found according to
      // replacement strategy. If cache is frozen, it is kept in sync with main
      // memory, but no new lines are allocated on read miss.
      else if ((check_mode() & 0x2) /* enabled (0b11) */ && cacheable_local) {

        srDebug()("addr", address)("Cache read miss will allocate cache line");

        cache_hit = allocate_line(get_tag(ahb_address), get_idx(ahb_address),
                                  get_offset(ahb_address), ahb_len,
                                  ahb_data, delay, debug, cacheable, is_dbg);

      } // Allocate cache line
      /// !In cache && (Frozen || !Cacheable)
      else {
        srDebug()("addr", address)("Cache read not cacheable");
        if (!(check_mode() & 0x2) /* frozen (0b01) */) {

          // Update debug information
          FROZENMISS_SET(*debug);
        }
      } // No cache update or allocate

      // Copy data
      if (!m_new_linefetch_en) {
        // ahb_data starts at beginning of requested word
        memcpy(data, ahb_data + byt, len);
      } else {
        // ahb_data contains whole cache line
        memcpy(data, ahb_data + offset, len);
      }

      // Update debug information
      rmisses++;
      CACHEREADMISS_SET(*debug, cache_hit);

    } // Cache miss

  /// --------------------------------------------------------------------------
  /// Bypass MMU || Disabled: Read from memory

  } else {

    srAnalyse()("addr", address)("Cache READ BYPASS");

    // Increment time
    *delay += clockcycle;

    // Read data from mem
    m_tlb_adaptor->mem_read(address, asi, data, len, delay, debug, is_dbg, cacheable, is_lock);

    // Update debug information
    bypassops++;
    CACHEBYPASS_SET(*debug);

  } // Bypass MMU || Disabled

  // Always return true; cacheability only matters on bus mem_if
  return true;
} //vectorcache::mem_read()

/// ----------------------------------------------------------------------------

/// mem_if::Write cache
/** @details
*   The behavior depends on the write_policy and write_alloc chosen:
*   - A cache hit:
*     - updates the copy in the cache;
*     - and either
*       - updates the memory for write-through; or
*       - does not update the memory for write-back.
*   - A cache miss:
*     - either
*       - allocates a new cache entry for write-allocate; or
*       - does not allocate a new cache entry for non-write-allocate;
*     - and either
*       - updates the memory for write-through or non-write-allocate; or
*       - does not update the memory for write-back and write-allocate.
*   NOTE: write-through is usually combined with non-write-allocate (allocating
*   does not help since memory will be written anyway), while write-back is
*   combined with write-allocate (allocating results in hits for subsequent writes
*   to the same block).
*   NOTE: The standard LEON3 cache is write-through + non-write-allocate.
*   The behavior is as follows:
*   (!bypass-mmu && (enabled || frozen))
*   ? read_cache
*     found? update_cache
*          : (!frozen && write-allocate && cacheable)? allocate_cache; found = true
*     (!found || write-through)? write_mem
*   : bypass-mmu
*     ? write_mem(phys)
*     : disabled? write_mem
*/
void vectorcache::mem_write(unsigned int address, unsigned int asi, unsigned char * data,
                            unsigned int len, sc_core::sc_time * delay,
                            unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock) {

  unsigned tag = get_tag(address);
  unsigned idx = get_idx(address);
  unsigned offset = get_offset(address);
  int cache_hit = -1;

  /// --------------------------------------------------------------------------
  /// !Bypass MMU && (Enabled || Frozen): Search cache

  if ((asi != 0x1c) /* not bypass MMU */
  && (check_mode() & 0x1) /* enabled (0b11) or frozen (0b01) */) {

    srAnalyse()("addr", address)("Cache WRITE ACCESS");

    // Power information for reading cache tag lines is updated in locate_line()
    cache_hit = locate_line(tag, idx, offset, len, delay);

    /// In cache: Update cache
    if (cache_hit != -1) {
      srAnalyse()("addr", address)("Cache WRITE HIT");

      srDebug()("addr", address)("Cache write hit will update cache line");

      cache_hit = update_line(tag, idx, offset, cache_hit, len,
                              data, delay, debug, cacheable, is_dbg);

      // Update debug information
      whits[cache_hit]++;
      CACHEWRITEHIT_SET(*debug, cache_hit);

    } // Cache hit

    /// ------------------------------------------------------------------------
    /// !In cache

    else {

      srAnalyse()("addr", address)("Cache WRITE MISS");

      // Update debug information
      wmisses++;
      CACHEWRITEMISS_SET(*debug);

    } // Cache miss

      srDebug()("addr", address)("Cache will issue memory write");

      /// TODO: Implement write buffer
      // Write data to mem
      m_tlb_adaptor->mem_write(address, asi, data, len, delay, debug, is_dbg, cacheable, is_lock);

  /// --------------------------------------------------------------------------
  /// Bypass MMU || Disabled

  } else {

    srAnalyse()("addr", address)("Cache WRITE BYPASS");

    // Increment time
    *delay += clockcycle;

    // Write data to mem
    m_tlb_adaptor->mem_write(address, asi, data, len, delay, debug, is_dbg, cacheable, is_lock);

    // Update debug information
    bypassops++;
    CACHEBYPASS_SET(*debug);

  } // Bypass MMU || Disabled
} // vectorcache::mem_write()

/// @} Interface Data Methods
/// ****************************************************************************
/// @name Interface Control Methods
/// @{

/// cache_if::Read cache configuration register
unsigned int vectorcache::read_config_reg(sc_core::sc_time *t) {

  unsigned tmp = CACHE_CONFIG_REG;

  srDebug()("CACHE_CONFIG_REG", tmp)("Read cache configuration register");

  *t += clockcycle;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif

  return (tmp);

} // vectorcache::read_config_reg()

/// ----------------------------------------------------------------------------

/// cache_if::Read cache tags (ASI 0xe)
/** @details
*   Diagnostic read of tags is possible by executing an LDA instruction with
*   ASI = 0xC for instruction cache tags and ASI = 0xe for data cache tags.
*   A cache line and way are indexed by the address bits making up the cache
*   offset and the least significant bits of the address bits making up the
*   address tag.
*/
void vectorcache::read_cache_tag(unsigned int address, unsigned int * data,
                                 sc_core::sc_time *t) {

  unsigned tmp;
  unsigned idx = get_idx(address);
  unsigned way = get_tag(address) & 0x3;

  // find the required cache line
  t_cache_line* line = (*lookup_line(idx, way));

  // build bitmask from tag fields
  // (! The atag field starts bit 10. It is not MSB aligned as in the actual tag layout.)
  uint32_t tmp_valid;
  line->tag[t_cache_line::VALID].bus_read(tmp_valid);

  tmp = line->tag[t_cache_line::ATAG].bus_read() << 10;
  tmp |= line->tag[t_cache_line::LRR].bus_read() << 9;
  tmp |= line->tag[t_cache_line::LOCK].bus_read() << 8;
  tmp |= tmp_valid;

  srDebug()("tag", line->tag[t_cache_line::ATAG])
           ("idx", idx)
           ("way", way)
           ("Diagnostic read cache tag");

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif

  // handover bitmask pointer (the tag)
  *data = tmp;

  // increment time
  *t += clockcycle;

} // vectorcache::read_cache_tag()

/// ----------------------------------------------------------------------------

/// cache_if::Write data cache tags (ASI 0xe)
/** @details
*   The tags can be directly written by executing a STA instruction with
*   ASI = 0xC for the instruction cache tags and ASI = 0xE for the data cache
*   tags. The cache line and set are indexed by the address bits making up the
*   cache offset and the least significant bits of the address bits making up
*   the address tag. D[31:10] is written into the ATAG field and the valid bits
*   are written with the D[7:0] of the write data. Bit D[9] is written into the
*   LRR bit (if enabled) and D[8] is written into the lock bit (if enabled).
*/
void vectorcache::write_cache_tag(unsigned int address, unsigned int * data,
                                  sc_core::sc_time *t) {

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(*data);
  #endif

  unsigned idx = get_idx(address);
  unsigned way = get_tag(address) & 0x3;

  // find the required cache line
  t_cache_line* line = (*lookup_line(idx, way));

  // update the tag with write data
  // (! The atag field is expected to start at bit 10. Not MSB aligned as in tag layout.)
  line->tag[t_cache_line::ATAG].bus_write( *data >> 10);
  line->tag[t_cache_line::LRR].bus_write((*data & 0x100) >> 9);
  // lock bit can only be set, if line locking is enabled
  // locking only works in multi-way configurations. the last way must never be locked.
  line->tag[t_cache_line::LOCK].bus_write(((m_setlock) && (way != m_sets))? ((*data & 0x100) >> 8) : 0);
  line->tag[t_cache_line::VALID].bus_write((*data & 0xff));

  srDebug()("tag", line->tag[t_cache_line::ATAG])
           ("idx", idx)
           ("way", way)
           ("lrr", line->tag[t_cache_line::LRR])
           ("lock", line->tag[t_cache_line::LOCK])
           ("valid", line->tag[t_cache_line::VALID])
           ("Diagnostic write cache tag");

  // increment time
  *t += clockcycle;

} // vectorcache::write_cache_tag()

/// ----------------------------------------------------------------------------

/// cache_if::Read data cache entry/data (ASI 0xf)
/** @details
*   Similar to instruction tag read, a data sub-block may be read by executing
*   an LDA instruction with ASI = 0xD for instruction cache data and ASI = 0xF
*   for data cache data. The sub-block to be read in the indexed cache line and
*   set is selected by A[4:2].
*/
void vectorcache::read_cache_entry(unsigned int address, unsigned int * data,
                                   sc_core::sc_time *t) {

  unsigned idx = get_idx(address);
  unsigned sb = (address << (32 - m_offset_bits) >> (34 - m_offset_bits));
  unsigned way = get_tag(address) & 0x3;

  // find the required cache line
  t_cache_line* line = (*lookup_line(idx, way));

  *data = line->entry.get_int(sb);

  srDebug()("idx", idx)
           ("subblock", sb)
           ("way", way)
           ("data", *data)
           ("Diagnostic read cache entry");

  // increment time
  *t += clockcycle;

} // vectorcache::read_cache_entry()

/// ----------------------------------------------------------------------------

/// cache_if::Write data cache entry/data (ASI 0xd)
/** @details
*   A data sub-block can be directly written by executing a STA instruction with
*   ASI = 0xD for the instruction cache data and ASI = 0xF for the data cache
*   data. The sub-block to be read in indexed cache line and set is selected by
*   A[4:2].
*/
void vectorcache::write_cache_entry(unsigned int address, unsigned int * data,
                                    sc_core::sc_time *t) {

  unsigned idx = get_idx(address);
  unsigned sb = (address << (32 - m_offset_bits) >> (34 - m_offset_bits));
  unsigned way = get_tag(address) & 0x3;

  // find the required cache line
  t_cache_line* line = (*lookup_line(idx, way));

  line->entry.set_int(sb, *data);

  srDebug()("idx", idx)
           ("subblock", sb)
           ("way", way)
           ("data", *data)
           ("Diagnostic write cache entry");

  // increment time
  *t += clockcycle;

} // vectorcache::write_cache_entry()

/// ----------------------------------------------------------------------------

/// cache_if::Flush cache
void vectorcache::flush(sc_core::sc_time *t, unsigned int * debug, bool is_dbg) {

//  unsigned addr;
  unsigned i_line = 0;
//  bool cacheable = true;

  // for all cache lines
  for (std::vector<t_cache_line*>::iterator line = cache_mem->begin();
       line < cache_mem->end(); line++, i_line++) {

// it's a write-through cache. we should never need the following code
/*    for (unsigned entry = 0; entry < m_wordsperline; entry++) {

      // check for valid data
      if ((*line)->tag.dirty && (*line)->tag[t_cache_line::VALID] & (1 << entry)) {

        // construct address from tag
        addr = ((*line)->tag[t_cache_line::ATAG] << (m_idx_bits + m_offset_bits));
        addr |= ((i_line/(m_sets+1)) << m_offset_bits);
        addr |= (entry << 2);

        srDebug()("addr", addr)
                 ("line", i_line)
                 ("idx", i_line/(m_sets+1))
                 ("way", i_line%(m_sets+1))
                 ("data", (*line)->entry.get_int(entry))
                 ("Cache flush");

        m_tlb_adaptor->mem_write(addr, 0x8,(unsigned char*)&((*line)->entry.get_int(entry)),
                                 4, t, debug, is_dbg, cacheable, false);

      }
    }*/
    // invalidate all entries
    (*line)->tag[t_cache_line::VALID].bus_write(0);
  }

  // Update debug information
  CACHEFLUSH_SET(*debug);
  srDebug()("flush set", *debug)("Finished cache flush");

} // vectorcache::flush()

/// ----------------------------------------------------------------------------

/// Snooping function (invalidates cache lines)
void vectorcache::snoop_invalidate(const t_snoop& snoop, const sc_core::sc_time& delay) {

  unsigned address;
  unsigned tag;
  unsigned idx;
  unsigned offset;
  unsigned way;

  // Is the cache enabled
  if (check_mode() & 0x3) {

    for (address = snoop.address; address < snoop.address + snoop.length; address += 4) {
      // Extract index and tag from address
      tag    = get_tag(address);
      idx    = get_idx(address);
      offset = get_offset(address);
      way    = 0;

      for (std::vector<t_cache_line*>::iterator line = lookup_line(idx, 0);
           way <= m_sets; line++, way++) {

        // Check the cache tag
        if (((*line)->tag[t_cache_line::ATAG]).bus_read() == tag) {

          uint32_t tmp;
          if (!m_new_linefetch_en) {
            (*line)->tag[t_cache_line::VALID].bus_read(tmp);

            (*line)->tag[t_cache_line::VALID].bus_write(tmp & ~offset2valid(offset));
          } else {
            (*line)->tag[t_cache_line::VALID].bus_write(0);
          }
        }
      }
    }
  }
} // vectorcache::snoop_invalidate()

/// @} Interface Control Methods
/// ****************************************************************************
/// @name Internal Methods
/// @{

/// Transforms a cache-line offset into a valid mask
/** @details
*   Cache lines can hold either 16 bytes or 32 bytes. Valid bits are defined for
*   each word, hence 4 or 8 bits per cache line.
*   The len parameter is the number of bytes to be accessed from the cache line
*   and defaults to 4. Offset is in bytes and is first divided by 4 to give the
*   word offset. Since lines can have a max of 8 words, we mask an 8-bit valid
*   field to find out whether the data at the calculated word offset is valid.
*   NOTE: This function does not perform alignment checks.
*/
unsigned int vectorcache::offset2valid(unsigned int offset, unsigned int len) {
  // checks
  if (!len || (offset+len) > m_bytesperline) {
    srWarn()("Invalid offset/len for calculation of valid mask");
    return 0;
  }

  // Byte, half-word or word access
  unsigned ret = 0x1 << (offset >> 2);

  // Multi-word access
  for (int i_word = (len>>2)-1; i_word > 0; i_word--) {
    ret |= ret << 1;
  }

  // Unaligned access
  if ((offset & 3) || (len & 3)) {
    ret |= ret << 1;
  }

  return ret;
} // vectorcache::offset2valid()

/// ----------------------------------------------------------------------------

/// Selects way to be refilled depending on replacement strategy
unsigned int vectorcache::replacement_selector(unsigned int idx, unsigned int mode) {

  std::vector<t_cache_line*>::iterator line = lookup_line(idx, 0);
  unsigned way = 0, way_select = 0;
  uint32_t min_lru;

  switch (mode) {

    // Direct mapped
    case 0:

      // There is only one way.
      way_select = 0;
      break;

    // LRU - least-recently used
    case 1:

      // LRU replaces the line, which hasn't been used for the longest time.
      // The LRU algorithm needs extra "flip-flops" per cache line to store
      // access history. Within the TLM model the LRU bits will be attached to
      // the tag ram.

      // Find the cache line with the lowest LRU value.
      min_lru = m_max_lru;

      for (; way <= m_sets; line++, way++) {

        // The last way will never be locked.
        uint32_t tmp_lru;
        (*line)->tag[t_cache_line::LRU].bus_read(tmp_lru);
        uint32_t tmp_lock;
        (*line)->tag[t_cache_line::LRU].bus_read(tmp_lock);
        if ((tmp_lru <= min_lru) && (tmp_lock == 0)) {
          min_lru = tmp_lru;
          way_select = way;
        }
      }
      srDebug()("selected way", way_select)("LRU Replacement");
      break;

    // LRR - least recently replaced
    case 2:

      // The LRR algorithm uses one extra bit in tag rams to store replacement
      // history. Supossed to work only for 2-way associative caches.

      // The last way (way 1) will never be locked.
      way_select = 1;

      for (; way <= 2; line++, way++) {

        if (((*line)->tag[t_cache_line::LRR].bus_read() == 0) && ((*line)->tag[t_cache_line::LOCK].bus_read() == 0)) {

          srDebug()("selected way", way)("LRR Replacement");
          way_select = way;
          break;
        }
      }
      break;

    // Pseudo-random
    default:

      // Random replacement is implemented through modulo-N counter that selects
      // the line to be evicted on cache miss.

      do {

        way_select = m_pseudo_rand % (m_sets + 1);
        m_pseudo_rand++;

      }
      // The last way will never be locked.
      while (line[way_select]->tag[t_cache_line::LOCK].bus_read() != 0);

      srDebug()("selected way", way_select)("Pseudo Random Replacement");
  }

  return way_select;

} // vectorcache::replacement_selector()

/// ----------------------------------------------------------------------------

/// Updates the LRR bits for every line replacement
void vectorcache::lrr_update(unsigned int idx, unsigned int way_select) {

  std::vector<t_cache_line*>::iterator line = lookup_line(idx, 0);

  // LRR may only be used for 2-way associative caches.
  for (unsigned way = 0; way < 2; line++, way++) {

    // Switch the lrr bit on for the selected way and off for the remaining.
    (*line)->tag[t_cache_line::LRR].bus_write((way == way_select)? 1 : 0);

    srDebug()("way", way)("LRR", (*line)->tag[t_cache_line::LRR])("LRR update");

  }

} // vectorcache::lrr_update()

/// ----------------------------------------------------------------------------

/// Updates the LRU counters for every cache hit
void vectorcache::lru_update(unsigned int idx, unsigned int way_select) {

  std::vector<t_cache_line*>::iterator line = lookup_line(idx, 0);
  uint32_t pivot = (*lookup_line(idx, way_select))->tag[t_cache_line::LRU].bus_read();
  unsigned lru;

  for (unsigned way = 0; way <= m_sets; line++, way++) {
    lru = (*line)->tag[t_cache_line::LRU].bus_read();

    // LRU: Counter for each line of a way
    if (way == way_select) {
      (*line)->tag[t_cache_line::LRU].bus_write(m_max_lru);
    } else if ((*line)->tag[t_cache_line::LRU].bus_read() > pivot) {
      uint32_t tmp_lru;
      (*line)->tag[t_cache_line::LRU].bus_read(tmp_lru);
      (*line)->tag[t_cache_line::LRU].bus_write(tmp_lru-1);
    }

    srDebug()("way", way)("old LRU", lru)("new LRU", (*line)->tag[t_cache_line::LRU])("LRU update");

  }

} // vectorcache::lru_update()

/// ----------------------------------------------------------------------------

/// Searches for a cache line in all cache ways. Returns found way, otherwise -1.
int vectorcache::locate_line(unsigned int const tag,
                             unsigned int const idx,
                             unsigned int const offset,
                             unsigned int const len,
                             sc_core::sc_time * delay) {
  unsigned way = 0;
  bool found = false;

  // Lookup all cache ways
  for (std::vector<t_cache_line*>::iterator line = lookup_line(idx, 0);
       way <= m_sets; line++, way++) {

    // Check the cache tag
    if ((*line)->tag[t_cache_line::ATAG].bus_read() == tag) {

      // Check the valid bit
      uint32_t tmp_valid;
      (*line)->tag[t_cache_line::VALID].bus_read(tmp_valid);
      if ((!m_new_linefetch_en && (tmp_valid & offset2valid(offset, len)) == offset2valid(offset, len))
      || (m_new_linefetch_en && (tmp_valid & 0x1))) {

        srDebug()("way", way)
                 ("valid", (*line)->tag[t_cache_line::VALID])
                 ("valid mask",offset2valid(offset, len))
                 ("Cache hit in current way");
        found = true;
        break;
      } else {

        srDebug()("way", way)
                 ("valid", (*line)->tag[t_cache_line::VALID])
                 ("valid mask",offset2valid(offset, len))
                 ("Cache hit but invalid data in current way");

      } // Cache hit but invalid
    } else {
      srDebug()("way", way)("Cache miss in current way");
    }
  } // loop m_sets

  // Increment time
  *delay += clockcycle;

  // Update power information
  // Read all cache tag lines in parallel
  if (m_pow_mon) dyn_tag_reads += m_sets+1;

  if (found)
    return way;

  return -1;
} // vectorcache::locate_line()

/// ----------------------------------------------------------------------------

/// Allocates a cache line in a given cache way.
int vectorcache::update_line (unsigned const tag,
                              unsigned const idx,
                              unsigned const offset,
                              unsigned const way,
                              unsigned const len,
                              unsigned char* const data,
                              sc_core::sc_time * delay,
                              unsigned int * debug,
                              bool& cacheable, bool is_dbg) {

    t_cache_line* line = (*lookup_line(idx, way));

    // Update data in cache
    // This is written generically to serve both aligned reads and non-aligned
    // writes.
    line->entry.copy_to(data, len, offset >> 2, offset & 0x3);

    // Update tag and flags for line allocate
    if (line->tag[t_cache_line::ATAG].bus_read() != tag) {

      line->tag[t_cache_line::ATAG].bus_write(tag);

      if (m_repl == 2) lrr_update(idx, way);

      line->tag[t_cache_line::VALID].bus_write(0);

    }

    // Update flags for line allocate or update
    if (!m_new_linefetch_en) {
      uint32_t tmp_valid;
      line->tag[t_cache_line::VALID].bus_read(tmp_valid);
      line->tag[t_cache_line::VALID].bus_write(tmp_valid | offset2valid(offset, len));
    } else {
      line->tag[t_cache_line::VALID].bus_write(0x1);
    }
    if (m_repl == 1) lru_update(idx, way);

    // Increment time
    // One 32-bit load/store can be served per cycle (GRLIB IP 71.3.1). We
    // therefore calculate 1cc for len <= 4B and 1cc for every additional 4B.
    *delay += (1 + ((len - 1) >> 2)) * clockcycle;

    // Update power information
    if (m_pow_mon) {
      // Write access to data ram
      dyn_data_writes += 1 + ((len - 1) >> 2);
      // Write to tag ram (replacement or setting valid bits)
      dyn_tag_writes++;
    }

    srDebug()("tag", tag)
             ("idx", idx)
             ("offset", offset)
             ("way", way)
             ("data", *data)
             ("Updated cache line");

    return way;
} // vectorcache::update_line()

/// ----------------------------------------------------------------------------

/// Reads data from memory and inserts it into cache.
/// Returns the allocated way, otherwise -1.
/// Used by mem_read() and mem_write() with write-allocate.
int vectorcache::allocate_line (unsigned const tag,
                                unsigned const idx,
                                unsigned const offset,
                                unsigned const len,
                                unsigned char* const data,
                                sc_core::sc_time * delay, unsigned int * debug, bool& cacheable, bool is_dbg) {

  unsigned way = 0;
  bool found = false;

  // Easiest option for replacement is using invalid cache lines.
  for (std::vector<t_cache_line*>::iterator line = lookup_line(idx, 0);
       way <= m_sets; line++, way++) {

    uint32_t tmp_valid;
    (*line)->tag[t_cache_line::VALID].bus_read(tmp_valid);
    if ((!m_new_linefetch_en && (tmp_valid & offset2valid(offset, len)) == 0 /* == offset2valid(offset, len) instead of 0? */)
    || (m_new_linefetch_en && (tmp_valid & 0x1) == 0)) {

      srDebug()("way", way)("Allocate cache line: Found invalid cache line; will use for refill");
      found = true;
      break;
    }
  }

  // Otherwise choose depending on replacement strategy.
  if (!found) {
    // TODO: late binding
    way = replacement_selector(idx, m_repl);
    srDebug()("way", way)("Allocate cache line: Found cache line by replacement selector");
  }

  return update_line(tag, idx, offset, way, len, data, delay, debug, cacheable, is_dbg);
} // vectorcache::allocate_line()

/// @} Internal Methods
/// ****************************************************************************
/// @name Diagnostic Methods
/// @{

/// Displays cache lines for debugging
void vectorcache::dbg_out(unsigned int idx) {

  #ifdef LITTLE_ENDIAN_BO

  swap_Endianess(idx);

  #endif

  unsigned way = 0;

  for (std::vector<t_cache_line*>::iterator line = lookup_line(idx, 0);
       way <= m_sets; line++, way++) {

    // display the tag
    srDebug()("tag", (*line)->tag[t_cache_line::ATAG])
             ("way", way)
             ("valid", (*line)->tag[t_cache_line::VALID])
             ("Diagnostic cache line display (big-endian)");

    // display all entries
    for (unsigned j = 0; j < m_wordsperline; j++) {

      std::cout << "Entry: " << j << " - ";

      for (unsigned k = 0; k < 4; k++) {
        std::cout << hex << std::setw(2) << (unsigned)(*line)->entry.get_char(j, k);
      }

      std::cout << " " << std::endl;
    }
  }
} // vectorcache::debug_out()

/// ----------------------------------------------------------------------------

/// Sets clock cycle latency using sc_clock argument
void vectorcache::clkcng(sc_core::sc_time &clk) {
  clockcycle = clk;
} // vectorcache::clkcng

/// ----------------------------------------------------------------------------

/// Print execution statistic at end of simulation
void vectorcache::end_of_simulation() {

  uint64_t total_rhits = 0;
  uint64_t total_whits = 0;

  v::report << name() << " ******************************************** " << v::endl;
  v::report << name() << " * Caching statistic:                        " << v::endl;
  v::report << name() << " * -------------------" << v::endl;

  for (uint32_t i=0; i<=m_sets; i++) {

    v::report << name() << " * Read hits way" << i << ": " << rhits[i] << v::endl;

    total_rhits += rhits[i];

  }

  v::report << name() << " * Total Read Hits: " << total_rhits << v::endl;
  v::report << name() << " * Read Misses:  " << rmisses << v::endl;

  // avoid / 0
  if (total_rhits+rmisses != 0) {

    v::report << name() << " * Read Hit Rate: " << (double)total_rhits/(double)(total_rhits+rmisses) << v::endl;

  }

  for (uint32_t i = 0; i <= m_sets; i++) {

    v::report << name() << " * Write hits way" << i << ": " << whits[i] << v::endl;

    total_whits += whits[i];

  }

  v::report << name() << " * Total Write Hits: " << total_whits << v::endl;
  v::report << name() << " * Write Misses: " << wmisses << v::endl;

  // avoid / 0
  if (total_whits + wmisses != 0) {

  v::report << name() << " * Write Hit Rate: " << (double)total_whits/(double)(total_whits+wmisses) << v::endl;

  }

  v::report << name() << " * Bypass ops:   " << bypassops << v::endl;
  v::report << name() << " ******************************************** " << v::endl;

} // vectorcache::end_of_simulation()

/// @name Diagnostic Methods
/// ****************************************************************************
///@} mmu_cache
