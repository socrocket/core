/***********************************************************************/
/* Project:    HW-SW SystenC Co-Simulation SoC Validation Platform     */
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

#include "amba.h"

template<unsigned int BUSWIDTH, int pindex = 0, int paddr = 0, int pmask = 0xFFF, int ncpu = 2, int eirq = 1>
class irqmp_tb : public sc_core::sc_module {
  public:
    //bus communication via socket
    amba::amba_master_socket<BUSWIDTH> master_sock;

    //Non-AMBA-Inputs of IRQMP
    sc_core::sc_out<bool>            rst;
    sc_core::sc_out<sc_uint<32> >    apbi_pirq;
    sc_core::sc_out<l3_irq_out_type> irqi[ncpu];

    SC_HAS_PROCESS(irqmp_tb);

    //constructor
    irqmp_tb(sc_core::sc_module_name nm);

    //define TLM write and read transactions
    void write(uint32_t addr, uint32_t data, uint32_t width);
    uint32_t read(uint32_t addr, uint32_t width);

    //stimuli (make use of TLM write / read transaction functions defined above)
    void run();
};

#include "irqmp_tb.tpp"

#endif
