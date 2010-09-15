/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       Generic_memory.h                                        */
/*             memory module modeling all types of memory supported by */
/*             mctrl: RAM, std I/O, SRAM, SDRAM                        */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
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

template <typename T = uint8_t>
class Generic_memory : public sc_core::sc_module
{
public:
    //Slave socket: communication with mctrl
    tlm_utils::simple_target_socket<Generic_memory> slave_socket;

    //constructor / destructor
    Generic_memory(sc_module_name name);
    ~Generic_memory();

    //memory instance (key: int address, value: int data)
    //ROM and SRAM words are modelled as 4 single bytes for easier management,
    //i.e. we need the template parameter T to be uint32_t (SDRAM, IO) or uint8_t (ROM, SRAM)
    std::map<uint32_t, T>  memory;

    //blocking transport functions
    void b_transport(tlm::tlm_generic_payload& gp, sc_time& delay);

    //read from memory
      //ROM, SRAM: byte addressable
      void read_8(uint32_t address, unsigned char* data_ptr, uint8_t length);
      //IO, SDRAM: word addressable
      void read_32(uint32_t address, uint32_t* data_ptr, uint8_t length);

    //write into memory
      //ROM, SRAM: byte addressable
      void write_8(uint32_t address, unsigned char* data, uint8_t length);
      //IO, SDRAM: word addressable
      void write_32(uint32_t address, uint32_t* data, uint8_t length);

    //erase sdram required for deep power down and PASR mode
    void erase_memory(uint32_t start_address, uint32_t end_address, unsigned int length);

};

#include "generic_memory.tpp"

#endif
