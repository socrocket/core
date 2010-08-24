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
  mmu_cache mmu_cache(0,  //  int dsu = 0, 
		      1,  //  int icen = 1 (icache enabled)
		      3,  //  int irepl = 3 (icache random replacement)
		      1,  //  int isets = 1 (2 instruction cache sets)
		      0,  //  int ilinesize = 0 (1 word per icache line)
		      8,  //  int isetsize = 8 (256kB per icache set)
		      0,  //  int isetlock = 0 (no icache locking)
		      1,  //  int dcen = 1 (dcache enabled)
		      3,  //  int drepl = 3 (dcache random replacement)
		      1,  //  int dsets = 1 (2 data cache sets)
		      0,  //  int dlinesize = 0 (1 word per dcache line)
		      8,  //  int dsetsize = 8 (256kB per dcache set)
		      0,  //  int dsetlock = 0 (no dcache locking)
		      0,  //  int dsnoop = 0 (no cache snooping)
		      1,  //  int ilram = 1 (instr. localram disable)
		      9,  //  int ilramsize = 9 (512kB ilram size)
		      0x0000008e, //  int ilramstart = 8e (0x8e000000 default ilram start address)
		      1,  //  int dlram = 1 (data localram disable)
		      9,  //  int dlramsize = 9 (512kB dlram size)
		      0x0000008f, //  int dlramstart = 8f (0x8f000000 default dlram start address)
		      0,  //  int cached = 0 (fixed cacheability mask)
		      0,  //  int mmu_en = 0 (mmu not present)
		      3,  //  int itlb_num = 3 (8 itlbs - not present)
		      3,  //  int dtlb_num = 3 (8 dtlbs - not present)
		      0,  //  int tlb_type = 0 (split tlb mode - not present)
		      1,  //  int tlb_rep = 1 (random replacement)
		      0,  //  int mmupgsz = 0 (4kB mmu page size)
		      "mmu_cache",   // name of sysc module
		      CACHE_MASTER_ID,   // id of the AHB master
		      sc_core::sc_time(0, sc_core::SC_NS),              // icache delay for read hit
		      sc_core::sc_time(LOCAL_CLOCK, sc_core::SC_NS),    // icache delay for read miss,
		      sc_core::sc_time(0, sc_core::SC_NS),              // dcache delay for read hit
		      sc_core::sc_time(LOCAL_CLOCK, sc_core::SC_NS),    // dcache delay for read miss
		      sc_core::sc_time(LOCAL_CLOCK, sc_core::SC_NS),    // dcache delay for write (hit and miss)
		      sc_core::sc_time(0, sc_core::SC_NS),              // itlb delay on read hit
		      sc_core::sc_time(2*LOCAL_CLOCK, sc_core::SC_NS),  // itlb delay on read miss
		      sc_core::sc_time(0, sc_core::SC_NS),              // dtlb delay on read hit
		      sc_core::sc_time(2*LOCAL_CLOCK, sc_core::SC_NS)); // dtlb delay on read miss

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
  
 
