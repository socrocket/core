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
// Title:      dcio_payload_extension.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implements the payload
//             extensions for communication with dcio socket of
//             mmu_cache.
//
// Method:
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************


#include "dcio_payload_extension.h"

// constructor
dcio_payload_extension::dcio_payload_extension(void) :
    asi(0), flush(0), flushl(0), lock(0) {
}

// destructor
dcio_payload_extension::~dcio_payload_extension(void) {
}

// override virtual copy_from method
void dcio_payload_extension::copy_from(const tlm_extension_base &extension) {

    asi = static_cast<dcio_payload_extension const &> (extension).asi;
    flush = static_cast<dcio_payload_extension const &> (extension).flush;
    flushl = static_cast<dcio_payload_extension const &> (extension).flushl;
    lock = static_cast<dcio_payload_extension const &> (extension).lock;
    debug = static_cast<dcio_payload_extension const &> (extension).debug;
    fail = static_cast<dcio_payload_extension const &> (extension).fail;
}

// override virtual clone method
tlm::tlm_extension_base * dcio_payload_extension::clone(void) const {

    return new dcio_payload_extension(*this);

}
