// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup memory Memory
/// @{
/// @file basememory.h
/// Base memory functions. Including statistical analysis. All functions ending
/// with _dbg are debugging functions, hence they will not influence the
/// statistical analysis.
///
/// @date 2014-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Jan Wagner
///

#ifndef MODELS_MEMORY_BASEMEMORY_H_
#define MODELS_MEMORY_BASEMEMORY_H_

#include "models/memory/arraystorage.h"
#include "models/memory/mapstorage.h"
#include "models/memory/storage.h"

class BaseMemory {
  public:
    enum implementation_type {
      ARRAY = 0,
      MAP = 1
    };

    typedef implementation_type type;

    BaseMemory(const implementation_type &type, const uint32_t &size);

    ~BaseMemory();

    uint8_t read(const uint32_t &addr);

    uint8_t read_dbg(const uint32_t &addr);

    void write(const uint32_t &addr, const uint8_t &byte);

    void write_dbg(const uint32_t &addr, const uint8_t &byte);

    void write_block(const uint32_t &addr, uint8_t *data, const uint32_t &len);

    void write_block_dbg(const uint32_t &addr, uint8_t *data, const uint32_t &len);

    void read_block(const uint32_t &addr, uint8_t *data, const uint32_t &len);

    void read_block_dbg(const uint32_t &addr, uint8_t *data, const uint32_t &len);

    void erase(const uint32_t &start, const uint32_t &end);

    void erase_dbg(const uint32_t &start, const uint32_t &end);

    // byte read count
    unsigned long long reads;

    // byte write count
    unsigned long long writes;

    // word (4 bytes) read count
    unsigned long long reads32;

    // word (4 bytes) write count
    unsigned long long writes32;

  private:
    Storage *storage;
};

#endif  // MODELS_MEMORY_BASEMEMORY_H_
/// @}
