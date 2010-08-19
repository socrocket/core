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

class mmu_cache_if {

  public:

  // amba master interface functions
  virtual void amba_write(unsigned int addr, unsigned char * data, unsigned int length) {};
  virtual void amba_read(unsigned int addr, unsigned char * data, unsigned int length) {};

  // read cache control register
  virtual unsigned int read_ccr() = 0;

  virtual ~mmu_cache_if() {};

};

#endif // __MMU_CACHE_IF_H__
