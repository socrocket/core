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
#include "ahb_ct_mem.h"

#include <ostream>


// **********************************************************
// * Testbed for mmu_cache development:


//                                 ---------------------
//                                 |  mmu_cache (lt)   |                       -------------   
// --------   TLM2 Generic Payl.   |  ---------------  |     AHB Payload       |           |
// |      |-----------|------------|  |             |  |-----------|-----------| ahb_lt_ct |
// |  tb  |m_initiator| icio target|  | ivector     |  | ahb_master|slave_sock | transactor|  ...
// | (lt) |socket (lt)| socket (lt)|  | cache (c++) |  | (lt)      |(lt)       | (lt -> ct)|  ->
// |      |-----------|------------|  |             |  |-----------|-----------|           |
// --------                        |  ---------------  |                       -------------
//                                 |                   |
//                                 ---------------------

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
  // args: name of sysc module, id of the AHB master, icache delay for read hit, icache delay for read miss
  mmu_cache<> mmu_cache("mmu_cache", CACHE_MASTER_ID, sc_core::sc_time(0, sc_core::SC_NS), sc_core::sc_time(LOCAL_CLOCK, sc_core::SC_NS));

  // create AHB bus
  ahb_simple_bus<32> ahb_bus("AHB_Bus");

  // create LT CT Adapter
  amba::AMBA_LT_CT_Adapter<32> ahb_lt_ct("AHB_LT_CT",amba::amba_AHB);

  // create AHB memory (1MB from address base 0)
  ahb_ct_mem<32> ahb_mem("AHB_MEM",0, 0x1000);

  // *** BIND SOCKETS

  // connect testbench (cpu) to mmu-cache
  tb.m_initiator_socket(mmu_cache.icio);

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
  
 
