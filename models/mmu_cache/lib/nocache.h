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
// Title:      nocache.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Cache placeholder. Implements the cache
//             interface and forwards request to mmu or ahb interface
//             depending on the configuration.
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

#ifndef __NOCACHE_H_
#define __NOCACHE_H_

#include "tlm.h"
#include "signalkit.h"

#include "verbose.h"
#include "cache_if.h"
#include "mem_if.h"

class nocache : public sc_core::sc_module, public cache_if {

    public:

        // external interface functions
        // -----------------------------------------------------------
        /// read from cache
        virtual bool mem_read(unsigned int address, unsigned char * data,
                              unsigned int len, sc_core::sc_time * t,
                              unsigned int * debug);
        /// write through cache
        virtual void mem_write(unsigned int address, unsigned char * data,
                               unsigned int len, sc_core::sc_time * t,
                               unsigned int * debug);
        /// flush cache
        virtual void flush(sc_core::sc_time * t, unsigned int * debug);

        /// read data cache tags (ASI 0xe)
        virtual void read_cache_tag(unsigned int address, unsigned int * data, sc_core::sc_time *t);

        /// write data cache tags (ASI 0xe)
        virtual void write_cache_tag(unsigned int address, unsigned int * data, sc_core::sc_time *t);

        /// read data cache entries/data (ASI 0xf)
        virtual void read_cache_entry(unsigned int address, unsigned int * data, sc_core::sc_time *t);

        /// write data cache entries/data (ASI 0xf)
        virtual void write_cache_entry(unsigned int address, unsigned int * data, sc_core::sc_time *t);

        /// read cache configuration register (ASI 0x2)
        virtual unsigned int read_config_reg(sc_core::sc_time *t);

        virtual unsigned int check_mode();

	/// dummy snooping function
	virtual void snoop_invalidate(const t_snoop& snoop, const sc_core::sc_time& delay);

	/// Helper functions for definition of clock cycle
	void clk(sc_core::sc_clock &clk);
	void clk(sc_core::sc_time &period);
	void clk(double period, sc_core::sc_time_unit base);

        // debug and helper functions
        // --------------------------
        /// display of cache lines for debug
        virtual void dbg_out(unsigned int line);

        // constructor
        nocache(sc_core::sc_module_name name, mem_if * _mem_adapter);

        // destructor
        virtual ~nocache() {
        }
        ;

    private:

        mem_if * m_mem_adapter;

};

#endif // __NOCACHE_H_
