// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup memory Memory
/// @{
/// @file arraystorage.h
/// Adressable storage implememtation based on arrays. Supposed to be uses by memories.
///
/// @date 2014-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Jan Wagner
///

#ifndef MODELS_MEMORY_ARRAYSTORAGE_H_
#define MODELS_MEMORY_ARRAYSTORAGE_H_

#include "gaisler/memory/storage.h"

class ArrayStorage : public Storage {
  public:
    explicit ArrayStorage(sc_core::sc_module_name mn);

    ~ArrayStorage();

    void set_size(const uint32_t &size);

    uint64_t get_size() const;

    void write(const uint32_t &addr, const uint8_t &byte);

    uint8_t read(const uint32_t &addr) const;

    void write_block(const uint32_t &addr, const uint8_t *ptr, const uint32_t &len);

    void read_block(const uint32_t &addr, uint8_t *ptr, const uint32_t &len) const;

    void erase(const uint32_t &start, const uint32_t &end);

    uint8_t *get_dmi_ptr();

    bool allow_dmi_rw();
  private:
    uint8_t *data;
  protected:
    uint64_t m_size;
};

#endif  // MODELS_MEMORY_ARRAYSTORAGE_H_
/// @}
