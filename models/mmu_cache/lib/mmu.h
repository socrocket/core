/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mmu.h - Class definition of a memory management unit.   */
/*             The mmu can be configured to have split or combined     */
/*             TLBs for instructions and data. The TLB size can be     */
/*             configured as well. The memory page size is currently   */
/*             currently fixed to 4kB.                                 */
/*                                                                     */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                              */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#ifndef __MMU_H__
#define __MMU_H__

#include <tlm.h>
#include <iostream>
#include <map>

#include "mmu_cache_if.h"
#include "mmu_if.h"
#include "defines.h"

// implementation of a memory management unit
// ------------------------------------------
class mmu : public mmu_if, public sc_core::sc_module {

  public:

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
  mmu(sc_core::sc_module_name name,
      mmu_cache_if &_parent,
      sc_core::sc_time itlb_hit_response_delay,
      sc_core::sc_time itlb_miss_response_delay,
      sc_core::sc_time dtlb_hit_response_delay,
      sc_core::sc_time dtlb_miss_response_delay,
      unsigned int itlbnum,
      unsigned int dtlbnum,
      unsigned int tlb_type,
      unsigned int tlb_rep,
      unsigned int mmupgsz);

  // member functions
  // ----------------
  // mmu interface functions:
  // In case the MMU is enabled, the bus accesses of the i/d cache are diverted
  // using the interface functions below (mmu_cache_if).
  void itlb_read(unsigned int addr, unsigned int * data, unsigned int length);
  void dtlb_write(unsigned int addr, unsigned int * data, unsigned int length);
  void dtlb_read(unsigned int addr, unsigned int * data, unsigned int length);

  public:

  // data members
  // ------------
  // pointer to mmu_cache module (ahb interface functions)
  mmu_cache_if * m_parent;

  // instruction and data tlb pointers
  // (depending on configuration may point to a shared tlb implementation)
  std::map<t_VAT, t_PTE_context> * itlb;
  std::map<t_VAT, t_PTE_context> * dtlb;

  // mmu parameters
  // --------------
  // number of instruction tlbs
  unsigned int m_itlbnum;
  // number of data tlbs
  unsigned int m_dtlbnum;
  // tlb type (bit0 - split/combined, bit1 - standard/fast write buffer)
  unsigned int m_tlb_type;
  // tlb replacment strategy (no inform. found yet - tmp use random)
  unsigned int m_tlb_rep;
  // mmu page size
  unsigned int m_mmupgsz;

  // delay parameters
  // ----------------
  sc_core::sc_time m_itlb_hit_response_delay;
  sc_core::sc_time m_itlb_miss_response_delay;
  sc_core::sc_time m_dtlb_hit_response_delay;
  sc_core::sc_time m_dtlb_miss_response_delay;

};

#endif // __MMU_H__
