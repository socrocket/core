// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file memdevice.cpp
/// Contains the implementation of a baseclass for all memory tlm models. It
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

#include "core/common/memdevice.h"
#include "core/common/verbose.h"

MEMDevice::MEMDevice(sc_module_name name, MEMDevice::device_type type, uint32_t banks, uint32_t bsize, uint32_t bits, uint32_t cols) : 
  BaseModule<DefaultBase>(name),
  g_type("type", type, m_generics),
  g_banks("banks", banks, m_generics),
  g_bsize("bsize", bsize, m_generics),
  g_bits("bits", bits, m_generics),
  g_cols("cols", cols, m_generics) {
    init_mem_generics();
}

MEMDevice::~MEMDevice() {
}

void MEMDevice::init_mem_generics() {
  // TODO describe mem generics
  g_type.add_properties()
    ("enum", "ROM, IO, SRAM, SDRAM")
    ("Defines the type of the memory");
}

const char *MEMDevice::get_device_info() const {
  return NULL;
}

const MEMDevice::device_type MEMDevice::get_type() const {
  return MEMDevice::device_type(static_cast<uint32_t>(g_type));
}

const std::string MEMDevice::get_type_name() const {
  const std::string names[4] = {"rom", "io", "sram", "sdram"};
  return names[g_type];
}

const uint32_t MEMDevice::get_banks() const {
  return g_banks;
}

const uint32_t MEMDevice::get_bsize() const {
  return g_bsize;
}

const uint32_t MEMDevice::get_bits() const {
  return g_bits;
}

const uint32_t MEMDevice::get_cols() const {
  return g_cols;
}

const uint32_t MEMDevice::get_size() const {
  return g_bsize * ((g_banks < 5) ? g_banks : 8);
}
/// @}
