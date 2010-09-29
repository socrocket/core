// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       mem_if.h - To be implemented by classes which           *
// *             provide a memory interface for public use.              * 
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision$                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************


#ifndef __MEM_IF_H__
#define __MEM_IF_H__

class mem_if {

 public:

  // amba master interface functions
  virtual void mem_write(unsigned int addr, unsigned char * data, unsigned int length, sc_core::sc_time * t, unsigned int * debug) {};  virtual void mem_read(unsigned int addr, unsigned char * data, unsigned int length, sc_core::sc_time * t, unsigned int * debug) {};

  virtual ~mem_if() {}

};

#endif // __MEM_IF_H__
