// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup ahbmem
/// @{
/// @file ahbmem.cpp
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///
/// Provide a test bench memory class with AHB slave interface. DMI and
/// streaming width fields are ignored. Delays are not modeled. Address checking
/// is performed in that way, that transactions are only executed if the slave
/// select condition defined in grlib user manual holds. Transactions generating
/// a correct slave select but exceeding the memory region due to their length
/// are reported as warning and executed anyhow.

#include <fstream>

#include "models/ahbmem/ahbmem.h"
#include "common/verbose.h"

/// Constructor
AHBMem::AHBMem(const sc_core::sc_module_name nm,  // Module name
  uint16_t haddr_,                                // AMBA AHB address (12 bit)
  uint16_t hmask_,                                // AMBA AHB address mask (12 bit)
  amba::amba_layer_ids ambaLayer,                 // abstraction layer
  uint32_t slave_id,
  bool cacheable,
  uint32_t wait_states,
  bool pow_mon) :
    AHBSlave<>(nm,
      slave_id,
      0x01,                                       // Gaisler
      0x00E,                                      // AHB Mem,
      0,
      0,
      ambaLayer,
      BAR(AHBDevice::AHBMEM, hmask_, cacheable, 0, haddr_)),
    BaseMemory(BaseMemory::ARRAY, get_size()),
    ahbBaseAddress(static_cast<uint32_t>((hmask_) & haddr_) << 20),
    ahbSize(~(static_cast<uint32_t>(hmask_) << 20) + 1),
    mhaddr(haddr_),
    mhmask(hmask_),
    mcacheable(cacheable),
    mwait_states(wait_states),
    m_pow_mon(pow_mon),
    sta_power_norm("power.ahbmem.sta_power_norm", 1269.53125, true),                  // Normalized static power input
    int_power_norm("power.ahbmem.int_power_norm", 1.61011e-12, true),                 // Normalized internal power input
    dyn_read_energy_norm("power.ahbmem.dyn_read_energy_norm", 7.57408e-13, true),     // Normalized read energy input
    dyn_write_energy_norm("power.ahbmem.dyn_write_energy_norm", 7.57408e-13, true),   // Normalized write energy iput
    power("power"),
    sta_power("sta_power", 0.0, power),           // Static power output
    int_power("int_power", 0.0, power),           // Internal power of module (dyn. switching independent)
    swi_power("swi_power", 0.0, power),           // Switching power of modules
    power_frame_starting_time("power_frame_starting_time", SC_ZERO_TIME, power),
    dyn_read_energy("dyn_read_energy", 0.0, power),             // Energy per read access
    dyn_write_energy("dyn_write_energy", 0.0, power),           // Energy per write access
    dyn_reads("dyn_reads", 0ull, power),            // Read access counter for power computation
    dyn_writes("dyn_writes", 0ull, power) {         // Write access counter for power computation
  // haddr and hmask must be 12 bit
  assert(!((mhaddr | mhmask) >> 12));

  // Register power callback functions
  if (m_pow_mon) {
    GC_REGISTER_TYPED_PARAM_CALLBACK(&sta_power, gs::cnf::pre_read, AHBMem, sta_power_cb);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&int_power, gs::cnf::pre_read, AHBMem, int_power_cb);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&swi_power, gs::cnf::pre_read, AHBMem, swi_power_cb);
  }

  // Display AHB slave information
  v::info << name() << "********************************************************************" << v::endl;
  v::info << name() << "* Create AHB simulation memory with following parameters:           " << v::endl;
  v::info << name() << "* haddr/hmask: " << v::uint32 << mhaddr << "/" << v::uint32 << mhmask << v::endl;
  v::info << name() << "* hindex: " << slave_id << v::endl;
  v::info << name() << "* Slave base address: 0x" << std::setw(8) << std::setfill('0') << hex <<
  get_base_addr()                     << v::endl;
  v::info << name() << "* Slave size (bytes): 0x" << std::setw(8) << std::setfill('0') << hex <<
  get_size()                          << v::endl;
  v::info << name() << "********************************************************************" << v::endl;
}

void AHBMem::dorst() {
  erase(0, get_size()-1);
}

/// Destructor
AHBMem::~AHBMem() {
  // Delete memory contents
  GC_UNREGISTER_CALLBACKS();
}

/// Encapsulated functionality
uint32_t AHBMem::exec_func(
    tlm::tlm_generic_payload &trans,  // NOLINT(runtime/references)
    sc_core::sc_time &delay,          // NOLINT(runtime/references)
    bool debug) {
  uint32_t words_transferred;

  // Is the address for me
  if (!((mhaddr ^ (trans.get_address() >> 20)) & mhmask)) {
    // Warn if access exceeds slave memory region
    if ((trans.get_address() + trans.get_data_length()) >

        (ahbBaseAddress + ahbSize)) {
      v::warn << name() << "Transaction exceeds slave memory region" << endl;
    }

    if (trans.is_write()) {
      // write simulation memory
      write_block(trans.get_address(), trans.get_data_ptr(), trans.get_data_length());

      // Base delay is one clock cycle per word
      words_transferred = (trans.get_data_length() < 4) ? 1 : (trans.get_data_length() >> 2);

      if (m_pow_mon) {
        dyn_writes += words_transferred;
      }

      // Total delay is base delay + wait states
      delay += clock_cycle * (words_transferred + mwait_states);
      trans.set_response_status(tlm::TLM_OK_RESPONSE);
    } else {
      // read simulation memory
      read_block(trans.get_address(), trans.get_data_ptr(), trans.get_data_length());

      // Base delay is one clock cycle per word
      words_transferred = (trans.get_data_length() < 4) ? 1 : (trans.get_data_length() >> 2);

      if (m_pow_mon) {
        dyn_reads += words_transferred;
      }

      // Total delay is base delay + wait states
      delay += clock_cycle * (words_transferred + mwait_states);
      trans.set_response_status(tlm::TLM_OK_RESPONSE);

      // set cacheability
      if (mcacheable) {
        ahb.validate_extension<amba::amba_cacheable>(trans);
      }
    }

    v::debug << name() << "Delay increment: " << delay << v::endl;
  } else {
    // address not valid
    v::error << name() << "Address not within permissable slave memory space" << v::endl;
    trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
  }

  return trans.get_data_length();
}

// Returns clock cycle time for e.g. use in AHBSlave parent
sc_core::sc_time AHBMem::get_clock() {
  return clock_cycle;
}

void AHBMem::writeByteDBG(const uint32_t address, const uint8_t byte) {
  write_dbg(address, byte);
}

// Automatically called at the beginning of the simulation
void AHBMem::start_of_simulation() {
  // Initialize power model
  if (m_pow_mon) {
    power_model();
  }
}

// Calculate power/energy values from normalized input data
void AHBMem::power_model() {
  // Static power calculation (pW)
  sta_power = sta_power_norm * (get_size() << 3);

  // Cell internal power (uW)
  int_power = int_power_norm * (get_size() << 3) * 1 / (clock_cycle.to_seconds());

  // Energy per read access (uJ)
  dyn_read_energy =  dyn_read_energy_norm * 32 * (get_size() << 3);

  // Energy per write access (uJ)
  dyn_write_energy = dyn_write_energy_norm * 32 * (get_size() << 3);
}

// Static power callback
gs::cnf::callback_return_type AHBMem::sta_power_cb(
    gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
    gs::cnf::callback_type reason) {
  // Nothing to do !!
  // Static power of AHBMem is constant !!
  return GC_RETURN_OK;
}

// Internal power callback
gs::cnf::callback_return_type AHBMem::int_power_cb(
    gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
    gs::cnf::callback_type reason) {
  // Nothing to do !!
  // AHBMem internal power is constant !!
  return GC_RETURN_OK;
}

// Switching power callback
gs::cnf::callback_return_type AHBMem::swi_power_cb(
    gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
    gs::cnf::callback_type reason) {
  swi_power =
    ((dyn_read_energy *
      dyn_reads) + (dyn_write_energy * dyn_writes)) / (sc_time_stamp() - power_frame_starting_time).to_seconds();
  return GC_RETURN_OK;
}

// Automatically called at the end of the simulation
void AHBMem::end_of_simulation() {
  v::report << name() << " **************************************************** " << v::endl;
  v::report << name() << " * AHBMem Statistics: " << v::endl;
  v::report << name() << " * ------------------ " << v::endl;
  print_transport_statistics(name());
  v::report << name() << " * ************************************************** " << v::endl;
}
/// @}
