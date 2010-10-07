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
// Title:      dvectorcache.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Class definition of a data
//             cache. The cache can be configured direct mapped or
//             set associative. Set-size, line-size and replacement
//             strategy can be defined through constructor arguments.
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

#ifndef __DVECTORCACHE_H__
#define __DVECTORCACHE_H__

#include "tlm.h"

//#include <vector>
//#include <iostream>
//#include "math.h"

#include "defines.h"
//#include "mmu_cache.h"
#include "vectorcache.h"
//#include "mmu.h"
//#include "mem_if.h"

// implementation of cache memory and controller
// ---------------------------------------------
/// @brief Data cache implementation for TrapGen LEON3 simulator
class dvectorcache : public vectorcache {

    public:

        // implement ccr check
        unsigned int check_mode();

        // constructor
        // args: sysc module name, pointer to AHB read/write methods (of parent), delay on read hit, delay on read miss (incr), number of sets, setsize in kb, linesize in b, replacement strategy
        /// @brief Constructor of data cache
        /// @param name                              SystemC module name
        /// @param mmu_cache                         Pointer to top-level class of cache subsystem (mmu_cache) for access to AHB bus interface
        /// @param tlb_adaptor                       Pointer to memory management unit
        /// @param hit_read_response_delay           Delay for a cache read hit
        /// @param miss_read_response_delay          Delay for a cache read miss
        /// @param write_response_delay              Delay for a cache write access (hit/miss)
        /// @param sets                              Number of cache sets
        /// @param setsize                           Size of a cache set (in kbytes)
        /// @param setlock                           Enable cache line locking
        /// @param linesize                          Size of a cache line (in bytes)
        /// @param repl                              Cache replacement strategy
        /// @param lram                              Local RAM configured
        /// @param lramstart                         The 8 MSBs of the local ram start address (16MB segment)
        /// @param lramsize                          Size of local ram (size in kbyte = 2^lramsize)
        dvectorcache(sc_core::sc_module_name name, mmu_cache_if * _mmu_cache,
                     mem_if * _tlb_adaptor, unsigned int mmu_en,
                     sc_core::sc_time dcache_hit_read_response_delay,
                     sc_core::sc_time dcache_miss_read_response_delay,
                     sc_core::sc_time dcache_write_response_delay,
                     unsigned int sets, unsigned int setsize,
                     unsigned int setlock, unsigned int linesize,
                     unsigned int repl, unsigned int lram,
                     unsigned int lramstart, unsigned int lramsize) :
            vectorcache(name, _mmu_cache,
                    _tlb_adaptor,
                    mmu_en,
                    0, // burst fetch forbidden
                    dcache_hit_read_response_delay,
                    dcache_miss_read_response_delay,
                    dcache_write_response_delay, sets, setsize, setlock,
                    linesize, repl, lram, lramstart, lramsize) {
        }

};

#endif // __DVECTORCACHE_H__

