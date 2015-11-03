// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup memory Memory
/// @{
/// @file memory.cpp
/// Implementation of the generic memory model to be used with the SoCRocket
/// MCTRL. Can be configured as ROM, IO, SRAM or SDRAM. Underlying memory is
/// implemented as a flexible vmap. Recommended for simulation of large,
/// sparsely populated memories.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Dennis Bode
///

#include "gaisler/memory/memory.h"

#include <tlm.h>

using namespace sc_core;
using namespace std;
using namespace tlm;

SR_HAS_MODULE(Memory);

// Constructor implementation
Memory::Memory(ModuleName name,
  MEMDevice::device_type type,
  uint32_t banks,
  uint32_t bsize,
  uint32_t bits,
  uint32_t cols,
  std::string implementation,
  bool powmon) :
  MemoryPower(name, type, banks, bsize, bits, cols, powmon),
  bus("bus"),
  m_writes("bytes_written", 0ull, m_performance_counters),
  m_reads("bytes_read", 0ull, m_performance_counters),
  g_storage_type("storage", implementation, m_generics),
  g_elf_file("elf_file", "", m_generics) {
  // TLM 2.0 socket configuration
  gs::socket::config<tlm::tlm_base_protocol_types> bus_cfg;
  bus_cfg.use_mandatory_phase(BEGIN_REQ);
  bus_cfg.use_mandatory_phase(END_REQ);
  // mem_cfg.treat_unknown_as_ignorable();
  bus.set_config(bus_cfg);

  // Obtain pointer to GreenControl API
  m_api = gs::cnf::GCnf_Api::getApiInstance(this);

  // Register TLM 2.0 transport functions
  bus.register_b_transport(this, &Memory::b_transport);
  bus.register_transport_dbg(this, &Memory::transport_dbg);
  bus.register_get_direct_mem_ptr(this, &Memory::get_direct_mem_ptr);

  GC_REGISTER_TYPED_PARAM_CALLBACK(&m_reads, gs::cnf::pre_read, Memory, m_reads_cb);
  GC_REGISTER_TYPED_PARAM_CALLBACK(&m_writes, gs::cnf::pre_read, Memory, m_writes_cb);

  // Module configuration report
  srInfo()
    ("device_type", get_type_name())
    ("banks", banks)
    ("bsize", bsize)
    ("bit_width", bits)
    ("cols", cols)
    ("implementation", implementation)
    ("pow_mon", powmon)
    ("Ceated a Memory with this parameters");
}

Memory::~Memory() {
  GC_UNREGISTER_CALLBACKS();
}

void Memory::before_end_of_elaboration() {
  set_storage(g_storage_type, get_size());
}

// Automatically called at start of simulation
void Memory::start_of_simulation() {
  // Intitialize power model
  power_model();
}

// Print execution statistic at end of simulation
void Memory::end_of_simulation() {
  v::report << name() << " ********************************************" << v::endl;
  v::report << name() << " * " << get_type_name() << " Memory Statistic:" << v::endl;
  v::report << name() << " * -----------------------------------------" << v::endl;
  v::report << name() << " * Bytes read:    " << m_reads << v::endl;
  v::report << name() << " * Bytes written: " << m_writes << v::endl;
  v::report << name() << " ******************************************** " << v::endl;
}

// read count callback
gs::cnf::callback_return_type Memory::m_reads_cb(gs::gs_param_base &changed_param,
  gs::cnf::callback_type reason) {
  m_reads = reads;
  return GC_RETURN_OK;
}

// write count callback
gs::cnf::callback_return_type Memory::m_writes_cb(gs::gs_param_base &changed_param,
  gs::cnf::callback_type reason) {
  m_writes = writes;
  return GC_RETURN_OK;
}

// TLM 2.0 blocking transport function
void Memory::b_transport(tlm::tlm_generic_payload &gp, sc_time &delay) {
  // Extract erase extension
  ext_erase *ers;
  gp.get_extension(ers);
  gp.set_dmi_allowed(m_storage->allow_dmi_rw());

  if (ers) {
    // Check erase extension first:
    // The start address is encoded in the TLM address field.
    uint32_t start = gp.get_address();
    // The end address is encoded in the TLM data field as a uint32_t.
    uint32_t end = *reinterpret_cast<uint32_t *>(gp.get_data_ptr());

    if (end < start) {
      v::error << name() << "Error in erasing memory!" << v::endl;

      gp.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    } else {
      // Erase designated memory region
      erase(start, end);

      gp.set_response_status(tlm::TLM_OK_RESPONSE);

      // Count write operations for power calculation
      // dyn_writes += (end-start) >> 2;

      v::debug << name() << "Erase memory from " << v::uint32 << start << " to " << v::uint32 << end << "." << v::endl;
    }
  } else {
    // Read or write transaction
    tlm::tlm_command cmd = gp.get_command();
    uint32_t addr        = gp.get_address();
    uint32_t len         = gp.get_data_length();
    unsigned char *ptr   = gp.get_data_ptr();

    if (cmd == tlm::TLM_READ_COMMAND) {
      read_block(addr, ptr, len);

      srAnalyse()
        ("addr", addr)
        ("len", len)
        ("type", "read")
        ("Memory read transaction");

      gp.set_response_status(tlm::TLM_OK_RESPONSE);
    } else if (cmd == tlm::TLM_WRITE_COMMAND) {
      write_block(addr, ptr, len);

      srAnalyse()
        ("addr", addr)
        ("len", len)
        ("type", "write")
        ("Memory write transaction");

      gp.set_response_status(tlm::TLM_OK_RESPONSE);
    } else {
      srWarn()
        ("addr", addr)
        ("len", len)
        ("Command not valid / or TLM_IGNORE");
    }
  }
}

// TLM 2.0 debug transport function
unsigned int Memory::transport_dbg(tlm::tlm_generic_payload &gp) {
  tlm::tlm_command cmd = gp.get_command();
  uint32_t addr        = gp.get_address();
  uint32_t len         = gp.get_data_length();
  unsigned char *ptr   = gp.get_data_ptr();

  switch (cmd) {
  case tlm::TLM_READ_COMMAND:

    read_block_dbg(addr, ptr, len);

    v::debug << name() << "Debug read memory at " << v::uint32 << addr << " with length " << len << "." << v::endl;
    gp.set_response_status(tlm::TLM_OK_RESPONSE);
    return len;

  case tlm::TLM_WRITE_COMMAND:

    write_block_dbg(addr, ptr, len);

    v::debug << name() << "Debug write memory at " << v::uint32 << addr << " with length " << len << "." << v::endl;
    gp.set_response_status(tlm::TLM_OK_RESPONSE);
    return len;

  default:
    return 0;
  }
}

bool Memory::get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data) {
  // access to ROM adress space
  dmi_data.allow_read_write();
  dmi_data.set_dmi_ptr(m_storage->get_dmi_ptr());
  dmi_data.set_start_address(0);
  dmi_data.set_end_address(get_bsize() * ((get_banks()<5)? get_banks() : 8));
  dmi_data.set_read_latency(SC_ZERO_TIME);
  dmi_data.set_write_latency(SC_ZERO_TIME);
  return m_storage->allow_dmi_rw();
}

/// @}
