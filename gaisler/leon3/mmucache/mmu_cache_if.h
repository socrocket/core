// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file mmu_cache_if.h
/// MMU cache interface class for passing pointers to the AHB interface to the
/// components of mmu_cache (ivectorcache, dvectorcache).
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///


#ifndef __MMU_CACHE_IF_H__
#define __MMU_CACHE_IF_H__

#include "gaisler/leon3/mmucache/mem_if.h"

#include <stdint.h>

class mmu_cache_if : public mem_if {

 public:

  // Send an interrupt over the central IRQ interface
  virtual void set_irq(uint32_t tt) {

  };
  
  // Send an exception to the CPU
  virtual void trigger_exception(unsigned int exception) {

  };

  // Reads the cache control register
  virtual unsigned int read_ccr(bool internal) {
    return (0);
  };

  // Writes the cache control register
  virtual void write_ccr(unsigned char *data, unsigned int len,
                         sc_core::sc_time *delay, unsigned int * debug, bool is_dbg) {
  };

  virtual ~mmu_cache_if() {
  };
};

#endif // __MMU_CACHE_IF_H__
/// @}
