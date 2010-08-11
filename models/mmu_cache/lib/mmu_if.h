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

class mmu_if {

  public:

  // mmu interface functions
  virtual void itlb_read(unsigned int addr, unsigned int * data, unsigned int length, unsigned int * debug) {};
  virtual void itlb_write(unsigned int addr, unsigned int * data, unsigned int length, unsigned int * debug) {};
  virtual void dtlb_write(unsigned int addr, unsigned int * data, unsigned int length, unsigned int * debug) {};
  virtual void dtlb_read(unsigned int addr, unsigned int * data, unsigned int length, unsigned int * debug) {};

  virtual ~mmu_if() {};

};

#endif // __MMU_IF_H__
