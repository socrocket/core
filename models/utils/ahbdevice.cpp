// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file ahbdevice.cpp
/// contains the implementation of a baseclass for all ahb tlm models. It
/// implements the the device information register needed for the plug and play
/// interface.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#include <math.h>
#include <algorithm>
#include "models/utils/ahbdevice.h"
#include "common/verbose.h"

// Standard constructor
AHBDevice::AHBDevice(uint32_t busid, uint8_t vendorid, uint16_t deviceid,
  uint8_t version, uint8_t irq, uint32_t bar0,
  uint32_t bar1, uint32_t bar2, uint32_t bar3) {                  // :
  // m_performance_counters("performance_counters"),
  // m_reads("bytes_read", 0llu, m_performance_counters), m_writes("bytes_written", 0llu, m_performance_counters)
  m_register[0] = (irq & 0x1F) | ((version & 0x1F) << 5)
                  | ((deviceid & 0xFFF) << 12) | (vendorid << 24);
  m_register[1] = m_register[2] = m_register[3] = 0;
  m_register[4] = bar0;
  m_register[5] = bar1;
  m_register[6] = bar2;
  m_register[7] = bar3;

  m_busid = busid;
  // sc_module *self = dynamic_cast<sc_module *>(this);
  // if(self) {
  // m_api = gs::cnf::GCnf_Api::getApiInstance(self);
  // } else {
  // v::error << "AHBDevice" << "A AHBDevice instance must also inherit from sc_module when it gets instantiated. "
  //                        << "To ensure the performance counter will work correctly" << v::endl;
  // }
}

AHBDevice::~AHBDevice() {
}

const uint16_t AHBDevice::get_device_id() const throw() {
  return (m_register[0] >> 12) & 0xFFF;
}

const uint8_t AHBDevice::get_vendor_id() const throw() {
  return (m_register[0] >> 24) & 0xFF;
}

// Output APB slave information
void AHBDevice::print_device_info(char *name) const {
  v::info << name << "AHB slave @" << v::uint32 << (uint32_t)get_base_addr_()
          << " size: " << v::uint32 << (uint32_t)get_size_() << " byte" << v::endl;
}

// Returns the device info record of the device
const uint32_t *AHBDevice::get_device_info() const throw() {
  return m_register;
}

// Returns the device type entry for BAR bar
const AHBDevice::device_type AHBDevice::get_bar_type(uint32_t bar) const throw() {
  return static_cast<AHBDevice::device_type>(m_register[4 + bar] & 0xf);
}

// Extracts the 12bit MSB address for BAR bar
const uint32_t AHBDevice::get_bar_base(uint32_t bar) const throw() {
  return (m_register[4 + bar] >> 20) & 0xFFF;
}

// Extracts the 12bit address mask for BAR bar.
const uint32_t AHBDevice::get_bar_mask(uint32_t bar) const throw() {
  return (m_register[4 + bar] >>  4) & 0xFFF;
}

// Calculates the base address (32bit) of the device for BAR bar.
const uint32_t AHBDevice::get_bar_addr(uint32_t bar) const throw() {
  uint32_t addr = get_bar_base(bar);
  uint32_t mask = get_bar_mask(bar);
  return (addr & mask) << 20;
}

// Calculates the size in bytes of the device address space for BAR bar.
const uint32_t AHBDevice::get_bar_size(uint32_t bar) const throw() {
  uint32_t mask = get_bar_mask(bar);
  return ((~mask & 0xFFF) + 1) << 20;
}

const uint32_t AHBDevice::get_bar_relative_addr(uint32_t bar, uint32_t addr) const throw() {
  return addr - get_bar_addr(bar);
}

// Returns the base address (32bit) of the device (lowest of the BAR entries)
// (Required by GreenSocs dependencies)
sc_dt::uint64 AHBDevice::get_base_addr() throw() {
  uint32_t addr = get_bar_addr(0);
  if (get_bar_addr(1)) {
    addr = std::min(addr, get_bar_addr(1));
  }
  if (get_bar_addr(2)) {
    addr = std::min(addr, get_bar_addr(2));
  }
  if (get_bar_addr(3)) {
    addr = std::min(addr, get_bar_addr(3));
  }
  return addr;
}

// Returns the base address (32bit) of the device (lowest of the BAR entries)
const uint32_t AHBDevice::get_base_addr_() const throw() {
  uint32_t addr = get_bar_addr(0);
  if (get_bar_addr(1)) {
    addr = std::min(addr, get_bar_addr(1));
  }
  if (get_bar_addr(2)) {
    addr = std::min(addr, get_bar_addr(2));
  }
  if (get_bar_addr(3)) {
    addr = std::min(addr, get_bar_addr(3));
  }
  return addr;
}

// Returns the total size (bytes) of the device address space (all BARs)
// (Required by GreenSocs dependencies)
sc_dt::uint64 AHBDevice::get_size() throw() {
  uint32_t addr = get_bar_addr(0);
  uint32_t size = get_bar_size(0);
  uint32_t old = addr;
  if (get_bar_addr(1)) {
    addr = max(addr, get_bar_addr(1));
    if (addr != old) {
      size = get_bar_addr(1);
      old = addr;
    }
  }
  if (get_bar_addr(2)) {
    addr = max(addr, get_bar_addr(2));
    if (addr != old) {
      size = get_bar_addr(2);
      old = addr;
    }
  }
  if (get_bar_addr(3)) {
    addr = max(addr, get_bar_addr(3));
    if (addr != old) {
      size = get_bar_addr(3);
      old = addr;
    }
  }
  return (addr + size) - get_base_addr();
}

// Returns the total size (bytes) of the device address space (all BARs)
const uint32_t AHBDevice::get_size_() const throw() {
  uint32_t addr = get_bar_addr(0);
  uint32_t size = get_bar_size(0);
  uint32_t old = addr;
  if (get_bar_addr(1)) {
    addr = max(addr, get_bar_addr(1));
    if (addr != old) {
      size = get_bar_addr(1);
      old = addr;
    }
  }
  if (get_bar_addr(2)) {
    addr = max(addr, get_bar_addr(2));
    if (addr != old) {
      size = get_bar_addr(2);
      old = addr;
    }
  }
  if (get_bar_addr(3)) {
    addr = max(addr, get_bar_addr(3));
    if (addr != old) {
      size = get_bar_addr(3);
      old = addr;
    }
  }
  return (addr + size) - get_base_addr_();
}

uint32_t BAR(AHBDevice::device_type type, uint16_t mask, bool cacheable,
  bool prefetchable, uint16_t address) throw() {
  return static_cast<uint8_t>(type) | (mask << 4) | (cacheable << 16)
         | (prefetchable << 17) | (address << 20);
}

// Returns the bus id of the device
const uint32_t AHBDevice::get_busid() const throw() {
  return m_busid;
}

void AHBDevice::transport_statistics(tlm::tlm_generic_payload &gp) throw() {  // NOLINT(runtime/references)
  if (gp.is_write()) {
    // m_writes += gp.get_data_length();
  } else if (gp.is_read()) {
    // m_reads += gp.get_data_length();
  }
}

void AHBDevice::print_transport_statistics(const char *name) const throw() {
  // v::report << name << " * Bytes read: " << m_reads << v::endl;
  // v::report << name << " * Bytes written: " << m_writes << v::endl;
}
/// @}
