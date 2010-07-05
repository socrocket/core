/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       generic_memory.cpp                                      */
/*             implementation of the generic_memory module             */
/*                                                                     */
/* Modified on $Date: 2010-06-09 10:30:16 +0200 (Wed, 09 Jun 2010) $   */
/*          at $Revision: 10 $                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#include "generic_memory.h"

//constructor
Generic_memory::Generic_memory(sc_core::sc_module_name name) :
       // construct and name sockets
       mem_rom("mem_rom"),
       mem_stdio("mem_stdio"),
       mem_sram("mem_sram"),
       mem_sdram("mem_sdram") {

  //process registration
}

