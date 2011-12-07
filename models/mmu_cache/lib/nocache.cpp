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
// Title:      nocache.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Cache placeholder. Implements the cache
//             interface and forwards request to mmu or ahb interface
//             depending on the configuration.
//
// Method:
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

#include "nocache.h"

/// constructor
nocache::nocache(sc_core::sc_module_name name, mem_if * _mem_adapter) :
    sc_module(name), m_mem_adapter(_mem_adapter) {
    // nothing to do
}

/// memory interface write function: forwards calls to mmu or bus interface
void nocache::mem_write(unsigned int addr, unsigned char *data,
                        unsigned int length, sc_core::sc_time *t,
                        unsigned int * debug, bool is_dbg) {

    m_mem_adapter->mem_write(addr, data, length, t, debug, is_dbg);

}

/// memory interface read functions: forwards calls to mmu or bus interface
bool nocache::mem_read(unsigned int addr, unsigned char * data,
                       unsigned int length, sc_core::sc_time *t,
                       unsigned int * debug, bool is_dbg) {

    return(m_mem_adapter->mem_read(addr, data, length, t, debug, is_dbg));
    
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
