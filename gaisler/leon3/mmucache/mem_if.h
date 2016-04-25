// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file mem_if.h
/// To be implemented by classes which provide a memory interface for public
/// use.
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///


#ifndef __MEM_IF_H__
#define __MEM_IF_H__

class mem_if {

    public:

        // amba master interface functions
        virtual void mem_write(uint32_t addr, uint32_t asi, uint8_t * data,
                               uint32_t length, sc_core::sc_time * t,
                               uint32_t * debug, bool is_dbg, bool &cacheable, bool is_lock = false) {
        };

        virtual bool mem_read(uint32_t addr, uint32_t asi, uint8_t * data,
                              uint32_t length, sc_core::sc_time * t,
                              uint32_t * debug, bool is_dbg, bool &cacheable, bool is_lock = false) {
	  return true;

        };

        virtual ~mem_if() {
        }

};

#endif // __MEM_IF_H__
/// @}
