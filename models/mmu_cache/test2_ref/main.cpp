// ***********************************************************************/
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
// *                                                                     */
// * File:       main.cpp - Top level file (sc_main) for                 */
// *             lt_ct_cacheline_test. This test focusses on the         */
// *             replacement and invalidation of cacheline elements.     */
// *	         MMU and localrams are disabled. The caches have two     */
// *	         sets and a size of 64kb each. The size of the           */
// *             cachelines is set to the maximum (32 bytes).            */        
// *                                                                     */
// *                                                                     */
// * Modified on $Date: 2010-09-17 12:37:30 +0200 (Fri, 17 Sep 2010) $   */
// *          at $Revision: 123 $                                        */
// *                                                                     */
// * Principal:  European Space Agency                                   */
// * Author:     VLSI working group @ IDA @ TUBS                         */
// * Maintainer: Thomas Schuster                                         */
// ***********************************************************************/

#include "defines.h"
#include "locals.h"

#include "tlm.h"
#include "amba.h"

#include "testbench.h"
#include "mmu_cache_wrapper.h"

#include "cpu_lt_rtl_adapter.h"

#include "ahb_rtl_ct.h"
#include "ahb_simple_slave.h"


int sc_main(int argc, char** argv) {

  // *** CREATE MODULES

  // create testbench
  testbench tb("tb");

  // create cache adapter
  cpu_lt_rtl_adapter cpu_lt_rtl("cpu_lt_rtl");

  // CREATE MMU Cache
  // ----------------
  
  mmu_cache_wrapper mmu_cache("mmu_cache","work.mmu_cache_wrapper");  

  // create rtl_ct adapter
  CAHB_RTL_CT ahb_rtl_ct("ahb_rtl_ct",0, 100000); 

  // create simulation memory
  ahb_slave < 32 >  ahb_mem("ahb_mem", 0, 0x200000, true);


  // *** BIND SOCKETS

  // connect testbench to cpu_lt_rtl_adapter
  tb.instruction_initiator_socket(cpu_lt_rtl.icio);
  tb.data_initiator_socket(cpu_lt_rtl.dcio);

  // connect cpu_lt_rtl_adapter to dut (mmu_cache)
  sc_signal<icache_in_type> i_ici("i_ici");
  cpu_lt_rtl.ici(i_ici);
  mmu_cache.ici(i_ici);

  sc_signal<icache_out_type> i_ico("i_ico");
  cpu_lt_rtl.ico(i_ico);
  mmu_cache.ico(i_ico);

  sc_signal<dcache_in_type> i_dci("i_dci");
  cpu_lt_rtl.dci(i_dci);
  mmu_cache.dci(i_dci);

  sc_signal<dcache_out_type> i_dco("i_dco");
  cpu_lt_rtl.dco(i_dco);
  mmu_cache.dco(i_dco);

  // connect dut (mmu_cache) to rtl_ct adapter
  sc_signal<ahb_mst_in_type> i_ahbi("i_ahbi");
  mmu_cache.ahbi(i_ahbi);
  ahb_rtl_ct.ahbi(i_ahbi);
  
  sc_signal<ahb_mst_out_type_adapter> i_ahbo("i_ahbo");
  mmu_cache.ahbo(i_ahbo);
  ahb_rtl_ct.ahbo(i_ahbo);

  // connect rtl_ct adapter to simulation memory
  ahb_rtl_ct.ct.master_sock(ahb_mem.slave_socket);

  // wire up dummies
  sc_signal<sc_logic> tie_low;
  sc_signal<sc_logic> tie_high;

  tie_low.write(SC_LOGIC_0);
  tie_high.write(SC_LOGIC_1);

  mmu_cache.fpuholdn(tie_high);
  mmu_cache.hclken(tie_high);

  sc_signal<sc_uint<32> > i_hirqi;
  sc_signal<sc_uint<32> > i_hirqo;
  sc_signal<sc_uint<32> > i_hconfig_0;
  sc_signal<sc_uint<32> > i_hconfig_1;
  sc_signal<sc_uint<16> > i_hindex;

  i_hirqi.write(0);

  ahb_rtl_ct.hirqi(i_hirqi);
  ahb_rtl_ct.hirqo(i_hirqo);
  ahb_rtl_ct.hconfig_0(i_hconfig_0);
  ahb_rtl_ct.hconfig_1(i_hconfig_1);
  ahb_rtl_ct.hindex(i_hindex);

  // wire up clock
  sc_signal<sc_logic> i_clk;
  tb.signal_clk(i_clk);

  ahb_rtl_ct.clk(tb.clock);

  cpu_lt_rtl.clk(tb.clock);

  ahb_mem.clk(tb.clock);

  mmu_cache.clk(i_clk);
  mmu_cache.sclk(i_clk);
  mmu_cache.hclk(i_clk);

  // wire up reset
  sc_signal<bool> i_rst;
  sc_signal<sc_logic> i_rst_scl;

  tb.rst(i_rst);
  tb.rst_scl(i_rst_scl);

  cpu_lt_rtl.rst(i_rst);
  ahb_rtl_ct.reset(i_rst);
  mmu_cache.rst(i_rst_scl);
  
  // start simulation
  sc_core::sc_start();

  return 0;

}
  
 
