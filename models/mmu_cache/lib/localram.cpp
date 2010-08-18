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
  scratchpad = new std::vector<t_cache_data>(m_lrsize, m_default_entry);

  DUMP(this->name(), " ******************************************************************************* ");
  DUMP(this->name(), " * Created local ram with following parameters:                                  ");
  DUMP(this->name(), " * start address " << std::hex << m_lrstart);
  DUMP(this->name(), " * size in bytes " << std::hex << m_lrsize);
  DUMP(this->name(), " ******************************************************************************* ");

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
  for(unsigned int i=0; i<len; i++) { *(data+i) = (*scratchpad)[(addr - m_lrstart) >> 2].c[byt+i]; }

  DUMP(this->name(),"Read from address: " << std::hex << addr);
 
  // update debug information
  SCRATCHPAD_SET(*debug);

}

// write to scratchpad
void localram::write(unsigned int addr, unsigned char *data, unsigned int len, sc_core::sc_time *t, unsigned int *debug) {

  assert(addr - m_lrstart < (m_lrsize << 2));

  // byte offset
  unsigned int byt = addr & 0x3;

  // memcpy ??
  for(unsigned int i=0; i<len; i++) { (*scratchpad)[(addr - m_lrstart) >> 2].c[byt+i] = *(data + i); }

  DUMP(this->name(),"Write to address: " << std::hex << addr);

  // update debug information
  SCRATCHPAD_SET(*debug);

}
