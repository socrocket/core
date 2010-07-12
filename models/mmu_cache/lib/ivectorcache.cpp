/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       ivectorcache.cpp - Implementation of an instruction     */
/*             cache. The cache can be configured direct mapped or     */
/*             set associative. Set-size, line-size and replacement    */
/*             strategy can be defined through constructor arguments.  */
/*                                                                     */
/*                                                                     */
/* Modified on $Date$                                                  */
/*          at $Revision$                                              */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#include "ivectorcache.h"

// constructor args: 
// - sysc module name 
// - pointer to AHB read/write methods (of parent)
// - pointer to MMU interface functions
// - mmu enabled
// - delay on read hit
// - delay on read miss (incr)
// - number of sets 
// - setsize in kb
// - linesize in bytes
// - replacement strategy  
ivectorcache::ivectorcache(sc_core::sc_module_name name,
			   mmu_cache_if &_parent, 
			   mmu_if * _mmu, 
			   int mmu_en, 
			   sc_core::sc_time icache_hit_read_response_delay, 
			   sc_core::sc_time icache_miss_read_response_delay, 
			   int sets, 
			   int setsize, 
			   int linesize, 
			   int repl) : sc_module(name),
				       m_sets(sets),
				       m_setsize(setsize),
				       m_linesize(linesize),
				       m_offset_bits((unsigned int)log2(linesize)),
				       m_number_of_vectors((m_setsize << 10) / linesize),
				       m_idx_bits((unsigned int)log2(m_number_of_vectors)),
				       m_tagwidth(32-m_idx_bits-m_offset_bits),
				       m_repl(repl),
				       m_mmu_en(mmu_en),
				       m_icache_hit_read_response_delay(icache_hit_read_response_delay),
				       m_icache_miss_read_response_delay(icache_miss_read_response_delay)
{

  // initialize cache line allocator
  memset(&m_default_cacheline, 0, sizeof(t_cache_line));

  // create the cache sets
  for (unsigned int i=0; i < m_sets; i++) {

    DUMP(this->name(),"Create cache set " << i);
    std::vector<t_cache_line> *cache_set = new std::vector<t_cache_line>(m_number_of_vectors , m_default_cacheline);

    cache_mem.push_back(cache_set);

    // create one cache_line struct per set
    t_cache_line *current_cacheline = new t_cache_line;
    m_current_cacheline.push_back(current_cacheline);
  }

  DUMP(this->name(), " ******************************************************************************* ");
  DUMP(this->name(), " * Created cache memory with following parameters:                               ");
  DUMP(this->name(), " * number of cache sets " << m_sets);
  DUMP(this->name(), " * size of each cache set " << m_setsize << " kb");
  DUMP(this->name(), " * bytes per line " << m_linesize << " (offset bits: " << m_offset_bits << ")");
  DUMP(this->name(), " * number of cache lines per set " << m_number_of_vectors << " (index bits: " << m_idx_bits << ")");
  DUMP(this->name(), " * Width of cache tag in bits " << m_tagwidth);
  DUMP(this->name(), " ******************************************************************************* ");

  // hook up to top level (amba if)
  m_parent = &_parent;

  // hook up to mmu
  m_mmu = _mmu;

}

// destructor
ivectorcache::~ivectorcache() {

}

// external interface functions
// ----------------------------

// call to read from cache
void ivectorcache::read(unsigned int address, unsigned int *data, sc_core::sc_time *t) {

  int set_select = -1;
  int cache_hit = -1;

  // extract index and tag from address
  unsigned int tag    = (address >> (m_idx_bits+m_offset_bits));
  unsigned int idx    = ((address << m_tagwidth) >> (m_tagwidth+m_offset_bits));
  unsigned int offset = ((address << (32-m_offset_bits)) >> (32-m_offset_bits));

  DUMP(this->name(),"ACCESS with idx: " << std::hex << idx << " tag: " << std::hex << tag << " offset: " << std::hex << offset);

  // lookup all cachesets
  for (unsigned int i=0; i < m_sets; i++){

    m_current_cacheline[i] = lookup(i, idx);

    //DUMP(this->name(), "Set :" << i << " atag: " << (*m_current_cacheline[i]).tag.atag << " valid: " << (*m_current_cacheline[i]).tag.valid << " entry: " << (*m_current_cacheline[i]).entry[offset>>2].i);

    // check the cache tag
    if ((*m_current_cacheline[i]).tag.atag == tag) {

      //DUMP(this->name(),"Correct atag found in set " << i);
     
      // check the valid bit (math.h pow is mapped to the coproc, hence it should be pretty fast)
      if ((*m_current_cacheline[i]).tag.valid & (unsigned int)(pow((double)2,(double)(offset >> 2))) != 0) {

	DUMP(this->name(),"Cache Hit in Set " << i);
	
	// write data pointer
	*data = (*m_current_cacheline[i]).entry[offset >> 2].i;
	
	// increment time
	*t+=m_icache_hit_read_response_delay;


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

  // in case no matching tag was found or data is not valid
  if (cache_hit==-1) {

    // increment time
    *t += m_icache_miss_read_response_delay; 

    // check whether we have a mmu
    if (m_mmu_en == 1) {

      // mmu enabled: forward request to mmu
      m_mmu->itlb_read(address, data, 4);

    } else {
    
      // no mmu: access ahb interface directly
      m_parent->amba_read(address, data, 4);
      DUMP(this->name(),"Received data from main memory " << std::hex << *data);

    }

    // !!!! The replacement mechanism still needs to be verified.
    // This is only a first shot.

    // check for unvalid data which can be replaced without harm
    for (unsigned int i=0; i<m_sets; i++){

      if (((*m_current_cacheline[i]).tag.valid & (unsigned int)(pow((double)2,(double)(offset >> 2)))) == 0) {

	// select unvalid data for replacement
	set_select = i;
	DUMP(this->name(), "Set " << set_select << " has no valid data - will use for refill.");
	break;
      }
    }

    // in case there is no free set anymore
    if (set_select == -1) {

      // select set according to replacement strategy
      set_select = replacement_selector(m_repl);
      DUMP(this->name(),"Set " << set_select << " selected for refill by replacement selector.");
      
    }

    // fill in the new data
    (*m_current_cacheline[set_select]).entry[offset >> 2].i = *data;

    // fill in the new atag
    (*m_current_cacheline[set_select]).tag.atag  = tag;
 
    // switch on the valid bit
    (*m_current_cacheline[set_select]).tag.valid |= (unsigned int)(pow((double)2,(double)(offset >> 2)));

    //DUMP(this->name(),"Updated entry: " << std::hex << (*m_current_cacheline[set_select]).entry[offset >> 2].i << " valid bits: " << std::hex << (*m_current_cacheline[set_select]).tag.valid);

   }
}

unsigned int ivectorcache::replacement_selector(unsigned int mode) {
  
  // random replacement
  if (mode == 0) {

    // todo: check RTL for implementation details
    return(rand() % m_sets);
  } 
  else {

    DUMP(this->name(), "LRU not implemented yet!!");
  }

  return 0;
}


// call to flush cache
void ivectorcache::flush(sc_core::sc_time *t) {
}

// internal behavioral functions
// -----------------------------

// reads a cache line from a cache set
t_cache_line * ivectorcache::lookup(unsigned int set, unsigned int idx) {

  // return the cache line from the selected set
  return (&(*cache_mem[set])[idx]);

}

// debug and helper functions
// --------------------------

// displays cache lines at stdout for debug
void ivectorcache::dbg_read(unsigned int set, unsigned int start_idx, unsigned int number_of_entries) {

  t_cache_line dbg_cacheline;

  DUMP(this->name(), " ******************************************************************************************************* ");
  DUMP(this->name(), " * dbg_read(set=" << set << ", start_idx=" << start_idx << ", number_of_entries=" << number_of_entries);

  // for all selected indices do
  for (unsigned int i = start_idx; i < (start_idx + number_of_entries); i++) {

    // read the cacheline from set
    dbg_cacheline = (*cache_mem[set])[i];

    // display the tag 
    DUMP(this->name(), "ATAG: 0x" << std::hex << dbg_cacheline.tag.atag << " VALID: 0x" << std::hex << dbg_cacheline.tag.valid);

    // display all entries
    for (unsigned int j = 0; j < (m_linesize >> 2); j++) {

      DUMP(this->name(), std::hex << dbg_cacheline.entry[j].i);

    }
  }
}
  


