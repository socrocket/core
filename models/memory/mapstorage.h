// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup memory Memory
/// @{
/// @file mapstorage.h
/// Adressable storage implememtation based on maps. Supposed to be uses by memories.
///
/// @date 2014-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Jan Wagner
///

#ifndef MODELS_MEMORY_MAPSTORAGE_H_
#define MODELS_MEMORY_MAPSTORAGE_H_

#include "core/common/vmap.h"

#include "core/models/memory/storage.h"

class MapStorage : public Storage {
  public:
    explicit MapStorage(const uint32_t &size);

    ~MapStorage();

    void write(const uint32_t &addr, const uint8_t &byte);

    uint8_t read(const uint32_t &addr) const;

    void write_block(const uint32_t &addr, const uint8_t *ptr, const uint32_t &len);

    void read_block(const uint32_t &addr, uint8_t *ptr, const uint32_t &len) const;

    void erase(const uint32_t &start, const uint32_t &end);
  private:
    typedef vmap<uint32_t, uint8_t> map_mem;
    map_mem data;
};

#endif  // MODELS_MEMORY_MAPSTORAGE_H_
/// @}
