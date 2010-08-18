// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       main.cpp - Top level file (sc_main) for                 *
// *             lt_ct_localram_test. The mmu_cache is connected         *
// *             to a testbench (lt) and a cycle timed AHB bus.          *
// *	         The configuration under test has ilram and dlram        *
// *	         scratchpads of 512 kByte. The mmu is disabled.          *
// *                                                                     *
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision$                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************


#include "tlm.h"

#include "testbench.h"
#include "mmu_cache.h"
#include "defines.h"
#include "locals.h"

#include "amba.h"
#include "ahb_simple_bus.h"
#include "adapters/AMBA_LT_CT_Adapter.h"
#include "ahb_simple_slave.h"

#include <ostream>


// **********************************************************
// * Testbed for mmu_cache development:
//
//
//                                 ---------------------
//                                 |  mmu_cache (lt)   |
//                                 | ----------------- |
// --------   TLM2 Generic Payl.   | | ivectorcache  | |
// |      |-----------|------------| ----------------- |                       -------------   
// |      |instruction|icio target | ----------------- |     AHB Payload       |           |
// |      |init.sock  |socket(lt)  | | dvectorcache  | |-----------|-----------| ahb_lt_ct |
// |  tb  |-----------|------------| ----------------- | ahb_master|slave_sock | transactor|  ...
// | (lt) |-----------|------------| -------   ------- | (lt)      |(lt)       | (lt -> ct)|  ->
// |      |data       |dcio target | |ilram|   |dlram| |-----------|-----------|           |
// |      |init.sock  |socket (lt) | -------   - ----- |                       -------------
// |      |-----------|------------|                   |
// --------                        ---------------------
//
//
//
//
//
//                                                                   -------------
//                               |---------|                         |           |
//       |-----------|-----------|         |-----------|-------------|           |
//  ...  |master_sock|slave_sock | ahb_bus |master_sock|slave_socket |  ahb_mem  |
//  ->   |(ct)       |(ct)       |  (ct)   |(ct)       |(ct)         |    (ct)   |
//       |-----------|-----------|         |-----------|-------------|           |
//                               |---------|                         |           |
//                                                                   -------------

int sc_main(int argc, char** argv) {

  // create clock
  sc_core::sc_clock clk("clk", LOCAL_CLOCK, sc_core::SC_NS);

  // *** CREATE MODULES

  // create testbench
  testbench tb("Testbench");

  // CREATE MMU Cache
  // ----------------
  // constructor args: 
  // - name of sysc module
  // - id of the AHB master
  // - icache delay for read hit
  // - icache delay for read miss,
  // - dcache delay for read hit
  // - dcache delay for read miss
  // - dcache delay for write (hit and miss)
  // - itlb delay on read hit
  // - itlb delay on read miss
  // - dtlb delay on read hit
  // - dtlb delay on read miss
  //
  // template parameters:
  // <int dsu = 0, 
  //  int icen = 1 (icache enabled)
  //  int irepl = 3 (icache random replacement)
  //  int isets = 1 (2 instruction cache sets)
  //  int ilinesize = 0 (1 word per icache line)
  //  int isetsize = 8 (256kB per icache set)
  //  int isetlock = 0 (no icache locking)

  //  int dcen = 1 (dcache enabled)
  //  int drepl = 3 (dcache random replacement)
  //  int dsets = 1 (2 data cache sets)
  //  int dlinesize = 0 (1 word per dcache line)
  //  int dsetsize = 8 (256kB per dcache set)
  //  int dsetlock = 0 (no dcache locking)
  //  int dsnoop = 0 (no cache snooping)

  //  int ilram = 1 (instr. localram disable)
  //  int ilramsize = 9 (1kB ilram size)
  //  int ilramstart = 8e (0x8e000000 default ilram start address)

  //  int dlram = 1 (data localram disable)
  //  int dlramsize = 9 (1kB dlram size)
  //  int dlramstart = 8f (0x8f000000 default dlram start address)

  //  int cached = 0 (fixed cacheability mask)
  //  int mmu_en = 0 (mmu not present)
  //  int itlb_num = 3 (8 itlbs - not present)
  //  int dtlb_num = 3 (8 dtlbs - not present)
  //  int tlb_type = 0 (split tlb mode - not present)
  //  int tlb_rep = 1 (random replacement)
  //  int mmupgsz = 0 (4kB mmu page size)>
  mmu_cache<0,1,3,1,0,8,0,1,3,1,0,8,0,0,1,9,0x0000008e,1,9,0x0000008f,0,0,3,3,0,1,0> mmu_cache("mmu_cache", 
			CACHE_MASTER_ID, 
			sc_core::sc_time(0, sc_core::SC_NS), 
			sc_core::sc_time(LOCAL_CLOCK, sc_core::SC_NS),
			sc_core::sc_time(0, sc_core::SC_NS),
			sc_core::sc_time(LOCAL_CLOCK, sc_core::SC_NS),
			sc_core::sc_time(LOCAL_CLOCK, sc_core::SC_NS),
			sc_core::sc_time(0, sc_core::SC_NS),
			sc_core::sc_time(2*LOCAL_CLOCK, sc_core::SC_NS),
			sc_core::sc_time(0, sc_core::SC_NS),
			sc_core::sc_time(2*LOCAL_CLOCK, sc_core::SC_NS));

  // create AHB bus
  ahb_simple_bus<32> ahb_bus("AHB_Bus");

  // create LT CT Adapter
  amba::AMBA_LT_CT_Adapter<32> ahb_lt_ct("AHB_LT_CT",amba::amba_AHB);

  // create AHB memory (1MB from address base 0)
  //ahb_ct_mem<32> ahb_mem("AHB_MEM",0, 0x1000);
  ahb_slave<32> ahb_mem("AHB_MEM",0,0x100000);

  // *** BIND SOCKETS

  // connect testbench (cpu) to mmu-cache
  tb.instruction_initiator_socket(mmu_cache.icio);
  tb.data_initiator_socket(mmu_cache.dcio);

  // connect mmu-cache to LT_CT adapter
  mmu_cache.ahb_master(ahb_lt_ct.slave_sock);

  // connect LT_CT adapter to bus
  ahb_lt_ct.master_sock(ahb_bus.slave_sock);

  // connect AHB bus to AHB memory
  ahb_bus.master_sock(ahb_mem.slave_socket);

  // connect CT moduls to clock
  ahb_mem.clk(clk);
  ahb_bus.clk(clk);
  ahb_lt_ct.clk(clk);

   // start simulation
  sc_core::sc_start();

  return 0;

}
  
 
