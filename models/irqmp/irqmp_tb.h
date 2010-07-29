/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       irqmp_tb_tlm.h                                          */
/*             header file defining the irqmp_tb template              */
/*             can be used for systemc or vhdl simulation w/ modelsim  */
/*             includes irqmp_tb.tpp at the bottom                     */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TU Braunschweig              */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef IRQMP_TB_H
#define IRQMP_TB_H

#include "apbtestbench.h"
#include "multisignalhandler.h"

template<unsigned int BUSWIDTH, int pindex = 0, int paddr = 0, int pmask = 0xFFF, int ncpu = 2, int eirq = 1>
class irqmp_tb : public APBTestbench, MultiSignalSender, MultiSignalTarget<irqmp_tb> {
  public:
    //Non-AMBA-Inputs of IRQMP
    gs_generic_signal::target_signal_multi_socket<
              irqmp_tb<BUSWIDTH, pindex, paddr, pmask, ncpu, eirq> >  out;
    gs_generic_signal::initiator_signal_multi_socket                  in;

    SC_HAS_PROCESS(irqmp_tb);

    //constructor
    irqmp_tb(sc_core::sc_module_name nm);

    //stimuli (make use of TLM write / read transaction functions defined above)
    void run();
};

#include "irqmp_tb.tpp"

#endif
