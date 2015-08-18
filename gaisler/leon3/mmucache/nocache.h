// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file nocache.h
/// Cache placeholder. Implements the cache interface and forwards request to
/// mmu or ahb interface depending on the configuration.
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef __NOCACHE_H_
#define __NOCACHE_H_

#include "core/common/base.h"
#include "core/common/systemc.h"
#include "core/common/sr_signal.h"

#include "core/common/verbose.h"
#include "gaisler/leon3/mmucache/cache_if.h"
#include "gaisler/leon3/mmucache/mem_if.h"

class nocache : public DefaultBase, public cache_if {

    public:

        // External interface functions (mem_if):
        // -----------------------------------------------------------
        /// read from cache
        virtual bool mem_read(unsigned int address, unsigned int asi, unsigned char * data,
                              unsigned int len, sc_core::sc_time * t,
                              unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock = false);
        /// write through cache
        virtual void mem_write(unsigned int address, unsigned int asi, unsigned char * data,
                               unsigned int len, sc_core::sc_time * t,
                               unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock = false);
        /// flush cache
        virtual void flush(sc_core::sc_time * t, unsigned int * debug, bool is_dbg);

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

        /// implement ccr check (not there)
        virtual unsigned int check_mode();

        /// implement cache type function
        virtual t_cache_type get_cache_type();

	/// dummy snooping function
	virtual void snoop_invalidate(const t_snoop& snoop, const sc_core::sc_time& delay);

	/// Helper functions for definition of clock cycle
	void clkcng(sc_core::sc_time &clk);

        // debug and helper functions
        // --------------------------
        /// display of cache lines for debug
        virtual void dbg_out(unsigned int line);

        // constructor
        nocache(ModuleName name, mem_if * _mem_adapter);

        // destructor
        virtual ~nocache() {
        }
        ;

    private:

        mem_if * m_mem_adapter;

};

#endif // __NOCACHE_H_
/// @}
