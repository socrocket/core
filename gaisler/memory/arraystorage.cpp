// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup memory
/// @{
/// @file arraystorage.cpp
/// source file defining the implementation of the arraysorage model.
///
/// @date 2014-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Jan Wagner
///

#include "gaisler/memory/arraystorage.h"
#include "core/common/sr_report.h"

SR_HAS_MEMORYSTORAGE(ArrayStorage);

ArrayStorage::ArrayStorage(sc_core::sc_module_name mn) : Storage(mn), data(NULL), m_size(0) {
}

ArrayStorage::~ArrayStorage() {
  delete[] data;
}

void ArrayStorage::set_size(const uint32_t &size) {
  if(data) {
    delete data;
  }
  data = new uint8_t[size];
  m_size = size;
  srDebug()
    ("size", m_size)
    ("set_size");

}

uint64_t ArrayStorage::get_size() const {
  srDebug()
    ("size", m_size)
    ("get_size");
  return m_size;
}

void ArrayStorage::write(const uint32_t &addr, const uint8_t &byte) {
  data[addr] = byte;
}

uint8_t ArrayStorage::read(const uint32_t &addr) const {
  return data[addr];
}

void ArrayStorage::erase(const uint32_t &start, const uint32_t &end) {
  memset(&data[start], 0, end-start);
}

void ArrayStorage::write_block(const uint32_t &addr, const uint8_t *ptr, const uint32_t &len) {
  memcpy(&data[addr], ptr, len);
}

void ArrayStorage::read_block(const uint32_t &addr, uint8_t *ptr, const uint32_t &len) const {
  memcpy(ptr, &data[addr], len);
}

uint8_t *ArrayStorage::get_dmi_ptr() {
  return data;
}

bool ArrayStorage::allow_dmi_rw() {
  return true;
}
/// @}
