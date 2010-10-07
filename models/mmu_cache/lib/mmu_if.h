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
// Title:      mmu_if.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    MMU cache interface class for passing
//             pointers to the mmu interface functions to the
//             components of mmu_cache (ivectorcache, dvectorcache).   */
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


#ifndef __MMU_IF_H__
#define __MMU_IF_H__

#include <tlm.h>
#include <map>

#include "defines.h"

class mmu_if {

    public:

        // page descriptor cache (PDC) lookup
        virtual unsigned int tlb_lookup(unsigned int addr, std::map<t_VAT,
                t_PTE_context> * tlb, unsigned int tlb_size,
                                        sc_core::sc_time * t,
                                        unsigned int * debug) = 0;

        // read mmu internal registers (ASI 0x19)
        virtual unsigned int read_mcr() = 0;
        virtual unsigned int read_mctpr() = 0;
        virtual unsigned int read_mctxr() = 0;
        virtual unsigned int read_mfsr() = 0;
        virtual unsigned int read_mfar() = 0;

        // write mmu internal registers (ASI 0x19)
        virtual void write_mcr(unsigned int * data) = 0;
        virtual void write_mctpr(unsigned int * data) = 0;
        virtual void write_mctxr(unsigned int * data) = 0;

        // diagnostic read/write of instruction PDC (ASI 0x5)
        virtual void diag_read_itlb(unsigned int addr, unsigned int * data) = 0;
        virtual void diag_write_itlb(unsigned int addr, unsigned int * data) = 0;

        // diagnostic read/write of data PDC or shared instruction and data PDC (ASI 0x6)
        virtual void diag_read_dctlb(unsigned int addr, unsigned int * data) = 0;
        virtual void
                diag_write_dctlb(unsigned int addr, unsigned int * data) = 0;

        virtual ~mmu_if() {
        }
        ;

};

#endif // __MMU_IF_H__
