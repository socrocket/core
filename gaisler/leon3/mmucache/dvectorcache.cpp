// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file dvectorcache.cpp
/// Implementation of a data cache. The cache can be configured direct mapped or
/// set associative. Set-size, line-size and replacement strategy can be defined
/// through constructor arguments.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include "gaisler/leon3/mmucache/dvectorcache.h"

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
    sta_dtag_power_norm * m_number_of_vectors * 32 * (m_sets + 1) +
    sta_ddata_power_norm * pow(2.0, (double)m_setsize) * (m_sets + 1) * 8;

  // Cell internal power = controller + dtag ram + ddata ram
  int_power = int_power_norm * 1/(clockcycle.to_seconds()) +
    int_dtag_power_norm * m_number_of_vectors * 32 * (m_sets +1) * 1/(clockcycle.to_seconds()) +
    int_ddata_power_norm * pow(2.0, (double)m_setsize) * (m_sets + 1) * 8 * 1/(clockcycle.to_seconds());

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
gs::cnf::callback_return_type dvectorcache::sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Static power of dcache is constant !!
  return GC_RETURN_OK;
}

// Internal power callback
gs::cnf::callback_return_type dvectorcache::int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Dcache internal power is constant !!
  return GC_RETURN_OK;
}

// Switching power callback
gs::cnf::callback_return_type dvectorcache::swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  swi_power = ((dyn_tag_read_energy * dyn_tag_reads) + (dyn_tag_write_energy * dyn_tag_writes) + (dyn_data_read_energy * dyn_data_reads) + (dyn_data_write_energy * dyn_data_writes)) / (sc_time_stamp() - power_frame_starting_time).to_seconds();
  return GC_RETURN_OK;
}

// Automatically called at start of simulation
void dvectorcache::start_of_simulation() {

  // Initialize power model
  if (m_pow_mon) {

    power_model();

  }
}
/// @}