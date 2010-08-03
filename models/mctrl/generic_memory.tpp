/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       generic_memory.tpp                                      */
/*             implementation of the generic_memory module             */
/*                                                                     */
/* Modified on $Date: 2010-06-09 10:30:16 +0200 (Wed, 09 Jun 2010) $   */
/*          at $Revision: 10 $                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#include "mctrl.h"


//macros to enhance readability of function definitions
#define PRE_GENERIC_MEMORY template <int hindex,    int pindex,   int romaddr, int rommask, \
                                     int ioaddr,    int iomask,   int ramaddr, int rammask, \
                                     int paddr,     int pmask,    int wprot,   int invclk,  \
                                     int fast,      int romasel,  int sdrasel, int srbanks, \
                                     int ram8,      int ram16,    int sden,    int sepbus,  \
                                     int sdbits,    int sdlsb,    int oepol,   int syncrst, \
                                     int pageburst, int scantest, int mobile,  typename T>

#define POST_GENERIC_MEMORY(TYPE) <hindex,    pindex,   romaddr, \
                                   rommask,   ioaddr,   iomask,  \
                                   ramaddr,   rammask,  paddr,   \
                                   pmask,     wprot,    invclk,  \
                                   fast,      romasel,  sdrasel, \
                                   srbanks,   ram8,     ram16,   \
                                   sden,      sepbus,   sdbits,  \
                                   sdlsb,     oepol,    syncrst, \
                                   pageburst, scantest, mobile, TYPE >


//scope
PRE_GENERIC_MEMORY Generic_memory POST_GENERIC_MEMORY(T)::
//constructor
Generic_memory(sc_core::sc_module_name name) :
       // construct and name socket
       slave_socket("slave_socket") {

  // register transport functions to sockets
  slave_socket.register_b_transport (this, &Generic_memory::b_transport);
}

//explicit declaration of standard destructor required for linking
PRE_GENERIC_MEMORY Generic_memory POST_GENERIC_MEMORY(T)::~Generic_memory() {
}


//-----------TLM--TRANSPORT--FUNCTIONS-----------//

//scope: same template parameters as Mctrl
PRE_GENERIC_MEMORY void Generic_memory POST_GENERIC_MEMORY(T)::
//blocking transport for ROM
b_transport(tlm::tlm_generic_payload& gp, sc_time& delay) {
  unsigned int streaming_width = gp.get_streaming_width();
  //check TLM COMMAND field
  tlm::tlm_command cmd = gp.get_command();
  if (cmd == 0) {
    switch (streaming_width) {
      //64 bit --> SDRAM
      case 64: {
        uint64_t *data_ptr64 = reinterpret_cast<uint64_t *>( gp.get_data_ptr() );
        *data_ptr64 = read_2x32( gp.get_address() );
      } break;
      //32 bit --> SRAM / ROM or SDRAM / IO --> 4x8 or 1x32
      case 32: {
        uint32_t* data_ptr32 = reinterpret_cast<uint32_t *>( gp.get_data_ptr() );
        //SDRAM or IO: map of uint32_t type
        if (sizeof( memory[gp.get_address()] ) == 4) {
          *data_ptr32 = read_1x32( gp.get_address() );
        }
        //SRAM or ROM: map of uint8_t type
        else if (sizeof( memory[gp.get_address()] ) == 1) {
          *data_ptr32 = read_4x8( gp.get_address() );
        }
      } break;
      //16 bit --> SRAM or ROM --> 4x8
      case 16: {
        uint32_t* data_ptr16 = reinterpret_cast<uint32_t *>( gp.get_data_ptr() );
        *data_ptr16 = read_4x8( gp.get_address() );
      } break;
      //8 bit --> SRAM or ROM --> 4x8
      case 8: {
        uint32_t* data_ptr8 = reinterpret_cast<uint32_t *>( gp.get_data_ptr() );
        *data_ptr8 = read_4x8( gp.get_address() );
      }
    }

    //update response status
    gp.set_response_status(tlm::TLM_OK_RESPONSE);
  }
  else if (cmd == 1) {
    switch (streaming_width) {
      case 64: {
        uint64_t data64 = *reinterpret_cast<uint64_t *>( gp.get_data_ptr() );
        write_2x32(gp.get_address(), data64);
      } break;
      case 32: {
        //SDRAM or IO: map of uint32_t type
        if (sizeof( memory[gp.get_address()] ) == 4) {
          uint32_t data32 = *reinterpret_cast<uint32_t *>( gp.get_data_ptr() );
          write_1x32( gp.get_address(), data32 );
        }
        //SRAM or ROM: map of uint8_t type
        else if (sizeof( memory[gp.get_address()] ) == 1) {
          uint32_t data4x8 = *reinterpret_cast<uint32_t *>(gp.get_data_ptr());
          write_4x8( gp.get_address(), data4x8 );
        }
      } break;
      case 16: {
        uint16_t data16 = *reinterpret_cast<uint16_t *>(gp.get_data_ptr());
        write_2x8( gp.get_address(), data16 );
      } break;
      case 8: {
        uint8_t data8 = *reinterpret_cast<uint8_t *>(gp.get_data_ptr());
        write_1x8( gp.get_address(), data8 );
      }
    }

    //update response status
    gp.set_response_status(tlm::TLM_OK_RESPONSE);
  }
}


//-----------------WRITE--FUNCTIONS----------------//

//scope
PRE_GENERIC_MEMORY void Generic_memory POST_GENERIC_MEMORY(T)::
//write into ROM / SRAM: 32 bit access
write_4x8(uint32_t address, uint32_t data) {

  //split word into 4 bytes
  memory[address+3] = ((data >> 24) & 0x000000FF);
  memory[address+2] = ((data >> 16) & 0x000000FF);
  memory[address+1] = ((data >> 8)  & 0x000000FF);
  memory[address] = (data & 0x000000FF);
}


//scope
PRE_GENERIC_MEMORY void Generic_memory POST_GENERIC_MEMORY(T)::
//write into ROM / SRAM: 16 bit access
write_2x8(uint32_t address, uint16_t data) {

  //split halfword into 2 bytes
  memory[address+1] = ((data >> 8) & 0x00FF);
  memory[address] = (data & 0x00FF);
}


//scope
PRE_GENERIC_MEMORY void Generic_memory POST_GENERIC_MEMORY(T)::
//write into ROM / SRAM: 8 bit access
write_1x8(uint32_t address, uint8_t data) {
  memory[address] = data;
}


//scope
PRE_GENERIC_MEMORY void Generic_memory POST_GENERIC_MEMORY(T)::
//write into IO / SDRAM: 32 bit access
write_1x32(uint32_t address, uint32_t data) {
  memory[address] = data;
}


//scope
PRE_GENERIC_MEMORY void Generic_memory POST_GENERIC_MEMORY(T)::
//write into SDRAM: 64 bit access
write_2x32(uint32_t address, uint64_t data) {

  //split 64 bit dword into two words
  uint32_t word1 = static_cast<uint32_t> ((data >> 32) & 0x00000000FFFFFFFF);
  uint32_t word2 = static_cast<uint32_t> (data & 0x00000000FFFFFFFF);
  memory[address + 4] = word1;
  memory[address] = word2;
}


//-----------------READ--FUNCTIONS-----------------//

//scope
PRE_GENERIC_MEMORY uint32_t Generic_memory POST_GENERIC_MEMORY(T)::
//read from ROM / SRAM: 32 bit access only
read_4x8(uint32_t address) {
  uint32_t mem_word = 0;

  //Independent from 8, 16, or 32 bit access, 4 adjacent 8 bit words
  //will be read. Possible burst return must be handled by MCTRL.
  for (int i=3; i>=0; i--) {
    mem_word = mem_word << 8;
    mem_word |= memory[address + i];
  }
  return mem_word;
}


//scope
PRE_GENERIC_MEMORY uint32_t Generic_memory POST_GENERIC_MEMORY(T)::
//read from IO / SDRAM: 32 bit access
read_1x32(uint32_t address) {
  return memory[address];
}


//scope
PRE_GENERIC_MEMORY uint64_t Generic_memory POST_GENERIC_MEMORY(T)::
//read from SDRAM: 64 bit access
read_2x32(uint32_t address) {
  uint64_t mem_dword = memory[address + 4];
  mem_dword = (mem_dword << 32) | memory[address];
  return mem_dword;
}
