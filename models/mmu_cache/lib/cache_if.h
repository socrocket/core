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
// Title:      cache_if.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Unified cache interface definition
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

#ifndef __CACHE_IF_H__
#define __CACHE_IF_H__

#include "tlm.h"
#include "socrocket.h"
#include "mem_if.h"

class cache_if : public mem_if {

    public:

        /// flush cache
        virtual void flush(sc_core::sc_time * t, unsigned int * debug, bool is_dbg) = 0;
        /// read data cache tags (ASI 0xe)
        virtual void read_cache_tag(unsigned int address, unsigned int * data,
                                    sc_core::sc_time *t) = 0;
        /// write data cache tags (ASI 0xe)
        virtual void write_cache_tag(unsigned int address, unsigned int * data,
                                     sc_core::sc_time *t) = 0;
        /// read data cache entries/data (ASI 0xf)
        virtual void
                read_cache_entry(unsigned int address, unsigned int * data,
                                 sc_core::sc_time *t) = 0;
        /// write data cache entries/data (ASI 0xf)
        virtual void
                write_cache_entry(unsigned int address, unsigned int * data,
                                  sc_core::sc_time *t) = 0;
        /// read cache configuration register (ASI 0x2)
        virtual unsigned int read_config_reg(sc_core::sc_time *t) = 0;

	/// returns the mode bits of the cache
        virtual unsigned int check_mode() = 0;

	/// snooping function (invalidates cache line(s))
	virtual void snoop_invalidate(const t_snoop &snoop, const sc_core::sc_time& delay) = 0;

	// Helper functions for definition of clock cycle
	virtual void clkcng(sc_core::sc_time &clk) = 0;

        // debug and helper functions
        // --------------------------
        /// display of cache lines for debug
        virtual void dbg_out(unsigned int line) = 0;

        virtual ~cache_if() {
        }

};

#endif // __CACHE_IF_H__
