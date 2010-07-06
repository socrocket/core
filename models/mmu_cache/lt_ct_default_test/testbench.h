/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       testbench.h - Class definition of the                   */
/*             stimuli generator/monitor for the current testbench.    */
/*                                                                     */
/* Modified on $Date$                                                  */
/*          at $Revision$                                              */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#ifndef __TESTBENCH_H__
#define __TESTBENCH_H__

#include <tlm.h>
#include <ostream>
#include <tlm_utils/simple_initiator_socket.h>

#include "defines.h"

class testbench : public sc_core::sc_module {

 public:

  // Constructor
  testbench(sc_core::sc_module_name name);

  // member functions
  void initiator_thread(void);
  inline void write(unsigned int addr, unsigned int data, unsigned int length);
  inline unsigned int read(unsigned int addr, unsigned int width);
  
 public:
  // variables and object declaration
  typedef tlm::tlm_generic_payload *gp_ptr;
  tlm_utils::simple_initiator_socket<testbench> m_initiator_socket; 

 private:
  tlm::tlm_response_status gp_status;

};

#endif // __TESTBENCH_H__

