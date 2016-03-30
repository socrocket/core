// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup memory Memory
/// @{
/// @file basememory.h
/// Base memory functions. Including statistical analysis. All functions ending
/// with _dbg are debugging functions, hence they will not influence the
/// statistical analysis.
///
/// @date 2014-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Jan Wagner
///

#ifndef MODELS_MEMORY_BASEMEMORY_H_
#define MODELS_MEMORY_BASEMEMORY_H_

#include "gaisler/memory/arraystorage.h"
#include "gaisler/memory/mapstorage.h"
#include "gaisler/memory/storage.h"
#include "core/common/scireg.h"
#include "core/common/sr_report.h"

class BaseMemory : public scireg_ns::scireg_region_if {
  public:
    BaseMemory();

    ~BaseMemory();

    void set_storage(std::string implementation, uint32_t size);

    uint8_t read(const uint32_t &addr);

    uint8_t read_dbg(const uint32_t &addr);

    void write(const uint32_t &addr, const uint8_t &byte);

    void write_dbg(const uint32_t &addr, const uint8_t &byte);

    void write_block(const uint32_t &addr, uint8_t *data, const uint32_t &len);

    void write_block_dbg(const uint32_t &addr, const uint8_t *data, const uint32_t &len);

    void read_block(const uint32_t &addr, uint8_t *data, const uint32_t &len);

    void read_block_dbg(const uint32_t &addr, uint8_t *data, const uint32_t &len) const;

    void erase(const uint32_t &start, const uint32_t &end);

    void erase_dbg(const uint32_t &start, const uint32_t &end);

    /// Get the region_type of this region:
    virtual scireg_ns::scireg_response scireg_get_region_type(scireg_ns::scireg_region_type& t) const {
      t = scireg_ns::SCIREG_MEMORY;
      return scireg_ns::SCIREG_SUCCESS;
    }

    /// Read a vector of "size" bytes at given offset in this region:
    virtual scireg_ns::scireg_response scireg_read(scireg_ns::vector_byte& v, sc_dt::uint64 size, sc_dt::uint64 offset=0) const {
      read_block_dbg(offset, static_cast<uint8_t *>(&v[0]), size);
      return scireg_ns::SCIREG_SUCCESS;
    }

    /// Write a vector of "size" bytes at given offset in this region:
    virtual scireg_ns::scireg_response scireg_write(const scireg_ns::vector_byte& v, sc_dt::uint64 size, sc_dt::uint64 offset=0) {
      write_block_dbg(offset, static_cast<const uint8_t *>(&v[0]), size);
      return scireg_ns::SCIREG_SUCCESS;
    }

    /// Get bit width and byte width of this region
    virtual sc_dt::uint64 scireg_get_bit_width() const {
      return m_storage->get_size() * 8;
    }

    /// If this region is a register field, these functions return low bit and high bit positions:
    virtual sc_dt::uint64 scireg_get_low_pos() const {
      return 31;
    }
    virtual sc_dt::uint64 scireg_get_high_pos() const {
      return 0;
    }

    /// Query to see if DMI access has been granted to this region. "size" and offset can be used to constrain the range.
    virtual scireg_ns::scireg_response scireg_get_dmi_granted(bool& granted, sc_dt::uint64 size, sc_dt::uint64 offset=0) const {
      return scireg_ns::SCIREG_UNSUPPORTED;
    }

    scireg_ns::scireg_response scireg_add_callback(scireg_ns::scireg_callback &cb) {
      callback_vector.push_back(&cb);
      return scireg_ns::SCIREG_SUCCESS;
    }

    scireg_ns::scireg_response scireg_remove_callback(scireg_ns::scireg_callback& cb) {
      ::std::vector<scireg_ns::scireg_callback*>::iterator it;
      it = find(callback_vector.begin(), callback_vector.end(), &cb);
      if (it != callback_vector.end())
        callback_vector.erase(it);
      return scireg_ns::SCIREG_SUCCESS;
    }

    virtual const char* get_name() const = 0;

    virtual scireg_ns::scireg_response scireg_get_string_attribute(const char *& s, scireg_ns::scireg_string_attribute_type t) const {
       switch (t) {
       case scireg_ns::SCIREG_NAME:
         s = get_name();
         return scireg_ns::SCIREG_SUCCESS;
       case scireg_ns::SCIREG_STRING_VALUE:
         return scireg_ns::SCIREG_FAILURE;
       default:
         return scireg_ns::SCIREG_FAILURE;
       }

       return scireg_ns::SCIREG_FAILURE;
     }

    /// byte read count
    unsigned long long reads;

    /// byte write count
    unsigned long long writes;

    /// word (4 bytes) read count
    unsigned long long reads32;

    /// word (4 bytes) write count
    unsigned long long writes32;

  protected:
    void execute_callbacks(const scireg_ns::scireg_callback_type &type, const uint32_t &offset, const uint32_t &size) {
      scireg_ns::scireg_callback* p;
      ::std::vector<scireg_ns::scireg_callback*>::iterator it;
      for (it = callback_vector.begin(); it != callback_vector.end(); ++it)
      {
        p = *it;
        if (p->type == type) {
          p->offset = offset;
          p->size = size;
          p->do_callback(*this);
        }
      }
    }

    Storage *m_storage;
    ::std::vector<scireg_ns::scireg_callback*> callback_vector;
};

#endif  // MODELS_MEMORY_BASEMEMORY_H_
/// @}
