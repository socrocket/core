// Calculate power/energy values from normalized input data

#include "memorypower.h"

static const char *power_params[4][4] = {
  // ROM
  // sta_norm,   int_norm,     dyn_read_e,     dyn_write_e
  {"1269.53125", "1.61011e-6", "7.57408e-13",  "7.57408e-13"},
  // IO
  // sta_norm,   int_norm,     dyn_read_e,     dyn_write_e
  {"1269.53125", "1.61011e-6", "7.57408e-13", "7.57408e-13"},
  // SRAM
  // sta_norm,   int_norm,     dyn_read_e,     dyn_write_e
  {"1269.53125", "1.61011e-6", "7.57408e-13", "7.57408e-13"},
  // SDRAM
  // sta_norm,   int_norm,     dyn_read_e,     dyn_write_e
  {"2539.0625", "3.22021e-6", "15e-13", "15e-13"}
};

MemoryPower::MemoryPower (sc_module_name name,
        MEMDevice::device_type type,
        uint32_t banks,
        uint32_t bsize,
        uint32_t bits,
        uint32_t cols,
        BaseMemory::implementation_type implementation,
        bool pow_mon):
//  m_performance_counters("performance_counters"),
//  m_reads("bytes_read", 0ull, m_performance_counters),
//  m_writes("bytes_writen", 0ull, m_performance_counters),
  sc_module(name),
  MEMDevice(type, banks, bsize, bits, cols),
  BaseMemory(implementation, bsize*((banks<5)?banks:8)),
  m_pow_mon(pow_mon),
  power("power"),
  sta_power_norm("sta_power_norm", power_params[type][0], power), // Normalized static power input
  int_power_norm("int_power_norm", power_params[type][1], power), // Normalized internal power input (act. independ)
  dyn_read_energy_norm("dyn_read_energy_norm", power_params[type][2], power), // Normalized read energy input
  dyn_write_energy_norm("dyn_write_energy_norm", power_params[type][3], power), // Normalized write energy input
  sta_power("sta_power", 0.0 , power),  // Static power output
  int_power("int_power", 0.0, power),  // Internal power output
  swi_power("swi_power", 0.0, power),  // Switching power output
  power_frame_starting_time("power_frame_starting_time", SC_ZERO_TIME, power),
  dyn_read_energy("dyn_read_energy", 0.0, power), // Energy per read access
  dyn_write_energy("dyn_write_energy", 0.0, power), // Energy per write access
  dyn_reads("dyn_reads", 0ull, power), // Read access counter for power computation
  dyn_writes("dyn_writes", 0ull, power) // Write access counter for power computation
{
 // Register power callback functions
 if (m_pow_mon) {
   GC_REGISTER_TYPED_PARAM_CALLBACK(&swi_power, gs::cnf::pre_read, MemoryPower, swi_power_cb);
   GC_REGISTER_TYPED_PARAM_CALLBACK(&dyn_reads, gs::cnf::pre_read, MemoryPower, dyn_reads_cb);
   GC_REGISTER_TYPED_PARAM_CALLBACK(&dyn_writes, gs::cnf::pre_read, MemoryPower, dyn_writes_cb);
 }
};

MemoryPower::~MemoryPower() {

  GC_UNREGISTER_CALLBACKS();

}

void MemoryPower::power_model()
{
  if (m_pow_mon) {
    // Static power calculation (pW)
    sta_power = sta_power_norm * (get_bsize() << 3);
   
    // Cell internal power (uW)
    int_power = int_power_norm * (get_bsize() << 3) * 1/(clock_cycle.to_seconds());
   
    // Energy per read access (uJ)
    dyn_read_energy =  dyn_read_energy_norm * 32 * (get_bsize() << 3);
   
    // Energy per write access (uJ)
    dyn_write_energy = dyn_write_energy_norm * 32 * (get_bsize() << 3);
  }
}

// switching power callback
gs::cnf::callback_return_type MemoryPower::swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {
  swi_power = ((dyn_read_energy * reads32) + (dyn_write_energy * writes32)) / (sc_time_stamp() - power_frame_starting_time).to_seconds();
  return GC_RETURN_OK;
}

gs::cnf::callback_return_type MemoryPower::dyn_reads_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {
  dyn_reads = reads32;
  return GC_RETURN_OK;
}

gs::cnf::callback_return_type MemoryPower::dyn_writes_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {
  dyn_writes = writes32;
  return GC_RETURN_OK;
}
