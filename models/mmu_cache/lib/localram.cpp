//*********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is 
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      localram.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implementation of a local RAM that
//             can be attached to the icache and dcache controllers.
//             The LocalRAM enables fast 0-waitstate access
//             to instructions or data.
//
// Method:
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#include "localram.h"
#include "verbose.h"

/// constructor
localram::localram(sc_core::sc_module_name name, unsigned int lrsize,
                   unsigned int lrstart) :
                   sc_module(name), m_lrsize(lrsize<<10), m_lrstart(lrstart << 24) {

    // parameter check
    // ---------------
    // scratchpad size max 512 kbyte
    assert(m_lrsize <= 524288);

    // initialize allocator
    m_default_entry.i = 0;

    // create the actual ram
    scratchpad = new t_cache_data[m_lrsize>>2];

    v::info << this->name()
            << " ******************************************************************************* "
            << v::endl;
    v::info << this->name()
            << " * Created local ram with following parameters:                                  "
            << v::endl;
    v::info << this->name() << " * start address " << std::hex << m_lrstart
            << v::endl;
    v::info << this->name() << " * size in bytes " << std::hex << m_lrsize
            << v::endl;
    v::info << this->name()
            << " ******************************************************************************* "
            << v::endl;

}

/// destructor
localram::~localram() {

    // free the memory
    delete (scratchpad);

}

/// read from scratchpad
void localram::read(unsigned int addr, unsigned char *data, unsigned int len,
                    sc_core::sc_time *t, unsigned int *debug) {

    assert(addr - m_lrstart < m_lrsize);

    // byte offset
    unsigned int byt = addr & 0x3;

    // memcpy ??
    for (unsigned int i = 0; i < len; i++) {
        *(data + i) = scratchpad[(addr - m_lrstart) >> 2].c[byt + i];
    }

    v::debug << this->name() << "Read from address: " << std::hex << addr
            << v::endl;

    // update debug information
    SCRATCHPAD_SET(*debug);

}

// write to scratchpad
void localram::write(unsigned int addr, unsigned char *data, unsigned int len,
                     sc_core::sc_time *t, unsigned int *debug) {

    assert(addr - m_lrstart < m_lrsize);

    // byte offset
    unsigned int byt = addr & 0x3;

    // memcpy ??
    for (unsigned int i = 0; i < len; i++) {
        scratchpad[(addr - m_lrstart) >> 2].c[byt + i] = *(data + i);
    }

    v::debug << this->name() << "Write to address: " << std::hex << addr
            << v::endl;

    // update debug information
    SCRATCHPAD_SET(*debug);

}
