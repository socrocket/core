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
// Title:      memdevice.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Contains the implementation of a baseclass
//             for all memory tlm models. It implements the the device
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

#include "memdevice.h"
#include "verbose.h"

MEMDevice::MEMDevice(MEMDevice::device_type type, uint32_t banks, uint32_t bsize, uint32_t bits, uint32_t cols) : m_type(type), m_banks(banks), m_bsize(bsize), m_bits(bits), m_cols(cols) {

  switch(type) {

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
    return  m_bsize;
}

const uint32_t MEMDevice::get_bits() const {
    return m_bits;
}

const uint32_t MEMDevice::get_cols() const {
    return m_cols;
}
