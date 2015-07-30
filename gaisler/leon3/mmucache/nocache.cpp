// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file nocache.cpp
/// Cache placeholder. Implements the cache interface and forwards request to
/// mmu or ahb interface depending on the configuration.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include "gaisler/leon3/mmucache/nocache.h"

/// constructor
nocache::nocache(ModuleName name, mem_if * _mem_adapter) :
    sc_module(name), m_mem_adapter(_mem_adapter) {
    // nothing to do
}

/// memory interface write function: forwards calls to mmu or bus interface
void nocache::mem_write(unsigned int addr, unsigned int asi, unsigned char *data,
                        unsigned int length, sc_core::sc_time *t,
                        unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock) {

    m_mem_adapter->mem_write(addr, asi, data, length, t, debug, is_dbg, is_lock, cacheable);
    if (asi != 8) {
    v::warn << name() << "asi 0x" << hex <<asi
            << v::endl;
    }

}

/// memory interface read functions: forwards calls to mmu or bus interface
bool nocache::mem_read(unsigned int addr, unsigned int asi, unsigned char * data,
                       unsigned int length, sc_core::sc_time *t,
                       unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock) {

    if (asi != 8) {
    v::warn << name() << "asi 0x" << hex <<asi
            << v::endl;
    }
    return(m_mem_adapter->mem_read(addr, asi, data, length, t, debug, is_dbg, is_lock, cacheable));

}

/// forbidden access to flush
void nocache::flush(sc_core::sc_time *t, unsigned int *debug, bool is_dbg) {

    v::warn << name() << "Can not flush non-existing cache!" << v::endl;
}

/// forbidden read of cache tags
void nocache::read_cache_tag(unsigned int address, unsigned int * data,
                             sc_core::sc_time *t) {

    v::warn << name() << "Can not read tags of non-existing cache!"
            << v::endl;
}

/// forbidden write of cache tags
void nocache::write_cache_tag(unsigned int address, unsigned int *data,
                              sc_core::sc_time *t) {

    v::warn << name() << "Can not write tags of non-existing cache!"
            << v::endl;
}

/// forbidden read of cache entries
void nocache::read_cache_entry(unsigned int address, unsigned int *data,
                               sc_core::sc_time *t) {

    v::warn << name() << "Can not read entries of non-existing cache!"
            << v::endl;
}

/// forbidden write of cache entries
void nocache::write_cache_entry(unsigned int address, unsigned int *data,
                                sc_core::sc_time *t) {

    v::warn << name() << "Can not write entries of non-existing cache!"
            << v::endl;
}

unsigned int nocache::read_config_reg(sc_core::sc_time *t) {

    v::warn << name()
            << "Can not read config register of non-existing cache!" << v::endl;
    return 0;
}

/// forbidden check of cache mode
unsigned int nocache::check_mode() {

    v::warn << name() << "Can not check mode of non-existing cache!"
            << v::endl;
    return 0;
}

/// this is a cache dummy
cache_if::t_cache_type nocache::get_cache_type() {

  return cache_if::nocache;

}

/// forbidden debug output
void nocache::dbg_out(unsigned int line) {

    v::warn << name()
            << "Can not trigger debug output of non-exisiting cache!"
            << v::endl;

}

// dummy snooping function
void nocache::snoop_invalidate(const t_snoop& snoop, const sc_core::sc_time& delay) {

  v::warn << name() << "Can not snoop non-existing cache!" << v::endl;

}

// Helper for setting clock cycle latency using sc_clock argument
void nocache::clkcng(sc_core::sc_time &clk) {
  // nothing to do
}
/// @}
