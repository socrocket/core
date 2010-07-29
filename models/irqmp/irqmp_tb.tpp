/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       irqmp_tb.tpp                                            */
/*             stimulus file for the IRQMP module                      */
/*             can be used for systemc or vhdl simulation w/ modelsim  */
/*             included by testbench header file                       */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef IRQMP_TB_TPP
#define IRQMP_TB_TPP

#include "amba.h"
#include "irqmpreg.h"

//constructor
template<unsigned int BUSWIDTH, int pindex, int paddr, int pmask, int ncpu, int eirq> 
  irqmp_tb<BUSWIDTH, pindex, paddr, pmask, ncpu, eirq>::irqmp_tb(sc_core::sc_module_name nm)
  : APB_Testbench(nm), in("irqmp_in"), out("irqmp_out") {

  SC_THREAD(run);
  
  sender_config sconf;
  //conf.use_mandatory_extension<bar>();
  sconf.use_mandatory_extension<rst>();
  sconf.use_optional_extension<irq0>();
  sconf.use_optional_extension<irq1>();
  sconf.use_optional_extension<irq2>();
  sconf.use_optional_extension<irq3>();
  sconf.use_optional_extension<irq4>();
  sconf.use_optional_extension<irq5>();
  sconf.use_optional_extension<irq6>();
  sconf.use_optional_extension<irq7>();
  sconf.use_optional_extension<irq8>();
  sconf.use_optional_extension<irq9>();
  sconf.use_optional_extension<irq10>();
  sconf.use_optional_extension<irq11>();
  sconf.use_optional_extension<irq12>();
  sconf.use_optional_extension<irq13>();
  sconf.use_optional_extension<irq14>();
  sconf.use_optional_extension<irq15>();
  sconf.use_optional_extension<set_irq>();
  //sconf.use_optional_extension<clr_irq>();
  register_sender_socket(in, sconf);

  target_config tconf;
/*
  tconf.use_optional_extension<irq0>();
  tconf.use_optional_extension<irq1>();
  tconf.use_optional_extension<irq2>();
  tconf.use_optional_extension<irq3>();
  tconf.use_optional_extension<irq4>();
  tconf.use_optional_extension<irq5>();
  tconf.use_optional_extension<irq6>();
  tconf.use_optional_extension<irq7>();
  tconf.use_optional_extension<irq8>();
  tconf.use_optional_extension<irq9>();
  tconf.use_optional_extension<irq10>();
  tconf.use_optional_extension<irq11>();
  tconf.use_optional_extension<irq12>();
  tconf.use_optional_extension<irq13>();
  tconf.use_optional_extension<irq14>();
  tconf.use_optional_extension<irq15>();
  tconf.use_optional_extension<rst0>();
  tconf.use_optional_extension<rst1>();
  tconf.use_optional_extension<rst2>();
  tconf.use_optional_extension<rst3>();
  tconf.use_optional_extension<rst4>();
  tconf.use_optional_extension<rst5>();
  tconf.use_optional_extension<rst6>();
  tconf.use_optional_extension<rst7>();
  tconf.use_optional_extension<rst8>();
  tconf.use_optional_extension<rst9>();
  tconf.use_optional_extension<rst10>();
  tconf.use_optional_extension<rst11>();
  tconf.use_optional_extension<rst12>();
  tconf.use_optional_extension<rst13>();
  tconf.use_optional_extension<rst14>();
  tconf.use_optional_extension<rst15>();
*/
  register_target_socket(this, out, tconf);
 
}

//stimuli
template<unsigned int BUSWIDTH, int pindex, int paddr, int pmask, int ncpu, int eirq>
  void irqmp_tb<BUSWIDTH, pindex, paddr, pmask, ncpu, eirq>::run() {
    sc_core::sc_time t;
    //trivial one word write
    SIGNAL_SET(rst, 0);
    wait(30, sc_core::SC_NS);
    SIGNAL_SET(rst, 1);
    wait(10, sc_core::SC_NS);

    SET(IRQMP_PROC_IR_MASK(0),   0x0000000F);
    SET(IRQMP_PROC_IR_MASK(1),   0x0000003C);
/*    SET(IRQMP_PROC_IR_MASK(2),   0x000000F1);
    SET(IRQMP_PROC_IR_MASK(3),   0x000003C1);
    SET(IRQMP_PROC_IR_MASK(4),   0x00000F01);
    SET(IRQMP_PROC_IR_MASK(5),   0x00003C01);
    SET(IRQMP_PROC_IR_MASK(6),   0x0000F001);
    SET(IRQMP_PROC_IR_MASK(7),   0x0003C001);
    SET(IRQMP_PROC_IR_MASK(8),   0x000F0001);
    SET(IRQMP_PROC_IR_MASK(9),   0x003C0001);
    SET(IRQMP_PROC_IR_MASK(10),  0x00F00001);
    SET(IRQMP_PROC_IR_MASK(11),  0x03C00001);
    SET(IRQMP_PROC_IR_MASK(12),  0x0F000001);
    SET(IRQMP_PROC_IR_MASK(13),  0x3C000001);
    SET(IRQMP_PROC_IR_MASK(14),  0xF0000001);
    SET(IRQMP_PROC_IR_MASK(15),  0xFFFFFFFF);
*/
    SET(IRQMP_IR_LEVEL,  0x000000FF);
    SET(IRQMP_BROADCAST, 0x0000AAAA);

    cout << endl << "Printing is definitely no trouble at all.";

    wait(50, sc_core::SC_NS);
    for (int i_cpu=0; i_cpu<ncpu; i_cpu++) {
      SET(IRQMP_PROC_IR_FORCE(i_cpu), 0x0000A0A0);
    }
    wait(20, sc_core::SC_NS);
    sc_core::sc_stop();
}

#endif
