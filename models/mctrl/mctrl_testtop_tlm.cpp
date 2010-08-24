/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mctrl_testtop_tlm.cpp                                   */
/*             test file for systemc implementation of mctrl + memory  */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
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
  const int hindex = 0;
  const int pindex = 0;
  const int romaddr = 0;
  const int rommask = 3584;
  const int ioaddr = 512;
  const int iomask = 3584;
  const int ramaddr = 1024;
  const int rammask = 3072;
  const int paddr = 0;
  const int pmask = 4095;
  const int wprot = 0;
  const int invclk = 0;
  const int fast = 0;
  const int romasel = 28;
  const int sdrasel = 29;
  const int srbanks = 4;
  const int ram8 = 0;
  const int ram16 = 0;
  const int sden = 0;
  const int sepbus = 0;
  const int sdbits = 32;
  const int sdlsb = 2;
  const int oepol = 0;
  const int syncrst = 0;
  const int pageburst = 0;
  const int scantest = 0;
  const int mobile = 0;

  //instantiate mctrl, generic memory, and testbench
  Mctrl mctrl_inst0("mctrl_inst0", hindex,    pindex,   romaddr, rommask,   ioaddr,   iomask,
                                   ramaddr,   rammask,  paddr,   pmask,     wprot,    invclk,
                                   fast,      romasel,  sdrasel, srbanks,   ram8,     ram16,
                                   sden,      sepbus,   sdbits,  sdlsb,     oepol,    syncrst,
                                   pageburst, scantest, mobile);
  Generic_memory POST_GENERIC_MEMORY(uint8_t ) generic_memory_rom("generic_memory_rom",
                                hindex,    pindex,   romaddr, rommask,   ioaddr,   iomask,
                                ramaddr,   rammask,  paddr,   pmask,     wprot,    invclk,
                                fast,      romasel,  sdrasel, srbanks,   ram8,     ram16,
                                sden,      sepbus,   sdbits,  sdlsb,     oepol,    syncrst,
                                pageburst, scantest, mobile);
  Generic_memory POST_GENERIC_MEMORY(uint32_t) generic_memory_io("generic_memory_io",
                                hindex,    pindex,   romaddr, rommask,   ioaddr,   iomask,
                                ramaddr,   rammask,  paddr,   pmask,     wprot,    invclk,
                                fast,      romasel,  sdrasel, srbanks,   ram8,     ram16,
                                sden,      sepbus,   sdbits,  sdlsb,     oepol,    syncrst,
                                pageburst, scantest, mobile);
  Generic_memory POST_GENERIC_MEMORY(uint8_t ) generic_memory_sram("generic_memory_sram",
                                hindex,    pindex,   romaddr, rommask,   ioaddr,   iomask,
                                ramaddr,   rammask,  paddr,   pmask,     wprot,    invclk,
                                fast,      romasel,  sdrasel, srbanks,   ram8,     ram16,
                                sden,      sepbus,   sdbits,  sdlsb,     oepol,    syncrst,
                                pageburst, scantest, mobile);
  Generic_memory POST_GENERIC_MEMORY(uint32_t) generic_memory_sdram("generic_memory_sdram",
                                hindex,    pindex,   romaddr, rommask,   ioaddr,   iomask,
                                ramaddr,   rammask,  paddr,   pmask,     wprot,    invclk,
                                fast,      romasel,  sdrasel, srbanks,   ram8,     ram16,
                                sden,      sepbus,   sdbits,  sdlsb,     oepol,    syncrst,
                                pageburst, scantest, mobile);
  Mctrl_tb mctrl_tb("mctrl_tb", hindex,    pindex,   romaddr, rommask,   ioaddr,   iomask,
                                ramaddr,   rammask,  paddr,   pmask,     wprot,    invclk,
                                fast,      romasel,  sdrasel, srbanks,   ram8,     ram16,
                                sden,      sepbus,   sdbits,  sdlsb,     oepol,    syncrst,
                                pageburst, scantest, mobile);

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




