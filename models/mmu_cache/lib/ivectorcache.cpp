// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       ivectorcache.cpp - Implementation of an instruction     *
// *             cache. The cache can be configured direct mapped or     *
// *             set associative. Set-size, line-size and replacement    *
// *             strategy can be defined through constructor arguments.  *
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision$                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************

#include "ivectorcache.h"
#include "verbose.h"

// overwrite write function
void ivectorcache::write(unsigned int address, unsigned char * data, unsigned int len, sc_core::sc_time * t, unsigned int * debug) {

    v::info << this->name() << "Forbidden to write icache!" << v::endl;
    assert(false);

}

// implement ccr check
unsigned int ivectorcache::check_mode() {

    return(m_mmu_cache->read_ccr() & 0x3);

}
