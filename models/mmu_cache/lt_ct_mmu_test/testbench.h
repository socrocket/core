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
#include "locals.h"
#include "icio_payload_extension.h"
#include "dcio_payload_extension.h"

class testbench : public sc_core::sc_module {

 public:

  // Constructor
  testbench(sc_core::sc_module_name name);

  // member functions
  // ----------------
  // the main testbench thread
  void initiator_thread(void);

  // issues a data write transaction
  void dwrite(unsigned int addr, unsigned int data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock, unsigned int * debug);
  // issues a data read transaction
  unsigned int dread(unsigned int addr, unsigned int width, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock, unsigned int * debug);
  // issues an instruction read transaction
  unsigned int iread(unsigned int addr, unsigned int width, unsigned int flush, unsigned int flushl, unsigned int fline, unsigned int * debug);
  
 public:
  // variables and object declaration

  // TLM2.0 initiator sockets for instructions and data
  tlm_utils::simple_initiator_socket<testbench> instruction_initiator_socket; 
  tlm_utils::simple_initiator_socket<testbench> data_initiator_socket;

 private:
  tlm::tlm_response_status gp_status;

};

#endif // __TESTBENCH_H__

