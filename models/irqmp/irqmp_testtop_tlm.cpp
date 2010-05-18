/***********************************************************************/
/* Project:    Hardware-Software Co-Simulation SoC Validation Platform */
/*                                                                     */
/* File:       irqmp_testtop_tlm.cpp                                   */
/*             test file for systemc implementation of irqmp           */
/*                                                                     */
/* Date:       18.05.2010                                              */
/* Revision:   1                                                       */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#include "amba.h"
#include "irqmp.h"
#include "irqmpreg.h"
#include "irqmp_tb.h"

int sc_main(int argc, char** argv) {
  //set generics
  const int buswidth = 32;
  const int pindex = 0;
  const int paddr = 0;
  const int pmask = 0xFFF;
  const int ncpu = 2;
  const int eirq = 1;

  //irqmp signals
  sc_core::sc_signal<bool>                rst("rst");
  sc_core::sc_signal<l3_irq_out_type>     irqi[ncpu];
  sc_core::sc_signal<l3_irq_in_type>      irqo[ncpu];
  sc_core::sc_signal<sc_dt::sc_uint<32> > apbi_pirq("apbi_pirq"),
                                          apbo_pconfig_0("apbo_pconfig_0"),
                                          apbo_pconfig_1("apbo_pconfig_1");
  sc_core::sc_signal<sc_dt::sc_uint<16> > apbo_pindex("pindex");

  //instantiate testbench and irqmp
  irqmp_tb<buswidth, pindex, paddr, pmask, ncpu, eirq> irqmp_tb("irqmp_tb");
  Irqmp<pindex, paddr, pmask, ncpu, eirq> irqmp_inst0("irqmp");

  //bus communication via sockets (TLM)
  irqmp_tb.master_sock(irqmp_inst0.bus);

  //direct connection of all other signals
  irqmp_inst0.rst(rst);
  irqmp_inst0.apbi_pirq(apbi_pirq);
  irqmp_inst0.apbo_pindex(apbo_pindex);
  irqmp_inst0.apbo_pconfig_0(apbo_pconfig_0);
  irqmp_inst0.apbo_pconfig_1(apbo_pconfig_1);
  for (int i_cpu=0; i_cpu<ncpu; i_cpu++) {
    irqmp_inst0.irqi[i_cpu](irqi[i_cpu]);
    irqmp_inst0.irqo[i_cpu](irqo[i_cpu]);
  }
  sc_core::sc_start();
  return 0;
}




