// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       localram.cpp - Implementation of a local RAM that       *
// *             can be attached to the icache and dcache controllers.   *
// *             The LocalRAM enables fast 0-waitstate access            *
// *             to instructions or data.                                *
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision$                                           *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************

#include "localram.h"
#include "verbose.h"

/// constructor
localram::localram(sc_core::sc_module_name name, 
		   unsigned int lrsize,
		   unsigned int lrstart) : sc_module(name),
					   m_lrsize((unsigned int)pow(2,(lrsize+8))),
					   m_lrstart(lrstart << 24)
{
  // parameter check
  // ---------------
  // scratchpad size max 512 kbyte (131072 words)
  assert(m_lrsize < 131073);

  // initialize allocator
  m_default_entry.i = 0;

  // create the actual ram
  scratchpad = new t_cache_data[m_lrsize];

  v::info << this->name() <<  " ******************************************************************************* " << v::endl;
  v::info << this->name() <<  " * Created local ram with following parameters:                                  " << v::endl;
  v::info << this->name() <<  " * start address " << std::hex << m_lrstart << v::endl;
  v::info << this->name() <<  " * size in bytes " << std::hex << m_lrsize << v::endl;
  v::info << this->name() <<  " ******************************************************************************* " << v::endl;

}

/// destructor
localram::~localram() {

  // free the memory
  delete(scratchpad);

}

/// read from scratchpad
void localram::read(unsigned int addr, unsigned char *data, unsigned int len, sc_core::sc_time *t, unsigned int *debug) {

  assert(addr - m_lrstart < (m_lrsize << 2));
  
  // byte offset
  unsigned int byt = addr & 0x3;

  // memcpy ??
  for(unsigned int i=0; i<len; i++) { *(data+i) = scratchpad[(addr - m_lrstart) >> 2].c[byt+i]; }

  v::info << this->name() << "Read from address: " << std::hex << addr << v::endl;
 
  // update debug information
  SCRATCHPAD_SET(*debug);

}

// write to scratchpad
void localram::write(unsigned int addr, unsigned char *data, unsigned int len, sc_core::sc_time *t, unsigned int *debug) {

  assert(addr - m_lrstart < (m_lrsize << 2));

  // byte offset
  unsigned int byt = addr & 0x3;

  // memcpy ??
  for(unsigned int i=0; i<len; i++) { scratchpad[(addr - m_lrstart) >> 2].c[byt+i] = *(data + i); }

  v::info << this->name() << "Write to address: " << std::hex << addr << v::endl;

  // update debug information
  SCRATCHPAD_SET(*debug);

}
