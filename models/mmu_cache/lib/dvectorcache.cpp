/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       dvectorcache.cpp - Implementation of a data             */
/*             cache. The cache can be configured direct mapped or     */
/*             set associative. Set-size, line-size and replacement    */
/*             strategy can be defined through constructor arguments.  */
/*                                                                     */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#include "dvectorcache.h"

// constructor
// args: sysc module name, pointer to AHB read/write methods (of parent), delay on read hit, delay on read miss (incr), number of sets, setsize in kb, linesize in b, replacement strategy  
dvectorcache::dvectorcache(sc_core::sc_module_name name, 
			   mmu_cache_if &_parent, 
			   sc_core::sc_time dcache_hit_read_response_delay, 
			   sc_core::sc_time dcache_miss_read_response_delay, 
			   sc_core::sc_time dcache_write_response_delay,
			   int sets, 
			   int setsize, 
			   int linesize, 
			   int repl) 
  : sc_module(name),
    m_dcache_hit_read_response_delay(dcache_hit_read_response_delay),
    m_dcache_miss_read_response_delay(dcache_miss_read_response_delay),
    m_dcache_write_response_delay(dcache_write_response_delay)
  
{

  // initialize cache line allocator
  m_default_cacheline.tag.atag    = 0;
  m_default_cacheline.tag.lrr     = 0;
  m_default_cacheline.tag.lock    = 0;
  m_default_cacheline.tag.valid   = 0;
  m_default_cacheline.entry[0].i  = 0;
  m_default_cacheline.entry[1].i  = 0;
  m_default_cacheline.entry[2].i  = 0;
  m_default_cacheline.entry[3].i  = 0;

  m_sets              = sets;                                         // number of cache sets
  m_setsize           = setsize;                                      // cache-set size in kB
  m_linesize          = linesize;                                     // bytes per cache line
  m_offset_bits       = (unsigned int)log2(linesize);                 // number of bits used for addressing members of the cacheline
  m_number_of_vectors = (m_setsize << 10) / linesize;                 // number of elements/vectors in cache
  m_idx_bits          = (unsigned int)log2(m_number_of_vectors);      // width of address index
  m_tagwidth          = 32-m_idx_bits-m_offset_bits;                  // width of cache tag
  m_repl               = repl;                                        // replacment strategy: repl or random

  // create the cache sets
  for (unsigned int i=0; i < m_sets; i++) {

    DUMP(this->name(),"Create cache set " << i);
    vector<t_cache_line> *cache_set = new vector<t_cache_line>(m_number_of_vectors , m_default_cacheline);

    cache_mem.push_back(cache_set);

    // create one cache_line struct per set
    t_cache_line *current_cacheline = new t_cache_line;
    m_current_cacheline.push_back(current_cacheline);
  }

  cout << " ******************************************************************************* " << endl;
  cout << " * Created cache memory with following parameters:                               " << endl;
  cout << " * number of cache sets " << m_sets << endl;
  cout << " * size of each cache set " << m_setsize << " kb" << endl;
  cout << " * bytes per line " << m_linesize << " (offset bits: " << m_offset_bits << ")" << endl;
  cout << " * number of cache lines per set " << m_number_of_vectors << " (index bits: " << m_idx_bits << ")" << endl;
  cout << " * Width of cache tag in bits " << m_tagwidth << endl;
  cout << " ******************************************************************************* " << endl;

  // hook up to top level (amba if)
  m_parent = &_parent;

}

// destructor
dvectorcache::~dvectorcache() {

}

// external interface functions
// ----------------------------

// read from cache
void dvectorcache::read(unsigned int address, unsigned int *data, sc_core::sc_time *t) {

  int set_select = -1;
  int cache_hit = -1;

  // extract index and tag from address
  unsigned int tag    = (address >> (m_idx_bits+m_offset_bits));
  unsigned int idx    = ((address << m_tagwidth) >> (m_tagwidth+m_offset_bits));
  unsigned int offset = ((address << (32-m_offset_bits)) >> (32-m_offset_bits));

  DUMP(name(),"READ ACCESS with idx: " << hex << idx << " tag: " << hex << tag << " offset: " << hex << offset);

  // lookup all cachesets
  for (unsigned int i=0; i < m_sets; i++){

    m_current_cacheline[i] = lookup(i, idx);

    //DUMP(name(), "Set :" << i << " atag: " << (*m_current_cacheline[i]).tag.atag << " valid: " << (*m_current_cacheline[i]).tag.valid << " entry: " << (*m_current_cacheline[i]).entry[offset>>2].i);

    // check the cache tag
    if ((*m_current_cacheline[i]).tag.atag == tag) {

      //cout << "Correct atag found in set " << i << endl;
     
      // check the valid bit (math.h pow is mapped to the coproc, hence it should be pretty fast)
      if ((*m_current_cacheline[i]).tag.valid & (unsigned int)(pow((double)2,(double)(offset >> 2))) != 0) {

	DUMP(name(),"Cache Hit in Set " << i);
	
	// write data pointer
	*data = (*m_current_cacheline[i]).entry[offset >> 2].i;
	
	// increment time
	*t+=m_dcache_hit_read_response_delay;


	// valid data in set i
	cache_hit = i;
	break;
      }	
      else {
	
	DUMP(name(),"Tag Hit but data not valid in set " << i);
      }

    }
    else {
      
      DUMP(name(),"Cache miss in set " << i);

    }
  }

  // in case no matching tag was found or data is not valid:
  // -------------------------------------------------------
  // read miss - On a data cache read miss to a cachable location 4 bytes of data
  // are loaded into the cache from main memory.
  if (cache_hit==-1) {

    // increment time
    *t += m_dcache_miss_read_response_delay; 

    // read from main memory
    m_parent->amba_read(address, data, 4);
    DUMP(name(),"Received data from main memory " << hex << *data);

    // !!!! The replacement mechanism still needs to be verified.
    // This is only a first shot.

    // check for unvalid data which can be replaced without harm
    for (unsigned int i=0; i<m_sets; i++){

      if (((*m_current_cacheline[i]).tag.valid & (unsigned int)(pow((double)2,(double)(offset >> 2)))) == 0) {

	// select unvalid data for replacement
	set_select = i;
	DUMP(name(), "Set " << set_select << " has no valid data - will use for refill.");
	break;
      }
    }

    // in case there is no free set anymore
    if (set_select == -1) {

      // select set according to replacement strategy
      set_select = replacement_selector(m_repl);
      DUMP(name(),"Set " << set_select << " selected for refill by replacement selector.");
      
    }

    //cout << "select set " << set_select << " for refill" << endl;

    //cout << "old entry: " << hex << (*m_current_cacheline[set_select]).entry[offset >> 2].i << " old valid bits: " << hex << (*m_current_cacheline[set_select]).tag.valid << endl;

    // fill in the new data
    (*m_current_cacheline[set_select]).entry[offset >> 2].i = *data;

    // fill in the new atag
    (*m_current_cacheline[set_select]).tag.atag  = tag;
 
    // switch on the valid bit
    (*m_current_cacheline[set_select]).tag.valid |= (unsigned int)(pow((double)2,(double)(offset >> 2)));

    //DUMP(name(),"Updated entry: " << hex << (*m_current_cacheline[set_select]).entry[offset >> 2].i << " valid bits: " << hex << (*m_current_cacheline[set_select]).tag.valid);

   }
}

// write to/through cache:
// -----------------------
// The write policy for stores is write-through with no-allocate on write miss.
// - on hits it writes to cache and main memory;
// - on misses it updates the block in main memory not bringing that block to the cache;
//   Subsequent writes to the block will update main memory because Write Through policy is employed. 
//   So, some time is saved not bringing the block in the cache on a miss because it appears useless anyway.

void dvectorcache::write(unsigned int address, unsigned int * data, unsigned int *byt, sc_core::sc_time * t) {

  // extract index and tag from address
  unsigned int tag    = (address >> (m_idx_bits+m_offset_bits));
  unsigned int idx    = ((address << m_tagwidth) >> (m_tagwidth+m_offset_bits));
  unsigned int offset = ((address << (32-m_offset_bits)) >> (32-m_offset_bits));

  DUMP(name(),"WRITE ACCESS with idx: " << hex << idx << " tag: " << hex << tag << " offset: " << hex << offset);

  // lookup all cachesets
  for (unsigned int i=0; i < m_sets; i++){

    m_current_cacheline[i] = lookup(i, idx);

    //DUMP(name(), "Set :" << i << " atag: " << (*m_current_cacheline[i]).tag.atag << " valid: " << (*m_current_cacheline[i]).tag.valid << " entry: " << (*m_current_cacheline[i]).entry[offset>>2].i);

    // check the cache tag
    if ((*m_current_cacheline[i]).tag.atag == tag) {

      //cout << "Correct atag found in set " << i << endl;
     
      // check the valid bit (math.h pow is mapped to the coproc, hence it should be pretty fast)
      if ((*m_current_cacheline[i]).tag.valid & (unsigned int)(pow((double)2,(double)(offset >> 2))) != 0) {

	DUMP(name(),"Cache Hit in Set " << i);
	
	// write data to cache (todo: impl. byte access)
	(*m_current_cacheline[i]).entry[offset >> 2].i = *data;
	
	// valid bit is already set
	
	// increment time
	*t+=m_dcache_write_response_delay;

	break;
      }	
      else {
	
	DUMP(name(),"Tag Hit but data not valid in set " << i);
      }

    }
    else {
      
      DUMP(name(),"Cache miss in set " << i);

    }
  }

  // write data to main memory
  // todo: - implement byte access
  //       - implement write buffer

  // The write buffer (WRB) consists of 3x32bit registers. It is used to temporarily
  // hold store data until it is sent to the destination device. For half-word
  // or byte stores, the data has to be properly aligned for writing to word-
  // addressed device, before writing the WRB.
  m_parent->amba_write(address, data, 4);

} 
unsigned int dvectorcache::replacement_selector(unsigned int mode) {
  
  // random replacement
  if (mode == 0) {

    // todo: check RTL for implementation details
    return(rand() % m_sets);
  } 
  else {

    cout << "LRU not implemented yet!!" << endl;
  }

  return 0;
}


// call to flush cache
void dvectorcache::flush(sc_core::sc_time *t) {
}

// internal behavioral functions
// -----------------------------

// reads a cache line from a cache set
t_cache_line * dvectorcache::lookup(unsigned int set, unsigned int idx) {

  // return the cache line from the selected set
  return (&(*cache_mem[set])[idx]);

}

// debug and helper functions
// --------------------------

// displays cache lines at stdout for debug
void dvectorcache::dbg_read(unsigned int set, unsigned int start_idx, unsigned int number_of_entries) {

  t_cache_line dbg_cacheline;

  cout << " ******************************************************************************************************* " << endl;
  cout << " dbg_read(set=" << set << ", start_idx=" << start_idx << ", number_of_entries=" << number_of_entries << endl << endl;

  // for all selected indices do
  for (unsigned int i = start_idx; i < (start_idx + number_of_entries); i++) {

    // read the cacheline from set
    dbg_cacheline = (*cache_mem[set])[i];

    // display the tag 
    cout << "ATAG: 0x" << hex << dbg_cacheline.tag.atag << " VALID: 0x" << hex << dbg_cacheline.tag.valid;

    // display all entries
    for (unsigned int j = 0; j < (m_linesize >> 2); j++) {

      cout << "0x" << hex << dbg_cacheline.entry[j].i;

    }

    // line break
    cout << endl;

  }
}
  


