// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file defines.h
/// Header file with global defines
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <tlm.h>

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
typedef union {
        unsigned int i;
        unsigned char c[4];
} t_cache_data;

// cacheline consists of tag and up to 8 entries (depending on configuration)
typedef struct {
        t_cache_tag tag;
        t_cache_data entry[8];
} t_cache_line;

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
  unsigned int offset_mask;
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
