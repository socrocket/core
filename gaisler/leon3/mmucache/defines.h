// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file defines.h
/// Header file with global defines
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <tlm.h>
#include "core/common/scireg.h"
#include "core/common/sr_register.h"
#include "core/common/sr_report.h"
#include "core/common/base.h"

// Structure of a cache tag
// ========================
// atag  - width of the cache tag:    32bit virt. addr
//                                  - index bits        (log2(cache_set_size/cache_line_size))
//                                  - offset bits       (log2(cache_line_size))
//                                  ------------------
//                                  = atag width
// e.g. 1Kbyte set, 32 bytes/line:  32bit (virt.) - 5bit (index) - 5bit (offset) = 22bit
// e.g. 4Kbyte set, 16 bytes/line:  32bit (virt.) - 8bit (index) - 4bit (offset) = 20bit

// valid - one bit per word per cacheline (e.g. 32 bytes/line -> 8 valid bits), valid[0] refers to first address

typedef struct {
        unsigned int atag;
        unsigned int lrr;
        int lru;
        unsigned int lock;
        unsigned int valid;
} t_cache_tag;

// single cache data entry

class t_cache_data : public sc_object, public scireg_ns::scireg_region_if {
  public:
    t_cache_data(sc_module_name name, uint32_t size) :
      sc_object(name),
      scireg_ns::scireg_region_if() {
        i = new uint32_t[size];
        c = reinterpret_cast<uint8_t*>(&i[0]);
        this->size = size;
      srInfo()
        ("size", size)
        ("get_bit_width");
      };

    ~t_cache_data() {
      srDebug()
        ("deleting cache data");
      delete[] i;
    }

    uint32_t get_int(const int32_t &index) const {
      srDebug()
        ("index", index)
        ("get_int");
      this->execute_callbacks(scireg_ns::SCIREG_READ_ACCESS, (index << 2), 4);
      return i[index];
    }

    void set_int(const uint32_t &index, const uint32_t &data) {
      srDebug()
        ("index", index)
        ("set_int");
      i[index] = data;
      this->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS, (index << 2), 4);
    }

    void copy_from_dbg(void *destination, const size_t num, const uint32_t index, const uint32_t position) const {
      srDebug()
        ("length", num)
        ("index", index)
        ("position", position)
        ("copy_from_dbg");
      memcpy(destination, &c[(index << 2) + position], num);
    }

    void copy_from(void *destination, const size_t num, const uint32_t index, const uint32_t position) const {
      this->execute_callbacks(scireg_ns::SCIREG_READ_ACCESS, (index << 2) + position, num);
      copy_from_dbg(destination, num, index, position);
    }

    void copy_to_dbg(const void *source, const size_t num, const uint32_t index, const uint32_t position) {
      srDebug()
        ("length", num)
        ("index", index)
        ("position", position)
        ("copy_to_dbg");

      memcpy(&c[(index << 2) + position], source, num);
    }

    void copy_to(const void *source, const size_t num, const uint32_t index, const uint32_t position) {
      copy_to_dbg(source, num, index, position);
      this->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS, (index << 2) + position, num);
    }

    uint8_t get_char(const uint32_t &index, const uint32_t &position) const {
      srDebug()
        ("position", position)
        ("index", index)
        ("get_char");
      this->execute_callbacks(scireg_ns::SCIREG_READ_ACCESS, (index << 2)+position, 1);
      return c[(index << 2) + position];
    }

    void set_char(const uint32_t &index, const uint32_t &position, const uint8_t &data) {
      srDebug()
        ("position", position)
        ("index", index)
        ("set_char");
      c[(index << 2) + position] = data;
      this->execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS, (index << 2)+position, 1);
    }

    /// Get the region_type of this region:
    virtual scireg_ns::scireg_response scireg_get_region_type(scireg_ns::scireg_region_type& t) const {
      t = scireg_ns::SCIREG_MEMORY;
      return scireg_ns::SCIREG_SUCCESS;
    }

    /// Write a vector of "size" bytes at given offset in this region:
    virtual scireg_ns::scireg_response scireg_write(const scireg_ns::vector_byte& v, sc_dt::uint64 size, sc_dt::uint64 offset=0) {
      copy_to_dbg(static_cast<const uint8_t *>(&v[0]), size, 0, offset);
      return scireg_ns::SCIREG_SUCCESS;
    }

    /// Read a vector of "size" bytes at given offset in this region:
    virtual scireg_ns::scireg_response scireg_read(scireg_ns::vector_byte& v, sc_dt::uint64 size, sc_dt::uint64 offset=0) const {
      copy_from_dbg(static_cast<uint8_t *>(&v[0]), size, 0, offset);
      return scireg_ns::SCIREG_SUCCESS;
    }

    virtual sc_dt::uint64 scireg_get_bit_width() const {
      return this->size * 4 * 8;
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

    virtual scireg_ns::scireg_response scireg_get_string_attribute(const char *& s, scireg_ns::scireg_string_attribute_type t) const {
      switch (t) {
        case scireg_ns::SCIREG_NAME:
          s = name();
          return scireg_ns::SCIREG_SUCCESS;

        case scireg_ns::SCIREG_DESCRIPTION:
          return scireg_ns::SCIREG_UNSUPPORTED;

        case scireg_ns::SCIREG_STRING_VALUE:
          return scireg_ns::SCIREG_UNSUPPORTED;
      }

      return scireg_ns::SCIREG_FAILURE;
    }

  private:
    uint8_t size;
    ::std::vector<scireg_ns::scireg_callback*> callback_vector;
    uint32_t *i;
    uint8_t *c;

    void execute_callbacks(const scireg_ns::scireg_callback_type &type, const uint32_t &offset, const uint32_t &size) const {
      scireg_ns::scireg_callback* p;
      ::std::vector<scireg_ns::scireg_callback*>::const_iterator it;
      for (it = callback_vector.begin(); it != callback_vector.end(); ++it)
      {
        p = *it;
        if (p->type == type) {
          p->offset = offset;
          p->size = size;
          p->do_callback(*(const_cast<t_cache_data*>(this)));
        }
      }
    }
};

// cacheline consists of tag and up to 8 entries (depending on configuration)
//typedef struct {
class t_cache_line : public sc_module, public scireg_ns::scireg_region_if {
  public:
        sr_register_bank<uint32_t, uint32_t> tag;
        t_cache_data entry;

        t_cache_line(sc_core::sc_object &parent, sc_module_name name, const uint32_t linesize) :
          sc_module(name),
          scireg_ns::scireg_region_if(),
          tag("tag"),
          entry("entry", linesize) {
            this->parent = &parent;
            tag.create_register(
                "valid",      // name
                VALID,        // addr
                0,            // init value
                0xFFFFFFFF); // write mask
            tag.create_register(
                "atag",       // name
                ATAG,         // addr
                0,            // init value
                0xFFFFFFFF); // write mask
            tag.create_register(
                "lrr",        // name
                LRR,          // addr
                0,            // init value
                0xFFFFFFFF); // write mask
            tag.create_register(
                "lru",        // name
                LRU,          // addr
                0,            // init value
                0xFFFFFFFF); // write mask
            tag.create_register(
                "lock",       // name
                LOCK,         // addr
                0,            // init value
                0xFFFFFFFF); // write mask
            srDebug()("name", name)("create");
          }

        /// Get the region_type of this region:
        virtual scireg_ns::scireg_response scireg_get_region_type(scireg_ns::scireg_region_type& t) const {
          t = scireg_ns::SCIREG_BANK;
          return scireg_ns::SCIREG_SUCCESS;
        }

        static const uint32_t VALID ;
        static const uint32_t ATAG  ;
        static const uint32_t LRR   ;
        static const uint32_t LRU   ;
        static const uint32_t LOCK  ;

  protected:
        sc_core::sc_object *parent;

};

//} t_cache_line;

// structure of a tlb entry (page descriptor cache entry)
// ========================
// virtual address tag:
// - 20bit (for 4 kB page size)
// - 19bit (for 8 kb page size)
// - 18bit (for 16 kb page size)
// - 17bit (for 32 kb page size)
//
// page descriptor cache entry
// context tag      - ?? bits
// page table entry - 24bit

// page descriptor cache entry
typedef struct {
  unsigned int tlb_no;
  unsigned int context;
  unsigned int pte;
  uint64_t lru;
  uint64_t page_size;
} t_PTE_context;

// virtual address tag
typedef unsigned int t_VAT;

// payload pointer
typedef tlm::tlm_generic_payload *gp_ptr;

// Structure of the debug extension for the dcio and icio payload extensions.
// ==========================================================================
// [21]    MMU state    - 0 tlb hit, 1 tlb miss
// [20-16] TLB          - for tlb hit:  number of the tlb that delivered the hit  (!!! not implemented yet !!!)
//                      - for tlb miss: number of the tlb that delivered the miss
// [15]    Reserved
// [14]    Frozen Miss  - If the cache is frozen, no new lines are allocated on a read miss.
//                        However, unvalid data will be replaced as long the tag of the
//                        line does not change. The FM bit is switch on, when the
//                        result of a read miss is not cached.
// [13]    Cache Bypass - Is set to 1 if cache bypass was used (cache disabled in ccr)
// [12]    Scratchpad   - Is set to 1 for scratchpad access
// [11-5]  Reserved
// [4]     Flush        - Set to 1 if transaction causes a cache flush
// [3-2]   Cache State  - 00 read hit, 01, read miss, 10, write hit, 11 write miss
// [1-0]   Cache Set    - for read hit:  contains number of set that delivered the hit
//                       for read miss: number of set containing the new data
//                       for write hit: number of set that delivered the hit
//                       write miss:    0b00 (no update on write miss)


// MACROS for updating debug information:
#define FROZENMISS_SET(debug) (debug |= 0x2000)
#define CACHEBYPASS_SET(debug) (debug |= 0x1000)
#define SCRATCHPAD_SET(debug) (debug |= 0x800)

#define CACHEREADHIT_SET(debug, cache_set)  ((debug  &= 0xfffff7f0)  |= (cache_set & 0x3))
#define CACHEREADMISS_SET(debug, cache_set) (((debug &= 0xfffff7f0)  |= 0x4) |= (cache_set & 0x3))
#define CACHEWRITEHIT_SET(debug, cache_set) (((debug &= 0xfffff7f0)  |= 0x8) |= (cache_set & 0x3))
#define CACHEWRITEMISS_SET(debug)           ((debug  &= 0xfffff7f0)  |= 0xc)

#define CACHEFLUSH_SET(debug) (debug |= 0x10)

#define TLBHIT_SET(debug) (debug &= ~(1 << 21));
#define TLBMISS_SET(debug) (debug |= (1 << 21));

// MACROS for evaluating debug information
#define FROZENMISS_CHECK(debug) (debug & 0x2000)
#define CACHEBYPASS_CHECK(debug) (debug & 0x1000)
#define SCRATCHPAD_CHECK(debug) (debug & 0x800)

#define CACHEREADHIT_CHECK(debug)    ((debug & 0xc) == 0)
#define CACHEREADMISS_CHECK(debug)   ((debug & 0xc) == 4)
#define CACHEWRITEHIT_CHECK(debug)   ((debug & 0xc) == 8)
#define CACHEWRITEMISS_CHECK(debug)  (((debug & 0xc) == 0xc) && ((debug & 0x3) == 0))

#define CACHEFLUSH_CHECK(debug) ((debug & 0x10) == 0x10)

#define TLBHIT_CHECK(debug) ((debug & (1 << 21)) == 0)
#define TLBMISS_CHECK(debug) ((debug & (1 << 21)) != 0)

enum check_t { NOCHECK, FROZENMISS, NOTFROZENMISS, CACHEBYPASS, SCRATCHPAD, CACHEREADHIT, CACHEREADMISS, CACHEWRITEHIT, CACHEWRITEMISS, CACHEFLUSH, TLBHIT, TLBMISS};

#endif // __DEFINES_H__
/// @}
