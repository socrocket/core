/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mmu.h - Implementation of a memory management unit.     */
/*             The mmu can be configured to have split or combined     */
/*             TLBs for instructions and data. The TLB size can be     */
/*             configured as well. The memory page size is currently   */
/*             currently fixed to 4kB.                                 */
/*                                                                     */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#include "mmu.h"

// constructor args:
// - sysc module name,
// - delay on an instruction tlb hit
// - delay on an instruction tlb miss (per page table lookup)
// - delay on a data tlb hit
// - delay on a data tlb miss (per page table lookup)
// - number of instruction tlbs
// - number of data tlbs
// - tlb type
// - tlb replacement strategy
// - tlb mmu page size (default 4kB)
mmu::mmu(sc_core::sc_module_name name,
      mmu_cache_if &_parent,
      sc_core::sc_time itlb_hit_response_delay,
      sc_core::sc_time itlb_miss_response_delay,
      sc_core::sc_time dtlb_hit_response_delay,
      sc_core::sc_time dtlb_miss_response_delay,
      unsigned int itlbnum,
      unsigned int dtlbnum,
      unsigned int tlb_type,
      unsigned int tlb_rep,
      unsigned int mmupgsz) : sc_module(name),
			     m_itlbnum(itlbnum),
			     m_dtlbnum(dtlbnum),
			     m_tlb_type(tlb_type),
			     m_tlb_rep(tlb_rep),
			     m_mmupgsz(mmupgsz),
			     m_itlb_hit_response_delay(itlb_hit_response_delay),
			     m_itlb_miss_response_delay(itlb_miss_response_delay),
			     m_dtlb_hit_response_delay(dtlb_hit_response_delay),
			     m_dtlb_miss_response_delay(dtlb_miss_response_delay)
{

  // initialize internal registers
  MMU_CONTROL_REG = 0;
  MMU_CONTEXT_TABLE_POINTER_REG = 0;
  MMU_CONTEXT_REG = 0;
  MMU_FAULT_STATUS_REG = 0;
  MMU_FAULT_ADDRESS_REG = 0;

  // generate associative memory (map) for instruction tlb
  itlb = new std::map<t_VAT, t_PTE_context>;

  // are we in split tlb mode?
  if (m_tlb_type & 0x1) {

    // generate another associative memory (map) for data tlb
    dtlb = new std::map<t_VAT, t_PTE_context>;
    DUMP(this->name(),"Created split instruction and data TLBs.");

  } else {

    // combined tlb mode -> share instruction tlb
    dtlb = itlb;
    DUMP(this->name(),"Created combined instruction and data TLBs.");

  }

  // hook up to top level (amba if)
  m_parent = &_parent;

}
 
// instruction read interface
void mmu::itlb_read(unsigned int addr, unsigned int * data, unsigned int length) {

  // Pages are always aligned on 4K-byte boundaries; hence, the lower-order
  // 12 bits of a physical address are always the same as the low-order 12 bits of
  // the virtual address.
  unsigned int paddr;

  // mmu enabled ?
  if (MMU_CONTROL_REG & 0x1) {

    DUMP(this->name(),"ITLB READ request (active mode) virtual address: " << std::hex << addr);
    // virtual address translation
    paddr=tlb_lookup(addr, itlb, m_itlbnum);
 
  }
  // mmu in bypass mode
  else {

    DUMP(this->name(),"ITLB READ request (bypass mode)!");
    // physical addressing
    paddr=addr;
  }

  // forward request to amba interface
  m_parent->amba_read(paddr, data, length);
}

// instruction write interface (for flushing)
void mmu::itlb_write(unsigned int addr, unsigned int * data, unsigned int length) {

  // Pages are always aligned on 4K-byte boundaries; hence, the lower-order
  // 12 bits of a physical address are always the same as the low-order 12 bits of
  // the virtual address.
  unsigned int paddr;

  // mmu enabled ?
  if (MMU_CONTROL_REG & 0x1) {

    DUMP(this->name(),"ITLB WRITE request (active mode) virtual address: " << std::hex << addr);
    // virtual address translation
    paddr=tlb_lookup(addr, itlb, m_itlbnum);

  }
  // mmu in bypass mode
  else {

    DUMP(this->name(),"ITLB WRITE request (bypass mode)!");
    // physical addressing
    paddr=addr;
  }

  // forward request to amba interface
  m_parent->amba_write(addr, data, length);
}

// data read interface
void mmu::dtlb_read(unsigned int addr, unsigned int * data, unsigned int length) {

  // Pages are always aligned on 4K-byte boundaries; hence, the lower-order
  // 12 bits of a physical address are always the same as the low-order 12 bits of
  // the virtual address.
  unsigned int paddr;

  // mmu enabled ?
  if (MMU_CONTROL_REG & 0x1) {
    
    DUMP(this->name(),"DTLB READ request (active mode) virtual address: " << std::hex << addr);
    // virtual address translation
    paddr=tlb_lookup(addr, dtlb, m_dtlbnum);    

  }
  // mmu in bypass mode
  else {

    DUMP(this->name(),"DTLB READ request (bypass mode)!");
    // physical addressing
    paddr=addr;

  }

  // forward request to amba interface
  m_parent->amba_read(addr, data, length);
}

// data write interface
void mmu::dtlb_write(unsigned int addr, unsigned int * data, unsigned int length) {

  // Pages are always aligned on 4K-byte boundaries; hence, the lower-order
  // 12 bits of a physical address are always the same as the low-order 12 bits of
  // the virtual address.
  unsigned int paddr;

  // mmu enabled ?
  if (MMU_CONTROL_REG & 0x1) {

    DUMP(this->name(),"DTLB WRITE request (active mode) virtual address: " << std::hex << addr);
    // virtual address translation
    paddr=tlb_lookup(addr, dtlb, m_dtlbnum);    

  }
  // mmu in bypass mode
  else {

    DUMP(this->name(),"DTLB WRITE request (bypass mode)!");
    // physical addressing
    paddr=addr;

  }

  // forward request to amba interface
  m_parent->amba_write(addr, data, length);
}

// look up a tlb (page descriptor cache)
// and return physical address
unsigned int mmu::tlb_lookup(unsigned int addr, std::map<t_VAT, t_PTE_context> * tlb, unsigned int tlb_size) {

  // Pages are always aligned on 4K-byte boundaries; hence, the lower-order
  // 12 bits of a physical address are always the same as the low-order 12 bits of
  // the virtual address.
  unsigned int offset = (addr & 0x3ff);
  t_VAT vpn = (addr >> 12);

  // The Virtual Address Tag consists of three indices, which are used to look
  // up the three level page table in main memory. The indices are extracted from
  // the tag and translated to byte addresses (shift left 2 bit)
  unsigned int idx1 = (vpn >> 12) << 2;
  unsigned int idx2 = (vpn << 20) >> 24;
  unsigned int idx3 = (vpn << 26) >> 24;

  // locals for intermediate results
  t_PTE_context tmp;
  unsigned int paddr;
  unsigned int * data;
  bool context_miss = false;

  // search virtual address tag in ipdc (associative)
  pdciter = tlb->find(vpn);

  // tlb hit
  if (pdciter != itlb->end()) {

    DUMP(this->name(),"PDC hit on address: " << std::hex << addr);

    // read the PDC entry
    tmp = pdciter->second;

    // check the context tag
    if (tmp.context == MMU_CONTEXT_REG) {

      DUMP(this->name(),"CONTEXT hit");

      // build physical address from PTE and offset, and return
      paddr = ((tmp.pte<<12)|offset);
      return(paddr);

    }
    else {

      DUMP(this->name(),"CONTEXT miss");
      context_miss = true;

    }
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
  // physical address bus when a translation completes
  // [7] C - Cacheable. If this bit is one, the page is cacheable by an instruction
  // and/or data cache.
  // [6] M - Modified. This bit is set to one by the MMU when the page is accessed
  // for writing (except when accessed via passthrough ASI).
  // [5] R - Referenced. this bit is set to one by the MMU when the page is accessed
  // (except when accessed via passthrough ASI)
  // [4-2] Access Permissions (see page 248 of Sparc Reference Manual)
  // [1-0] ET - Entry type. (0 - reserved, 1 - PTD, 2 - PTE, 3 - reserved)

  // tlb miss processing  
  DUMP(this->name(),"START TLB MISS PROCESSING FOR VIRTUAL ADDRESS: " << std::hex << addr);

  // ***************************************
  // 3-Level TLB table walk
  // ***************************************

  // todo: should we wrap this in a function ??
  
  // 1. load from 1st-level page table
  m_parent->amba_read(MMU_CONTEXT_TABLE_POINTER_REG+idx1, data, 4);

  // page table entry (PTE) or page table descriptor (PTD) (to level 2)
  if (*data & 0x3 == 0x2) {

    DUMP(this->name(),"1-Level Page Table returned PTE: " << std::hex << *data);

    // In case of a virtual address tag miss a new PDC entry is created.
    // For context miss the existing entry will be replaced.
    if ((!context_miss)&&(tlb->size()==tlb_size)) {
      
      DUMP(this->name(),"TLB full" << std::hex << *data);
      // kick out an entry to make room for a new one
      // todo !!
    }

    // add to PDC
    tmp.context = MMU_CONTEXT_REG;
    tmp.pte     = *data;

    (*tlb)[vpn] = tmp; 

    // build physical address from PTE and offset
    paddr = (((*data) << 4)|offset);

    return(paddr);
  }
  else if ((*data & 0x3) == 0x1) { 

    DUMP(this->name(),"1-Level Page Table returned PTD: " << std::hex << *data);

  }
  else {

    DUMP(this->name(),"Error in 1-Level Page Table / Entry type not valid");
    return(0);
  }

  // 2. load from 2nd-level page table
  m_parent->amba_read((((*data)>>2)<<6)+idx2, data, 4);

  // page table entry (PTE) or page table descriptor (PTD) (to level 3)
  if (*data & 0x3 == 0x2) {

    DUMP(this->name(),"2-Level Page Table returned PTE: " << std::hex << *data);

    // In case of a virtual address tag miss a new PDC entry is created.
    // For context miss the existing entry will be replaced.
    if ((!context_miss)&&(tlb->size()==tlb_size)) {
      
      DUMP(this->name(),"TLB full" << std::hex << *data);
      // kick out an entry to make room for a new one
      // todo !!
    }

    // add to PDC
    tmp.context = MMU_CONTEXT_REG;
    tmp.pte     = *data;

    (*tlb)[vpn] = tmp; 

    // build physical address from PTE and offset
    paddr = (((*data) << 4)|offset);

    return(paddr);
  }
  else if ((*data & 0x3) == 0x1) { 

    DUMP(this->name(),"2-Level Page Table returned PTD: " << std::hex << *data);

  }
  else {

    DUMP(this->name(),"Error in 2-Level Page Table / Entry type not valid");
    return(0);
  }

  // 3. load from 3rd-level page table
  m_parent->amba_read((((*data)>>2)<<6)+idx3, data, 4);

  // 3rd-level page table must contain PTE (PTD not allowed)
  if (*data & 0x3 == 0x2) {

    DUMP(this->name(),"3-Level Page Table returned PTE: " << std::hex << *data);

    // In case of a virtual address tag miss a new PDC entry is created.
    // For context miss the existing entry will be replaced.
    if ((!context_miss)&&(tlb->size()==tlb_size)) {
      
      DUMP(this->name(),"TLB full" << std::hex << *data);
      // kick out an entry to make room for a new one
      // todo !!
    }

    // add to PDC
    tmp.context = MMU_CONTEXT_REG;
    tmp.pte     = *data;

    (*tlb)[vpn] = tmp; 

    // build physical address from PTE and offset
    paddr = (((*data) << 4)|offset);

    return(paddr);
  }
  else {

    DUMP(this->name(),"Error in 3-Level Page Table / Entry type not valid");
    return(0);
  }
}

// read MMU Control Register
unsigned int mmu::read_mcr() {

  return(MMU_CONTROL_REG);

}

// write MMU Control Register
void mmu::write_mcr(unsigned int * data) {

  // only PSO [7], NF [1] and E [0] are writable
  MMU_CONTROL_REG = (*data & 0x00000083);

}

// read MMU Context Pointer Register
unsigned int mmu::read_mctpr() {

  return(MMU_CONTEXT_TABLE_POINTER_REG);

}

void mmu::write_mctpr(unsigned int * data) {

  // [1-0] reserved, must read as zero
  MMU_CONTEXT_TABLE_POINTER_REG = (*data & 0xfffffffc);

}

// read MMU Context Register
unsigned int mmu::read_mctxr() {

  return(MMU_CONTEXT_REG);

}

// write MMU Context Register
void mmu::write_mctxr(unsigned int * data) {

  MMU_CONTEXT_REG = *data;

}

// read MMU Fault Status Register
unsigned int mmu::read_mfsr() {

  return(MMU_FAULT_STATUS_REG);

}

// read MMU Fault Address Register
unsigned int mmu::read_mfar() {

  return(MMU_FAULT_ADDRESS_REG);

}
