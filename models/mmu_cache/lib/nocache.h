// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       nocache.h - Cache placeholder. Implements the cache     *
// *             interface and forwards request to mmu or ahb interface  *
// *             depending on the configuration.                         * 
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision$                                        *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************

#ifndef __NOCACHE_H_
#define __NOCACHE_H_

#include "tlm.h"

#include "verbose.h"
#include "cache_if.h"
#include "mem_if.h"

class nocache : public sc_core::sc_module, public cache_if {

 public:

  // external interface functions  
  // -----------------------------------------------------------
  /// read from cache
  virtual void mem_read(unsigned int address, unsigned char * data, unsigned int len, sc_core::sc_time * t, unsigned int * debug);
  /// write through cache
  virtual void mem_write(unsigned int address, unsigned char * data, unsigned int len, sc_core::sc_time * t, unsigned int * debug); 
  /// flush cache
  virtual void flush(sc_core::sc_time * t, unsigned int * debug);
  /// read data cache tags (ASI 0xe)
  virtual void read_cache_tag(unsigned int address, unsigned int * data, sc_core::sc_time *t);
  /// write data cache tags (ASI 0xe)
  virtual void write_cache_tag(unsigned int address, unsigned int * data, sc_core::sc_time *t);
  /// read data cache entries/data (ASI 0xf)
  virtual void read_cache_entry(unsigned int address, unsigned int * data, sc_core::sc_time *t);
  /// write data cache entries/data (ASI 0xf)
  virtual void write_cache_entry(unsigned int address, unsigned int * data, sc_core::sc_time *t);
  /// read cache configuration register (ASI 0x2)
  virtual unsigned int read_config_reg(sc_core::sc_time *t);

  virtual unsigned int check_mode();

  // debug and helper functions
  // --------------------------
  /// display of cache lines for debug
  virtual void dbg_out(unsigned int line);

  // constructor
  nocache(sc_core::sc_module_name name, mem_if * _mem_adapter);

  // destructor
  virtual ~nocache() {};
 
 private:

  mem_if * m_mem_adapter;

};

#endif // __NOCACHE_H_
