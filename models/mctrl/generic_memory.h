/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       Generic_memory.h                                        */
/*             memory module modeling all types of memory supported by */
/*             mctrl: RAM, std I/O, SRAM, SDRAM                        */
/*                                                                     */
/* Modified on $Date: 2010-06-09 10:30:16 +0200 (Wed, 09 Jun 2010) $   */
/*          at $Revision: 10 $                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef GENERIC_MEMORY_H
#define GENERIC_MEMORY_H

#include <boost/config.hpp>
#include <systemc.h>
#include <tlm.h>
#include <greenreg.h>
#include <greenreg_ambasocket.h>
#include "greencontrol/all.h"
#include "tlm_utils/simple_target_socket.h"

class Generic_memory
{
public:
    //Slave sockets: Answer communication with mctrl
    tlm_utils::simple_target_socket<Generic_memory> mem_rom;
    tlm_utils::simple_target_socket<Generic_memory> mem_stdio; //might need to support greenreg device handling
    tlm_utils::simple_target_socket<Generic_memory> mem_sram;
    tlm_utils::simple_target_socket<Generic_memory> mem_sdram;

    //constructor / destructor
    Generic_memory(sc_module_name name);
    ~Generic_memory(){}

    //processes for read / write / refresh only cause delay according to memory type
    //                                     (i.e. according to the called socket)

};


#endif
