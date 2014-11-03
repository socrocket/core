// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup memory
/// @{
/// @file basememory.cpp
/// source file defining the implementation of the basememory model.
///
/// @date 2014-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Jan Wagner
///

#include "core/models/memory/basememory.h"

BaseMemory::BaseMemory(const implementation_type &type, const uint32_t &size) {
  reads = 0;
  reads32 = 0;
  writes = 0;
  writes32 = 0;

  switch (type) {
  case BaseMemory::ARRAY: storage = new ArrayStorage(size);
  case BaseMemory::MAP: storage = new MapStorage(size);
  default: storage = new ArrayStorage(size);
  }
}

BaseMemory::~BaseMemory() {
  delete storage;
}

uint8_t BaseMemory::read(const uint32_t &addr) {
  reads++;
  reads32++;
  return this->read_dbg(addr);
}

void BaseMemory::write(const uint32_t &addr, const uint8_t &byte) {
  writes++;
  writes32++;
  this->write_dbg(addr, byte);
}

uint8_t BaseMemory::read_dbg(const uint32_t &addr) {
  return storage->read(addr);
}

void BaseMemory::write_dbg(const uint32_t &addr, const uint8_t &byte) {
  storage->write(addr, byte);
}

void BaseMemory::erase(const uint32_t &start, const uint32_t &end) {
  this->erase_dbg(start, end);
}

void BaseMemory::erase_dbg(const uint32_t &start, const uint32_t &end) {
  storage->erase(start, end);
}

void BaseMemory::write_block(const uint32_t &addr, uint8_t *data, const uint32_t &len) {
  writes += len;
  writes32 += ((len - 1) >> 2) + 1;
  write_block_dbg(addr, data, len);
}

void BaseMemory::write_block_dbg(const uint32_t &addr, const uint8_t *data, const uint32_t &len) {
  storage->write_block(addr, data, len);
}

void BaseMemory::read_block(const uint32_t &addr, uint8_t *data, const uint32_t &len) {
  reads += len;
  reads32 += ((len - 1) >> 2) + 1;
  read_block_dbg(addr, data, len);
}

void BaseMemory::read_block_dbg(const uint32_t &addr, uint8_t *data, const uint32_t &len) const {
  storage->read_block(addr, data, len);
}
/// @}
