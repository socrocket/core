// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       cache_if.h - Unified cache interface definition         * 
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision$                                        *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************

#ifndef __CACHE_IF_H__
#define __CACHE_IF_H__

#include "tlm.h"
#include "mem_if.h"

class cache_if : public mem_if {

 public:

  /// flush cache
  virtual void flush(sc_core::sc_time * t, unsigned int * debug) = 0;
  /// read data cache tags (ASI 0xe)
  virtual void read_cache_tag(unsigned int address, unsigned int * data, sc_core::sc_time *t) = 0;
  /// write data cache tags (ASI 0xe)
  virtual void write_cache_tag(unsigned int address, unsigned int * data, sc_core::sc_time *t) = 0;
  /// read data cache entries/data (ASI 0xf)
  virtual void read_cache_entry(unsigned int address, unsigned int * data, sc_core::sc_time *t) = 0;
  /// write data cache entries/data (ASI 0xf)
  virtual void write_cache_entry(unsigned int address, unsigned int * data, sc_core::sc_time *t) = 0;
  /// read cache configuration register (ASI 0x2)
  virtual unsigned int read_config_reg(sc_core::sc_time *t) = 0;

  virtual unsigned int check_mode() = 0;

  // debug and helper functions
  // --------------------------
  /// display of cache lines for debug
  virtual void dbg_out(unsigned int line) = 0; 

  virtual ~cache_if() {}

};

#endif // __CACHE_IF_H__
