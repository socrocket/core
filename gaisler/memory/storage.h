// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup memory memory
/// @{
/// @file storage.h
/// Interface definition for storage-implementations for memories
///
/// @date 2014-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Jan Wagner
///

#ifndef  MODELS_MEMORY_STORAGE_H_
#define MODELS_MEMORY_STORAGE_H_
#include "core/common/systemc.h"
#include "core/common/sr_registry.h"

#define \
  SR_HAS_MEMORYSTORAGE_GENERATOR(type, factory, isinstance) \
  static SrModuleRegistry __sr_module_registry_##type##__("MemoryStorage", #type, &factory, &isinstance, __FILE__); \
  volatile SrModuleRegistry *__sr_module_registry_##type = &__sr_module_registry_##type##__;

#define \
  SR_HAS_MEMORYSTORAGE(type) \
    sc_core::sc_object *create_##type(sc_core::sc_module_name mn) { \
      return new type(mn); \
    } \
    bool isinstance_of_##type(sc_core::sc_object *obj) { \
      return dynamic_cast<type *>(obj) != NULL; \
    } \
    SR_HAS_MEMORYSTORAGE_GENERATOR(type, create_##type, isinstance_of_##type);



class Storage : public sc_core::sc_object {
  public:
    Storage(sc_core::sc_module_name mn) : sc_core::sc_object(mn) {};
    virtual ~Storage() {};

    virtual void set_size(const uint32_t &size) = 0;

    virtual uint64_t get_size() const = 0;

    virtual void write(const uint32_t &addr, const uint8_t &byte) = 0;

    virtual uint8_t read(const uint32_t &addr) const = 0;

    virtual void write_block(const uint32_t &addr, const uint8_t *data, const uint32_t &len) = 0;

    virtual void read_block(const uint32_t &addr, uint8_t *data, const uint32_t &len) const = 0;

    virtual void erase(const uint32_t &start, const uint32_t &end) = 0;

    virtual uint8_t *get_dmi_ptr() { return NULL; }

    virtual bool allow_dmi_rw() { return false; }

  protected:
    uint64_t m_size;
};

#endif  // MODELS_MEMORY_STORAGE_H_
/// @}
