//*********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is 
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      apbdevice.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Contains the implementation of a baseclass
//             for all apb tlm models. It implements the the device
//             information register needed for the plug and play
//             interface.
//
// Method:
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#include "apbdevice.h"
#include "verbose.h"

APBDevice::APBDevice(uint32_t busid, uint8_t vendorid, uint16_t deviceid,
                     uint8_t version, uint8_t irq, APBDevice::device_type type, 
                     uint16_t mask, bool cacheable,
                     bool prefetchable, uint16_t address) {
    m_register[0] = (irq & 0x1F) | ((version & 0x1F) << 5)
            | ((deviceid & 0xFFF) << 12) | (vendorid << 24);
    m_register[1] = (static_cast<uint8_t>(type) | (mask << 4) | 
                    (cacheable << 16) | (prefetchable << 17) | (address << 20));

    m_busid = busid;
}

APBDevice::~APBDevice() {
}

const uint16_t APBDevice::get_device_id() const {
    return (m_register[0] >> 12) & 0xFFF; 
}

const uint8_t APBDevice::get_vendor_id() const {
    return (m_register[0] >> 24) & 0xFF; 
}

void APBDevice::print_device_info(char *name) const {
    // Display APB slave information
    v::info << name << "APB slave @" << v::uint32 << get_base_addr_()
                    << " size: " << v::uint32 << get_size_() << " byte" << v::endl;
}

// Returns pointer to PNP device information
const uint32_t *APBDevice::get_device_info() const {
    return m_register;
}

// Returns the device type
const APBDevice::device_type APBDevice::get_type() const {
    return static_cast<APBDevice::device_type>(m_register[1] & 0xf);
}

// Returns the 12 bit MSB address of the device
const uint32_t APBDevice::get_base() const {
    return (m_register[1] >> 20) & 0xFFF;
}

// Returns the 12 bit address mask of the device
const uint32_t APBDevice::get_mask() const {
    return  (m_register[1] >>  4) & 0xFFF;
}

// The 32 bit base address of the device
// (Required for compatiblity with GreenSocs)
sc_dt::uint64 APBDevice::get_base_addr() {

    uint32_t addr = get_base();
    uint32_t mask = get_mask();
    return (addr & mask) << 8;
}

// The 32 bit base address of the device
const uint32_t APBDevice::get_base_addr_() const {

    uint32_t addr = get_base();
    uint32_t mask = get_mask();
    return (addr & mask) << 8;
}

// Size of the device address space in bytes
// (Required for compatibility with GreenSocs)
sc_dt::uint64 APBDevice::get_size() {
    uint32_t mask = get_mask();
    return (((~mask & 0xFFF) + 1) << 8);
}


// Size of the device address space in bytes
const uint32_t APBDevice::get_size_() const {
    uint32_t mask = get_mask();
    return (((~mask & 0xFFF) + 1) << 8);
}

const uint32_t APBDevice::get_relative_addr(uint32_t addr) const {
    return addr - get_base_addr_();
}


// Returns the bus id of the device
const uint32_t APBDevice::get_busid() const {

  return m_busid;

}
