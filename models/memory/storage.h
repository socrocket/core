// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup memory memory
/// @{
/// @file storage.h
/// Interface definition for storage-implementations for memories
///
/// @date 2014-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Jan Wagner
///

#ifndef  MODELS_MEMORY_STORAGE_H_
#define MODELS_MEMORY_STORAGE_H_
#include <systemc.h>

class Storage {
  public:
    virtual void write(const uint32_t &addr, const uint8_t &byte) = 0;

    virtual uint8_t read(const uint32_t &addr) = 0;

    virtual void write_block(const uint32_t &addr, uint8_t *data, const uint32_t &len) = 0;

    virtual void read_block(const uint32_t &addr, uint8_t *data, const uint32_t &len) = 0;

    virtual void erase(const uint32_t &start, const uint32_t &end) = 0;
};

#endif  // MODELS_MEMORY_STORAGE_H_
/// @}
