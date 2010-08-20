// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       tlb_adaptor.h - Provides access to instruction          *
// *             and data tlb through unified interface functions        * 
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision $                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************

#ifndef __TLB_ADAPTOR_H__
#define __TLB_ADAPTOR_H__

#include <tlm.h>

#include "mmu_cache_if.h"
#include "mmu_if.h"
#include "mem_if.h"

#include "defines.h"

class tlb_adaptor : public sc_core::sc_module, public mem_if {

 public:

  /// constructor
  tlb_adaptor(sc_core::sc_module_name name,
	      mmu_cache_if * top,
	      mmu_if * _mmu,
	      std::map<t_VAT, t_PTE_context> * tlb,
	      unsigned int tlbnum) : sc_module(name), 
    m_mmu_cache(top),
    m_mmu(_mmu), 
    m_tlb(tlb),
    m_tlbnum(tlbnum) {

    // nothing to do

  }

  /// destructor
  ~tlb_adaptor() {

    // nothing to do

  }

  /// implementation of mem_read function from mem_if.h
  virtual void mem_read(unsigned int addr, unsigned char * data, unsigned int len, sc_core::sc_time * t, unsigned int * debug) {

    unsigned int paddr;
    
    // mmu enabled
    if (m_mmu->read_mcr() & 0x1) {

      paddr=m_mmu->tlb_lookup(addr, m_tlb, m_tlbnum, t, debug);

    }
    // mmu in bypass mode
    else {
      
      paddr=addr;
    
    }
    
    // forward request to amba interface
    m_mmu_cache->mem_read(paddr, data, len, t, debug);
  
  }
  
  /// implementation of mem_write function from mem_if.h
  virtual void mem_write(unsigned int addr, unsigned char * data, unsigned int len, sc_core::sc_time * t, unsigned int * debug) {

    unsigned int paddr;

    // mmu enabled
    if (m_mmu->read_mcr() & 0x1) {

      paddr=m_mmu->tlb_lookup(addr, m_tlb, m_tlbnum, t, debug);

    } 
    // mmu in bypass mode
    else {
      
      paddr=addr;

    }
    
    // forward request to mmu amba interface
    m_mmu_cache->mem_write(paddr, data, len, t, debug);

  }

 public:

  mmu_cache_if * m_mmu_cache;
  mmu_if * m_mmu;

  std::map<t_VAT, t_PTE_context> * m_tlb;

  unsigned int m_tlbnum;

};


#endif // __TLB_ADAPTOR_H__
