// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file localram.cpp
/// Implementation of scratchpad/localram. Can be attached to the icache and
/// dcache controllers. The localram enables fast 0-waitstate access to
/// instructions or data.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include "gaisler/leon3/mmucache/localram.h"
#include "core/common/verbose.h"

/// constructor
localram::localram(ModuleName name,
		   unsigned int lrsize,
                   unsigned int lrstart,
		   bool pow_mon) :
                   sc_module(name),
       m_default_entry("default_entry", 1),
		   m_lrsize(lrsize<<10),
		   m_lrstart(lrstart << 24),
		   m_pow_mon(pow_mon),
		   m_performance_counters("performance_counters"),
		   sreads("read_transactions", 0ull, m_performance_counters),
		   swrites("written_transactions", 0ull, m_performance_counters),
		   sreads_byte("bytes_read", 0ull, m_performance_counters),
		   swrites_byte("bytes_written", 0ull, m_performance_counters),
                   sta_power_norm("power.mmu_cache.localram.sta_power_norm", 1269.53125, true), // Normalized static power input
                   int_power_norm("power.mmu_cache.localram.int_power_norm", 1.61011e-12, true), // Normalized dynamic internal power input (activ. independent)
                   dyn_read_energy_norm("power.mmu_cache.localram.dyn_read_energy_norm", 7.57408e-13, true), // Normalized read energy input
                   dyn_write_energy_norm("power.mmu_cache.localram.dyn_write_energy_norm", 7.57408e-13, true), // Normalized write energy iput
                   power("power"),
                   sta_power("sta_power", 0.0, power), // Static power output
                   int_power("int_power", 0.0, power), // Internal power (activation independent)
                   swi_power("swi_power", 0.0, power), // Switching power
                   power_frame_starting_time("power_frame_starting_time", SC_ZERO_TIME, power),
                   dyn_read_energy("dyn_read_energy", 0.0, power), // Energy per read access
                   dyn_write_energy("dyn_write_energy", 0.0, power), // Energy per write access
                   dyn_reads("dyn_reads", 0ull, power), // Read access counter for power computation
                   dyn_writes("dyn_writes", 0ull, power) // Write access counter for power computation
{

    // Parameter check
    // ---------------
    // Scratchpad size max 512 kbyte
    assert(m_lrsize <= 524288);

    m_api = gs::cnf::GCnf_Api::getApiInstance(this);

    // Initialize allocator
    m_default_entry.set_int(0, 0);

    // Create the actual ram
    scratchpad = new t_cache_data("scratchpad", m_lrsize>>2);

    // Register power callback functions
    if (m_pow_mon) {

      GC_REGISTER_TYPED_PARAM_CALLBACK(&sta_power, gs::cnf::pre_read, localram, sta_power_cb);
      GC_REGISTER_TYPED_PARAM_CALLBACK(&int_power, gs::cnf::pre_read, localram, int_power_cb);
      GC_REGISTER_TYPED_PARAM_CALLBACK(&swi_power, gs::cnf::pre_read, localram, swi_power_cb);

    }

    // Configuration report
    v::info << this->name() << " ******************************************************************************* " << v::endl;
    v::info << this->name() << " * Created localram with following parameters:                                   " << v::endl;
    v::info << this->name() << " * ------------------------------------------- " << v::endl;
    v::info << this->name() << " * lrstart (start address): " << std::hex << m_lrstart << v::endl;
    v::info << this->name() << " * lrsize: " << std::hex << m_lrsize  << " bytes" << v::endl;
    v::info << this->name() << " ******************************************************************************* "  << v::endl;

}

// Destructor
localram::~localram() {

  // Free the memory
  delete (scratchpad);

  GC_UNREGISTER_CALLBACKS();

}

// Read from scratchpad
bool localram::mem_read(unsigned int addr, unsigned int asi, unsigned char *data, unsigned int len,
			sc_core::sc_time *delay, unsigned int *debug, bool is_dbg, bool &cacheable) {

  if(!((addr - m_lrstart) < m_lrsize)) {

    v::error << name() << "Read with address " << hex << addr << " out of range!!" << v::endl;

  }

  // Byte offset
  unsigned int byt = addr & 0x3;

  // Copy data to payload pointer
  for (unsigned int i = 0; i < len; i++) {
    //*(data + i) = scratchpad[(addr - m_lrstart) >> 2].c[byt + i];
    *(data + i) = scratchpad->get_char((addr - m_lrstart) >> 2, byt + i);
    sreads_byte++;
  }

  v::debug << this->name() << "Read from address: " << std::hex << addr << v::endl;

  // Increment read counter (statistics)
  sreads++;

  // Update debug information
  SCRATCHPAD_SET(*debug);

  *delay = ((len-1)>>2)*clockcycle;

  return true;

}

// Write to scratchpad
void localram::mem_write(unsigned int addr, unsigned int asi, unsigned char *data, unsigned int len,
			 sc_core::sc_time *delay, unsigned int *debug, bool is_dbg, bool &cacheable) {

  if(!((addr - m_lrstart) < m_lrsize)) {

    v::error << name() << "Write with address " << hex << addr << " out of range!!" << v::endl;

  }

  // byte offset
  unsigned int byt = addr & 0x3;

  // memcpy ??
  for (unsigned int i = 0; i < len; i++) {
    //scratchpad[(addr - m_lrstart) >> 2].c[byt + i] = *(data + i);
    scratchpad->set_char((addr - m_lrstart) >> 2, byt + i, *(data + i));
    swrites_byte++;
  }

  v::debug << this->name() << "Write to address: " << std::hex << addr << v::endl;

  // Increment write counter (statistics)
  swrites++;

  // update debug information
  SCRATCHPAD_SET(*debug);

  *delay = ((len-1)>>2)*clockcycle;

}

// Automatically called at the beginning of the simulation
void localram::start_of_simulation() {

  // Initialize power model
  if (m_pow_mon) {

    power_model();

  }
}

// Calculate power/energy values from normalized input data
void localram::power_model() {

  // Static power calculatin (pW)
  sta_power = sta_power_norm * (m_lrsize << 3);

  // Cell internal power (uW)
  int_power = int_power_norm * (m_lrsize << 3) * 1/(clockcycle.to_seconds());

  // Energy per read access (uJ)
  dyn_read_energy = dyn_read_energy_norm * 32 * (m_lrsize << 3);

  // Energy per write access (uJ)
  dyn_write_energy = dyn_write_energy_norm * 32 * (m_lrsize << 3);

}

// Static power callback
gs::cnf::callback_return_type localram::sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Static power of localram is constant !!
  return GC_RETURN_OK;
}

// Internal power callback
gs::cnf::callback_return_type localram::int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Internal power of localram is constant !!
  return GC_RETURN_OK;
}

// Switching power callback
gs::cnf::callback_return_type localram::swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  swi_power = ((dyn_read_energy * dyn_reads) + (dyn_write_energy * dyn_writes)) / (sc_time_stamp() - power_frame_starting_time).to_seconds();
  return GC_RETURN_OK;
}

// Print execution statistic at end of simulation
void localram::end_of_simulation() {

  // Localram execution statistic
  v::report << name() << " ******************************************************* " << v::endl;
  v::report << name() << " * Scratchpad statisitics: " << v::endl;
  v::report << name() << " * -----------------------" << v::endl;
  v::report << name() << " * Read accesses:  " << sreads  << " (Bytes: " << sreads_byte << ")" << v::endl;
  v::report << name() << " * Write accesses: " << swrites << " (Bytes: " << swrites_byte << ")" << v::endl;
  v::report << name() << " ******************************************************* " << v::endl;

}

// Helper for setting clock cycle latency using sc_clock argument
void localram::clkcng(sc_core::sc_time &clk) {
  clockcycle = clk;
}
/// @}
