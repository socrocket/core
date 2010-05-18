/***********************************************************************/
/* Project:    Hardware-Software Co-Simulation SoC Validation Platform */
/*                                                                     */
/* File:       irqmp_testtop_ct.cpp                                    */
/*             test file for vhdl implementation of irqmp              */
/*             needs sc_wrapper around the vhdl module for simulation  */
/*                                                                     */
/* Date:       18.05.2010                                              */
/* Revision:   1                                                       */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#include <vector>
using std::vector;

#include "amba.h"
#include "./adapters/AMBA_LT_CT_Adapter.h"
#include "ct_rtl.h"
#include "irqmp_tb.h"
#include "irqmp_sc_wrapper.h"
#include "irqmpreg.h"
#include <cstring>


int sc_main(int argc, char** argv) {
  //set generics
  const int buswidth = 32;
  const int pindex = 0;
  const int paddr = 0;
  const int pmask = 0xFFF;
  const int ncpu = 2;
  const int eirq = 1;

  //irqmp signals
  sc_core::sc_clock clk("clock",sc_core::sc_time(10,sc_core::SC_NS));
  sc_core::sc_signal<bool>                      rst("rst");
  sc_core::sc_signal<apb_slv_in_type>           apbi("apbi");
  sc_core::sc_signal<apb_slv_out_type>          apbo("apbo");
//  sc_core::sc_signal<l3_irq_out_type>           irqi[ncpu];
//  sc_core::sc_signal<l3_irq_in_type>            irqo[ncpu];

  sc_core::sc_signal<l3_irq_out_type>           irqi_0("irqi_0");
  sc_core::sc_signal<l3_irq_out_type>           irqi_1("irqi_1");
  sc_core::sc_signal<l3_irq_in_type>            irqo_0("irqo_0");
  sc_core::sc_signal<l3_irq_in_type>            irqo_1("irqo_1");

  //extracted signals for testing convenience
  sc_core::sc_signal<sc_dt::sc_uint<32> > apbi_pirq("apbi_pirq"),
                                          apbo_pirq("apbo_pirq"),
                                          apbo_pconfig_0("apbo_pconfig_0"),
                                          apbo_pconfig_1("apbo_pconfig_1");
  sc_core::sc_signal<sc_dt::sc_uint<16> > apbo_pindex("pindex");

  //component instantiation
  irqmp_tb<buswidth, pindex, paddr, pmask, ncpu, eirq> irqmp_tb("irqmp_tb"); //testbench
  amba::AMBA_LT_CT_Adapter<32> lt_ct("LT_CT", amba::amba_APB);               //LT-CT adapter
  ct_rtl ct_rtl("CT_RTL", 0x0, 0x40);                                        //CT-RTL adapter
  irqmp<> irqmp_inst0("irqmp", "work.irqmp");                                //IRQMP

  /*  communication between testbench, adapters, and IRQMP instance:
   *  testbench      --> TLM (socket communication)
   *  LT-CT adapter  --> TLM (socket communication)
   *  CT-RTL adapter --> CT side:  socket communication
   *                     RTL side: sc_signal communication
   */

  // 1) testbench to LT-CT, no clock required (LT)
  irqmp_tb.master_sock(lt_ct.slave_sock);
  irqmp_tb.rst(rst);
  irqmp_tb.apbi_pirq(apbi_pirq);
  irqmp_tb.irqi[0](irqi_0);
  irqmp_tb.irqi[1](irqi_1);

//In a perfect world the following would be possible. Somehow.
/*
  for (int i_cpu=0; i_cpu<ncpu; i_cpu++) {
    irqi[i_cpu].mti_set_typename("sc_core::sc_signal<l3_irq_out_type>[2]");
    irqo[i_cpu].mti_set_typename("sc_core::sc_signal<l3_irq_in_type>[2]");
    irqmp_tb.irqi[i_cpu](irqi[i_cpu]);
  }
*/

  // 2) LT-CT to CT-RTL, clock introduced on CT side
  lt_ct.master_sock(ct_rtl.m_rtl.slave_sock);
  lt_ct.clk(clk);

  // 3) CT-RTL to sc_main signals. All bus signals required in RTL description.
  ct_rtl.reset(rst);
  ct_rtl.apbi(apbi);
  ct_rtl.apbo(apbo);
  ct_rtl.apbi_pirq(apbi_pirq);
  ct_rtl.apbo_pirq(apbo_pirq);
  ct_rtl.apbo_pindex(apbo_pindex);
  ct_rtl.apbo_pconfig_0(apbo_pconfig_0);
  ct_rtl.apbo_pconfig_1(apbo_pconfig_1);
  ct_rtl.clk(clk);

  // 4) sc_main signals (from CT-RTL) to IRQMP instance
  //    All signals required, but some are now encapsulated in apbi + apbo
  irqmp_inst0.rst(rst);
  irqmp_inst0.clk(clk);
  irqmp_inst0.apbi(apbi);
  irqmp_inst0.apbo(apbo);
  irqmp_inst0.irqi[0](irqi_0);
  irqmp_inst0.irqi[1](irqi_1);
  irqmp_inst0.irqo[0](irqo_0);
  irqmp_inst0.irqo[1](irqo_1);

/*
  for (int i_cpu=0; i_cpu<ncpu; i_cpu++) {
    irqmp_inst0.irqi[i_cpu](irqi[i_cpu]);
    irqmp_inst0.irqo[i_cpu](irqo[i_cpu]);
  }
*/

  sc_core::sc_start();
  return 0;
}
