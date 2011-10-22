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
// Title:      mem_if.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    To be implemented by classes which
//             provide a memory interface for public use.
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


#ifndef __MEM_IF_H__
#define __MEM_IF_H__

class mem_if {

    public:

        // amba master interface functions
        virtual void mem_write(unsigned int addr, unsigned char * data,
                               unsigned int length, sc_core::sc_time * t,
                               unsigned int * debug, bool is_dbg) {
        };

        virtual bool mem_read(unsigned int addr, unsigned char * data,
                              unsigned int length, sc_core::sc_time * t,
                              unsigned int * debug, bool is_dbg) {
	  return true;

        };

        virtual ~mem_if() {
        }

};

#endif // __MEM_IF_H__
