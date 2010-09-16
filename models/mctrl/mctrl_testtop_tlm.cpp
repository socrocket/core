/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mctrl_testtop_tlm.cpp                                   */
/*             test file for systemc implementation of mctrl + memory  */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                        */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#include "amba.h"
#include "generic_memory.h"
#include "mctrl.h"
#include "mctrlreg.h"
#include "mctrl_tb.h"

int sc_main(int argc, char** argv) {
  //set generics
  const int romasel = 28;
  const int sdrasel = 29;
  const int romaddr = 0;
  const int rommask = 0xE00;
  const int ioaddr = 0x200;
  const int iomask = 0xE00;
  const int ramaddr = 0x400;
  const int rammask = 0xC00;
  const int paddr = 0x0;
  const int pmask = 0xFFF;
  const int wprot = 0;
  const int srbanks = 4;
  const int ram8 = 0;
  const int ram16 = 0;
  const int sepbus = 0;
  const int sdbits = 32;
  const int mobile = 0;
  const int sden = 0;

  //instantiate mctrl, generic memory, and testbench
  Mctrl mctrl_inst0("mctrl_inst0", romasel, sdrasel, romaddr, rommask, ioaddr, iomask,
                                   ramaddr, rammask, paddr,   pmask,   wprot,  srbanks,
                                   ram8,    ram16,   sepbus,  sdbits,  mobile, sden);
  Generic_memory <uint8_t>  generic_memory_rom("generic_memory_rom");
  Generic_memory <uint32_t> generic_memory_io("generic_memory_io");
  Generic_memory <uint8_t>  generic_memory_sram("generic_memory_sram");
  Generic_memory <uint32_t> generic_memory_sdram("generic_memory_sdram");
  Mctrl_tb mctrl_tb("mctrl_tb", romasel, sdrasel, romaddr, rommask, ioaddr, iomask,
                                ramaddr, rammask, paddr,   pmask,   wprot,  srbanks,
                                ram8,    ram16,   sepbus,  sdbits,  mobile, sden);

  //bus communication via amba sockets (TLM)
  mctrl_tb.apb_master_sock(mctrl_inst0.apb);  //config registers
  mctrl_tb.ahb_master_sock(mctrl_inst0.ahb);  //memory access

  //memory communication via simple TLM sockets
  mctrl_inst0.mctrl_rom(generic_memory_rom.slave_socket);
  mctrl_inst0.mctrl_io(generic_memory_io.slave_socket);
  mctrl_inst0.mctrl_sram(generic_memory_sram.slave_socket);
  mctrl_inst0.mctrl_sdram(generic_memory_sdram.slave_socket);

  sc_core::sc_start();
  return 0;
}




