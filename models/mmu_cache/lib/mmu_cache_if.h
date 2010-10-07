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
// Title:      mmu_cache_if.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    MMU cache interface class for passing
//             pointers to the AHB interface to the components of
//             mmu_cache (ivectorcache, dvectorcache).
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


#ifndef __MMU_CACHE_IF_H__
#define __MMU_CACHE_IF_H__

#include "mem_if.h"

class mmu_cache_if : public mem_if {

    public:

        // read cache control register
        virtual unsigned int read_ccr() {
            return (0);
        }
        ;
        virtual void write_ccr(unsigned char *data, unsigned int len,
                               sc_core::sc_time *delay) {
        }
        ;

        virtual ~mmu_cache_if() {
        }
        ;
};

#endif // __MMU_CACHE_IF_H__
