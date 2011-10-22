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
// Title:      ivectorcache.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implementation of an instruction
//             cache. The cache can be configured direct mapped or
//             set associative. Set-size, line-size and replacement
//             strategy can be defined through constructor arguments.
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

#include "ivectorcache.h"
#include "verbose.h"

// Overwrite write function
void ivectorcache::mem_write(unsigned int address, unsigned char * data,
                             unsigned int len, sc_core::sc_time * t,
                             unsigned int * debug, bool is_dbg) {

    v::info << this->name() << "Forbidden to write icache!" << v::endl;
    assert(false);

}

// Implement ccr check
unsigned int ivectorcache::check_mode() {

  unsigned int tmp = m_mmu_cache->read_ccr();

  swap_Endianess(tmp);

  return (tmp & 0x3);

}
