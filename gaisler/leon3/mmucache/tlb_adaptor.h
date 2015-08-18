// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file tlb_adaptor.h
/// Provides access to instruction and data tlb through unified interface
/// functions
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef __TLB_ADAPTOR_H__
#define __TLB_ADAPTOR_H__

#include <tlm.h>

#include "gaisler/leon3/mmucache/mmu_cache_if.h"
#include "gaisler/leon3/mmucache/mmu_if.h"
#include "gaisler/leon3/mmucache/mem_if.h"
#include "core/common/verbose.h"
#include "core/common/vendian.h"
#include "gaisler/leon3/mmucache/defines.h"

class tlb_adaptor : public DefaultBase, public mem_if {

    public:

        /// constructor
        tlb_adaptor(ModuleName name, mmu_cache_if * top,
                    mmu_if * _mmu, std::map<t_VAT, t_PTE_context> * tlb,
                    unsigned int tlbnum) :
            sc_module(name), m_mmu_cache(top), m_mmu(_mmu), m_tlb(tlb),
                    m_tlbnum(tlbnum) {

            // nothing to do

        }

        /// destructor
        ~tlb_adaptor() {

            // nothing to do

        }

        /// implementation of mem_read function from mem_if.h
        virtual bool mem_read(unsigned int addr, unsigned int asi, unsigned char * data,
                              unsigned int len, sc_core::sc_time * t,
                              unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock ) {

            unsigned int paddr;
            unsigned int mmu_ctrl = m_mmu->read_mcr();

            #ifdef LITTLE_ENDIAN_BO
            swap_Endianess(mmu_ctrl);
            #endif

            if ( mmu_ctrl & 0x1 ) { // mmu enabled
                uint64_t t_paddr;
                if( m_mmu->tlb_lookup(addr, asi, m_tlb, m_tlbnum, t, debug, is_dbg, cacheable, 0, &t_paddr ) )
                    return false;  // if not 0, tlb_lookup failed! -> no mem_read!
                paddr = (unsigned int) t_paddr;
            }
            else { // mmu in bypass mode
                paddr = addr;
            }

            // forward request to amba interface - return cacheability
            return m_mmu_cache->mem_read(paddr, asi, data, len, t, debug, is_dbg, cacheable, is_lock);
        }

        /// implementation of mem_write function from mem_if.h
        virtual void mem_write(unsigned int addr, unsigned int asi, unsigned char * data,
                               unsigned int len, sc_core::sc_time * t,
                               unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock ) {

            unsigned int paddr;
            unsigned int mmu_ctrl = m_mmu->read_mcr();

            #ifdef LITTLE_ENDIAN_BO
            swap_Endianess(mmu_ctrl);
            #endif

            if ( mmu_ctrl & 0x1 ) { // mmu enabled
                uint64_t t_paddr;
                if( m_mmu->tlb_lookup(addr, asi, m_tlb, m_tlbnum, t, debug, is_dbg, cacheable, 1, &t_paddr ) )
                    return;  // if not 0, tlb_lookup failed! -> no mem_write!
                paddr = (unsigned int) t_paddr;
            }
            else { // mmu in bypass mode
                paddr = addr;
            }

            // forward request to mmu amba interface
            m_mmu_cache->mem_write(paddr, asi, data, len, t, debug, is_dbg, is_lock, cacheable);
        }

    public:

        mmu_cache_if * m_mmu_cache;
        mmu_if * m_mmu;

        std::map<t_VAT, t_PTE_context> * m_tlb;

        unsigned int m_tlbnum;

};

#endif // __TLB_ADAPTOR_H__
/// @}
