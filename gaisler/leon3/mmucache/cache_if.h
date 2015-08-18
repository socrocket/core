// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file cache_if.h
/// Unified cache interface definition
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef __CACHE_IF_H__
#define __CACHE_IF_H__

#include "core/common/systemc.h"
#include "core/common/socrocket.h"
#include "gaisler/leon3/mmucache/mem_if.h"

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
        virtual void read_cache_entry(unsigned int address, unsigned int * data,
                                      sc_core::sc_time *t) = 0;
        /// write data cache entries/data (ASI 0xf)
        virtual void write_cache_entry(unsigned int address, unsigned int * data,
                                       sc_core::sc_time *t) = 0;
        /// read cache configuration register (ASI 0x2)
        virtual unsigned int read_config_reg(sc_core::sc_time *t) = 0;

	/// returns the mode bits of the cache
        virtual unsigned int check_mode() = 0;

        /// Data type describing type of cache implementing this interface
        enum t_cache_type { icache, dcache, nocache };

        /// Returns type of cache implementing this interface
        virtual t_cache_type get_cache_type()=0;

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
/// @}
