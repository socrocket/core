/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mctrl.h                                                 */
/*             header file defining the mctrl module template          */
/*             includes implementation file mctrl.tpp at the bottom    */
/*                                                                     */
/* Modified on $Date: 2010-06-09 10:30:16 +0200 (Wed, 09 Jun 2010) $   */
/*          at $Revision: 10 $                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef MCTRL_H
#define MCTRL_H

//#define MONOLITHIC_MODULE 0

#include <boost/config.hpp>
#include <systemc.h>
#include <tlm.h>
#include <greenreg.h>
#include <greenreg_ambasocket.h>
#include "greencontrol/all.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "mctrlreg.h"
#include "generic_memory.h"

template <int hindex = 0,    int pindex = 0,    int romaddr = 0,    int rommask = 3584,
          int ioaddr = 512,  int iomask = 3584, int ramaddr = 1024, int rammask = 3072,
          int paddr = 0,     int pmask = 4095,  int wprot = 0,      int invclk = 0,
          int fast = 0,      int romasel = 28,  int sdrasel = 29,   int srbanks = 4,
          int ram8 = 0,      int ram16 = 0,     int sden = 0,       int sepbus = 0,
          int sdbits = 32,   int sdlsb = 2,     int oepol = 0,      int syncrst = 0,
          int pageburst = 0, int scantest = 0,  int mobile = 0>
class Mctrl : public gs::reg::gr_device
{
public:
    //Slave socket with delayed switch; Receives instructions from CPU
    gs::reg::greenreg_socket< gs::amba::amba_slave<32> > bus;

    //Master sockets: Initiate communication with memory modules
    tlm_utils::simple_initiator_socket<Mctrl> mctrl_rom;
    tlm_utils::simple_initiator_socket<Mctrl> mctrl_stdio;
    tlm_utils::simple_initiator_socket<Mctrl> mctrl_sram;
    tlm_utils::simple_initiator_socket<Mctrl> mctrl_sdram;

#ifdef MONOLITHIC_MODULE
    //instantiate memory
    Generic_memory ("mem");
#endif

    //constructor / destructor
    Mctrl(sc_module_name name);
    ~Mctrl();

    //callback function for register read access

    //processes triggered by slave socket need to implement state machine
                 //read ROM
                 //write ROM
                 //read StdIO
                 //write StdIO
                 //read SRAM
                 //write SRAM
                 //read SDRAM
                 //write SDRAM

    //process to be launched regularly
                 //refresh SDRAM

};

#include "mctrl.tpp"

#endif

