// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       mmu_cache_if.h - MMU cache interface class for passing  *
// *             pointers to the AHB interface to the components of      *
// *             mmu_cache (ivectorcache, dvectorcache).                 * 
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision$                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************


#ifndef __MMU_CACHE_IF_H__
#define __MMU_CACHE_IF_H__

#include "mem_if.h"

class mmu_cache_if : public mem_if {

 public:

  // read cache control register
  virtual unsigned int read_ccr() { return(0);};
  virtual void write_ccr(unsigned char *data, unsigned int len, sc_core::sc_time *delay) {};
 
  virtual ~mmu_cache_if() {};
};

#endif // __MMU_CACHE_IF_H__
