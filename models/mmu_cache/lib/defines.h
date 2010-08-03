/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       defines.h - Header file with global defines             */
/*                                                                     */
/* Modified on $Date$                                                  */
/*          at $Revision$                                              */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#ifndef __DEFINES_H__
#define __DEFINES_H__

#include "tlm.h"

#define DUMP(name, msg) std::cout<<"@"<<sc_core::sc_time_stamp()<<" /"<<(unsigned)sc_core::sc_delta_count()<<" ("<<name  <<"): "<<msg<<std::endl

// structure of a cache tag
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
  unsigned int lock;
  unsigned int valid;
} t_cache_tag;

// single cache data entry
typedef union {
  unsigned int i;
  unsigned char c[4];
} t_cache_data;

// cacheline consists of tag and up to 4 entries (depending on configuration)
typedef struct {
  t_cache_tag   tag;
  t_cache_data  entry[4];
} t_cache_line;

// structure of a tlb entry (page descriptor cache entry)
// ========================
// virtual address tag 20bit (for 4 kB page size)
// 
// page descriptor cache entry
// context tag      - ?? bits
// page table entry - 24bit

// page descriptor cache entry
typedef struct {
  unsigned int context;
  unsigned int pte;
} t_PTE_context;

// virtual address tag
typedef unsigned int t_VAT;

// payload pointer
typedef tlm::tlm_generic_payload *gp_ptr;  

#endif // __DEFINES_H__
