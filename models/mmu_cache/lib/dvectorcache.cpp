// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       dvectorcache.cpp - Implementation of a data             *
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

#include "dvectorcache.h"

// implement ccr check
unsigned int dvectorcache::check_mode() {

    return(m_mmu_cache->read_ccr() & 0x3);

}
  
