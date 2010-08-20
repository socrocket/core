// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       tlb_if.h - Provides access to instruction and data tlb  *
// *             through unified interface functions                     * 
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision $                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************

#ifndef __TLB_IF_H__
#define __TLB_IF_H__

class tlb_if : public mem_if {

 public:

  /// constructor
  tlb_if(std::map<t_VAT, t_PTE_context> * tlb) {

    m_tlb = tlb;

  }

  virtual void mem_write(unsigned int addr, unsigned char * data, unsigned int length, sc_core::sc_time * t)

 privat:

  std::map<t_VAT, t_PTE_context> * m_tlb;

}


#endif // __TLB_IF_H__
