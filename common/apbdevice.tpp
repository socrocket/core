// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file apbdevice.cpp
/// Contains the implementation of a baseclass for all apb tlm models. It
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

#include "core/common/verbose.h"

template<class BASE>
APBDevice<BASE>::APBDevice(
    ModuleName mn,
    uint32_t busid,
    uint8_t vendorid,
    uint16_t deviceid,
    uint8_t version,
    uint8_t irq,
    AMBADeviceType type,
    uint16_t mask,
    bool cacheable,
    bool prefetchable,
    uint16_t address) :
  BASE(mn),
  g_pindex("pindex", busid, this->m_generics),
  g_pvendorid("pvendorid", vendorid, this->m_generics),
  g_pdeviceid("pdeviceid", deviceid, this->m_generics),
  g_pversion("pversion", version, this->m_generics),
  g_pirq("pirq", irq, this->m_generics),
  g_paddr("paddr", address, this->m_generics),
  g_pmask("pmask", mask, this->m_generics),
  g_ptype("ptype", type, this->m_generics),
  g_pcacheable("pcacheable", cacheable, this->m_generics),
  g_pprefetchable("pprefetchable", prefetchable, this->m_generics) {
  init_apb_generics();
}

template<class BASE>
APBDevice<BASE>::APBDevice(ModuleName mn) :
  BASE(mn),
  g_pindex("pindex", this->m_generics, false, false),
  g_pvendorid("pvendorid", this->m_generics, false, false),
  g_pdeviceid("pdeviceid", this->m_generics, false, false),
  g_pversion("pversion", this->m_generics, false, false),
  g_pirq("pirq", this->m_generics, false, false),
  g_paddr("paddr", this->m_generics, false, false),
  g_pmask("pmask", this->m_generics, false, false),
  g_ptype("ptype", this->m_generics, false, false),
  g_pcacheable("pcacheable", this->m_generics, false, false),
  g_pprefetchable("pprefetchable", this->m_generics, false, false) {
  init_apb_generics();
}

template<class BASE>
void APBDevice<BASE>::init_apb(uint32_t pindex, uint8_t pvendorid, uint16_t pdeviceid, uint8_t pversion,
      uint8_t pirq, AMBADeviceType ptype, uint16_t pmask,
      bool pcacheable, bool pprefetchable, uint16_t paddr) {
  g_pindex.init(pindex);
  this->m_api->addPar(&g_pindex);
  g_pvendorid.init(pvendorid);
  this->m_api->addPar(&g_pvendorid);
  g_pdeviceid.init(pdeviceid);
  this->m_api->addPar(&g_pdeviceid);
  g_pversion.init(pversion);
  this->m_api->addPar(&g_pversion);
  g_pirq.init(pirq);
  this->m_api->addPar(&g_pirq);
  g_paddr.init(paddr);
  this->m_api->addPar(&g_paddr);
  g_pmask.init(pmask);
  this->m_api->addPar(&g_pmask);
  g_ptype.init(ptype);
  this->m_api->addPar(&g_ptype);
  g_pcacheable.init(pcacheable);
  this->m_api->addPar(&g_pcacheable);
  g_pprefetchable.init(pprefetchable);
  this->m_api->addPar(&g_pprefetchable);
}

template<class BASE>
void APBDevice<BASE>::init_apb_generics() {
  g_pindex.add_properties()
    ("name", "APB Bus Index")
    ("The APB Bus Index is used to determinate the bus priority");

  g_pvendorid.add_properties()
    ("name", "Vendor Identifier")
    ("Sets the vendor identifier field in the AMBA Plug and Play area");

  g_pdeviceid.add_properties()
    ("name", "Device Type Identifier")
    ("Sets the device identifier field in the AMBA Plug and Play area");

  g_pversion.add_properties()
    ("name", "Version Number")
    ("The version number of the IP Core. Is used in the AMBA Plug and Play area");

  g_pirq.add_properties()
    ("name", "Interrupt Line")
    ("This generic is used to identify the interrupt line for this IP Core");

  g_paddr.add_properties()
    ("name", "APB Base Address")
    ("min", "0x000")
    ("max", "0xFFF")
    ("This address is used to identify the start of the memory area");

  g_pmask.add_properties()
    ("name", "APB Base Mask")
    ("min", "0x000")
    ("max", "0xFFF")
    ("This mask is used to identify the size of the memory area");

  g_ptype.add_properties()
    ("name", "APB Device Type")
    ("min", "0")
    ("max", "3")
    ("Defines the Type of the memory area");

  g_pcacheable.add_properties()
    ("name", "Cachable")
    ("Defines wether or not values in this memory area are cacheable");

  g_pprefetchable.add_properties()
    ("name", "Prefetchable")
    ("Defines wether or not values in this memory area are prefetchable");
}


template<class BASE>
APBDevice<BASE>::~APBDevice() {
}

template<class BASE>
uint16_t APBDevice<BASE>::get_apb_device_id() const {
  return g_pdeviceid & 0xFFF;
}

template<class BASE>
uint8_t APBDevice<BASE>::get_apb_vendor_id() const {
  return g_pvendorid & 0xFF;
}

template<class BASE>
void APBDevice<BASE>::print_apb_device_info(char *name) const {
  // Display APB slave information
  v::info << name << "APB slave @" << v::uint32 << get_apb_base_addr_()
          << " size: " << v::uint32 << get_apb_size_() << " byte" << v::endl;
}

// Returns pointer to PNP device information
template<class BASE>
const uint32_t *APBDevice<BASE>::get_apb_device_info() {
  m_register[0] = (g_pirq & 0x1F) | ((g_pversion & 0x1F) << 5)
                  | ((g_pdeviceid & 0xFFF) << 12) | (g_pvendorid << 24);
  m_register[1] = (static_cast<uint8_t>(g_ptype & 0xF) | (g_pmask << 4) |
                   (g_pcacheable << 16) | (g_pprefetchable << 17) | (g_paddr << 20));
  return m_register;
}

// Returns the device type
template<class BASE>
const AMBADeviceType APBDevice<BASE>::get_apb_type() const {
  return static_cast<AMBADeviceType>(g_ptype & 0xf);
}

// Returns the 12 bit MSB address of the device
template<class BASE>
uint32_t APBDevice<BASE>::get_apb_base() const {
  return g_paddr & 0xFFF;
}

// Returns the 12 bit address mask of the device
template<class BASE>
uint32_t APBDevice<BASE>::get_apb_mask() const {
  return g_pmask & 0xFFF;
}

// The 32 bit base address of the device
// (Required for compatiblity with GreenSocs)
template<class BASE>
sc_dt::uint64 APBDevice<BASE>::get_apb_base_addr() {
  uint32_t addr = get_apb_base();
  uint32_t mask = get_apb_mask();
  return (addr & mask) << 8;
}

// The 32 bit base address of the device
template<class BASE>
uint32_t APBDevice<BASE>::get_apb_base_addr_() const {
  uint32_t addr = get_apb_base();
  uint32_t mask = get_apb_mask();
  return (addr & mask) << 8;
}

// Size of the device address space in bytes
// (Required for compatibility with GreenSocs)
template<class BASE>
sc_dt::uint64 APBDevice<BASE>::get_apb_size() {
  uint32_t mask = get_apb_mask();
  return ((~mask & 0xFFF) + 1) << 8;
}

// Size of the device address space in bytes
template<class BASE>
uint32_t APBDevice<BASE>::get_apb_size_() const {
  uint32_t mask = get_apb_mask();
  return ((~mask & 0xFFF) + 1) << 8;
}

template<class BASE>
uint32_t APBDevice<BASE>::get_apb_relative_addr(uint32_t addr) const {
  return addr - get_apb_base_addr_();
}

// Returns the bus id of the device
template<class BASE>
uint32_t APBDevice<BASE>::get_apb_pindex() const {
  return g_pindex;
}
/// @}
