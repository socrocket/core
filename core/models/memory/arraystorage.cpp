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

#include "core/models/memory/arraystorage.h"

ArrayStorage::ArrayStorage(const uint32_t &size) {
  data = new uint8_t[size];
  erase(0, size);
}

ArrayStorage::~ArrayStorage() {
  delete[] data;
}

void ArrayStorage::write(const uint32_t &addr, const uint8_t &byte) {
  data[addr] = byte;
}

uint8_t ArrayStorage::read(const uint32_t &addr) {
  return data[addr];
}

void ArrayStorage::erase(const uint32_t &start, const uint32_t &end) {
  memset(&data[start], 0, end-start);
}

void ArrayStorage::write_block(const uint32_t &addr, uint8_t *ptr, const uint32_t &len) {
  memcpy(&data[addr], ptr, len);
}

void ArrayStorage::read_block(const uint32_t &addr, uint8_t *ptr, const uint32_t &len) {
  memcpy(ptr, &data[addr], len);
}
/// @}
