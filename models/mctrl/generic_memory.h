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

#include <map>
#include <boost/config.hpp>
#include <systemc.h>
#include <tlm.h>
#include <greenreg.h>
#include <greenreg_ambasocket.h>
#include "greencontrol/all.h"
#include "tlm_utils/simple_target_socket.h"

template <int hindex = 0,    int pindex = 0,    int romaddr = 0,    int rommask = 3584,
          int ioaddr = 512,  int iomask = 3584, int ramaddr = 1024, int rammask = 3072,
          int paddr = 0,     int pmask = 4095,  int wprot = 0,      int invclk = 0,
          int fast = 0,      int romasel = 28,  int sdrasel = 29,   int srbanks = 4,
          int ram8 = 0,      int ram16 = 0,     int sden = 0,       int sepbus = 0,
          int sdbits = 32,   int sdlsb = 2,     int oepol = 0,      int syncrst = 0,
          int pageburst = 0, int scantest = 0,  int mobile = 0>
class Generic_memory
{
public:
    //Slave sockets: Answer communication with mctrl
    tlm_utils::simple_target_socket<Generic_memory> mem_rom;
    tlm_utils::simple_target_socket<Generic_memory> mem_io; //might need to support greenreg device handling
    tlm_utils::simple_target_socket<Generic_memory> mem_sram;
    tlm_utils::simple_target_socket<Generic_memory> mem_sdram;

    //constructor / destructor
    Generic_memory(sc_module_name name);
    ~Generic_memory();

    //memory instances (key: int address, value: int data)
    std::map<uint32_t, uint32_t> rom;
    std::map<uint32_t, uint32_t> io;
    std::map<uint32_t, uint32_t> sram;
    std::map<uint32_t, uint32_t> sdram;

    //blocking transport functions
    void b_transport_rom(tlm::tlm_generic_payload& gp, sc_time& delay);
    void b_transport_io(tlm::tlm_generic_payload& gp, sc_time& delay);
    void b_transport_sram(tlm::tlm_generic_payload& gp, sc_time& delay);
    void b_transport_sdram(tlm::tlm_generic_payload& gp, sc_time& delay);

    //functions for read / write / refresh
    //ROM
    uint32_t read_rom(uint32_t address, uint8_t width);
    //IO
    void write_io(uint32_t address, uint32_t data);
    uint32_t read_io(uint32_t address);
    //SRAM
    void write_sram(uint32_t address, uint32_t data, uint8_t width);
    uint32_t read_sram(uint32_t address, uint8_t width);
    //SDRAM, overloaded write function for 32 vs 64 bit bus
    void write_sdram(uint32_t address, uint32_t data);
    void write_sdram(uint32_t address, uint64_t data);
    uint32_t read_sdram32(uint32_t address);
    uint64_t read_sdram64(uint32_t address);

};

#include "generic_memory.tpp"

#endif
