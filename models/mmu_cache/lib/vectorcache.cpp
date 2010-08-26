// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       vectorcache.h - Class definition of a virtual cache     *
// *             model. The cache can be configured direct mapped or     *
// *             set associative. Set-size, line-size and replacement    *
// *             strategy can be defined through constructor arguments.  *
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision $                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************

#include "vectorcache.h"

// constructor
// args: sysc module name, pointer to AHB read/write methods (of parent), delay on read hit, delay on read miss (incr), number of sets, setsize in kb, linesize in b, replacement strategy  
vectorcache::vectorcache(sc_core::sc_module_name name, 
			   mmu_cache_if * _mmu_cache,
			   mem_if *_tlb_adaptor,
			   unsigned int mmu_en,
			   unsigned int burst_en,
			   sc_core::sc_time hit_read_response_delay, 
			   sc_core::sc_time miss_read_response_delay, 
			   sc_core::sc_time write_response_delay,
			   unsigned int sets, 
			   unsigned int setsize, 
			   unsigned int setlock,
			   unsigned int linesize, 
			   unsigned int repl,
			   unsigned int lram,
			   unsigned int lramstart,
			   unsigned int lramsize) : sc_module(name),
						    m_mmu_cache(_mmu_cache),
						    m_tlb_adaptor(_tlb_adaptor),
						    m_burst_en(burst_en),
						    m_pseudo_rand(0),
						    m_sets(sets),
						    m_setsize(setsize),
						    m_setlock(setlock),
						    m_linesize(linesize),
						    m_wordsperline((unsigned int)pow(2,linesize)),
						    m_bytesperline((unsigned int)pow(2,linesize+2)),		    
						    m_offset_bits(linesize + 2),
						    m_number_of_vectors((unsigned int)pow(2,setsize+8-linesize)),
						    m_idx_bits(setsize+8-linesize),
						    m_tagwidth(32-m_idx_bits-m_offset_bits),
						    m_repl(repl),
						    m_mmu_en(mmu_en),
						    m_lram(lram),
						    m_lramstart(lramstart),
						    m_lramsize(lramsize),
						    m_hit_read_response_delay(hit_read_response_delay),
						    m_miss_read_response_delay(miss_read_response_delay),
						    m_write_response_delay(write_response_delay)
  
{

  // initialize cache line allocator
  memset(&m_default_cacheline, 0, sizeof(t_cache_line));

  // create the cache sets
  for (unsigned int i=0; i <= m_sets; i++) {

    DUMP(this->name(),"Create cache set " << i);
    std::vector<t_cache_line> *cache_set = new std::vector<t_cache_line>(m_number_of_vectors , m_default_cacheline);

    cache_mem.push_back(cache_set);

    // create one cache_line struct per set
    t_cache_line *current_cacheline = new t_cache_line;
    m_current_cacheline.push_back(current_cacheline);
  }

  DUMP(this->name(), " ******************************************************************************* ");
  DUMP(this->name(), " * Created cache memory with following parameters:                               ");
  DUMP(this->name(), " * number of cache sets " << (m_sets+1));
  DUMP(this->name(), " * size of each cache set " << (unsigned int)pow(2,m_setsize) << " kb");
  DUMP(this->name(), " * bytes per line " << m_bytesperline << " (offset bits: " << m_offset_bits << ")");
  DUMP(this->name(), " * number of cache lines per set " << m_number_of_vectors << " (index bits: " << m_idx_bits << ")");
  DUMP(this->name(), " * Width of cache tag in bits " << m_tagwidth);
  DUMP(this->name(), " * Replacement strategy: " << m_repl);
  DUMP(this->name(), " * Line Locking: " << m_setlock);
  DUMP(this->name(), " ******************************************************************************* ");

  // lru counter saturation
  switch (m_sets) {

    case 1: m_max_lru = 1;
            break;
    case 2: m_max_lru = 7;
            break;
    case 3: m_max_lru = 31;
            break;
    default: m_max_lru = 0;
  }

  // set up configuration register
  CACHE_CONFIG_REG = (m_mmu_en << 3);
  // config register contains linesize in words
  CACHE_CONFIG_REG |= ((m_lramstart & 0xff) << 4);
  CACHE_CONFIG_REG |= ((m_lramsize & 0xf) << 12);
  CACHE_CONFIG_REG |= ((m_linesize & 0x7) << 16);
  CACHE_CONFIG_REG |= ((m_lram & 0x1) << 19);
  CACHE_CONFIG_REG |= ((m_setsize & 0xf) << 20);
  CACHE_CONFIG_REG |= ((m_sets & 0x7) << 24);
  CACHE_CONFIG_REG |= ((m_repl & 0x3) << 28);

}

// destructor
vectorcache::~vectorcache() {
}

// external interface functions
// ----------------------------

/// read from cache
void vectorcache::read(unsigned int address, unsigned char *data, unsigned int len, sc_core::sc_time *t, unsigned int * debug) {

  int set_select = -1;
  int cache_hit = -1;

  unsigned int burst_len;
  unsigned int replacer_limit;

  // todo: handle cached/uncached access
  unsigned int asi = 0;

  // is the cache enabled (0b11) or frozen (0b01)
  if (check_mode() & 0x1) {

    // extract index and tag from address
    unsigned int tag    = (address >> (m_idx_bits+m_offset_bits));
    unsigned int idx    = ((address << m_tagwidth) >> (m_tagwidth+m_offset_bits));
    unsigned int offset = ((address << (32-m_offset_bits)) >> (32-m_offset_bits));
    unsigned int byt    = (address & 0x3);

    // space for data to refill a cache line of maximum size
    unsigned char ahb_data[32];
  
    DUMP(this->name(),"READ ACCESS idx: " << std::hex << idx << " tag: " << std::hex << tag << " offset: " << std::hex << offset);

    // lookup all cachesets
    for (unsigned int i=0; i <= m_sets; i++){

      m_current_cacheline[i] = lookup(i, idx);

      //DUMP(this->name(), "Set :" << i << " atag: " << (*m_current_cacheline[i]).tag.atag << " valid: " << (*m_current_cacheline[i]).tag.valid << " entry: " << (*m_current_cacheline[i]).entry[offset>>2].i);

      // asi == 1 forces cache miss
      if (asi != 1) {

	// check the cache tag
	if ((*m_current_cacheline[i]).tag.atag == tag) {

	  //DUMP(this->name(), "Correct atag found in set " << i);
     
	  // check the valid bit (math.h pow is mapped to the coproc, hence it should be pretty fast)
	  if (((*m_current_cacheline[i]).tag.valid & (unsigned int)(pow((double)2,(double)(offset >> 2)))) != 0) {

	    DUMP(this->name(),"Cache Hit in Set " << i);
	  
	    // update debug information
	    CACHEREADHIT_SET(*debug,i);

	    // update lru history
	    if (m_repl==1) {lru_update(i);}

	    // write data pointer
	    memcpy(data, &(*m_current_cacheline[i]).entry[offset>>2].c[byt],len);
	    //for(unsigned int j=0; j<len; j++) { *(data+j) = (*m_current_cacheline[i]).entry[offset>>2].c[byt+j]; }
	
	    // increment time
	    *t+=m_hit_read_response_delay;

	    // valid data in set i
	    cache_hit = i;
	    break;
	  }	
	  else {
	
	    DUMP(this->name(),"Tag Hit but data not valid in set " << i);
	  }
	}
	else {
      
	  DUMP(this->name(),"Cache miss in set " << i);

	}
      }
      else {
      
	DUMP(this->name(),"ASI force cache miss");
    
      }
    }
  
    // in case no matching tag was found or data is not valid:
    // -------------------------------------------------------
    // read miss - On a data cache read miss to a cachable location 4 bytes of data
    // are loaded into the cache from main memory.
    if (cache_hit==-1) {

      // increment time
      *t += m_miss_read_response_delay; 

      // Set length of bus transfer depending on mode:
      // ---------------------------------------------
      // If burst fetch is enabled, the cache line is filled starting at the missed address
      // until the end of the line. At the same time the instructions are forwarded to the IU (todo AT ??).
      if (m_burst_en && (m_mmu_cache->read_ccr() & 0x10000)) {
	
	burst_len = m_bytesperline - ((offset >> 2) << 2);
	replacer_limit = m_bytesperline-4;

      } else {

	burst_len = 4;
	replacer_limit = offset;

      }

      m_tlb_adaptor->mem_read(((address >> 2) << 2), ahb_data, burst_len, t, debug);

      // check for unvalid data which can be replaced without harm
      for (unsigned int i = 0; i <= m_sets; i++){

	if (((*m_current_cacheline[i]).tag.valid & (unsigned int)(pow((double)2,(double)(offset >> 2)))) == 0) {

	  // select unvalid data for replacement
	  set_select = i;
	  DUMP(this->name(), "Set " << set_select << " has no valid data - will use for refill.");
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
	  DUMP(this->name(),"Set " << set_select << " selected for refill by replacement selector.");
      
	}

	// fill in the new data (always the complete word)
	memcpy(&(*m_current_cacheline[set_select]).entry[offset >> 2], ahb_data, burst_len);

	// has the tag changed?
	if ((*m_current_cacheline[set_select]).tag.atag != tag) {

	  // fill in the new tag
	  (*m_current_cacheline[set_select]).tag.atag  = tag;

	  // switch off all the valid bits ...
	  (*m_current_cacheline[set_select]).tag.valid = 0;

	  // .. and switch on the ones for the new entries
	  for (unsigned int i = offset; i <= replacer_limit; i+=4) {
	    
	    (*m_current_cacheline[set_select]).tag.valid |= (unsigned int)(pow((double)2,(double)(i >> 2)));

	  }

	  // reset lru
	  (*m_current_cacheline[set_select]).tag.lru = m_max_lru;

	  // update lrr history
	  if (m_repl==2) {lrr_update(set_select);};

	} else {
      
	  // switch on the valid bits for the new entries
	  for (unsigned int i = offset; i <= replacer_limit; i+=4) {
 
	    (*m_current_cacheline[set_select]).tag.valid |= (unsigned int)(pow((double)2,(double)(i >> 2)));

	  }
	}

      }
      else {
	
	// Cache is frozen !!
	
	// Purpose of a cache freeze is to prevent data that is in the cache from being evicted:
	// The new data will only be filled in as long there is unvalid data in one of the sets (set_select != -1)
	// && the new data does not change the atag, because this would invalidate all the other entries
	// in the line (tag.atag == tag).
	if ((set_select != -1) && ((*m_current_cacheline[set_select]).tag.atag == tag)) {

	  // fill in the new data (always the complete word)
	  memcpy(&(*m_current_cacheline[set_select]).entry[offset >> 2], ahb_data, burst_len);
	  
	  // switch on the valid bits for the new entries
	  for (unsigned int i = offset; i <= replacer_limit; i+=4) {

	    (*m_current_cacheline[set_select]).tag.valid |= (unsigned int)(pow((double)2,(double)(i >> 2)));

	  }
	    
	} else {

	  // update debug information
	  FROZENMISS_SET(*debug);

	}

      }

      // update debug information
      CACHEREADMISS_SET(*debug, set_select);

      // copy the data requested by the processor
      memcpy(data, ahb_data+byt, len);
      //for (unsigned int j=0; j<len; j++) { data[j] = ahb_data[byt+j]; };

      //DUMP(this->name(),"Updated entry: " << std::hex << (*m_current_cacheline[set_select]).entry[offset >> 2].i << " valid bits: " << std::hex << (*m_current_cacheline[set_select]).tag.valid);
    }

  } else {
    
    DUMP(this->name(),"BYPASS read from address: " << std::hex << address);

    // cache is disabled
    // forward request to ahb interface (?? does it matter whether mmu is enabled or not ??)
    m_mmu_cache->mem_read(address, data, len, t, debug);

    // update debug information
    CACHEBYPASS_SET(*debug);

  }
}

// write to/through cache:
// -----------------------
// The write policy for stores is write-through with no-allocate on write miss.
// - on hits it writes to cache and main memory;
// - on misses it updates the block in main memory not bringing that block to the cache;
//   Subsequent writes to the block will update main memory because Write Through policy is employed. 
//   So, some time is saved not bringing the block in the cache on a miss because it appears useless anyway.

void vectorcache::write(unsigned int address, unsigned char * data, unsigned int len, sc_core::sc_time * t, unsigned int * debug) {

  // is the cache enabled (0x11) or frozen (0x01)
  if (check_mode() & 0x1) {

    // extract index and tag from address
    unsigned int tag    = (address >> (m_idx_bits+m_offset_bits));
    unsigned int idx    = ((address << m_tagwidth) >> (m_tagwidth+m_offset_bits));
    unsigned int offset = ((address << (32-m_offset_bits)) >> (32-m_offset_bits));
    unsigned int byt    = (address & 0x3);

    bool is_hit = false;

    DUMP(this->name(),"WRITE ACCESS with idx: " << std::hex << idx << " tag: " << std::hex << tag << " offset: " << std::hex << offset);

    // lookup all cachesets
    for (unsigned int i = 0; i <= m_sets; i++){

      m_current_cacheline[i] = lookup(i, idx);

      //DUMP(this->name(), "Set :" << i << " atag: " << (*m_current_cacheline[i]).tag.atag << " valid: " << (*m_current_cacheline[i]).tag.valid << " entry: " << (*m_current_cacheline[i]).entry[offset>>2].i);

      // check the cache tag
      if ((*m_current_cacheline[i]).tag.atag == tag) {

	//DUMP(this->name(),"Correct atag found in set " << i);
     
	// check the valid bit (math.h pow is mapped to the coproc, hence it should be pretty fast)
	if ((*m_current_cacheline[i]).tag.valid & (unsigned int)(pow((double)2,(double)(offset >> 2))) != 0) {

	  DUMP(this->name(),"Cache Hit in Set " << i);
	
	  // update lru history
	  if (m_repl==1) { lru_update(i);}

	  // update debug information
	  CACHEWRITEHIT_SET(*debug, i);
	  is_hit = true;
	
	  // write data to cache
	  for(unsigned int j=0; j<len; j++) { (*m_current_cacheline[i]).entry[offset >> 2].c[byt+j] = *(data+j); }
	
	  // valid is already set
	
	  // increment time
	  *t+=m_write_response_delay;

	  break;
	}	
	else {
	
	  DUMP(this->name(),"Tag Hit but data not valid in set " << i);
	}

      }
      else {
      
	DUMP(this->name(),"Cache miss in set " << i);
      }
    }

    // update debug information
    if(!is_hit) CACHEWRITEMISS_SET(*debug);
    
    // write data to main memory
    // todo: - implement byte access
    //       - implement write buffer

    // The write buffer (WRB) consists of 3x32bit registers. It is used to temporarily
    // hold store data until it is sent to the destination device. For half-word
    // or byte stores, the data has to be properly aligned for writing to word-
    // addressed device, before writing the WRB.

    m_tlb_adaptor->mem_write(address, data, len, t, debug);

  } else {

    DUMP(this->name(),"BYPASS write to address: " << std::hex << address);

    // cache is disabled
    // forward request to ahb interface (?? does it matter whether mmu is enabled or not ??)
    m_mmu_cache->mem_write(address, data, len, t, debug);
    
    // update debug information
    CACHEBYPASS_SET(*debug); 
  }
}

// call to flush cache
void vectorcache::flush(sc_core::sc_time *t, unsigned int * debug) {

  unsigned int addr;

  // for all cache sets
  for (unsigned int set=0; set <= m_sets; set++){

    // and all cache lines
    for (unsigned int line=0; line<m_number_of_vectors; line++) { 
      
      m_current_cacheline[set] = lookup(set, line);

      // and all cache line entries
      for (unsigned int entry=0; entry < m_wordsperline; entry++) {

	// check for valid data
	if ((*m_current_cacheline[set]).tag.valid & (1 << entry)) {

	  // construct address from tag
	  addr = ((*m_current_cacheline[set]).tag.atag << (m_idx_bits+m_offset_bits));
	  addr |= (line << m_offset_bits);
	  addr |= (entry << 2);

	  DUMP(this->name(),"FLUSH set: " << set << " line: " << line << " addr: " << std::hex << addr << " data: " << std::hex << (*m_current_cacheline[set]).entry[entry].i);

	  m_tlb_adaptor->mem_write(addr, (unsigned char *)&(*m_current_cacheline[set]).entry[entry], 4, t, debug);

	}
      }
    }
  }
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

void vectorcache::read_cache_tag(unsigned int address, unsigned int * data, sc_core::sc_time *t) {

  unsigned int tmp;

  unsigned int set = (address >> (m_idx_bits + 5)); 
  unsigned int idx = ((address << (32 - (m_idx_bits + 5))) >> (32 - m_idx_bits));

  // find the required cache line
  m_current_cacheline[set] = lookup(set, idx);

  // build bitmask from tag fields
  // (! The atag field starts bit 10. It is not MSB aligned as in the actual tag layout.)
  tmp =  (*m_current_cacheline[set]).tag.atag << 10;
  tmp |= (*m_current_cacheline[set]).tag.lrr  << 9;
  tmp |= (*m_current_cacheline[set]).tag.lock << 8;
  tmp |= (*m_current_cacheline[set]).tag.valid;

  DUMP(this->name(),"Diagnostic tag read set: " << std::hex << set << " idx: " << std::hex << idx << " - tag: " << std::hex << tmp);

  // handover bitmask pointer (the tag)
  *data = tmp;

  // increment time
  *t+=m_hit_read_response_delay;

}


// write data cache tags (ASI 0xe)
// --------------------------------------
// The tags can be directly written by executing a STA instruction with ASI = 0xC for the instruction
// cache tags and ASI = 0xE for the data cache tags. The cache line and set are indexed by the 
// address bits making up the cache offset and the least significant bits of the address bits making
// up the address tag. D[31:10] is written into the ATAG field and the valid bits are written with
// the D[7:0] of the write data. Bit D[9] is written into the LRR bit (if enabled) and D[8] is
// written into the lock bit (if enabled). 
void vectorcache::write_cache_tag(unsigned int address, unsigned int * data, sc_core::sc_time *t) {

  unsigned int set = (address >> (m_idx_bits + 5)); 
  unsigned int idx = ((address << (32 - (m_idx_bits + 5))) >> (32 - m_idx_bits));

  // find the required cache line
  m_current_cacheline[set] = lookup(set, idx);

  // update the tag with write data
  // (! The atag field is expected to start at bit 10. Not MSB aligned as in tag layout.)
  (*m_current_cacheline[set]).tag.atag  = *data >> 10;
  (*m_current_cacheline[set]).tag.lrr   = ((*data & 0x100) >> 9);
  // lock bit can only be set, if line locking is enabled
  // locking only works in multi-set configurations. the last set may never be locked.
  (*m_current_cacheline[set]).tag.lock = ((m_setlock) && (set != m_sets)) ? ((*data & 0x080) >> 8) : 0;
  (*m_current_cacheline[set]).tag.valid = (*data & 0xff);

  DUMP(this->name(),"Diagnostic tag write set: " << std::hex << set << " idx: " << std::hex << idx << " atag: " 
       << std::hex << (*m_current_cacheline[set]).tag.atag << " lrr: " << std::hex << (*m_current_cacheline[set]).tag.lrr 
       << " lock: " << std::hex << (*m_current_cacheline[set]).tag.lock << " valid: " << std::hex << (*m_current_cacheline[set]).tag.valid);

  // increment time
  *t+=m_hit_read_response_delay;

}

// read data cache entry/data (ASI 0xf)
// -------------------------------------------
// Similar to instruction tag read, a data sub-block may be read by executing an LDA instruction
// with ASI = 0xD for instruction cache data and ASI = 0xF for data cache data.
// The sub-block to be read in the indexed cache line and set is selected by A[4:2].
void vectorcache::read_cache_entry(unsigned int address, unsigned int * data, sc_core::sc_time *t) {

  unsigned int set = (address >> (m_idx_bits + 5)); 
  unsigned int idx = ((address << (32 - (m_idx_bits + 5))) >> (32 - m_idx_bits));
  unsigned int sb  = (address & 0x1f) >> 2;

  // find the required cache line
  m_current_cacheline[set] = lookup(set, idx);

  *data = (*m_current_cacheline[set]).entry[sb].i;

  DUMP(this->name(),"Diagnostic data read set: " << std::hex << set << " idx: " << std::hex << idx
       << " sub-block: " << sb << " - data: " << std::hex << *data);

  // increment time
  *t+=m_hit_read_response_delay;

}

/// write data cache entry/data (ASI 0xd)
// --------------------------------------------
// A data sub-block can be directly written by executing a STA instruction with ASI = 0xD for the
// instruction cache data and ASI = 0xF for the data cache data. The sub-block to be read in 
// indexed cache line and set is selected by A[4:2]. 
void vectorcache::write_cache_entry(unsigned int address, unsigned int * data, sc_core::sc_time *t) {

  unsigned int set = (address >> (m_idx_bits + 5)); 
  unsigned int idx = ((address << (32 - (m_idx_bits + 5))) >> (32 - m_idx_bits));
  unsigned int sb  = (address & 0x1f) >> 2;

  // find the required cache line
  m_current_cacheline[set] = lookup(set, idx);

  (*m_current_cacheline[set]).entry[sb].i = *data;

  DUMP(this->name(),"Diagnostic data write set: " << std::hex << set << " idx: " << std::hex << idx
       << " sub-block: " << sb << " - data: " << std::hex << *data);

  // increment time
  *t+=m_hit_read_response_delay;

}

// read cache configuration register
unsigned int vectorcache::read_config_reg(sc_core::sc_time *t) {

  *t+=m_hit_read_response_delay;

  return(CACHE_CONFIG_REG);

}


// internal behavioral functions
// -----------------------------

/// reads a cache line from a cache set
inline t_cache_line * vectorcache::lookup(unsigned int set, unsigned int idx) {

  // return the cache line from the selected set
  return (&(*cache_mem[set])[idx]);

}

/// select cache line to be replaced according to replacement policy
unsigned int vectorcache::replacement_selector(unsigned int mode) {

  unsigned int set_select;
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

      // LRU replaces the line which hasn't been used for the longest time.

      // The LRU algorithm needs extra "flip-flops"
      // per cache line to store access history.
      // Within the TLM model the LRU bits will be 
      // attached to tag ram.

      // find the line with the lowest lru
      min_lru = m_max_lru;

      for(unsigned int i = 0; i <= m_sets; i++) {

	DUMP(this->name(),"LRU Replacer Check Set: " << i);

	// the last set will never be locked
	if (((*m_current_cacheline[i]).tag.lru <= min_lru) && ((*m_current_cacheline[i]).tag.lock == 0)) { 

	  DUMP(this->name(),"LRU Replacer Select Set: " << i);
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
      
      for(unsigned int i = 0; i < 2; i++) {

	DUMP(this->name(),"LRR Replacer Check Set: " << i);

	if (((*m_current_cacheline[i]).tag.lrr == 0) && ((*m_current_cacheline[i]).tag.lock == 0)) {

	  DUMP(this->name(),"LRR Replacer Select Set: " << i);
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
        m_pseudo_rand ++; 

      } 

      // check line lock: the last sets will never be locked!!
      while ((*m_current_cacheline[set_select]).tag.lock != 0);
	
  }


  return set_select;

}

/// LRR replacement history updater
void vectorcache::lrr_update(unsigned int set_select) {

  // ! LRR may only be used for 2-way
  for(unsigned int i = 0; i < 2; i++) {

    // switch on lrr bit off selected set, switch off the other one
    (*m_current_cacheline[i]).tag.lrr = (i == set_select) ? 1 : 0;
    DUMP(this->name(),"Set " << i << " lrr: " << (*m_current_cacheline[i]).tag.lrr);

  }

}

/// LRU replacement history updater
void vectorcache::lru_update(unsigned int set_select) {

  for(unsigned int i = 0; i <= m_sets; i++) {

    // LRU: Counter for each line of a set
    // (2 way - 1 bit, 3 way - 3 bit, 4 way - 5 bit)
    (*m_current_cacheline[i]).tag.lru = (i == set_select) ? m_max_lru : (*m_current_cacheline[i]).tag.lru - 1;
    DUMP(this->name(),"Set " << i << " lru: " << (*m_current_cacheline[i]).tag.lru);

  }
}

// debug and helper functions
// --------------------------

// displays cache lines at stdout for debug
void vectorcache::dbg_out(unsigned int line) {

  t_cache_line dbg_cacheline;

  for(unsigned int i = 0; i <= m_sets;i++) {

    // read the cacheline from set
    dbg_cacheline = (*cache_mem[i])[line];

    // display the tag 
    DUMP(this->name(), "SET: " << i << " ATAG: 0x" << std::hex << dbg_cacheline.tag.atag << " VALID: 0x" << std::hex << dbg_cacheline.tag.valid);

    // display all entries
    for (unsigned int j = 0; j < m_wordsperline; j++) {

      DUMP(this->name(), "Entry: " << j << " - " << std::hex << dbg_cacheline.entry[j].i);

    }
  }
}
  
