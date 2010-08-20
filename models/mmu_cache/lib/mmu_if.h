/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mmu_if.h - MMU cache interface class for passing        */
/*             pointers to the mmu interface functions to the          */
/*             components of mmu_cache (ivectorcache, dvectorcache).   */ 
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/


#ifndef __MMU_IF_H__
#define __MMU_IF_H__

#include <tlm.h>
#include <map>

#include "defines.h"

class mmu_if {

  public:

  // page descriptor cache (PDC) lookup
  virtual unsigned int tlb_lookup(unsigned int addr, std::map<t_VAT, t_PTE_context> * tlb, unsigned int tlb_size, sc_core::sc_time * t, unsigned int * debug) = 0;

  // read mmu internal registers (ASI 0x19)
  virtual unsigned int read_mcr() = 0;
  virtual unsigned int read_mctpr() = 0;
  virtual unsigned int read_mctxr() = 0;
  virtual unsigned int read_mfsr() = 0;
  virtual unsigned int read_mfar() = 0;
  
  // write mmu internal registers (ASI 0x19)
  virtual void write_mcr(unsigned int * data) = 0;
  virtual void write_mctpr(unsigned int * data) = 0;
  virtual void write_mctxr(unsigned int * data) = 0;

  // diagnostic read/write of instruction PDC (ASI 0x5)
  virtual void diag_read_itlb(unsigned int addr, unsigned int * data) = 0;
  virtual void diag_write_itlb(unsigned int addr, unsigned int * data) = 0;
  
  // diagnostic read/write of data PDC or shared instruction and data PDC (ASI 0x6)
  virtual void diag_read_dctlb(unsigned int addr, unsigned int * data) = 0;
  virtual void diag_write_dctlb(unsigned int addr, unsigned int * data) = 0;

  virtual ~mmu_if() {};

};

#endif // __MMU_IF_H__
