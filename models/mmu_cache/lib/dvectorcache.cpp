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
// Title:      dvectorcache.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implementation of a data
//             cache. The cache can be configured direct mapped or
//             set associative. Set-size, line-size and replacement
//             strategy can be defined through constructor arguments.
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

#include "dvectorcache.h"

// Implement ccr check
unsigned int dvectorcache::check_mode() {

  unsigned int tmp;

  tmp = m_mmu_cache->read_ccr(true);

  return ((tmp >> 2) & 0x3);

}

// This is a data cache
cache_if::t_cache_type dvectorcache::get_cache_type() {

  return cache_if::dcache;

}

// Calculate power/energy values from normalized input data
void dvectorcache::power_model() {

  // Static power = controller + itag ram + idata ram
  sta_power = sta_power_norm +
    sta_dtag_power_norm * m_number_of_vectors * 32 +
    sta_ddata_power_norm * pow(2.0, (double)m_setsize) * (m_sets + 1) * 8;

  // Cell internal power = controller + dtag ram + ddata ram
  int_power = int_power_norm * 1/(clockcycle.to_seconds()*1.0e+6) + 
    int_dtag_power_norm * m_number_of_vectors * 32 * 1/(clockcycle.to_seconds()*1.0e+6) + 
    int_ddata_power_norm * pow(2.0, (double)m_setsize) * (m_sets + 1) * 8;

  // Energy per dtag read access (uJ)
  dyn_tag_read_energy = dyn_dtag_read_energy_norm * m_number_of_vectors * 32 * 32;

  // Energy per dtag write access (uJ)
  dyn_tag_write_energy = dyn_dtag_write_energy_norm * m_number_of_vectors * 32 * 32;

  // Energy per ddata read access (uJ)
  dyn_data_read_energy = dyn_ddata_read_energy_norm * pow(2.0, (double)m_setsize) * 8 * 32;

  // Energy per ddata write access (uJ)
  dyn_data_write_energy = dyn_ddata_write_energy_norm * pow(2.0, (double)m_setsize) * 8 * 32;

}

// Static power callback
void dvectorcache::sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Static power of dcache is constant !!

}

// Internal power callback
void dvectorcache::int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Dcache internal power is constant !!

}

// Switching power callback
void dvectorcache::swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  swi_power = ((dyn_tag_read_energy * dyn_tag_reads) + (dyn_tag_write_energy * dyn_tag_writes) + (dyn_data_read_energy * dyn_data_reads) + (dyn_data_write_energy * dyn_data_writes)) / (sc_time_stamp() - power_frame_starting_time).to_seconds();

}

// Automatically called at start of simulation
void dvectorcache::start_of_simulation() {

  // Initialize power model
  if (m_pow_mon) {

    power_model();

  }
}
