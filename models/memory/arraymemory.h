// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup memory
/// @{
/// @file arraymemory.h
/// Class defintion of the generic memory model to be used with the SoCRocket
/// MCTRL. Can be configured as ROM, IO, SRAM or SDRAM. Underlying memory is
/// implemented as a flat array. Recommended for fast simulation of small
/// memories.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Dennis Bode
///

#ifndef ARRAYMEMORY_H
#define ARRAYMEMORY_H

#include "memdevice.h"
#include "clkdevice.h"
#include "vmap.h"

#include <greencontrol/config.h>
#include <greensocket/target/single_socket.h>
#include "ext_erase.h"
#include <tlm.h>
#include <systemc.h>

#include "verbose.h"

/// @brief This class models a array memory. Depending on the configuration
/// it can be used as ROM, IO, SRAM or SDRAM, in conjunction with the SoCRocket MCTRL.
class ArrayMemory : public sc_core::sc_module, public MEMDevice, public CLKDevice {

 public:

  GC_HAS_CALLBACKS();

  /// Slave socket -  for communication with Mctrl
  gs::socket::target_socket<32> bus;

  /// Creates a new Instance of ArrayMemory
  ///
  /// @param name The SystemC name of the component to be created
  /// @param type The type of memory to be modeled (0-ROM, 1-IO, 2-SRAM, 3-SDRAM)
  /// @param banks Number of parallel banks
  /// @param bsize Size of one memory bank in bytes (all banks always considered to have equal size)
  /// @param bits Bit width of memory
  /// @param cols Number of SDRAM cols.
  ArrayMemory(sc_module_name name,
              MEMDevice::device_type type,
              uint32_t banks,
              uint32_t bsize,
              uint32_t bits,
              uint32_t cols = 0,
              bool pow_mon = false);

  /// Destructor
  ~ArrayMemory();

  uint8_t *memory;

  /// SystemC start of simulation callback
  void start_of_simulation();

  /// Calculate power/energy values from normalized input data
  void power_model();

  /// Static power callback
  gs::cnf::callback_return_type sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Dynamic/Internal power callback
  gs::cnf::callback_return_type int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Dynamic/Switching power callback
  gs::cnf::callback_return_type swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// SystemC end of simulation
  void end_of_simulation();

  /// TLM 2.0 blocking transport function
  void b_transport(tlm::tlm_generic_payload& gp, sc_time& delay);

  /// TLM 2.0 debug transport function
  unsigned int transport_dbg(tlm::tlm_generic_payload& gp);

  /// Read byte from functional memory
  uint8_t read(const uint32_t addr);

  /// Write byte to functional memory
  void write(const uint32_t addr, const uint8_t byte);

  /// Erase sdram - Required for deep power down and PASR mode
  void erase(uint32_t start, uint32_t end);

  /// Power monitoring
  bool m_pow_mon;

  /// GreenControl API Pointer
  gs::cnf::cnf_api *m_api;

  /// Performance Counter Array
  gs::gs_param_array m_performance_counters;

  /// Performance counter to store transaction byte reads
  gs::gs_param<unsigned long long> m_reads;

  /// Performance counter to store the transaction byte writes
  gs::gs_param<unsigned long long> m_writes;

  /// *****************************************************
  /// Power Modeling Parameters

  /// Normalized static power input
  gs::gs_param<double> sta_power_norm;

  /// Normalized internal power input (activation independent)
  gs::gs_param<double> int_power_norm;

  /// Normalized read access energy
  gs::gs_param<double> dyn_read_energy_norm;

  /// Normalized write access energy
  gs::gs_param<double> dyn_write_energy_norm;

  /// Parameter array for power data output
  gs::gs_param_array power;

  /// Static power of module
  gs::gs_param<double> sta_power;

  /// Internal power of module
  gs::gs_param<double> int_power;

  /// Switching power of module;
  gs::gs_param<double> swi_power;

  /// Power frame starting time
  gs::gs_param<sc_core::sc_time> power_frame_starting_time;

  /// Dynamic energy per read access
  gs::gs_param<double> dyn_read_energy;

  /// Dynamic energy per write access
  gs::gs_param<double> dyn_write_energy;

  /// Number of reads from memory (read & reset by monitor)
  gs::gs_param<unsigned long long> dyn_reads;

  /// Number of writes to memory (read & reset by monitor)
  gs::gs_param<unsigned long long> dyn_writes;
};

#endif
/// @}