/***********************************************************************/
/* Project:    Hardware-Software Co-Simulation SoC Validation Platform */
/*                                                                     */
/* File:       irqmp_tb.tpp                                            */
/*             stimulus file for the IRQMP module                      */
/*             can be used for systemc or vhdl simulation w/ modelsim  */
/*             included by testbench header file                       */
/*                                                                     */
/* Date:       18.05.2010                                              */
/* Revision:   1                                                       */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef IRQMP_TB_TPP
#define IRQMP_TB_TPP

#include "amba.h"
#include "irqmpreg.h"

#define SHOW \
{ std::printf("\n@%-7s /%-4d:", sc_core::sc_time_stamp().to_string().c_str(), (unsigned)sc_core::sc_delta_count());}
#define REG(name) \
{ std::cout << " "#name << ": 0x" << std::hex << std::setfill('0') << std::setw(2) << read(name, 4); }
#define SET(name, val) \
{ write(name, val, 4); }


//constructor
template<unsigned int BUSWIDTH, int pindex, int paddr, int pmask, int ncpu, int eirq> 
  irqmp_tb<BUSWIDTH, pindex, paddr, pmask, ncpu, eirq>::irqmp_tb(sc_core::sc_module_name nm)
  : master_sock ("msock", amba::amba_APB, amba::amba_LT, false) {

  SC_THREAD(run);
}


//TLM write transaction
template<unsigned int BUSWIDTH, int pindex, int paddr, int pmask, int ncpu, int eirq> 
  void irqmp_tb<BUSWIDTH, pindex, paddr, pmask, ncpu, eirq>::write(uint32_t addr, uint32_t data, uint32_t width) {
    sc_core::sc_time t;
    tlm::tlm_generic_payload gp;
      gp.set_command(tlm::TLM_WRITE_COMMAND);
      gp.set_address(addr);
      gp.set_data_length(width);
      gp.set_streaming_width(4);
      gp.set_byte_enable_ptr(NULL);
      gp.set_data_ptr((unsigned char*)&data);
    master_sock->b_transport(gp,t);
    SHOW;
    std::cout << " WRITE " << gp.get_response_string() << ": 0x" << std::hex << std::setfill('0') << std::setw(2) << gp.get_address();
    wait(t);
}

//TLM read transaction
template<unsigned int BUSWIDTH, int pindex, int paddr, int pmask, int ncpu, int eirq>
  uint32_t irqmp_tb<BUSWIDTH, pindex, paddr, pmask, ncpu, eirq>::read(uint32_t addr, uint32_t width) {
    sc_core::sc_time t;
    uint32_t data;
    tlm::tlm_generic_payload gp;
      gp.set_command(tlm::TLM_READ_COMMAND);
      gp.set_address(addr);
      gp.set_data_length(width);
      gp.set_streaming_width(4);
      gp.set_byte_enable_ptr(NULL);
      gp.set_data_ptr((unsigned char*)&data);
      gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    master_sock->b_transport(gp,t);
    SHOW;
    std::cout << " READ " << gp.get_response_string() << ": 0x" << std::hex << std::setfill('0') << std::setw(2) << gp.get_address();
    wait(t);
    return data;
}

//stimuli
template<unsigned int BUSWIDTH, int pindex, int paddr, int pmask, int ncpu, int eirq>
  void irqmp_tb<BUSWIDTH, pindex, paddr, pmask, ncpu, eirq>::run() {
    sc_core::sc_time t;
    //trivial one word write
    rst = 0;
    wait(30, sc_core::SC_NS);
    rst = 1;
    wait(20, sc_core::SC_NS);

    SET(IRQMP_PROC_IR_MASK(0),   0x0000000F);
    SET(IRQMP_PROC_IR_MASK(1),   0x0000003C);
    SET(IRQMP_PROC_IR_MASK(2),   0x000000F1);
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

    SET(IRQMP_IR_LEVEL,  0x000000FF);
    SET(IRQMP_BROADCAST, 0x0000AAAA);

    wait(10, sc_core::SC_NS);
    sc_core::sc_stop();
}

#endif
