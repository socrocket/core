/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       generic_memory.tpp                                      */
/*             implementation of the generic_memory module             */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#include "mctrl.h"
#include <assert.h>


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
  sc_core::sc_time cycle_time(BUS_CLOCK_CYCLE, SC_NS);

  unsigned int streaming_width = gp.get_streaming_width();
  //check TLM COMMAND field
  tlm::tlm_command cmd = gp.get_command();
  if (cmd == tlm::TLM_READ_COMMAND) {
    switch (streaming_width) {
      //64 bit --> SDRAM
      case 8:
      {
        uint8_t length = gp.get_data_length();
        uint32_t *data_ptr64 = reinterpret_cast<uint32_t *>( gp.get_data_ptr() );
        read_32( gp.get_address(), data_ptr64, length );
        break;
      }
      //32 bit --> SRAM / ROM or SDRAM / IO --> 4x8 or 1x32
      case 4:
      {
        //SDRAM or IO: map of uint32_t type
        if (sizeof( memory[gp.get_address()] ) == 4) {
          uint8_t length = gp.get_data_length();
          uint32_t* data_ptr32 = reinterpret_cast<uint32_t *>( gp.get_data_ptr() );
          read_32( gp.get_address(), data_ptr32, length );
        }
        //SRAM or ROM: map of uint8_t type
        else if (sizeof( memory[gp.get_address()] ) == 1) {
          uint8_t length = gp.get_data_length();
          unsigned char* data_ptr32 = reinterpret_cast<unsigned char *>( gp.get_data_ptr() );
          read_8( gp.get_address(), data_ptr32, length );
        }
        break;
      }
      //16 bit --> SRAM or ROM --> 4x8
      case 2:
      {
        uint8_t length = gp.get_data_length();
        unsigned char* data_ptr16 = reinterpret_cast<unsigned char *>( gp.get_data_ptr() );
        gp.set_streaming_width(4);
        read_8( gp.get_address(), data_ptr16, length );
        break;
      }
      //8 bit --> SRAM or ROM --> 4x8
      case 1:
      {
        uint8_t length = gp.get_data_length();
        unsigned char* data_ptr8 = reinterpret_cast<unsigned char *>( gp.get_data_ptr() );
        gp.set_streaming_width(4);
        read_8( gp.get_address(), data_ptr8, length );
      }
    }

    //update response status
    gp.set_response_status(tlm::TLM_OK_RESPONSE);
  }
  else if (cmd == tlm::TLM_WRITE_COMMAND) {
    switch (streaming_width) {
      case 8:
      {
        uint8_t length = gp.get_data_length();
        uint32_t* data64 = reinterpret_cast<uint32_t *>( gp.get_data_ptr() );
        write_32(gp.get_address(), data64, length);
        break;
      }
      case 4:
      {
        //SDRAM or IO: map of uint32_t type
        if (sizeof( memory[gp.get_address()] ) == 4) {
          uint8_t length = gp.get_data_length();
          uint32_t* data32 = reinterpret_cast<uint32_t *>( gp.get_data_ptr() );
          write_32( gp.get_address(), data32, length );
        }
        //SRAM or ROM: map of uint8_t type
        else if (sizeof( memory[gp.get_address()] ) == 1) {
        uint8_t length = gp.get_data_length();
        unsigned char* data8 = reinterpret_cast<unsigned char *>(gp.get_data_ptr());
        write_8( gp.get_address(), data8, length );
        }
        break;
      }
      case 2:
      {
        uint8_t length = gp.get_data_length();
        unsigned char* data8 = reinterpret_cast<unsigned char *>(gp.get_data_ptr());
        write_8( gp.get_address(), data8, length );
        break;
      }
      case 1:
      {
        uint8_t length = gp.get_data_length();
        unsigned char* data8 = reinterpret_cast<unsigned char *>(gp.get_data_ptr());
        write_8( gp.get_address(), data8, length );
      }
    }

    //update response status
    gp.set_response_status(tlm::TLM_OK_RESPONSE);
  }

}


//-----------------WRITE--FUNCTIONS----------------//

//scope
PRE_GENERIC_MEMORY void Generic_memory POST_GENERIC_MEMORY(T)::
//write into ROM / SRAM: 8 bit access
write_8(uint32_t address, unsigned char* data, uint8_t length) {
  for (uint8_t i=0; i<length; i++) {
    memory[address + i] = data[i];
  }
}


//scope
PRE_GENERIC_MEMORY void Generic_memory POST_GENERIC_MEMORY(T)::
//write into IO / SDRAM: 32 bit access
write_32(uint32_t address, uint32_t* data, uint8_t length) {
  for (uint8_t i=0; i<length/4; i++) {
    memory[address + 4*i] = data[i];
  }
}


//-----------------READ--FUNCTIONS-----------------//

//scope
PRE_GENERIC_MEMORY void Generic_memory POST_GENERIC_MEMORY(T)::
//read from ROM / SRAM: 32 bit access only
read_8(uint32_t address, unsigned char* data_ptr, uint8_t length) {

  //Independent from 8, 16, or 32 bit access, 4 adjacent 8 bit words
  //will be read. Address and length have to be aligned!
  assert( !(address % 4) );
  assert( !(length % 4) );

  //processor must take care of consistent memory allocation and length parameter in gp
  for (int i=0; i<length; i++) {
    data_ptr[i] = memory[address + i];
  }
}


//scope
PRE_GENERIC_MEMORY void Generic_memory POST_GENERIC_MEMORY(T)::
//read from IO / SDRAM: 32 bit access
read_32(uint32_t address, uint32_t* data_ptr, uint8_t length) {

  //processor must take care of consistent memory allocation and length parameter in gp
  for (int i=0; i<length/4; i++) {
    data_ptr[i] = memory[address + 4*i];
  }
}



