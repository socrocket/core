/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       apbtestbench.h                                          */
/*             header file defining the a generic APB testbench        */
/*             template to use for systemc or vhdl simulation          */
/*             all implementations are included for                    */
/*             maximum inline optiisatzion                             */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TU Braunschweig              */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef APB_TESTBENCH_H
#define APB_TESTBENCH_H

#include "amba.h"

//macro to brint a time stamp
#define SHOW { \
  std::cout << std::endl \
            << "@" \
            << sc_core::sc_time_stamp().to_string().c_str() \
            << " /" \
            << std::dec \
            << (unsigned)sc_core::sc_delta_count(); \
}

//macro to print a register value
#define REG(name) { \
  std::cout << " "#name \
            << ": 0x" \
            << std::hex \
            << std::setfill('0') \
            << std::setw(2) \
            << read(name, 4); \
}

//macro to set a register
#define SET(name, val) { \
  write(name, val, 4); \
}

// Testbench for GPTimer Models.
class APBTestbench : public sc_core::sc_module {
  public:
    //tlm interface
    amba::amba_master_socket<32>   master_sock;

    //constructor
    APBTestbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm)
    , master_sock("socket", amba::amba_APB, amba::amba_LT, false) {}
    
   //define TLM write transactions 
    inline void write(uint32_t addr, uint32_t data, uint32_t width) {
      sc_core::sc_time t;
      tlm::tlm_generic_payload *gp = master_sock.get_transaction();
      gp->set_command(tlm::TLM_WRITE_COMMAND);
      gp->set_address(addr);
      gp->set_data_length(width);
      gp->set_streaming_width(4);
      gp->set_byte_enable_ptr(NULL);
      gp->set_data_ptr((unsigned char*)&data);
      master_sock->b_transport(*gp,t);
      wait(t); 
      master_sock.release_transaction(gp);
    }
    
    //define TLM read transactions
    inline uint32_t read(uint32_t addr, uint32_t width) {
      sc_core::sc_time t;
      uint32_t data;
      tlm::tlm_generic_payload *gp = master_sock.get_transaction();
      gp->set_command(tlm::TLM_READ_COMMAND);
      gp->set_address(addr);
      gp->set_data_length(width);
      gp->set_streaming_width(4);
      gp->set_byte_enable_ptr(NULL);
      gp->set_data_ptr((unsigned char*)&data);
      gp->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
      master_sock->b_transport(*gp,t);
      wait(t); 
      master_sock.release_transaction(gp);
      return data;
    }
};

#endif // APB_TESTBENCH_H
