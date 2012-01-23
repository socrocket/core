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
// Title:      ahbdevice.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    contains the implementation of a baseclass
//             for all ahb tlm models. It implements the the device
//             information register needed for the plug and play
//             interface.
//
// Method:
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#include "ahbdevice.h"
#include "verbose.h"
#include "math.h"

using namespace std;

AHBDevice::AHBDevice(uint32_t busid, uint8_t vendorid, uint16_t deviceid,
                     uint8_t version, uint8_t irq, uint32_t bar0,
                     uint32_t bar1, uint32_t bar2, uint32_t bar3) :
    m_reads(0), m_writes(0) {
    m_register[0] = (irq & 0x1F) | ((version & 0x1F) << 5)
            | ((deviceid & 0xFFF) << 12) | (vendorid << 24);
    m_register[1] = m_register[2] = m_register[3] = 0;
    m_register[4] = bar0;
    m_register[5] = bar1;
    m_register[6] = bar2;
    m_register[7] = bar3;

    m_busid = busid;
}

AHBDevice::~AHBDevice() {
}

const uint16_t AHBDevice::get_device_id() const {
  return (m_register[0] >> 12) & 0xFFF; 
}

const uint8_t AHBDevice::get_vendor_id() const {
  return (m_register[0] >> 24) & 0xFF; 
}

// Output APB slave information
void AHBDevice::print_device_info(char *name) const {

    v::info << name << "AHB slave @" << v::uint32 << (uint32_t)get_base_addr_()
                    << " size: " << v::uint32 << (uint32_t)get_size_() << " byte" << v::endl;
}

// Returns the device info record of the device
const uint32_t *AHBDevice::get_device_info() const {
    return m_register;
}

// Returns the device type entry for BAR bar
const AHBDevice::device_type AHBDevice::get_bar_type(uint32_t bar) const {
    return static_cast<AHBDevice::device_type>(m_register[4 + bar] & 0xf); 
}

// Extracts the 12bit MSB address for BAR bar
const uint32_t AHBDevice::get_bar_base(uint32_t bar) const {
    return (m_register[4 + bar] >> 20) & 0xFFF;
}

// Extracts the 12bit address mask for BAR bar.
const uint32_t AHBDevice::get_bar_mask(uint32_t bar) const {
    return  (m_register[4 + bar] >>  4) & 0xFFF;
}

// Calculates the base address (32bit) of the device for BAR bar.
const uint32_t AHBDevice::get_bar_addr(uint32_t bar) const {
    uint32_t addr = get_bar_base(bar);
    uint32_t mask = get_bar_mask(bar);
    return (addr & mask) << 20;
}

// Calculates the size in bytes of the device address space for BAR bar.
const uint32_t AHBDevice::get_bar_size(uint32_t bar) const {
    uint32_t mask = get_bar_mask(bar);
    return (((~mask & 0xFFF) + 1) << 20);
}

// Returns the base address (32bit) of the device (lowest of the BAR entries)
// (Required by GreenSocs dependencies)
sc_dt::uint64 AHBDevice::get_base_addr() {
    uint32_t addr = get_bar_addr(0);
    if(get_bar_addr(1)) {
        addr = min(addr, get_bar_addr(1));
    }
    if(get_bar_addr(2)) {
        addr = min(addr, get_bar_addr(2));
    }
    if(get_bar_addr(3)) {
        addr = min(addr, get_bar_addr(3));
    }
    return addr;
}

// Returns the base address (32bit) of the device (lowest of the BAR entries)
const uint32_t AHBDevice::get_base_addr_() const {
    uint32_t addr = get_bar_addr(0);
    if(get_bar_addr(1)) {
        addr = min(addr, get_bar_addr(1));
    }
    if(get_bar_addr(2)) {
        addr = min(addr, get_bar_addr(2));
    }
    if(get_bar_addr(3)) {
        addr = min(addr, get_bar_addr(3));
    }
    return addr;
}

// Returns the total size (bytes) of the device address space (all BARs)
// (Required by GreenSocs dependencies)
sc_dt::uint64 AHBDevice::get_size() {
    uint32_t addr = get_bar_addr(0);
    uint32_t size = get_bar_size(0);
    uint32_t old = addr;
    if(get_bar_addr(1)) {
        addr = max(addr, get_bar_addr(1));
        if(addr!=old) {
          size = get_bar_addr(1);
          old = addr;
        }
    }
    if(get_bar_addr(2)) {
        addr = max(addr, get_bar_addr(2));
        if(addr!=old) {
          size = get_bar_addr(2);
          old = addr;
        }
    }
    if(get_bar_addr(3)) {
        addr = max(addr, get_bar_addr(3));
        if(addr!=old) {
          size = get_bar_addr(3);
          old = addr;
        }
    }
    return (addr + size) - get_base_addr();
}

// Returns the total size (bytes) of the device address space (all BARs)
const uint32_t AHBDevice::get_size_() const {
    uint32_t addr = get_bar_addr(0);
    uint32_t size = get_bar_size(0);
    uint32_t old = addr;
    if(get_bar_addr(1)) {
        addr = max(addr, get_bar_addr(1));
        if(addr!=old) {
          size = get_bar_addr(1);
          old = addr;
        }
    }
    if(get_bar_addr(2)) {
        addr = max(addr, get_bar_addr(2));
        if(addr!=old) {
          size = get_bar_addr(2);
          old = addr;
        }
    }
    if(get_bar_addr(3)) {
        addr = max(addr, get_bar_addr(3));
        if(addr!=old) {
          size = get_bar_addr(3);
          old = addr;
        }
    }
    return (addr + size) - get_base_addr_();
}

uint32_t BAR(AHBDevice::device_type type, uint16_t mask, bool cacheable,
             bool prefetchable, uint16_t address) {
    return (static_cast<uint8_t>(type) | (mask << 4) | (cacheable << 16)
            | (prefetchable << 17) | (address << 20));
}

// Returns the bus id of the device
const uint32_t AHBDevice::get_busid() const {

  return m_busid;

}

void AHBDevice::transport_statistics(tlm::tlm_generic_payload &gp) {
  if(gp.is_write()) {
    m_writes += gp.get_data_length();
  } else if(gp.is_read()){
    m_reads += gp.get_data_length();
  }
}

void AHBDevice::print_transport_statistics(const char *name) const {
  v::report << name << " * Bytes read: " << m_reads << v::endl;
  v::report << name << " * Bytes written: " << m_writes << v::endl;
}
