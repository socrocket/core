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

#include "gaisler/memory/basememory.h"
#include "core/common/sr_registry.h"
#include "core/common/sr_report.h"

BaseMemory::BaseMemory() {
  reads = 0;
  reads32 = 0;
  writes = 0;
  writes32 = 0;
}

BaseMemory::~BaseMemory() {
  delete m_storage;
}

void BaseMemory::set_storage(std::string implementation, uint32_t size) {
  sc_core::sc_object *obj = SrModuleRegistry::create_object_by_name("MemoryStorage", implementation, "storage");
  m_storage = dynamic_cast<Storage *>(obj);
  if(!m_storage) {
    srError("BaseMemory")
      ("storage", implementation)
      ("size", size)
      ("Memory Store not created");
  }
  m_storage->set_size(size);
}

uint8_t BaseMemory::read(const uint32_t &addr) {
  reads++;
  reads32++;
  this->execute_callbacks(scireg_ns::SCIREG_READ_ACCESS, addr, 4);
  return this->read_dbg(addr);
}

void BaseMemory::write(const uint32_t &addr, const uint8_t &byte) {
  writes++;
  writes32++;
  this->write_dbg(addr, byte);
  this->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS, addr, 4);
}

uint8_t BaseMemory::read_dbg(const uint32_t &addr) {
  return m_storage->read(addr);
}

void BaseMemory::write_dbg(const uint32_t &addr, const uint8_t &byte) {
  m_storage->write(addr, byte);
}

void BaseMemory::erase(const uint32_t &start, const uint32_t &end) {
  this->erase_dbg(start, end);
}

void BaseMemory::erase_dbg(const uint32_t &start, const uint32_t &end) {
  m_storage->erase(start, end);
}

void BaseMemory::write_block(const uint32_t &addr, uint8_t *data, const uint32_t &len) {
  writes += len;
  writes32 += ((len - 1) >> 2) + 1;
  write_block_dbg(addr, data, len);
  this->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS, addr, len);
}

void BaseMemory::write_block_dbg(const uint32_t &addr, const uint8_t *data, const uint32_t &len) {
  m_storage->write_block(addr, data, len);
}

void BaseMemory::read_block(const uint32_t &addr, uint8_t *data, const uint32_t &len) {
  reads += len;
  reads32 += ((len - 1) >> 2) + 1;
  read_block_dbg(addr, data, len);
  this->execute_callbacks(scireg_ns::SCIREG_READ_ACCESS, addr, len);
}

void BaseMemory::read_block_dbg(const uint32_t &addr, uint8_t *data, const uint32_t &len) const {
  m_storage->read_block(addr, data, len);
}
/// @}
