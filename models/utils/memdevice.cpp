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

#include "core/models/utils/memdevice.h"
#include "core/common/verbose.h"

MEMDevice::MEMDevice(MEMDevice::device_type type, uint32_t banks, uint32_t bsize, uint32_t bits,
  uint32_t cols) : m_type(type), m_banks(banks), m_bsize(bsize), m_bits(bits), m_cols(cols) {
  switch (type) {
  case MEMDevice::IO:
    m_type_name = "io";
    break;
  case MEMDevice::SRAM:
    m_type_name = "sram";
    break;
  case MEMDevice::SDRAM:
    m_type_name = "sdram";
    break;
  default:
    m_type_name = "rom";
  }
}

MEMDevice::~MEMDevice() {
}

const char *MEMDevice::get_device_info() const {
  return NULL;
}

const MEMDevice::device_type MEMDevice::get_type() const {
  return m_type;
}

const std::string MEMDevice::get_type_name() const {
  return m_type_name;
}

const uint32_t MEMDevice::get_banks() const {
  return m_banks;
}

const uint32_t MEMDevice::get_bsize() const {
  return m_bsize;
}

const uint32_t MEMDevice::get_bits() const {
  return m_bits;
}

const uint32_t MEMDevice::get_cols() const {
  return m_cols;
}
/// @}
