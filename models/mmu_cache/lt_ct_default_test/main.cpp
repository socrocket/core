/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       main.cpp - Top level file (sc_main) for                 */
/*             lt_ct_default_test. Purpose is the assemblence of       */
/*             a test environment for the mmu_cache module in its      */
/*             default configuration. The mmu_cache is connected to    */
/*             a testbench (lt) and a cycle timed AHB bus.             */
/*                                                                     */
/*                                                                     */
/* Modified on $Date$                                                  */
/*          at $Revision$                                              */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/


#include "tlm.h"

#include "testbench.h"
#include "mmu_cache.h"
#include "defines.h"

#include "amba.h"
#include "ahb_simple_bus.h"
#include "AMBA_LT_CT_Adapter.h"
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

  // create mmu-cache
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
  mmu_cache<> mmu_cache("mmu_cache", 
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
  ahb_slave<32> ahb_mem("AHB_MEM",0,0x1000);

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
  
 
