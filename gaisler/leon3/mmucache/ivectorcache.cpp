// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file ivectorcache.cpp
/// Implementation of an instruction cache. The cache can be configured direct
/// mapped or set associative. Set-size, line-size and replacement strategy can
/// be defined through constructor arguments.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include "gaisler/leon3/mmucache/ivectorcache.h"
#include "core/common/verbose.h"

// Overwrite write function
void ivectorcache::mem_write(unsigned int address, unsigned char * data,
                             unsigned int len, sc_core::sc_time * t,
                             unsigned int * debug, bool is_dbg, bool &cacheable) {

    v::info << this->name() << "Forbidden to write icache!" << v::endl;
    assert(false);

}

// Implement ccr check
unsigned int ivectorcache::check_mode() {

  unsigned int tmp = m_mmu_cache->read_ccr(true);

  return (tmp & 0x3);

}

// This is a instruction cache
cache_if::t_cache_type ivectorcache::get_cache_type() {

  return cache_if::icache;

}

// Calculate power/energy values from normalized input data
void ivectorcache::power_model() {

  // Static power = controller + itag ram + idata ram
  sta_power = sta_power_norm +
    sta_itag_power_norm * m_number_of_vectors * 32 +
    sta_idata_power_norm * pow(2.0, (double)m_setsize) * (m_sets + 1) * 8;

  // Cell internal power = controller + itag ram + idata ram
  int_power = int_power_norm * 1/(clockcycle.to_seconds()) +
    int_itag_power_norm * m_number_of_vectors * 32 * 1/(clockcycle.to_seconds()) +
    int_idata_power_norm * pow(2.0, (double)m_setsize) * (m_sets + 1) * 8;

  // Energy per itag read access (uJ)
  dyn_tag_read_energy = dyn_itag_read_energy_norm * m_number_of_vectors * 32 * 32;

  // Energy per itag write access (uJ)
  dyn_tag_write_energy = dyn_itag_write_energy_norm * m_number_of_vectors * 32 * 32;

  // Energy per idata read access (uJ)
  dyn_data_read_energy = dyn_idata_read_energy_norm * pow(2.0, (double)m_setsize) * 8 * 32;

  // Energy per idata write access (uJ)
  dyn_data_write_energy = dyn_idata_write_energy_norm * pow(2.0, (double)m_setsize) * 8 * 32;

}

// Static power callback
gs::cnf::callback_return_type ivectorcache::sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Static power of icache is constant !!
  return GC_RETURN_OK;
}

// Internal power callback
gs::cnf::callback_return_type ivectorcache::int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Icache internal power is constant !!
  return GC_RETURN_OK;
}

// Switching power callback
gs::cnf::callback_return_type ivectorcache::swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  swi_power = ((dyn_tag_read_energy * dyn_tag_reads) + (dyn_tag_write_energy * dyn_tag_writes) + (dyn_data_read_energy * dyn_data_reads) + (dyn_data_write_energy * dyn_data_writes)) / (sc_time_stamp() - power_frame_starting_time).to_seconds();
  return GC_RETURN_OK;
}

// Automatically called at start of simualtion
void ivectorcache::start_of_simulation() {

  // Initialize power model
  if (m_pow_mon) {

    power_model();

  }
}
/// @}
