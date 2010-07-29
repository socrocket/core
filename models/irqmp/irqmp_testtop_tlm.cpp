/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       irqmp_testtop_tlm.cpp                                   */
/*             test file for systemc implementation of irqmp           */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#include "amba.h"
#include "irqmp.h"
#include "irqmp_tb.h"
#include "irqmpsignals.h"

int sc_main(int argc, char** argv) {
  //set generics
  const int buswidth = 32;
  const int pindex = 0;
  const int paddr = 0;
  const int pmask = 0xFFF;
  const int ncpu = 2;
  const int eirq = 1;

  //instantiate testbench and irqmp
  irqmp_tb<buswidth, pindex, paddr, pmask, ncpu, eirq> irqmp_tb("irqmp_tb");
  Irqmp<pindex, paddr, pmask, ncpu, eirq> irqmp_inst0("irqmp");

  //bus communication via sockets (TLM)
  irqmp_tb.master_sock(irqmp_inst0.bus);

  //routing signal communication
  irqmp.out(irqmp_tb.out);
  irqmp_tb.out.set_source<irq0>(irqmp.out.name());
  irqmp_tb.out.set_source<irq1>(irqmp.out.name());
  irqmp_tb.out.set_source<irq2>(irqmp.out.name());
  irqmp_tb.out.set_source<irq3>(irqmp.out.name());
  irqmp_tb.out.set_source<irq4>(irqmp.out.name());
  irqmp_tb.out.set_source<irq5>(irqmp.out.name());
  irqmp_tb.out.set_source<irq6>(irqmp.out.name());
  irqmp_tb.out.set_source<irq7>(irqmp.out.name());
  irqmp_tb.out.set_source<irq8>(irqmp.out.name());
  irqmp_tb.out.set_source<irq9>(irqmp.out.name());
  irqmp_tb.out.set_source<irq10>(irqmp.out.name());
  irqmp_tb.out.set_source<irq11>(irqmp.out.name());
  irqmp_tb.out.set_source<irq12>(irqmp.out.name());
  irqmp_tb.out.set_source<irq13>(irqmp.out.name());
  irqmp_tb.out.set_source<irq14>(irqmp.out.name());
  irqmp_tb.out.set_source<irq15>(irqmp.out.name());
  irqmp_tb.out.set_source<rst0>(irqmp.out.name());
  irqmp_tb.out.set_source<rst1>(irqmp.out.name());
  irqmp_tb.out.set_source<rst2>(irqmp.out.name());
  irqmp_tb.out.set_source<rst3>(irqmp.out.name());
  irqmp_tb.out.set_source<rst4>(irqmp.out.name());
  irqmp_tb.out.set_source<rst5>(irqmp.out.name());
  irqmp_tb.out.set_source<rst6>(irqmp.out.name());
  irqmp_tb.out.set_source<rst7>(irqmp.out.name());
  irqmp_tb.out.set_source<rst8>(irqmp.out.name());
  irqmp_tb.out.set_source<rst9>(irqmp.out.name());
  irqmp_tb.out.set_source<rst10>(irqmp.out.name());
  irqmp_tb.out.set_source<rst11>(irqmp.out.name());
  irqmp_tb.out.set_source<rst12>(irqmp.out.name());
  irqmp_tb.out.set_source<rst13>(irqmp.out.name());
  irqmp_tb.out.set_source<rst14>(irqmp.out.name());
  irqmp_tb.out.set_source<rst15>(irqmp.out.name());

  irqmp_tb.in(irqmp.in);
  irqmp.in.set_source<rst>(irqmp_tb.in.name());
  irqmp.in.set_source<irq0>(irqmp_tb.in.name());
  irqmp.in.set_source<irq1>(irqmp_tb.in.name());
  irqmp.in.set_source<irq2>(irqmp_tb.in.name());
  irqmp.in.set_source<irq3>(irqmp_tb.in.name());
  irqmp.in.set_source<irq4>(irqmp_tb.in.name());
  irqmp.in.set_source<irq5>(irqmp_tb.in.name());
  irqmp.in.set_source<irq6>(irqmp_tb.in.name());
  irqmp.in.set_source<irq7>(irqmp_tb.in.name());
  irqmp.in.set_source<irq8>(irqmp_tb.in.name());
  irqmp.in.set_source<irq9>(irqmp_tb.in.name());
  irqmp.in.set_source<irq10>(irqmp_tb.in.name());
  irqmp.in.set_source<irq11>(irqmp_tb.in.name());
  irqmp.in.set_source<irq12>(irqmp_tb.in.name());
  irqmp.in.set_source<irq13>(irqmp_tb.in.name());
  irqmp.in.set_source<irq14>(irqmp_tb.in.name());
  irqmp.in.set_source<irq15>(irqmp_tb.in.name());
  irqmp.in.set_source<set_irq>(irqmp_tb.in.name());
  irqmp.in.set_source<clr_irq>(irqmp_tb.in.name());

  //start simulation
  sc_core::sc_start();
  return 0;
}




