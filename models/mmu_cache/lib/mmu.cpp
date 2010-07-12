/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mmu.h - Implementation of a memory management unit.     */
/*             The mmu can be configured to have split or combined     */
/*             TLBs for instructions and data. The TLB size can be     */
/*             configured as well. The memory page size is currently   */
/*             currently fixed to 4kB.                                 */
/*                                                                     */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#include "mmu.h"

// constructor args:
// - sysc module name,
// - delay on an instruction tlb hit
// - delay on an instruction tlb miss (per page table lookup)
// - delay on a data tlb hit
// - delay on a data tlb miss (per page table lookup)
// - number of instruction tlbs
// - number of data tlbs
// - tlb type
// - tlb replacement strategy
// - tlb mmu page size (default 4kB)
mmu::mmu(sc_core::sc_module_name name,
      mmu_cache_if &_parent,
      sc_core::sc_time itlb_hit_response_delay,
      sc_core::sc_time itlb_miss_response_delay,
      sc_core::sc_time dtlb_hit_response_delay,
      sc_core::sc_time dtlb_miss_response_delay,
      unsigned int itlbnum,
      unsigned int dtlbnum,
      unsigned int tlb_type,
      unsigned int tlb_rep,
      unsigned int mmupgsz) : sc_module(name),
			     m_itlbnum(itlbnum),
			     m_dtlbnum(dtlbnum),
			     m_tlb_type(tlb_type),
			     m_tlb_rep(tlb_rep),
			     m_mmupgsz(mmupgsz),
			     m_itlb_hit_response_delay(itlb_hit_response_delay),
			     m_itlb_miss_response_delay(itlb_miss_response_delay),
			     m_dtlb_hit_response_delay(dtlb_hit_response_delay),
			     m_dtlb_miss_response_delay(dtlb_miss_response_delay)
{

  // generate associative memory (map) for instruction tlb
  itlb = new std::map<t_VAT, t_PTE_context>;

  // are we in split tlb mode?
  if (m_tlb_type & 0x1) {

    // generate another associative memory (map) for data tlb
    dtlb = new std::map<t_VAT, t_PTE_context>;
    DUMP(this->name(),"Created split instruction and data TLBs.");

  } else {

    // combined tlb mode -> share instruction tlb
    dtlb = itlb;
    DUMP(this->name(),"Created combined instruction and data TLBs.");

  }

  // hook up to top level (amba if)
  m_parent = &_parent;

}
			     
// instruction read interface
void mmu::itlb_read(unsigned int addr, unsigned int * data, unsigned int length) {

  DUMP(this->name(),"ITLB READ request virtual address: " << std::hex << addr);

}

// data write interface
void mmu::dtlb_write(unsigned int addr, unsigned int * data, unsigned int length) {

  DUMP(this->name(),"DTLB WRITE request virtual address: " << std::hex << addr);

}

// data read interface
void mmu::dtlb_read(unsigned int addr, unsigned int * data, unsigned int length) {

  DUMP(this->name(),"DTLB READ request virtual address: " << std::hex << addr);

}
