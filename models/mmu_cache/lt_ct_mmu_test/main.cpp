/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       main.cpp - Top level file (sc_main) for                 */
/*             lt_ct_mmu_test. Purpose is the assemblence of       */
/*             a test environment for the mmu_cache module in its      */
/*             default configuration. The mmu_cache is connected to    */
/*             a testbench (lt) and a cycle timed AHB bus.             */
/*                                                                     */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/


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
//
// 
// --------   TLM2 Generic Payl.   ---------------------
// |      |-----------|------------|  mmu_cache (lt)   |                       -------------   
// |      |instruction|icio target |  ---------------  |     AHB Payload       |           |
// |      |init.sock  |socket(lt)  |  |ivectorcache |  |-----------|-----------| ahb_lt_ct |
// |  tb  |-----------|------------|  ---------------  | ahb_master|slave_sock | transactor|  ...
// | (lt) |-----------|------------|  ---------------  | (lt)      |(lt)       | (lt -> ct)|  ->
// |      |data       |dcio target |  |idatacache   |  |-----------|-----------|           |
// |      |init.sock  |socket (lt) |  ---------------  |                       -------------
// |      |-----------|------------|  ---------------  |
// --------                        |  |    srmmu    |  |
//                                 |  ---------------  |
//                                 ---------------------
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

  mmu_cache mmu_cache(0,  // <int dsu = 0, 
		      1,  //  int icen = 1 (icache enabled)
		      3,  //  int irepl = 3 (icache random replacement)
		      3,  //  int isets = 3 (4 instruction cache sets)
		      0,  //  int ilinesize = 0 (1 word per icache line)
		      0,  //  int isetsize = 0 (1kB per icache set)
		      0,  //  int isetlock = 0 (no icache locking)
		      1,  //  int dcen = 1 (dcache enabled)
		      3,  //  int drepl = 3 (dcache random replacement)
		      3,  //  int dsets = 3 (4 data cache sets)
		      0,  //  int dlinesize = 0 (1 word per dcache line
		      0,  //  int dsetsize = 0 (1kB per dcache set)
		      0,  //  int dsetlock = 0 (no dcache locking)
		      0,  //  int dsnoop = 0 (no cache snooping)
		      0,  //  int ilram = 0 (instr. localram disable)
		      0,  //  int ilramsize = 0 (1kB ilram size)
		      0x0000008e,  //  int ilramstart = 8e (0x8e000000 default ilram start address)
		      0,  //  int dlram = 0 (data localram disable)
		      0,  //  int dlramsize = 0 (1kB dlram size)
		      0x0000008f, //  int dlramstart = 8f (0x8f000000 default dlram start address)
		      0,  //  int cached = 0 (fixed cacheability mask)
		      1,  //  int mmu_en = 1 (mmu present)
		      3,  //  int itlb_num = 3 (8 itlbs)
		      3,  //  int dtlb_num = 3 (8 dtlbs)
		      0,  //  int tlb_type = 0 (split tlb mode)
		      1,  //  int tlb_rep = 1 (random replacement)
		      0,  //  int mmupgsz = 0 (4kB mmu page size)>
		      "mmu_cache",  // name of sysc module
		      CACHE_MASTER_ID,  // id of the AHB master
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
  //ahb_ct_mem<32> ahb_mem("AHB_MEM",0, 0x10000);
  ahb_slave<32> ahb_mem("AHB_MEM",0,0x10000);

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
  
 
