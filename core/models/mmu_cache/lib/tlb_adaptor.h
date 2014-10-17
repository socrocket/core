// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file tlb_adaptor.h
/// Provides access to instruction and data tlb through unified interface
/// functions
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef __TLB_ADAPTOR_H__
#define __TLB_ADAPTOR_H__

#include <tlm.h>

#include "core/models/mmu_cache/lib/mmu_cache_if.h"
#include "core/models/mmu_cache/lib/mmu_if.h"
#include "core/models/mmu_cache/lib/mem_if.h"
#include "core/common/verbose.h"
#include "core/common/vendian.h"
#include "core/models/mmu_cache/lib/defines.h"

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
                              unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock) {

            unsigned int paddr;
	    unsigned int mmu_ctrl = m_mmu->read_mcr();
      unsigned int tmp_data;
      //bool cacheable = true;

	    #ifdef LITTLE_ENDIAN_BO
	    swap_Endianess(mmu_ctrl);
	    #endif
      v::debug << name() << "MMU_CONTROL_REG " << hex << mmu_ctrl << endl;

            // mmu enabled≈
            if ((mmu_ctrl & 0x1) == 1) {

	      v::debug << name() << "MMU enabled - lookup TLB" << hex << (mmu_ctrl & 0x1) << v::endl;
              paddr = m_mmu->tlb_lookup(addr, m_tlb, m_tlbnum, t, debug, is_dbg, cacheable);

            }
            // mmu in bypass mode
            else {

        v::debug << name() << "MMU_CONTROL_REG " << hex << mmu_ctrl << endl;
	      v::debug << name() << "MMU disabled - physical addressing" << hex << (mmu_ctrl & 0x1)  << v::endl;
              paddr = addr;

            }

            // forward request to amba interface - return cacheability
            v::debug << name() << "tlb_adaptor cacheable: " << cacheable << v::endl;
            tmp_data = (unsigned int)(*data);
            swap_Endianess(tmp_data);
            v::debug << name() << "mem read data: 0x"<< hex << tmp_data << " vaddr: 0x" << hex << addr << " paddr: 0x" << hex << paddr << " mmu: " << (mmu_ctrl & 0x1) << v::endl;
            return (m_mmu_cache->mem_read(paddr, asi, data, len, t, debug, is_dbg, cacheable, is_lock) && cacheable);



        }

        /// implementation of mem_write function from mem_if.h
        virtual void mem_write(unsigned int addr, unsigned int asi, unsigned char * data,
                               unsigned int len, sc_core::sc_time * t,
                               unsigned int * debug, bool is_dbg, bool &cacheable, bool is_lock) {

            unsigned int paddr;
	    unsigned int mmu_ctrl = m_mmu->read_mcr();
      unsigned int tmp_data;

	    #ifdef LITTLE_ENDIAN_BO
	    swap_Endianess(mmu_ctrl);
	    #endif
      v::debug << name() << "MMU_CONTROL_REG " << hex << mmu_ctrl << endl;


            // mmu enabled
            if ((mmu_ctrl & 0x1) == 1) {

	            v::debug << name() << "MMU enabled - lookup TLB" << hex << (mmu_ctrl & 0x1)  << v::endl;
              v::debug << name() << "asi " << hex << asi << v::endl;  
              paddr = m_mmu->tlb_lookup(addr, m_tlb, m_tlbnum, t, debug, is_dbg, cacheable);

            }
            // mmu in bypass mode
            else {

        v::debug << name() << "MMU_CONTROL_REG " << hex << mmu_ctrl << endl;
	      v::debug << name() << "MMU disabled - physical addressing" << hex << (mmu_ctrl & 0x1)  << v::endl;
              paddr = addr;

            }

            // forward request to mmu amba interface
            tmp_data = (unsigned int)(*data);
            swap_Endianess(tmp_data);
            v::debug << name() << "mem write data: 0x"<< hex << tmp_data << " vaddr: 0x" << hex << addr << " paddr: 0x" << hex << paddr << " mmu: " << (mmu_ctrl & 0x1) << v::endl;
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
