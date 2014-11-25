// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file vectorcache.cpp
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

#include "core/models/mmu_cache/lib/vectorcache.h"

// constructor
// args: sysc module name, pointer to AHB read/write methods (of parent), delay on read hit, delay on read miss (incr), number of sets, setsize in kb, linesize in words, replacement strategy
vectorcache::vectorcache(ModuleName name,
                         mmu_cache_if * _mmu_cache, mem_if *_tlb_adaptor,
                         unsigned int mmu_en, unsigned int burst_en,
			 bool new_linefetch_en,
                         unsigned int sets, unsigned int setsize,
                         unsigned int setlock, unsigned int linesize,
                         unsigned int repl, unsigned int lram,
                         unsigned int lramstart, unsigned int lramsize,
			 bool pow_mon) :
    sc_module(name),
    m_mmu_cache(_mmu_cache),
    m_tlb_adaptor(_tlb_adaptor),
    m_burst_en(burst_en),
    m_new_linefetch_en(new_linefetch_en),
    m_pseudo_rand(0), 
    m_sets(sets-1), 
    m_setsize((unsigned int)log2((double)setsize)), 
    m_setlock(setlock), 
    m_linesize((unsigned int)log2((double)linesize)), 
    m_wordsperline(linesize), 
    m_bytesperline(m_wordsperline << 2), 
    m_offset_bits((unsigned int)log2((double)m_bytesperline)), 
    m_number_of_vectors(setsize*256/linesize), 
    m_idx_bits(m_setsize + 8 - m_linesize), 
    m_tagwidth(32 - m_idx_bits - m_offset_bits), 
    m_repl(repl), 
    m_mmu_en(mmu_en), 
    m_lram(lram), 
    m_lramstart(lramstart), 
    m_lramsize((unsigned int)log2((double)lramsize)),
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

    // initialize cache line allocator
    memset(&m_default_cacheline, 0, sizeof(t_cache_line));

    // Cache may have 1 to 4 sets
    if ((m_sets < 0)||(m_sets>3)) {

      v::error << this->name() << "Cache may have 1-4 sets (not " << m_sets + 1 << ")!" << v::endl;
      assert(0);

    }

    // Linesize may be 4 or 8 words
    if ((linesize!=4)&&(linesize!=8)) {

      v::error << this->name() << "Cache line size may be 4 or 8 words (not " << linesize << ")!" << v::endl;
      assert(0);

    }

    // Set size may be 1 to 256 kb
    if ((setsize < 1)||(setsize > 256)) {

      v::error << this->name() << "Size of cache set must be between 1 and 225 kb (not " << setsize << ")!" << v::endl;
      assert(0);

    }

    // LRR replacement may only be selected for two-way caches
    if ((m_repl==2)&(m_sets!=1)) {

      v::error << this->name() << "LRR replacement may only be selected for two-way associative caches!" << v::endl;
      assert(0);

    }

    // Direct mapped cache (replacement) only allowed for one-way caches
    if ((m_sets > 0) && (m_repl == 0)) {

      v::error << this->name() << "Replacement policy not valid. Direct mapped only allowed for one-way caches!" << v::endl;
      assert(0);

    }

    // create the cache sets
    for (unsigned int i = 0; i <= m_sets; i++) {

        v::debug << this->name() << "Create cache set " << i << v::endl;
        std::vector<t_cache_line> *cache_set = new std::vector<t_cache_line>(
                m_number_of_vectors, m_default_cacheline);

        cache_mem.push_back(cache_set);

        // create one cache_line struct per set
        t_cache_line *current_cacheline = new t_cache_line;
        m_current_cacheline.push_back(current_cacheline);
    }

    // Configuration report
    v::info << this->name() << " ******************************************************************************* " << v::endl;
    v::info << this->name() << " * Created cache memory with following parameters:                               " << v::endl;
    v::info << this->name() << " * ----------------------------------------------- " << v::endl;
    v::info << this->name() << " * mmu_en: " << mmu_en << v::endl;
    v::info << this->name() << " * burst_en: " << burst_en << v::endl;
    v::info << this->name() << " * sets: " << sets << v::endl;
    v::info << this->name() << " * setsize: " << setsize << v::endl;
    v::info << this->name() << " * setlock: " << m_setlock << v::endl;
    v::info << this->name() << " * linesize: " << linesize << v::endl;
    v::info << this->name() << " * repl (0-Direct Mapped, 1-LRU, 2-LRR, 3-RANDOM): " << m_repl << v::endl;
    v::info << this->name() << " * ---------------------------------------------------------- " << v::endl;
    v::info << this->name() << " * Size of each cache set " << (unsigned int)pow(2, (double)m_setsize) << " kb" << v::endl;
    v::info << this->name() << " * Bytes per line " << m_bytesperline << " (offset bits: " << m_offset_bits << ")" << v::endl;
    v::info << this->name() << " * Number of cache lines per set " << m_number_of_vectors << " (index bits: " << m_idx_bits <<" )" << v::endl;
    v::info << this->name() << " * Width of cache tag in bits " << m_tagwidth << v::endl;
    v::info << this->name() << " ******************************************************************************* "  << v::endl;

    // lru counter saturation
    switch (m_sets) {

        case 1:
            m_max_lru = 1;
            break;
        case 2:
            m_max_lru = 7;
            break;
        case 3:
            m_max_lru = 31;
            break;
        default:
            m_max_lru = 0;
    }

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

    // setsize must be between 1 and 256 kb (m_setsize 1 - 8)
    assert((m_setsize>=0)&&(m_setsize<=8));
    // enter setsize
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
    for(uint32_t i = 0; i<sets; i++) {

      rhits[i] = 0;
      whits[i] = 0;

    }

    rmisses = 0;
    wmisses = 0;
    bypassops = 0;

}

// Destructor
vectorcache::~vectorcache() {
}

// external interface functions
// ----------------------------

/// read from cache
bool vectorcache::mem_read(unsigned int address, unsigned int asi, unsigned char *data,
                           unsigned int len, sc_core::sc_time *delay,
                           unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock) {

  int set_select = -1;
  int cache_hit = -1;

  unsigned int burst_len = 0;
  unsigned int burst_address = 0;
  unsigned int replacer_limit = 0;
    
//  unsigned char correct_data[32];
//  m_tlb_adaptor->mem_read(address, asi, correct_data, len, delay, debug, is_dbg, is_lock, cacheable);
//  return true;
    
  // Is the cache enabled (0b11) or frozen (0b01) ?
  v::debug << this->name() << "read cacheable: " << cacheable << v::endl;
  if (((!is_dbg) && (check_mode() & 0x1)) && asi != 0x1c) {
  //if (1==2) {

    // extract index and tag from address
    unsigned int tag = (address >> (m_idx_bits + m_offset_bits));
    unsigned int idx = ((address << m_tagwidth) >> (m_tagwidth + m_offset_bits));
    unsigned int offset = ((address << (32 - m_offset_bits)) >> (32 - m_offset_bits));
    unsigned int byt = (address & 0x3);

    // space for data to refill a cache line of maximum size
    unsigned char ahb_data[32];


    v::debug << this->name() << "READ ACCESS with idx: " << hex << idx
             << " tag: " << hex << tag << " offset: " << hex
             << offset << v::endl;

    // lookup all cachesets
    for (unsigned int i = 0; i <= m_sets; i++) {

      m_current_cacheline[i] = lookup(i, idx);

      // ASIs 0-3 force cache miss
      if (asi > 3) {

        // Read access to all tag ram (parallel sets)
        if (m_pow_mon) dyn_tag_reads++;

        // Check the cache tag
        if ((*m_current_cacheline[i]).tag.atag == tag) {

          //v::debug << this->name() <<  "Correct atag found in set " << i << v::endl;

          // Check the valid bit
          if ((!m_new_linefetch_en && ((*m_current_cacheline[i]).tag.valid & offset2valid(offset, len)) == offset2valid(offset, len)) ||
	      (m_new_linefetch_en && ((*m_current_cacheline[i]).tag.valid & 0x1))) {

            v::debug << this->name() << "Cache Hit in Set " << i << "(valid: " << hex << (*m_current_cacheline[i]).tag.valid << " check mask: " << hex << offset2valid(offset, len) << ")" << v::endl;

            // update debug information
            CACHEREADHIT_SET(*debug,i);

            // update lru history
            if (m_repl == 1) {
              lru_update(i);
            }

            // write data pointer
            memcpy(data, &(*m_current_cacheline[i]).entry[offset >> 2].c[byt], len);

            // Read access to data ram that produced the hit
            if (m_pow_mon) dyn_data_reads++;

            // increment time
            *delay += ((len - 1) >> 2)*clockcycle;

            // valid data in set i
            cache_hit = i;

            // increment hit counter
            rhits[i]++;

            break;
          } else {
            
            v::debug << this->name() << "Tag Hit but data not valid in set " << i << v::endl;

            if (len == 8) {
              // dword - make sure to disable both words
              (m_new_linefetch_en == false) ? (*m_current_cacheline[i]).tag.valid &= ~offset2valid(offset, len) : (*m_current_cacheline[i]).tag.valid = 0;               

            }

          }

        } else {

          v::debug << this->name() << "Cache miss in set " << i << v::endl;

        }

      } else {

        v::debug << this->name() << "ASI force cache miss" << v::endl;

      }
    }

    // In case no matching tag was found or data is not valid:
    // -------------------------------------------------------
    // read miss - On a data cache read miss to a cacheable location 4 bytes of data
    // are loaded into the cache from main memory.
    if (cache_hit == -1) {

      // Increment miss counter
      rmisses++;

      // Increment time
      *delay += clockcycle;

      // Set length of bus transfer depending on mode:
      // ---------------------------------------------
      // If burst fetch is enabled, the cache line is filled starting at the missed address
      // until the end of the line.
      if ((m_burst_en && (m_mmu_cache->read_ccr(true) & 0x10000))||(m_new_linefetch_en)) {
	
	if (m_new_linefetch_en) {
	  burst_address = ((address >> (m_linesize+2)) << (m_linesize+2));  // Beginning of cache line
	  burst_len     = m_bytesperline;
	} else {
	  burst_address = ((address >> 2) << 2);
	  burst_len = m_bytesperline - ((offset >> 2) << 2);
	  replacer_limit = m_bytesperline - 4;
	}

      } else {

	burst_address = ((address >> 2) << 2);

        if (len == 8) {
          // 64bit
          burst_len = 8;
          replacer_limit = offset+4;
        } else {
          // 32bit or below
          burst_len =  4;
          replacer_limit = offset;
        }
      }

      v::debug << name() << "Actual address: 0x" << v::hex << address << " Burst address: 0x" << burst_address << " Burst length: 0x" << burst_len << v::endl;

      // Access ahb interface or mmu - return true if data is cacheable
      if (m_tlb_adaptor->mem_read(burst_address, asi, ahb_data, burst_len, delay, debug, is_dbg, is_lock, cacheable)) {
        v::debug << this->name() << "vectorcache read cacheable" << v::endl;

        // Check for unvalid data which can be replaced without harm
        for (unsigned int i = 0; i <= m_sets; i++) {

          if ((!m_new_linefetch_en && ((((*m_current_cacheline[i]).tag.valid) & offset2valid(offset, len)) == 0)) || 
	      (m_new_linefetch_en && ((((*m_current_cacheline[i]).tag.valid) & 0x1) == 0))) {

            // select unvalid data for replacement
            set_select = i;
            v::debug << this->name() << "Set " << set_select
                     << " has no valid data - will use for refill."
                     << v::endl;
            break;
          }
        }

        // Check for cache freeze
        if (check_mode() & 0x2) {

          // Cache not frozen !!

          // in case there is no free set anymore
          if (set_select == -1) {

            // select set according to replacement strategy (todo: late binding)
            set_select = replacement_selector(m_repl);
            v::debug << this->name() << "Set " << set_select
                     << " selected for refill by replacement selector."
                     << v::endl;

          }

          // fill in the new data (always the complete word)
	  if (!m_new_linefetch_en) {
	    memcpy(&(*m_current_cacheline[set_select]).entry[offset >> 2],
		   ahb_data, burst_len);
	  } else {
	    memcpy(&(*m_current_cacheline[set_select]).entry[0], ahb_data, m_bytesperline);
	  }

          if (m_pow_mon) {
            // Write access to data ram
            dyn_data_writes += (burst_len >> 2) + 1;
            // Write to tag ram (replacement or setting valid bits)
            dyn_tag_writes++;
          }

          // has the tag changed?
          if ((*m_current_cacheline[set_select]).tag.atag != tag) {

            // fill in the new tag
            (*m_current_cacheline[set_select]).tag.atag = tag;

            // switch off all the valid bits ...
            (*m_current_cacheline[set_select]).tag.valid = 0;

            // .. and switch on the ones for the new entries
	    if (!m_new_linefetch_en) {
	      for (unsigned int i = offset; i <= replacer_limit; i += 4) {
		((*m_current_cacheline[set_select]).tag.valid |= offset2valid(i));
	      }
	    } else {
	      (*m_current_cacheline[set_select]).tag.valid = 0x1;
	    }

            // reset lru
            (*m_current_cacheline[set_select]).tag.lru = m_max_lru;

            // update lrr history
            if (m_repl == 2) {
              lrr_update(set_select);
            };

          } else {

	    if (!m_new_linefetch_en) {
	      // switch on the valid bits for the new entries
	      for (unsigned int i = offset; i <= replacer_limit; i += 4) {
		((*m_current_cacheline[set_select]).tag.valid |= offset2valid(i));
	      }
	    } else {
	      (*m_current_cacheline[set_select]).tag.valid = 0x1;
	    }
          }

        } else {

          // Cache is frozen !!

          // Cache is kept in sync with main memory, but no new lines are allocated on read miss:
          // The new data will only be filled in as long there is unvalid data in one of the sets (set_select != -1)
          // && the new data does not change the atag, because this would invalidate all the other entries
          // in the line (tag.atag == tag).

          //v::debug << name() << "Cache is frozen" << v::endl;

          if ((set_select != -1) && ((*m_current_cacheline[set_select]).tag.atag == tag)) {

            //v::debug << name() << "Found set for replacing: " << set_select << v::endl;

            // fill in the new data (always the complete word)
	    if (!m_new_linefetch_en) {
	      memcpy(&(*m_current_cacheline[set_select]).entry[offset >> 2], ahb_data, burst_len);
	    } else {
	      memcpy(&(*m_current_cacheline[set_select]).entry[0], ahb_data, m_bytesperline);
	    }

            if (m_pow_mon) {
              // Write access to data ram
              dyn_data_writes += (burst_len >> 2) + 1;
              // Write to tag ram (valid bits)
              dyn_tag_writes++;
            }

	    if (!m_new_linefetch_en) {
	      // switch on the valid bits for the new entries
	      for (unsigned int i = offset; i <= replacer_limit; i += 4) {
		((*m_current_cacheline[set_select]).tag.valid |= offset2valid(i));
	      } 
            } else {
	      (*m_current_cacheline[set_select]).tag.valid = 0x1;
	    }

          } else {

            //v::debug << name() << "All sets occupied - frozen miss" << v::endl;

            // update debug information
            FROZENMISS_SET(*debug);

          }
        }
      } else {
        v::debug << this->name() << "vectorcache: read not cacheable" << v::endl;
      }

      // update debug information
      CACHEREADMISS_SET(*debug, set_select);

      // copy the data requested by the processor
      if (!m_new_linefetch_en) {
	memcpy(data, ahb_data + byt, len);
      } else {
	memcpy(data, ahb_data + offset, len);
      }
    }

/*    bool equal = true;
    for (int i = 0; i < len; i++) {
      if (correct_data[i] != data[i]) {
        //data[i] = ahb_data[i];
        equal = false;
      }
    }
    if (equal == false) {
      v::warn << name() << "cache and direct read deliver non equal data! addr: " << hex << address << v::endl;
    } else {
      v::debug << name() << "cache und direct read equal" << v::endl;
    }*/

  } else {

    v::debug << this->name() << "BYPASS read from address: " << hex
             << address << v::endl;

    // Increment bypass counter
    bypassops++;

    // Cache is disabled
    // Forward request to ahb interface (?? does it matter whether mmu is enabled or not ??)
    m_tlb_adaptor->mem_read(address, asi, data, len, delay, debug, is_dbg, is_lock, cacheable);

    // update debug information
    CACHEBYPASS_SET(*debug);

    // increment time
    *delay += clockcycle;
    /*bool equal = true;
    for (int i = 0; i < len; i++) {
      if (correct_data[i] != data[i]) {
        //data[i] = ahb_data[i];
        equal = false;
      }
    }
    if (equal == false) {
      v::warn << name() << "bypass cache and direct read deliver non equal data! addr: " << hex << address << v::endl;
    } else {
      v::debug << name() << "cache und direct read equal" << v::endl;
    }*/

  }

  // always return true - cacheability only matters on bus mem_if
  return true;
}

// write to/through cache:
// -----------------------
// The write policy for stores is write-through with no-allocate on write miss.
// - on hits it writes to cache and main memory;
// - on misses it updates the block in main memory not bringing that block to the cache;
//   Subsequent writes to the block will update main memory because Write Through policy is employed.
//   So, some time is saved not bringing the block in the cache on a miss because it appears useless anyway.

void vectorcache::mem_write(unsigned int address, unsigned int asi, unsigned char * data,
                            unsigned int len, sc_core::sc_time * delay,
                            unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock) {

    // is the cache enabled (0x11) or frozen (0x01)
    v::debug << "write cacheable: " << cacheable << v::endl;
    if (((check_mode() & 0x1) && asi != 0x1c)) {
    //if (1==2) {

        // extract index and tag from address
        unsigned int tag    = (address >> (m_idx_bits + m_offset_bits));
        unsigned int idx    = ((address << m_tagwidth) >> (m_tagwidth + m_offset_bits));
        unsigned int offset = ((address << (32 - m_offset_bits)) >> (32 - m_offset_bits));
        unsigned int byt    = (address & 0x3);

        bool is_hit = false;

        v::debug << this->name() << "WRITE ACCESS with idx: " << hex << idx
                << " tag: " << hex << tag << " offset: " << hex
                << offset << v::endl;

        // lookup all cachesets
        for (unsigned int i = 0; i <= m_sets; i++) {

            m_current_cacheline[i] = lookup(i, idx);

            //v::debug << this->name() <<  "Set :" << i << " atag: " << (*m_current_cacheline[i]).tag.atag << " valid: " << (*m_current_cacheline[i]).tag.valid << " entry: " << (*m_current_cacheline[i]).entry[offset>>2].i << v::endl;

            // Check the cache tag
            if ((*m_current_cacheline[i]).tag.atag == tag) {

	      //v::debug << this->name() << "Correct atag found in set " << i << v::endl;

                // Check the valid bits
	      if ((!m_new_linefetch_en && ((*m_current_cacheline[i]).tag.valid & offset2valid(offset, len)) == offset2valid(offset, len)) ||
		  (m_new_linefetch_en && ((*m_current_cacheline[i]).tag.valid & 0x1))) {

		v::debug << this->name() << "Cache Hit in Set " << i << " (valid: " << hex << (*m_current_cacheline[i]).tag.valid << " check mask: " << hex << offset2valid(offset, len) << ")" << v::endl;

		// update lru history
		if (m_repl == 1) {
		  lru_update(i);
		}
                    
		// update debug information
		CACHEWRITEHIT_SET(*debug, i);
		is_hit = true;

		if (len != 8) {
		  // write data to cache
		  for (unsigned int j = 0; j < len; j++) {
		    (*m_current_cacheline[i]).entry[offset >> 2].c[byt + j] = *(data + j);
		  }

		} else {
		  // is 64 bit

		  // write data to cache
		  for (unsigned int j = 0; j < 8; j++) {
		    (*m_current_cacheline[i]).entry[(offset+j) >> 2].c[(j % 4)] = *(data + j);
		  }
 		}

		// Increment hit counter
		whits[i]++;

		// valid is already set
		break;

	      } else {

		v::debug << this->name()
			 << "Tag Hit but data not valid in set " << i
			 << v::endl;

		// For 64bit access invalidate the upper word
		if (len == 8) {

		  if (((*m_current_cacheline[i]).tag.valid & offset2valid(offset+4)) != 0) {
                    
		    v::debug << this->name() << "64bit invalidate" << v::endl;
		    (*m_current_cacheline[i]).tag.valid &= ~offset2valid(offset+4);
		    
		  }
		}
	      }

            } else {

	      v::debug << this->name() << "Cache miss in set " << i << v::endl;
            }

            // increment time
            *delay += ((len - 1) >> 2)*clockcycle;

        }

        // update debug information
        if (!is_hit) {

            CACHEWRITEMISS_SET(*debug);
	    v::debug << name() << "ACCESS IS WRITEMISS " << hex << *debug << v::endl;
	    wmisses++;
	}
	    
        // write data to main memory
        // todo: - implement byte access
        //       - implement write buffer

        // The write buffer (WRB) consists of 3x32bit registers. It is used to temporarily
        // hold store data until it is sent to the destination device. For half-word
        // or byte stores, the data has to be properly aligned for writing to word-
        // addressed device, before writing the WRB.

        m_tlb_adaptor->mem_write(address, asi, data, len, delay, debug, is_dbg, is_lock, cacheable);

    } else {

        v::debug << this->name() << "BYPASS write to address: " << hex
                << address << v::endl;

	// increment bypass counter
	bypassops++;

        // cache is disabled
        // forward request to ahb interface (?? does it matter whether mmu is enabled or not ??)
        m_tlb_adaptor->mem_write(address, asi, data, len, delay, debug, is_dbg, is_lock, cacheable);

        // update debug information
        CACHEBYPASS_SET(*debug);

        *delay += clockcycle;
    }
}

// call to flush cache
void vectorcache::flush(sc_core::sc_time *t, unsigned int * debug, bool is_dbg) {

//    unsigned int addr;
//    bool cacheable = false;

    v::debug << name() << "Cache flush!" << v::endl;

    // for all cache sets
    for (unsigned int set = 0; set <= m_sets; set++) {

        // and all cache lines
        for (unsigned int line = 0; line < m_number_of_vectors; line++) {

            m_current_cacheline[set] = lookup(set, line);

            // it's a write-through cache. we should never need the following code
/*            // and all cache line entries
            for (unsigned int entry = 0; entry < m_wordsperline; entry++) {

                // check for valid data
                if ((*m_current_cacheline[set]).tag.valid & (1 << entry)) {

                    // construct address from tag
                    addr = ((*m_current_cacheline[set]).tag.atag << (m_idx_bits
                            + m_offset_bits));
                    addr |= (line << m_offset_bits);
                    addr |= (entry << 2);

                    v::debug << this->name() << "FLUSH set: " << set
                            << " line: " << line << " addr: " << hex
                            << addr << " data: " << hex
                            << (*m_current_cacheline[set]).entry[entry].i
                            << v::endl;

                    m_tlb_adaptor->mem_write(addr, 0x8,(unsigned char *)&(*m_current_cacheline[set]).entry[entry],
                                             4, t, debug, is_dbg, cacheable, false);

                }
            } */
            // invalidate all entries
            (*m_current_cacheline[set]).tag.valid = 0;   
        }
    }

    // Update debug information
    CACHEFLUSH_SET(*debug);

    v::debug << name() << "FLUSH SET DEBUG: " << hex << *debug << v::endl;
    v::debug << name() << "cache flush" << v::endl;
}

// ------------------------------
// About diagnostic cache access:
// ------------------------------
// Tags and data in the instruction and data cache can be accessed throuch ASI address space
// 0xC, 0xD, 0xE and 0xf by executing LDA and STA instructions. Address bits making up the cache
// offset will be used to index the tag to be accessed while the least significant bits of the
// bits making up the address tag will be used to index the cache set.
//
// In multi-way caches, the address of the tags and data of the ways are concatenated. The address
// of a tag or data is thus:
//
// ADDRESS = WAY & LINE & DATA & "00"
//
// Example: the tag for line 2 in way 1 of a 2x4 Kbyte cache with 16 byte line would be.
//
// A[13:12] = 1 (WAY); A[11:5] = 2 (TAG) -> TAG Address = 0x1040

// read data cache tags (ASI 0xe)
// -------------------------------------
// Diagnostic read of tags is possible by executing an LDA instruction with ASI = 0xC for
// instruction cache tags and ASI = 0xe for data cache tags. A cache line and set are indexed by
// the address bits making up the cache offset and the least significant bits of the address bits
// making up the address tag.

void vectorcache::read_cache_tag(unsigned int address, unsigned int * data,
                                 sc_core::sc_time *t) {

    unsigned int tmp;

    unsigned int set = ((address >> (m_idx_bits + m_offset_bits)) & 0x3);
    unsigned int idx = ((address << (32 - (m_idx_bits + m_offset_bits))) >> (32 - m_idx_bits));

    // find the required cache line
    m_current_cacheline[set] = lookup(set, idx);

    v::debug << name() << "ATAG: " << hex << (*m_current_cacheline[set]).tag.atag << v::endl;

    // build bitmask from tag fields
    // (! The atag field starts bit 10. It is not MSB aligned as in the actual tag layout.)
    tmp = (*m_current_cacheline[set]).tag.atag << 10;
    tmp |= (*m_current_cacheline[set]).tag.lrr << 9;
    tmp |= (*m_current_cacheline[set]).tag.lock << 8;
    tmp |= (*m_current_cacheline[set]).tag.valid;

    v::debug << this->name() << "Diagnostic tag read set: " << hex << set
	     << " idx: " << hex << idx << " - tag: " << hex << v::setw(8) << tmp
             << v::endl;

    #ifdef LITTLE_ENDIAN_BO
    swap_Endianess(tmp);
    #endif

    // handover bitmask pointer (the tag)
    *data = tmp;

    // increment time
    *t += clockcycle;

}

// write data cache tags (ASI 0xe)
// --------------------------------------
// The tags can be directly written by executing a STA instruction with ASI = 0xC for the instruction
// cache tags and ASI = 0xE for the data cache tags. The cache line and set are indexed by the
// address bits making up the cache offset and the least significant bits of the address bits making
// up the address tag. D[31:10] is written into the ATAG field and the valid bits are written with
// the D[7:0] of the write data. Bit D[9] is written into the LRR bit (if enabled) and D[8] is
// written into the lock bit (if enabled).
void vectorcache::write_cache_tag(unsigned int address, unsigned int * data,
                                  sc_core::sc_time *t) {

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(*data);
  #endif

  unsigned int set = ((address >> (m_idx_bits + m_offset_bits)) & 0x3);
  unsigned int idx = ((address << (32 - (m_idx_bits + m_offset_bits))) >> (32 - m_idx_bits));

  // find the required cache line
  m_current_cacheline[set] = lookup(set, idx);

  // update the tag with write data
  // (! The atag field is expected to start at bit 10. Not MSB aligned as in tag layout.)
  (*m_current_cacheline[set]).tag.atag = *data >> 10;
  (*m_current_cacheline[set]).tag.lrr = ((*data & 0x100) >> 9);
  // lock bit can only be set, if line locking is enabled
  // locking only works in multi-set configurations. the last set may never be locked.
  (*m_current_cacheline[set]).tag.lock
          = ((m_setlock) && (set != m_sets))? ((*data & 0x100) >> 8) : 0;
  (*m_current_cacheline[set]).tag.valid = (*data & 0xff);

  v::debug  << this->name() << "Diagnostic tag write set: " << hex << set
           << " idx: " << hex << idx << " atag: " << hex
           << (*m_current_cacheline[set]).tag.atag << " lrr: " << hex
           << (*m_current_cacheline[set]).tag.lrr << " lock: " << hex
           << (*m_current_cacheline[set]).tag.lock << " valid: " << hex
           << (*m_current_cacheline[set]).tag.valid << v::endl;

  // increment time
  *t += clockcycle;

}

// read data cache entry/data (ASI 0xf)
// -------------------------------------------
// Similar to instruction tag read, a data sub-block may be read by executing an LDA instruction
// with ASI = 0xD for instruction cache data and ASI = 0xF for data cache data.
// The sub-block to be read in the indexed cache line and set is selected by A[4:2].
void vectorcache::read_cache_entry(unsigned int address, unsigned int * data,
                                   sc_core::sc_time *t) {

  unsigned int set = ((address >> (m_idx_bits + m_offset_bits)) & 0x3);
  unsigned int idx = ((address << (32 - (m_idx_bits + m_offset_bits))) >> (32 - m_idx_bits));
  unsigned int sb = (address << (32 - m_offset_bits) >> (34 - m_offset_bits));

  // find the required cache line
  m_current_cacheline[set] = lookup(set, idx);

  *data = (*m_current_cacheline[set]).entry[sb].i;

  v::debug << this->name() << "Diagnostic data read set: " << hex << set
           << " idx: " << hex << idx << " sub-block: " << sb
           << " - data: " << hex << *data << v::endl;

  // increment time
  *t += clockcycle;

}

/// write data cache entry/data (ASI 0xd)
// --------------------------------------------
// A data sub-block can be directly written by executing a STA instruction with ASI = 0xD for the
// instruction cache data and ASI = 0xF for the data cache data. The sub-block to be read in
// indexed cache line and set is selected by A[4:2].
void vectorcache::write_cache_entry(unsigned int address, unsigned int * data,
                                    sc_core::sc_time *t) {

    unsigned int set = ((address >> (m_idx_bits + m_offset_bits)) & 0x3);
    unsigned int idx = ((address << (32 - (m_idx_bits + m_offset_bits))) >> (32 - m_idx_bits));
    unsigned int sb = (address << (32 - m_offset_bits) >> (34 - m_offset_bits));

    // find the required cache line
    m_current_cacheline[set] = lookup(set, idx);

    (*m_current_cacheline[set]).entry[sb].i = *data;

    v::debug << this->name() << "Diagnostic data write set: " << hex << set
            << " idx: " << hex << idx << " sub-block: " << sb
            << " - data: " << hex << *data << v::endl;

    // increment time
    *t += clockcycle;

}

// read cache configuration register
unsigned int vectorcache::read_config_reg(sc_core::sc_time *t) {

  unsigned int tmp = CACHE_CONFIG_REG;
  v::debug << this->name() << "read_config_reg 0x" << tmp <<  v::endl;

  *t += clockcycle;

  #ifdef LITTLE_ENDIAN_BO
  swap_Endianess(tmp);
  #endif

  return (tmp);

}

// internal behavioral functions
// -----------------------------

/// reads a cache line from a cache set
inline t_cache_line * vectorcache::lookup(unsigned int set, unsigned int idx) {

  if ((set <= m_sets) && (idx <= m_number_of_vectors)) {

    // return the cache line from the selected set
    return (&(*cache_mem[set])[idx]);

  } else {

    v::error << name() << "Parameters of cache lookup not valid - set: " << set << " (allowed 0 - " << m_sets << "), idx: " \
             << idx << " (allowed 0 - " << m_number_of_vectors << ")" << v::endl;

    memset(&m_default_cacheline, 0, sizeof(t_cache_line));
    return (&m_default_cacheline);

  }

}

/// select cache line to be replaced according to replacement policy
unsigned int vectorcache::replacement_selector(unsigned int mode) {

    unsigned int set_select = 0;
    int min_lru;

    // LRU - least recently used
    switch (mode) {

        // direct mapped
        case 0:

            // There is only one set
            set_select = 0;
            break;

            // LRU - least recently used
        case 1:

            // LRU replaces the line, which hasn't been used for the longest time.

            // The LRU algorithm needs extra "flip-flops"
            // per cache line to store access history.
            // Within the TLM model the LRU bits will be
            // attached to tag ram.

            // Find the cache line with the lowest LRU value
            min_lru = m_max_lru;

            for (unsigned int i = 0; i <= m_sets; i++) {

                v::debug << this->name() << "LRU Replacer Check Set: " << i
                        << v::endl;

                // the last set will never be locked
                if (((*m_current_cacheline[i]).tag.lru <= min_lru)
                        && ((*m_current_cacheline[i]).tag.lock == 0)) {

                    v::debug << this->name() << "LRU Replacer Select Set: " << i
                            << v::endl;
                    min_lru = (*m_current_cacheline[i]).tag.lru;
                    set_select = i;

                }
            }

            break;

            // LRR - least recently replaced
        case 2:

            // The LRR algorithm uses one extra bit in tag rams
            // to store replacement history. Supossed to work
            // only for 2-way associative caches.

            // default: set 1 (the highest) can not be locked
            set_select = 1;

            for (unsigned int i = 0; i < 2; i++) {

                v::debug << this->name() << "LRR Replacer Check Set: " << i
                        << v::endl;

                if (((*m_current_cacheline[i]).tag.lrr == 0)
                        && ((*m_current_cacheline[i]).tag.lock == 0)) {

                    v::debug << this->name() << "LRR Replacer Select Set: " << i
                            << v::endl;
                    set_select = i;
                    break;
                }
            }

            break;

            // pseudo - random
        default:

            // Random replacement is implemented through
            // modulo-N counter that selects the line to be
            // evicted on cache miss.

            do {

                set_select = m_pseudo_rand % (m_sets + 1);
                m_pseudo_rand++;

            }

            // check line lock: the last sets will never be locked!!
            while ((*m_current_cacheline[set_select]).tag.lock != 0);

    }

    return set_select;

}

/// LRR replacement history updater
void vectorcache::lrr_update(unsigned int set_select) {

    // ! LRR may only be used for 2-way
    for (unsigned int i = 0; i < 2; i++) {

        // switch on lrr bit off selected set, switch off the other one
        (*m_current_cacheline[i]).tag.lrr = (i == set_select)? 1 : 0;
        v::debug << this->name() << "Set " << i << " lrr: "
                << (*m_current_cacheline[i]).tag.lrr << v::endl;

    }

}

/// LRU replacement history updater
void vectorcache::lru_update(unsigned int set_select) {

  uint32_t tmp;

  for (unsigned int i = 0; i <= m_sets; i++) {

    // LRU: Counter for each line of a set
    // (2 way - 1 bit, 3 way - 3 bit, 4 way - 5 bit)

    tmp = (*m_current_cacheline[i]).tag.lru;

    if (i == set_select) {

      (*m_current_cacheline[i]).tag.lru = m_max_lru;

    } else {

      if (tmp > 0) {

        (*m_current_cacheline[i]).tag.lru--;

      }
    }

    v::debug << this->name() << "Set " << i << " lru: "
                << (*m_current_cacheline[i]).tag.lru << v::endl;

  }

}

/// Snooping function (invalidates cache lines)
void vectorcache::snoop_invalidate(const t_snoop& snoop, const sc_core::sc_time& delay) {

  unsigned int address;
  unsigned int tag;
  unsigned int idx;
  unsigned int offset;

  // Is the cache enabled
  if ((check_mode() & 0x3) == 0x3) {

    for (address = snoop.address; address < snoop.address + snoop.length; address+=4) {

      // Extract index and tag from address
      tag    = (address >> (m_idx_bits + m_offset_bits));
      idx    = ((address << m_tagwidth) >> (m_tagwidth + m_offset_bits));
      offset = ((address << (32 - m_offset_bits)) >> (32 - m_offset_bits));

      // Lookup all cachesets
      for (unsigned int i = 0; i <= m_sets; i++) {

        m_snoop_cacheline = lookup(i, idx);

        // Check the cache tag
        if (((*m_snoop_cacheline).tag.atag) == tag) {

          // Delete the valid bit
          ((*m_snoop_cacheline).tag.valid &= (~offset2valid(offset)));
        }
      }
    }
  }
}

// debug and helper functions
// --------------------------

// displays cache lines at stdout for debug
void vectorcache::dbg_out(unsigned int line) {

  #ifdef LITTLE_ENDIAN_BO

  swap_Endianess(line);

  #endif

  t_cache_line dbg_cacheline;

  for (unsigned int i = 0; i <= m_sets; i++) {

    // read the cacheline from set
    dbg_cacheline = (*cache_mem[i])[line];

    // display the tag
    std::cout << "SET: " << i << " ATAG: 0x" << hex
              << dbg_cacheline.tag.atag << " VALID: 0x" << hex
              << dbg_cacheline.tag.valid << v::endl;

    std::cout << "Byte:       0 1 2 3" << v::endl;
    std::cout << "-------------------" << v::endl;

    // display all entries
    for (unsigned int j = 0; j < m_wordsperline; j++) {

      std::cout << "Entry: " << j << " - ";

      for (unsigned int k = 0; k < 4; k++) {

	std::cout << hex << std::setw(2) << (unsigned int)dbg_cacheline.entry[j].c[k];

      }

      std::cout << " " << std::endl;
    }
  }
}

// Transforms a cache-line offset into a valid mask
inline unsigned int vectorcache::offset2valid(unsigned int offset, unsigned int len) {

  if (len != 8) {

    switch(offset>>2) {

    case 0x0: return 0x01;
    case 0x1: return 0x02;
    case 0x2: return 0x04;
    case 0x3: return 0x08;
    case 0x4: return 0x10;
    case 0x5: return 0x20;
    case 0x6: return 0x40;
    case 0x7: return 0x80;
    default: v::warn << name() << "Odd offset for calculation of valid mask!" << v::endl;
      return 0x00;

    }

  } else {

    switch(offset>>2) {

    case 0x0: return 0x03;
    case 0x2: return 0x0c;
    case 0x4: return 0x30;
    case 0x6: return 0xc0;
    default: v::warn << name() << "Odd offset for calculation of valid mask (dword)!" << v::endl;
      return 0x00;
    }
  }
}

// Print execution statistic at end of simulation
void vectorcache::end_of_simulation() {

  uint64_t total_rhits = 0;
  uint64_t total_whits = 0;

  v::report << name() << " ******************************************** " << v::endl;
  v::report << name() << " * Caching statistic:                        " << v::endl;
  v::report << name() << " * -------------------" << v::endl;

  for (uint32_t i=0;i<=m_sets;i++) {

    v::report << name() << " * Read hits set" << i << ": " << rhits[i] << v::endl;

    total_rhits += rhits[i];

  }

  v::report << name() << " * Total Read Hits: " << total_rhits << v::endl;
  v::report << name() << " * Read Misses:  " << rmisses << v::endl;

  // avoid / 0
  if (total_rhits+rmisses != 0) {

    v::report << name() << " * Read Hit Rate: " << (double)total_rhits/(double)(total_rhits+rmisses) << v::endl;

  }

  for (uint32_t i=0;i<=m_sets;i++) {

    v::report << name() << " * Write hits set" << i << ": " << whits[i] << v::endl;

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

}

// Helper for setting clock cycle latency using sc_clock argument
void vectorcache::clkcng(sc_core::sc_time &clk) {
  clockcycle = clk;
}
/// @}
