// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file ahbdevice.cpp
/// contains the implementation of a baseclass for all ahb tlm models. It
/// implements the the device information register needed for the plug and play
/// interface.
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
#include <math.h>
#include <algorithm>
#include "core/common/verbose.h"
#include "core/common/amba.h"

// Standard constructor
template<class BASE>
AHBDevice<BASE>::AHBDevice(
    ModuleName mn,
    uint32_t busid,
    uint8_t vendorid,
    uint16_t deviceid,
    uint8_t version,
    uint8_t irq,
    BAR bar0,
    BAR bar1,
    BAR bar2,
    BAR bar3) :
  BASE(mn),
  g_bars("bar", this->m_generics),
  g_bar0("0", g_bars),
  g_bar1("1", g_bars),
  g_bar2("2", g_bars),
  g_bar3("3", g_bars),
  g_hindex("hindex", busid, this->m_generics),
  g_hvendorid("vendorid", vendorid, this->m_generics),
  g_hdeviceid("deviceid", deviceid, this->m_generics),
  g_hversion("version", version, this->m_generics),
  g_hirq("hirq", irq, this->m_generics),
  g_bar0haddr("haddr", bar0.address, g_bar0),
  g_bar0hmask("hmask", bar0.mask, g_bar0),
  g_bar0htype("htype", bar0.type, g_bar0),
  g_bar0hcacheable("hcacheable", bar0.cacheable, g_bar0),
  g_bar0hprefetchable("hprefatchable", bar0.prefetchable, g_bar0),
  g_bar1haddr("haddr", bar1.address, g_bar1),
  g_bar1hmask("hmask", bar1.mask, g_bar1),
  g_bar1htype("htype", bar1.type, g_bar1),
  g_bar1hcacheable("hcacheable", bar1.cacheable, g_bar1),
  g_bar1hprefetchable("hprefatchable", bar1.prefetchable, g_bar1),
  g_bar2haddr("haddr", bar2.address, g_bar2),
  g_bar2hmask("hmask", bar2.mask, g_bar2),
  g_bar2htype("htype", bar2.type, g_bar2),
  g_bar2hcacheable("hcacheable", bar2.cacheable, g_bar2),
  g_bar2hprefetchable("hprefatchable", bar2.prefetchable, g_bar2),
  g_bar3haddr("haddr", bar3.address, g_bar3),
  g_bar3hmask("hmask", bar3.mask, g_bar3),
  g_bar3htype("htype", bar3.type, g_bar3),
  g_bar3hcacheable("hcacheable", bar3.cacheable, g_bar3),
  g_bar3hprefetchable("hprefatchable", bar3.prefetchable, g_bar3) {
  init_ahb_generics();
}

// Standard constructor
template<class BASE>
AHBDevice<BASE>::AHBDevice(
    ModuleName mn) :
  BASE(mn),
  g_bars("bar", this->m_generics),
  g_bar0("0", g_bars),
  g_bar1("1", g_bars),
  g_bar2("2", g_bars),
  g_bar3("3", g_bars),
  g_hindex("hindex",this->m_generics),
  g_hvendorid("vendorid",this->m_generics),
  g_hdeviceid("deviceid",this->m_generics),
  g_hversion("version",this->m_generics),
  g_hirq("hirq",this->m_generics),
  g_bar0haddr("haddr",g_bar0),
  g_bar0hmask("hmask",g_bar0),
  g_bar0htype("htype",g_bar0),
  g_bar0hcacheable("hcacheable",g_bar0),
  g_bar0hprefetchable("hprefatchable",g_bar0),
  g_bar1haddr("haddr",g_bar1),
  g_bar1hmask("hmask",g_bar1),
  g_bar1htype("htype",g_bar1),
  g_bar1hcacheable("hcacheable",g_bar1),
  g_bar1hprefetchable("hprefatchable",g_bar1),
  g_bar2haddr("haddr",g_bar2),
  g_bar2hmask("hmask",g_bar2),
  g_bar2htype("htype",g_bar2),
  g_bar2hcacheable("hcacheable",g_bar2),
  g_bar2hprefetchable("hprefatchable",g_bar2),
  g_bar3haddr("haddr",g_bar3),
  g_bar3hmask("hmask",g_bar3),
  g_bar3htype("htype",g_bar3),
  g_bar3hcacheable("hcacheable",g_bar3),
  g_bar3hprefetchable("hprefatchable",g_bar3) {
  init_ahb_generics();
}

// Standard constructor
template<class BASE>
void AHBDevice<BASE>::init(
    uint32_t busid,
    uint8_t vendorid,
    uint16_t deviceid,
    uint8_t version,
    uint8_t irq,
    BAR bar0,
    BAR bar1,
    BAR bar2,
    BAR bar3) {
  g_hindex.init(busid);
  g_hvendorid.init(vendorid);
  g_hdeviceid.init(deviceid);
  g_hversion.init(version);
  g_hirq.init(irq);
  g_bar0haddr.init(bar0.address);
  g_bar0hmask.init(bar0.mask);
  g_bar0htype.init(bar0.type);
  g_bar0hcacheable.init(bar0.cacheable);
  g_bar0hprefetchable.init(bar0.prefetchable);
  g_bar1haddr.init(bar1.address);
  g_bar1hmask.init(bar1.mask);
  g_bar1htype.init(bar1.type);
  g_bar1hcacheable.init(bar1.cacheable);
  g_bar1hprefetchable.init(bar1.prefetchable);
  g_bar2haddr.init(bar2.address);
  g_bar2hmask.init(bar2.mask);
  g_bar2htype.init(bar2.type);
  g_bar2hcacheable.init(bar2.cacheable);
  g_bar2hprefetchable.init(bar2.prefetchable);
  g_bar3haddr.init(bar3.address);
  g_bar3hmask.init(bar3.mask);
  g_bar3htype.init(bar3.type);
  g_bar3hcacheable.init(bar3.cacheable);
  g_bar3hprefetchable.init(bar3.prefetchable);
}

template<class BASE>
void AHBDevice<BASE>::init_ahb_generics() {
  g_hindex.add_properties()
    ("name", "AHB Bus Index")
    ("The AHB Bus Index is used to determinate the bus priority");

  g_hvendorid.add_properties()
    ("name", "Vendor Identifier")
    ("Sets the vendor identifier field in the AMBA Plug and Play area");

  g_hdeviceid.add_properties()
    ("name", "Device Type Identifier")
    ("Sets the device identifier field in the AMBA Plug and Play area");

  g_hversion.add_properties()
    ("name", "Version Number")
    ("The version number of the IP Core. Is used in the AMBA Plug and Play area");

  g_hirq.add_properties()
    ("name", "Interrupt Line")
    ("This generic is used to identify the interrupt line for this IP Core");

  g_bar0haddr.add_properties()
    ("name", "AHB Base Address for BAR0")
    ("min", "0x000")
    ("max", "0xFFF")
    ("This address is used to identify the start of the memory area behind BAR0");

  g_bar0hmask.add_properties()
    ("name", "AHB Base Mask for BAR0")
    ("min", "0x000")
    ("max", "0xFFF")
    ("This mask is used to identify the size of the memory area behind BAR0");

  g_bar0htype.add_properties()
    ("name", "AHB Device Type for BAR0")
    ("min", "0")
    ("max", "3")
    ("Defines the Type of the BAR0 memory area");

  g_bar0hcacheable.add_properties()
    ("name", "Cachable")
    ("Defines wether or not values in this memory area are cacheable");

  g_bar0hprefetchable.add_properties()
    ("name", "Prefetchable")
    ("Defines wether or not values in this memory area are prefetchable");

  g_bar1haddr.add_properties()
    ("name", "AHB Base Address for BAR1")
    ("min", "0x000")
    ("max", "0xFFF")
    ("This address is used to identify the start of the memory area behind BAR1");

  g_bar1hmask.add_properties()
    ("name", "AHB Base Mask for BAR1")
    ("min", "0x000")
    ("max", "0xFFF")
    ("This mask is used to identify the size of the memory area behind BAR1");

  g_bar1htype.add_properties()
    ("name", "AHB Device Type for BAR1")
    ("min", "0")
    ("max", "3")
    ("Defines the Type of the BAR1 memory area");

  g_bar1hcacheable.add_properties()
    ("name", "Cachable")
    ("Defines wether or not values in this memory area are cacheable");

  g_bar1hprefetchable.add_properties()
    ("name", "Prefetchable")
    ("Defines wether or not values in this memory area are prefetchable");

  g_bar2haddr.add_properties()
    ("name", "AHB Base Address for BAR2")
    ("min", "0x000")
    ("max", "0xFFF")
    ("This address is used to identify the start of the memory area behind BAR2");

  g_bar2hmask.add_properties()
    ("name", "AHB Base Mask for BAR2")
    ("min", "0x000")
    ("max", "0xFFF")
    ("This mask is used to identify the size of the memory area behind BAR2");

  g_bar2htype.add_properties()
    ("name", "AHB Device Type for BAR2")
    ("min", "0")
    ("max", "3")
    ("Defines the Type of the BAR2 memory area");

  g_bar2hcacheable.add_properties()
    ("name", "Cachable")
    ("Defines wether or not values in this memory area are cacheable");

  g_bar2hprefetchable.add_properties()
    ("name", "Prefetchable")
    ("Defines wether or not values in this memory area are prefetchable");

  g_bar3haddr.add_properties()
    ("name", "AHB Base Address for BAR3")
    ("min", "0x000")
    ("max", "0xFFF")
    ("This address is used to identify the start of the memory area behind BAR3");

  g_bar3hmask.add_properties()
    ("name", "AHB Base Mask for BAR3")
    ("min", "0x000")
    ("max", "0xFFF")
    ("This mask is used to identify the size of the memory area behind BAR3");

  g_bar3htype.add_properties()
    ("name", "AHB Device Type for BAR3")
    ("min", "0")
    ("max", "3")
    ("Defines the Type of the BAR3 memory area");

  g_bar3hcacheable.add_properties()
    ("name", "Cachable")
    ("Defines wether or not values in this memory area are cacheable");

  g_bar3hprefetchable.add_properties()
    ("name", "Prefetchable")
    ("Defines wether or not values in this memory area are prefetchable");
}

template<class BASE>
AHBDevice<BASE>::~AHBDevice() {
}

template<class BASE>
const uint16_t AHBDevice<BASE>::get_ahb_device_id() const throw() {
  return g_hdeviceid & 0xFFF;
}

template<class BASE>
const uint8_t AHBDevice<BASE>::get_ahb_vendor_id() const throw() {
  return g_hvendorid & 0xFF;
}

// Output APB slave information
template<class BASE>
void AHBDevice<BASE>::print_ahb_device_info(char *name) const {
  v::info << name << "AHB slave @" << v::uint32 << (uint32_t)get_ahb_base_addr_()
          << " size: " << v::uint32 << (uint32_t)get_ahb_size_() << " byte" << v::endl;
}

// Returns the device info record of the device
template<class BASE>
const uint32_t *AHBDevice<BASE>::get_ahb_device_info() throw() {
  m_register[0] = (g_hirq & 0x1F) | ((g_hversion & 0x1F) << 5)
                  | ((g_hdeviceid & 0xFFF) << 12) | (g_hvendorid << 24);
  m_register[1] = m_register[2] = m_register[3] = 0;
  m_register[4] = BAR(static_cast<AMBADeviceType>(g_bar0htype & 0xF), g_bar0hmask, g_bar0hcacheable, g_bar0hprefetchable, g_bar0haddr).toRegister();
  m_register[5] = BAR(static_cast<AMBADeviceType>(g_bar1htype & 0xF), g_bar1hmask, g_bar1hcacheable, g_bar1hprefetchable, g_bar1haddr).toRegister();
  m_register[6] = BAR(static_cast<AMBADeviceType>(g_bar2htype & 0xF), g_bar2hmask, g_bar2hcacheable, g_bar2hprefetchable, g_bar2haddr).toRegister();
  m_register[7] = BAR(static_cast<AMBADeviceType>(g_bar3htype & 0xF), g_bar3hmask, g_bar3hcacheable, g_bar3hprefetchable, g_bar3haddr).toRegister();
  return m_register;
}

// Returns the device type entry for BAR bar
template<class BASE>
const AMBADeviceType AHBDevice<BASE>::get_ahb_bar_type(uint32_t bar) const throw() {
  if(bar == 0) {
    return static_cast<AMBADeviceType>(g_bar0htype & 0xf);
  } else if(bar == 1) {
    return static_cast<AMBADeviceType>(g_bar1htype & 0xf);
  } else if(bar == 2) {
    return static_cast<AMBADeviceType>(g_bar2htype & 0xf);
  } else if(bar == 3) {
    return static_cast<AMBADeviceType>(g_bar3htype & 0xf);
  }
  return static_cast<AMBADeviceType>(g_bar0htype & 0xf);
}

// Extracts the 12bit MSB address for BAR bar
template<class BASE>
const uint32_t AHBDevice<BASE>::get_ahb_bar_base(uint32_t bar) const throw() {
  if(bar == 0) {
    return (g_bar0haddr & 0xFFF);
  } else if(bar == 1) {
    return (g_bar1haddr & 0xFFF);
  } else if(bar == 2) {
    return (g_bar2haddr & 0xFFF);
  } else if(bar == 3) {
    return (g_bar3haddr & 0xFFF);
  }
  return (g_bar0haddr & 0xFFF);
}

// Extracts the 12bit address mask for BAR bar.
template<class BASE>
const uint32_t AHBDevice<BASE>::get_ahb_bar_mask(uint32_t bar) const throw() {
  if(bar == 0) {
    return (g_bar0hmask & 0xFFF);
  } else if(bar == 1) {
    return (g_bar1hmask & 0xFFF);
  } else if(bar == 2) {
    return (g_bar2hmask & 0xFFF);
  } else if(bar == 3) {
    return (g_bar3hmask & 0xFFF);
  }
  return (g_bar0hmask & 0xFFF);
}

// Calculates the base address (32bit) of the device for BAR bar.
template<class BASE>
const uint32_t AHBDevice<BASE>::get_ahb_bar_addr(uint32_t bar) const throw() {
  uint32_t addr = get_ahb_bar_base(bar);
  uint32_t mask = get_ahb_bar_mask(bar);
  return (addr & mask) << 20;
}

// Calculates the size in bytes of the device address space for BAR bar.
template<class BASE>
const uint32_t AHBDevice<BASE>::get_ahb_bar_size(uint32_t bar) const throw() {
  uint32_t mask = get_ahb_bar_mask(bar);
  return ((~mask & 0xFFF) + 1) << 20;
}

template<class BASE>
const uint32_t AHBDevice<BASE>::get_ahb_bar_relative_addr(uint32_t bar, uint32_t addr) const throw() {
  return addr - get_ahb_bar_addr(bar);
}

// Returns the base address (32bit) of the device (lowest of the BAR entries)
// (Required by GreenSocs dependencies)
template<class BASE>
sc_dt::uint64 AHBDevice<BASE>::get_ahb_base_addr() throw() {
  uint32_t addr = get_ahb_bar_addr(0);
  if (get_ahb_bar_addr(1)) {
    addr = std::min(addr, get_ahb_bar_addr(1));
  }
  if (get_ahb_bar_addr(2)) {
    addr = std::min(addr, get_ahb_bar_addr(2));
  }
  if (get_ahb_bar_addr(3)) {
    addr = std::min(addr, get_ahb_bar_addr(3));
  }
  return addr;
}

// Returns the base address (32bit) of the device (lowest of the BAR entries)
template<class BASE>
const uint32_t AHBDevice<BASE>::get_ahb_base_addr_() const throw() {
  uint32_t addr = get_ahb_bar_addr(0);
  if (get_ahb_bar_addr(1)) {
    addr = std::min(addr, get_ahb_bar_addr(1));
  }
  if (get_ahb_bar_addr(2)) {
    addr = std::min(addr, get_ahb_bar_addr(2));
  }
  if (get_ahb_bar_addr(3)) {
    addr = std::min(addr, get_ahb_bar_addr(3));
  }
  return addr;
}

// Returns the total size (bytes) of the device address space (all BARs)
// (Required by GreenSocs dependencies)
template<class BASE>
sc_dt::uint64 AHBDevice<BASE>::get_ahb_size() throw() {
  uint32_t addr = get_ahb_bar_addr(0);
  uint32_t size = get_ahb_bar_size(0);
  uint32_t old = addr;
  if (get_ahb_bar_addr(1)) {
    addr = std::max(addr, get_ahb_bar_addr(1));
    if (addr != old) {
      size = get_ahb_bar_addr(1);
      old = addr;
    }
  }
  if (get_ahb_bar_addr(2)) {
    addr = std::max(addr, get_ahb_bar_addr(2));
    if (addr != old) {
      size = get_ahb_bar_addr(2);
      old = addr;
    }
  }
  if (get_ahb_bar_addr(3)) {
    addr = std::max(addr, get_ahb_bar_addr(3));
    if (addr != old) {
      size = get_ahb_bar_addr(3);
      old = addr;
    }
  }
  return (addr + size) - get_ahb_base_addr();
}

// Returns the total size (bytes) of the device address space (all BARs)
template<class BASE>
const uint32_t AHBDevice<BASE>::get_ahb_size_() const throw() {
  uint32_t addr = get_ahb_bar_addr(0);
  uint32_t size = get_ahb_bar_size(0);
  uint32_t old = addr;
  if (get_ahb_bar_addr(1)) {
    addr = std::max(addr, get_ahb_bar_addr(1));
    if (addr != old) {
      size = get_ahb_bar_addr(1);
      old = addr;
    }
  }
  if (get_ahb_bar_addr(2)) {
    addr = std::max(addr, get_ahb_bar_addr(2));
    if (addr != old) {
      size = get_ahb_bar_addr(2);
      old = addr;
    }
  }
  if (get_ahb_bar_addr(3)) {
    addr = std::max(addr, get_ahb_bar_addr(3));
    if (addr != old) {
      size = get_ahb_bar_addr(3);
      old = addr;
    }
  }
  return (addr + size) - get_ahb_base_addr_();
}

// Returns the bus id of the device
template<class BASE>
const uint32_t AHBDevice<BASE>::get_ahb_hindex() const throw() {
  return g_hindex;
}

template<class BASE>
void AHBDevice<BASE>::transport_statistics(tlm::tlm_generic_payload &gp) throw() {  // NOLINT(runtime/references)
  if (gp.is_write()) {
    // m_writes += gp.get_data_length();
  } else if (gp.is_read()) {
    // m_reads += gp.get_data_length();
  }
}

/// @}
