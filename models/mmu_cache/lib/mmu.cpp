//*********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is 
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      mmu.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implementation of a memory management unit.
//             The mmu can be configured to have split or combined
//             TLBs for instructions and data. The TLB size can be
//             configured as well. The memory page size is currently
//             currently fixed to 4kB.
//
//
// Method:
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#include "mmu.h"
#include "verbose.h"

mmu::mmu(sc_core::sc_module_name name, // sysc module name,
         mmu_cache_if * _mmu_cache, // pointer to memory interface
         unsigned int itlbnum, // number of instruction tlbs
         unsigned int dtlbnum, // number of data tlbs
         unsigned int tlb_type, // tlb type
         unsigned int tlb_rep, // tlb replacement strategy
         unsigned int mmupgsz) :
            sc_module(name), // tlb mmu page size (default 4kB)
            m_mmu_cache(_mmu_cache), m_itlbnum(itlbnum), m_dtlbnum(dtlbnum), 
	    m_itlblog2((unsigned int)log2((double)m_itlbnum)), 
	    m_dtlblog2((unsigned int)log2((double)m_dtlbnum)), 
	    m_tlb_type(tlb_type), m_tlb_rep(tlb_rep),
            m_mmupgsz(mmupgsz), clockcycle(10, sc_core::SC_NS) {

    // initialize internal registers

    // the number of instruction and data tlbs must be in the range of 2-32
    assert((m_itlbnum>=2)&&(m_itlbnum<=32));
    assert((m_dtlbnum>=2)&&(m_dtlbnum<=32));

    // initialize MMU control register (ITLB, DTLB)
    MMU_CONTROL_REG = 0;
    MMU_CONTROL_REG = ((m_itlblog2 << 21) | (m_dtlblog2 << 18));

    MMU_CONTEXT_TABLE_POINTER_REG = 0;
    MMU_CONTEXT_REG = 0;
    MMU_FAULT_STATUS_REG = 0;
    MMU_FAULT_ADDRESS_REG = 0;

    // generate associative memory (map) for instruction tlb
    itlb = new std::map<t_VAT, t_PTE_context>;
    itlb_adaptor = new tlb_adaptor("itlb_adaptor", _mmu_cache, this, itlb,
            m_itlbnum);

    // are we in split tlb mode?
    if (m_tlb_type == 0x0) {

        // generate another associative memory (map) for data tlb
        dtlb = new std::map<t_VAT, t_PTE_context>;
        dtlb_adaptor = new tlb_adaptor("dtlb_adaptor", _mmu_cache, this, dtlb,
                m_dtlbnum);

        // update MMU control register (ST)
        MMU_CONTROL_REG |= (1 << 14);

        v::info << this->name() << "Created split instruction and data TLBs."
                << v::endl;

    } else {

        // combined tlb mode -> share instruction tlb
        dtlb = itlb;
        dtlb_adaptor = itlb_adaptor;

        v::info << this->name()
                << "Created combined instruction and data TLBs." << v::endl;

    }

    // The page size can be 4k, 8k, 16k or 32k.
    // Depending on the configuration the indices for the address table
    // lookup have different range.

    switch (mmupgsz) {

        case 0: // 4 kB
            m_idx1 = 8;
            m_idx2 = 6;
            m_idx3 = 6;
            m_vtag_width = 20;

            break;
        case 2: // also 4 kB
            m_idx1 = 8;
            m_idx2 = 6;
            m_idx3 = 6;
            m_vtag_width = 20;
            break;
        case 3: // 8 kB
            m_idx1 = 7;
            m_idx2 = 6;
            m_idx3 = 6;
            m_vtag_width = 19;

            // update MMU control register (PSZ)
            MMU_CONTROL_REG |= (1 << 16);
            break;
        case 4: // 16 kB
            m_idx1 = 6;
            m_idx2 = 6;
            m_idx3 = 6;
            m_vtag_width = 18;

            // update MMU control register (PSZ)
            MMU_CONTROL_REG |= (2 << 16);
            break;
        case 5: // 32 kB
            m_idx1 = 4;
            m_idx2 = 7;
            m_idx3 = 6;
            m_vtag_width = 17;

            // update MMU control register (PSZ)
            MMU_CONTROL_REG |= (3 << 16);
            break;
        default: // not supported
            v::error << this->name() << "Selected mmupgsz not supported!"
                    << v::endl;
            assert(false);
    }

    v::info << this->name() << " ******************************************************************************* " << v::endl;
    v::info << this->name() << " * Created mmu with following parameters: " << v::endl;
    v::info << this->name() << " * number of instruction tlbs: " << m_itlbnum << v::endl;
    v::info << this->name() << " * number of data tlbs: " << m_dtlbnum << v::endl;
    v::info << this->name() << " * tlb type (0 - split, 1 - shared): " << m_tlb_type << v::endl;
    v::info << this->name() << " * MMU_CONTROL_REG: " << hex << MMU_CONTROL_REG << v::endl;
    v::info << this->name() << " * ***************************************************************************** " << v::endl;
}

// look up a tlb (page descriptor cache)
// and return physical address
unsigned int mmu::tlb_lookup(unsigned int addr,
                             std::map<t_VAT, t_PTE_context> * tlb,
                             unsigned int tlb_size, sc_core::sc_time * t,
                             unsigned int * debug) {

    // According to the SparcV8 Manual: Pages of the Reference MMU are always aligned on 4K-byte boundaries; hence, the lower-order
    // 12 bits of a physical address are always the same as the low-order 12 bits of
    // the virtual address. The Gaisler MMU additionally supports 8k, 16k and 32k alignment,
    unsigned int offset = ((addr << m_vtag_width) >> m_vtag_width);
    t_VAT vpn = (addr >> (32 - m_vtag_width));

    // The Virtual Address Tag consists of three indices, which are used to look
    // up the three level page table in main memory. The indices are extracted from
    // the tag and translated to byte addresses (shift left 2 bit)
    unsigned int idx1 = (vpn >> (m_idx2 + m_idx3) << 2);
    unsigned int idx2 = (vpn << (32 - m_idx2 - m_idx3)) >> (30 - m_idx3);
    unsigned int idx3 = (vpn << (32 - m_idx3)) >> (30 - m_idx3);

    // locals for intermediate results
    t_PTE_context tmp;
    unsigned int paddr;
    unsigned int data;

    bool context_miss = false;

    // search virtual address tag in ipdc (associative)
    v::info << this->name() << "lookup with VPN: " << std::hex << vpn
            << " and OFFSET: " << std::hex << offset << v::endl;
    pdciter = tlb->find(vpn);

    v::info << this->name() << "pdciter->first: " << std::hex << pdciter->first
            << " pdciter->second: " << std::hex << (pdciter->second).pte
            << v::endl;

    // tlb hit
    if (pdciter != tlb->end()) {

        v::info << this->name() << "Virtual Address Tag hit on address: "
                << std::hex << addr << v::endl;

        // read the PDC entry
        tmp = pdciter->second;

        // check the context tag
        if (tmp.context == MMU_CONTEXT_REG) {

            v::info << this->name() << "CONTEXT hit" << v::endl;

            // build physical address from PTE and offset, and return
            paddr = (((tmp.pte >> 8) << (32 - m_vtag_width)) | offset);

            // update debug information
            TLBHIT_SET(*debug);

            return (paddr);

        } else {

            v::info << this->name() << "CONTEXT miss" << v::endl;

            // update debug information
            TLBMISS_SET(*debug);
            context_miss = true;

        }
    } else {

        v::info << this->name() << "Virtual Address Tag miss" << v::endl;

        // update debug information
        TLBMISS_SET(*debug);

    }

    // COMPONSITON OF A PAGE TABLE DESCRIPTOR (PTD)
    // [31-2] PTP - Page Table Pointer. Physical address of the base of a next-level
    // page table. The PTP appears on bits 35 (32) through 6 of the physical address bus
    // during miss processing. The page table pointed to by a PTP must be aligned on a
    // boundary equal to the size of the page table. The sizes of the three levels of
    // page tables are: level 1 - 1024 bytes, level 2 - 256 bytes, level 3 - 256 bytes.
    // [1-0] ET - Entry type. (0 - reserved, 1 - PTD, 2 - PTE, 3 - reserved)

    // COMPOSITION OF A PAGE TABLE ENTRY (PTE)
    // [31-8] PPN - Physical Page Number. The PPN appears on bits 35 through 12 of the
    // physical address bus when a translation completes (for 4kb default configuration)
    // [7] C - Cacheable. If this bit is one, the page is cacheable by an instruction
    // and/or data cache.
    // [6] M - Modified. This bit is set to one by the MMU when the page is accessed
    // for writing (except when accessed via passthrough ASI).
    // [5] R - Referenced. this bit is set to one by the MMU when the page is accessed
    // (except when accessed via passthrough ASI)
    // [4-2] Access Permissions (see page 248 of Sparc Reference Manual)
    // [1-0] ET - Entry type. (0 - reserved, 1 - PTD, 2 - PTE, 3 - reserved)

    // tlb miss processing
    v::info << this->name()
            << "START TLB MISS PROCESSING FOR VIRTUAL ADDRESS: " << std::hex
            << addr << v::endl;

    // ***************************************
    // 3-Level TLB table walk
    // ***************************************

    // todo: should we wrap this in a function ??

    // 1. load from 1st-level page table
    m_mmu_cache->mem_read(MMU_CONTEXT_TABLE_POINTER_REG + idx1,
            (unsigned char *)&data, 4, t, debug);

    v::info << this->name() << "Back from read addr: " << std::hex
            << (MMU_CONTEXT_TABLE_POINTER_REG + idx1) << " data: " << std::hex
            << data << v::endl;

    // !!!! todo: why is is it always loosing the sc_module_name here ????

    // page table entry (PTE) or page table descriptor (PTD) (to level 2)
    if ((data & 0x3) == 0x2) {

        v::info << this->name() << "1-Level Page Table returned PTE: "
                << std::hex << data << v::endl;

        // In case of a virtual address tag miss a new PDC entry is created.
        // For context miss the existing entry will be replaced.
        if ((!context_miss) && (tlb->size() == tlb_size)) {

            v::info << this->name() << "TLB full" << std::hex << data
                    << v::endl;
            // kick out an entry to make room for a new one
            // todo !!
        }

        // add to PDC
        tmp.context = MMU_CONTEXT_REG;
        tmp.pte = data;

        (*tlb)[vpn] = tmp;

        // build physical address from PTE and offset
        paddr = (((tmp.pte >> 8) << (32 - m_vtag_width)) | offset);

        v::info << this->name() << "Mapping complete - Virtual Addr: "
                << std::hex << addr << " Physical Addr: " << std::hex << paddr
                << v::endl;
        return (paddr);
    } else if ((data & 0x3) == 0x1) {

        v::info << this->name() << "1st-Level Page Table returned PTD: "
                << std::hex << data << v::endl;

    } else {

        v::info << this->name()
                << "Error in 1st-Level Page Table / Entry type not valid"
                << v::endl;
        assert(false);
        return (0);
    }

    // 2. load from 2nd-level page table
    m_mmu_cache->mem_read((((data) >> 2) << 2) + idx2, (unsigned char *)&data,
            4, t, debug);

    // page table entry (PTE) or page table descriptor (PTD) (to level 3)
    if ((data & 0x3) == 0x2) {

        v::info << this->name() << "2-Level Page Table returned PTE: "
                << std::hex << data << v::endl;

        // In case of a virtual address tag miss a new PDC entry is created.
        // For context miss the existing entry will be replaced.
        if ((!context_miss) && (tlb->size() == tlb_size)) {

            v::info << this->name() << "TLB full" << std::hex << data
                    << v::endl;
            // kick out an entry to make room for a new one
            // todo !!
        }

        // add to PDC
        tmp.context = MMU_CONTEXT_REG;
        tmp.pte = data;

        (*tlb)[vpn] = tmp;

        // build physical address from PTE and offset
        paddr = (((tmp.pte >> 8) << (32 - m_vtag_width)) | offset);

        v::info << this->name() << "Mapping complete - Virtual Addr: "
                << std::hex << addr << " Physical Addr: " << std::hex << paddr
                << v::endl;
        return (paddr);
    } else if ((data & 0x3) == 0x1) {

        v::info << this->name() << "2-Level Page Table returned PTD: "
                << std::hex << data << v::endl;

    } else {

        v::info << this->name()
                << "Error in 2-Level Page Table / Entry type not valid"
                << v::endl;
        assert(false);
        return (0);
    }

    // 3. load from 3rd-level page table
    m_mmu_cache->mem_read((((data) >> 2) << 2) + idx3, (unsigned char *)&data,
            4, t, debug);

    // 3rd-level page table must contain PTE (PTD not allowed)
    if ((data & 0x3) == 0x2) {

        v::info << this->name() << "3-Level Page Table returned PTE: "
                << std::hex << data << v::endl;

        // In case of a virtual address tag miss a new PDC entry is created.
        // For context miss the existing entry will be replaced.
        if ((!context_miss) && (tlb->size() == tlb_size)) {

            v::info << this->name() << "TLB full" << std::hex << data
                    << v::endl;
            // kick out an entry to make room for a new one
            // todo !!
        }

        // add to PDC
        tmp.context = MMU_CONTEXT_REG;
        tmp.pte = data;

        (*tlb)[vpn] = tmp;

        // build physical address from PTE and offset
        paddr = (((tmp.pte >> 8) << (32 - m_vtag_width)) | offset);

        v::info << this->name() << "Mapping complete - Virtual Addr: "
                << std::hex << addr << " Physical Addr: " << std::hex << paddr
                << v::endl;
        return (paddr);
    } else {

        v::info << this->name()
                << "Error in 3-Level Page Table / Entry type not valid"
                << v::endl;
        assert(false);
        return (0);
    }
}

/// Read MMU Control Register
unsigned int mmu::read_mcr() {

    return (MMU_CONTROL_REG);

}

/// Write MMU Control Register
void mmu::write_mcr(unsigned int * data) {

    // only TD [15], NF [1] and E [0] are writable
    MMU_CONTROL_REG = (*data & 0x00008003);

}

/// Read MMU Context Table Pointer Register
unsigned int mmu::read_mctpr() {

    return (MMU_CONTEXT_TABLE_POINTER_REG);

}

/// Write MMU Context Table Pointer Register
void mmu::write_mctpr(unsigned int * data) {

    // [1-0] reserved, must read as zero
    MMU_CONTEXT_TABLE_POINTER_REG = (*data & 0xfffffffc);

}

/// Read MMU Context Register
unsigned int mmu::read_mctxr() {

    return (MMU_CONTEXT_REG);

}

/// Write MMU Context Register
void mmu::write_mctxr(unsigned int * data) {

    MMU_CONTEXT_REG = *data;

}

/// Read MMU Fault Status Register
unsigned int mmu::read_mfsr() {

    return (MMU_FAULT_STATUS_REG);

}

/// Read MMU Fault Address Register
unsigned int mmu::read_mfar() {

    return (MMU_FAULT_ADDRESS_REG);

}

// Diagnostic PDC access (Page 255 SparcV8 Man):
// =============================================
// VA[31-12] Virtual address
// VA[11-4]  PDC entry
// VA[3-2]   Register Selector

// Register Selector:
// ==================
// 0 - D[31-20] context, D[19-0] address tag
// 1 - PTE
// 2 - control bits (e.g. V, Level, LRU) ???
// 3 - Read:  Start compare in every PDC entry
//            if hit, return the PTE; if miss, return 0.
//     Write: For each PDC entry: if the contents of its
//            LRU counter is less than the stored data
//            value, increment its counter. Otherwise,
//            leave the LRU counter unchanged. In any
//            case, zero the LRU counter of the addressed
//            PDC entry.


/// Diagnostic read of instruction PDC (ASI 0x5)
void mmu::diag_read_itlb(unsigned int addr, unsigned int * data) {

    t_VAT vpn = (addr >> (32 - m_vtag_width));

    // diagnostic ITLB lookup (without bus access)
    if ((addr & 0x3) == 0x3) {

        pdciter = itlb->find(vpn);

        // found something ?
        if (pdciter != itlb->end()) {

            // hit
            *data = ((pdciter->second).pte);

        } else {

            // miss
            *data = 0;
        }
    }
}

/// Diagnostic write of instruction PDC (ASI 0x5)
void mmu::diag_write_itlb(unsigned int addr, unsigned int * data) {
}

/// Diagnostic read of data or shared instruction and data PDC (ASI 0x6)
void mmu::diag_read_dctlb(unsigned int addr, unsigned int * data) {

    t_VAT vpn = (addr >> (32 - m_vtag_width));

    // diagnostic ITLB lookup (without bus access)
    if ((addr & 0x3) == 0x3) {

        pdciter = dtlb->find(vpn);

        // found something ?
        if (pdciter != dtlb->end()) {

            // hit
            *data = ((pdciter->second).pte);

        } else {

            // miss
            *data = 0;
        }
    }
}

/// Diagnostic write of data or shared instruction and data PDC (ASI 0x6)
void mmu::diag_write_dctlb(unsigned int addr, unsigned int * data) {
}

/// return handle to itlb memory interface object
tlb_adaptor * mmu::get_itlb_if() {

    return (itlb_adaptor);

}

/// return handle to dtlb memory interface object
tlb_adaptor * mmu::get_dtlb_if() {

    return (dtlb_adaptor);

}

// Helper for setting clock cycle latency using sc_clock argument
void mmu::clk(sc_core::sc_clock &clk) {

  clockcycle = clk.period();

}

// Helper for setting clock cycle latency using sc_time argument
void mmu::clk(sc_core::sc_time &period) {

  clockcycle = period;

}

// Helper for setting clock cycle latency using a value-time_unit pair
void mmu::clk(double period, sc_core::sc_time_unit base) {

  clockcycle = sc_core::sc_time(period, base);

}
