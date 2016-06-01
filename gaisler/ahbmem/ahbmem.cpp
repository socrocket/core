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

#include "gaisler/ahbmem/ahbmem.h"
#include "core/common/sr_report.h"
#include "core/common/verbose.h"
#include "core/common/sr_registry.h"

SR_HAS_MODULE(AHBMem);

/// Constructor
AHBMem::AHBMem(const ModuleName nm,  // Module name
  uint16_t haddr,                                 // AMBA AHB address (12 bit)
  uint16_t hmask,                                 // AMBA AHB address mask (12 bit)
  AbstractionLayer ambaLayer,                 // abstraction layer
  uint32_t hindex,
  bool cacheable,
  uint32_t wait_states,
  bool pow_mon) :
    AHBSlave<>(nm,
      hindex,
      0x01,                                       // Gaisler
      0x00E,                                      // AHB Mem,
      0,
      0,
      ambaLayer,
      BAR(AHBMEM, hmask, cacheable, 0, haddr)),
    g_haddr("haddr", haddr, m_generics),
    g_hmask("hmask", hmask, m_generics),
    //g_hindex("hindex", hindex, m_generics),
    g_cacheable("cacheable", cacheable, m_generics),
    g_wait_states("wait_states", wait_states, m_generics),
    g_pow_mon("pow_mon", pow_mon, m_generics),
    g_storage_type("storage", "ArrayStorage", m_generics),
    sta_power_norm("sta_power_norm", 1269.53125, m_power),                  // Normalized static power input
    int_power_norm("int_power_norm", 1.61011e-12, m_power),                 // Normalized internal power input
    dyn_read_energy_norm("dyn_read_energy_norm", 7.57408e-13, m_power),     // Normalized read energy input
    dyn_write_energy_norm("dyn_write_energy_norm", 7.57408e-13, m_power),   // Normalized write energy iput
    sta_power("sta_power", 0.0, m_power),           // Static power output
    int_power("int_power", 0.0, m_power),           // Internal power of module (dyn. switching independent)
    swi_power("swi_power", 0.0, m_power),           // Switching power of modules
    power_frame_starting_time("power_frame_starting_time", SC_ZERO_TIME, m_power),
    dyn_read_energy("dyn_read_energy", 0.0, m_power),             // Energy per read access
    dyn_write_energy("dyn_write_energy", 0.0, m_power),           // Energy per write access
    dyn_reads("dyn_reads", 0ull, m_power),            // Read access counter for power computation
    dyn_writes("dyn_writes", 0ull, m_power) {         // Write access counter for power computation
  // haddr and hmask must be 12 bit
  assert(!((g_haddr | g_hmask) >> 12));

  // Register power callback functions
  if (g_pow_mon) {
    GC_REGISTER_TYPED_PARAM_CALLBACK(&sta_power, gs::cnf::pre_read, AHBMem, sta_power_cb);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&int_power, gs::cnf::pre_read, AHBMem, int_power_cb);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&swi_power, gs::cnf::pre_read, AHBMem, swi_power_cb);
  }
  AHBMem::init_generics();
  ahb.register_get_direct_mem_ptr(this, &AHBMem::get_direct_mem_ptr);
  // Display AHB slave information
  srInfo("/configuration/ahbmem/ahbslave")
     ("addr", (uint64_t)get_ahb_base_addr())
     ("size", (uint64_t)get_ahb_size())
     ("AHB Slave Configuration");

  srInfo("/configuration/ahbmem/generics")
    ("haddr", g_haddr)
    ("hmask", g_hmask)
    //("hindex", g_hindex)
    ("cacheable", g_cacheable)
    ("wait_states", g_wait_states)
    ("Create AHB simulation memory");
}

AHBMem::AHBMem(
  const ModuleName nm,  // Module name
  AbstractionLayer ambaLayer,                 // abstraction layer
  uint16_t haddr,                                 // AMBA AHB address (12 bit)
  uint16_t hmask,                                 // AMBA AHB address mask (12 bit)
  uint32_t hindex,
  bool cacheable,
  uint32_t wait_states,
  bool pow_mon
  ):
    AHBSlave<>(nm,
      hindex,
      0x01,                                       // Gaisler
      0x00E,                                      // AHB Mem,
      0,
      0,
      ambaLayer,
      BAR(AHBMEM, hmask, cacheable, 0, haddr)),
    g_haddr("haddr", haddr, m_generics),
    g_hmask("hmask", hmask, m_generics),
    //g_hindex("hindex", hindex, m_generics),
    g_cacheable("cacheable", cacheable, m_generics),
    g_wait_states("wait_states", wait_states, m_generics),
    g_pow_mon("pow_mon", pow_mon, m_generics),
    g_storage_type("storage", "ArrayStorage", m_generics),
    sta_power_norm("sta_power_norm", 1269.53125, m_power),                  // Normalized static power input
    int_power_norm("int_power_norm", 1.61011e-12, m_power),                 // Normalized internal power input
    dyn_read_energy_norm("dyn_read_energy_norm", 7.57408e-13, m_power),     // Normalized read energy input
    dyn_write_energy_norm("dyn_write_energy_norm", 7.57408e-13, m_power),   // Normalized write energy iput
    sta_power("sta_power", 0.0, m_power),           // Static power output
    int_power("int_power", 0.0, m_power),           // Internal power of module (dyn. switching independent)
    swi_power("swi_power", 0.0, m_power),           // Switching power of modules
    power_frame_starting_time("power_frame_starting_time", SC_ZERO_TIME, m_power),
    dyn_read_energy("dyn_read_energy", 0.0, m_power),             // Energy per read access
    dyn_write_energy("dyn_write_energy", 0.0, m_power),           // Energy per write access
    dyn_reads("dyn_reads", 0ull, m_power),            // Read access counter for power computation
    dyn_writes("dyn_writes", 0ull, m_power) {         // Write access counter for power computation
  // haddr and hmask must be 12 bit
  assert(!((g_haddr | g_hmask) >> 12));

  // Register power callback functions
  if (g_pow_mon) {
    GC_REGISTER_TYPED_PARAM_CALLBACK(&sta_power, gs::cnf::pre_read, AHBMem, sta_power_cb);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&int_power, gs::cnf::pre_read, AHBMem, int_power_cb);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&swi_power, gs::cnf::pre_read, AHBMem, swi_power_cb);
  }
  AHBMem::init_generics();
  ahb.register_get_direct_mem_ptr(this, &AHBMem::get_direct_mem_ptr);
  // Display AHB slave information
  srInfo("/configuration/ahbmem/ahbslave")
     ("addr", (uint64_t)get_ahb_base_addr())
     ("size", (uint64_t)get_ahb_size())
     ("AHB Slave Configuration");

  srInfo("/configuration/ahbmem/generics")
    ("haddr", g_haddr)
    ("hmask", g_hmask)
    //("hindex", g_hindex)
    ("cacheable", g_cacheable)
    ("wait_states", g_wait_states)
    ("Create AHB simulation memory");
}

/// Destructor
AHBMem::~AHBMem() {
  // Delete memory contents
  GC_UNREGISTER_CALLBACKS();
}

void AHBMem::init_generics() {
  // set name, type, default, range, hint and description for gs_configs
  /*g_hindex.add_properties()
    ("name", "Bus Index")
    ("vhdl_name","hindex")
    ("range", "0..15")
    ("Slave index at the AHB bus");*/

  g_haddr.add_properties()
    ("name", "AHB Address")
    ("vhdl_name","haddr")
    ("base","hex")
    ("range", "0..0xFFF")
    ("The 12bit MSB address at the AHB bus");

  g_hmask.add_properties()
    ("name", "AHB Mask")
    ("vhdl_name","hmask")
    ("base","hex")
    ("range", "0..0xFFF")
    ("The 12bit AHB address mask");

  g_cacheable.add_properties()
    ("name", "Memory Cachability")
    ("If true the AHB Bus will set the cachability flag for all transactions from the memory");

  g_wait_states.add_properties()
    ("name", "Wait States")
    ("Number of wait states for each transaction");

  g_pow_mon.add_properties()
    ("name", "Power Monitoring")
    ("If true enable power monitoring");

  g_storage_type.add_properties()
    ("name", "Memory Storage Type")
    ("enum", "ArrayStorage, MapStorage")
    ("Defines the type of memory used as a backend implementation");
}

void AHBMem::dorst() {
  erase(0, get_ahb_size()-1);
}

/// Encapsulated functionality
uint32_t AHBMem::exec_func(
    tlm::tlm_generic_payload &trans,  // NOLINT(runtime/references)
    sc_core::sc_time &delay,          // NOLINT(runtime/references)
    bool debug) {
  uint32_t words_transferred;

  // Is the address for me
  if (!((g_haddr ^ (trans.get_address() >> 20)) & g_hmask)) {
    // Warn if access exceeds slave memory region
    if ((trans.get_address() + trans.get_data_length()) >

        (get_ahb_bar_addr(0) + get_ahb_bar_size(0))) {
      srWarn(name())
        ("base", get_ahb_bar_addr(0))
        ("size", get_ahb_bar_size(0))
        ("addr", trans.get_address())
        ("length", trans.get_data_length())
        ("Transaction exceeds slave memory region");
    }
    trans.set_dmi_allowed(m_storage->allow_dmi_rw());
    if (trans.is_write()) {
      // write simulation memory
      write_block(get_ahb_bar_relative_addr(0,trans.get_address()), trans.get_data_ptr(), trans.get_data_length());
      
      // Base delay is one clock cycle per word
      words_transferred = (trans.get_data_length() < 4) ? 1 : (trans.get_data_length() >> 2);
 
      if (g_pow_mon) {
        dyn_writes += words_transferred;
      }

      // Total delay is base delay + wait states
      delay += clock_cycle * (words_transferred + g_wait_states);
      trans.set_response_status(tlm::TLM_OK_RESPONSE);
    } else {
      // read simulation memory
      read_block(get_ahb_bar_relative_addr(0,trans.get_address()), trans.get_data_ptr(), trans.get_data_length());

      // Base delay is one clock cycle per word
      words_transferred = (trans.get_data_length() < 4) ? 1 : (trans.get_data_length() >> 2);

      if (g_pow_mon) {
        dyn_reads += words_transferred;
      }

      // Total delay is base delay + wait states
      delay += clock_cycle * (words_transferred + g_wait_states);
      trans.set_response_status(tlm::TLM_OK_RESPONSE);

      // set cacheability
      if (g_cacheable) {
        ahb.validate_extension<amba::amba_cacheable>(trans);
      }
    }

    srDebug(name())
      ("delay", delay)
      ("Delay increment!");
  } else {
    // address not valid
    srError(name())
      ("taddress", (uint64_t)trans.get_address())
      ("taddr", (uint64_t)trans.get_address() >> 20)
      ("haddr", g_haddr)
      ("hmask", g_hmask)
      ("Address not within permissable slave memory space");
    trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
  }

  return trans.get_data_length();
}

// Returns clock cycle time for e.g. use in AHBSlave parent
sc_core::sc_time AHBMem::get_clock() {
  return clock_cycle;
}

bool AHBMem::get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data) {
  // access to ROM adress space
  dmi_data.allow_read_write();
  dmi_data.set_dmi_ptr(m_storage->get_dmi_ptr());
  dmi_data.set_start_address(0);
  dmi_data.set_end_address(get_ahb_bar_size(0));
  dmi_data.set_read_latency(SC_ZERO_TIME);
  dmi_data.set_write_latency(SC_ZERO_TIME);
  v::info << name() << "allow_dmi_rw is: " << v::uint32 << m_storage->allow_dmi_rw() << v::endl;
  return m_storage->allow_dmi_rw();
}

void AHBMem::writeByteDBG(const uint32_t address, const uint8_t byte) {
  write_dbg(address, byte);
}

void AHBMem::end_of_elaboration() {
  set_storage(g_storage_type, get_ahb_bar_size(0));
}

// Automatically called at the beginning of the simulation
void AHBMem::start_of_simulation() {
  // Initialize power model
  if (g_pow_mon) {
    power_model();
  }
}

// Calculate power/energy values from normalized input data
void AHBMem::power_model() {
  // Static power calculation (pW)
  sta_power = sta_power_norm * (get_ahb_size() << 3);

  // Cell internal power (uW)
  int_power = int_power_norm * (get_ahb_size() << 3) * 1 / (clock_cycle.to_seconds());

  // Energy per read access (uJ)
  dyn_read_energy =  dyn_read_energy_norm * 32 * (get_ahb_size() << 3);

  // Energy per write access (uJ)
  dyn_write_energy = dyn_write_energy_norm * 32 * (get_ahb_size() << 3);
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
