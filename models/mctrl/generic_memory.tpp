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

//memory read / write delays
#define ROM_READ_DELAY 42
#define ROM_WRITE_DELAY 42
#define IO_READ_DELAY 42
#define IO_WRITE_DELAY 42
#define SRAM_READ_DELAY 42
#define SRAM_WRITE_DELAY 42
#define SDRAM_READ_DELAY 42
#define SDRAM_WRITE_DELAY 42

//masks for 8-bit / 16-bit memory access
#define BYTE_ONE_MASK_N   0xFFFFFF00
#define BYTE_TWO_MASK_N   0xFFFF00FF
#define BYTE_THREE_MASK_N 0xFF00FFFF
#define BYTE_FOUR_MASK_N  0x00FFFFFF
#define HW_ONE_MASK_N     0xFFFF0000
#define HW_TWO_MASK_N     0x0000FFFF

//macros to enhance readability of function definitions
#define PRE_MCTRL template <int hindex,    int pindex,   int romaddr, int rommask, \
                            int ioaddr,    int iomask,   int ramaddr, int rammask, \
                            int paddr,     int pmask,    int wprot,   int invclk,  \
                            int fast,      int romasel,  int sdrasel, int srbanks, \
                            int ram8,      int ram16,    int sden,    int sepbus,  \
                            int sdbits,    int sdlsb,    int oepol,   int syncrst, \
                            int pageburst, int scantest, int mobile>

#define POST_MCTRL <hindex,    pindex,   romaddr, \
                    rommask,   ioaddr,   iomask,  \
                    ramaddr,   rammask,  paddr,   \
                    pmask,     wprot,    invclk,  \
                    fast,      romasel,  sdrasel, \
                    srbanks,   ram8,     ram16,   \
                    sden,      sepbus,   sdbits,  \
                    sdlsb,     oepol,    syncrst, \
                    pageburst, scantest, mobile >

//scope: same template parameters as Mctrl
PRE_MCTRL Generic_memory POST_MCTRL::
//constructor
Generic_memory(sc_core::sc_module_name name) :
       // construct and name sockets
       mem_rom("mem_rom"),
       mem_io("mem_io"),
       mem_sram("mem_sram"),
       mem_sdram("mem_sdram") {

  //process registration

  // register transport functions to sockets
  mem_rom.register_b_transport (this, &Generic_memory::b_transport_rom);
  mem_io.register_b_transport (this, &Generic_memory::b_transport_io);
  mem_sram.register_b_transport (this, &Generic_memory::b_transport_sram);
  mem_sdram.register_b_transport (this, &Generic_memory::b_transport_sdram);

}

//explicit declaration of standard destructor required for linking
PRE_MCTRL Generic_memory POST_MCTRL::~Generic_memory() {
}


//-----------TLM--TRANSPORT--FUNCTIONS-----------//

//scope: same template parameters as Mctrl
PRE_MCTRL void Generic_memory POST_MCTRL::
//blocking transport for ROM
b_transport_rom(tlm::tlm_generic_payload& gp, sc_time& delay) {
  tlm::tlm_command cmd = gp.get_command();
  //check TLM COMMAND field
  if (cmd == 0) {
    //call read function
    unsigned char* data_ptr = gp.get_data_ptr();
    *data_ptr = read_rom( gp.get_address() );
    //add delay
    sc_core::sc_time t(ROM_READ_DELAY, SC_NS);
    delay += t;
  }
}

//scope: same template parameters as Mctrl
PRE_MCTRL void Generic_memory POST_MCTRL::
//blocking transport for IO
b_transport_io(tlm::tlm_generic_payload& gp, sc_time& delay) {
  tlm::tlm_command cmd = gp.get_command();
  //call write (1) or read (0) function according to TLM COMMAND field
  if (cmd == 0) {
    //call read function
    unsigned char* data_ptr = gp.get_data_ptr();
    *data_ptr = read_io( gp.get_address() );
    //add delay
    sc_core::sc_time t(IO_READ_DELAY, SC_NS);
    delay += t;
  }
  else if (cmd == 1) {
    //call write function
    write_io (gp.get_address(), *gp.get_data_ptr() );
    //add delay
    sc_core::sc_time t(IO_WRITE_DELAY, SC_NS);
    delay += t;
  }
}

//scope: same template parameters as Mctrl
PRE_MCTRL void Generic_memory POST_MCTRL::
//blocking transport for SRAM
b_transport_sram(tlm::tlm_generic_payload& gp, sc_time& delay) {
  tlm::tlm_command cmd = gp.get_command();
  //call write (1) or read (0) function according to TLM COMMAND field
  if (cmd == 0) {
    //call read function
    unsigned char* data_ptr = gp.get_data_ptr();
    *data_ptr = read_sram( gp.get_address() );
    //add delay
    sc_core::sc_time t(SRAM_READ_DELAY, SC_NS);
    delay += t;
  }
  else if (cmd == 1) {
    //call 32 bit write function
    if (gp.get_streaming_width() == 32) {
      uint32_t data = *gp.get_data_ptr();
      write_sram (gp.get_address(), data);
    }
    //call 16 bit write function
    else if (gp.get_streaming_width() == 16) {
      uint16_t data = *gp.get_data_ptr();
      write_sram (gp.get_address(), data);
    }
    //call 8 bit write function
    else if (gp.get_streaming_width() == 8) {
      uint8_t data = *gp.get_data_ptr();
      write_sram (gp.get_address(), data);
    }
    //add delay
    sc_core::sc_time t(SRAM_WRITE_DELAY, SC_NS);
    delay += t;
  }
}

//scope: same template parameters as Mctrl
PRE_MCTRL void Generic_memory POST_MCTRL::
//blocking transport for SDRAM
b_transport_sdram(tlm::tlm_generic_payload& gp, sc_time& delay) {
  tlm::tlm_command cmd = gp.get_command();
  //call write (1) or read (0) function according to TLM COMMAND field
  if (cmd == 0) {
    //call 32 bit read function
    unsigned char* data_ptr = gp.get_data_ptr();
    if (gp.get_streaming_width() == 32) {
      uint32_t mem_word = read_sdram32( gp.get_address() );
      *data_ptr = mem_word;
    }
    //call 64 bit read function
    else if (gp.get_streaming_width() == 64) {
      uint64_t mem_dword = read_sdram64( gp.get_address() );
      *data_ptr = mem_dword;
    }
    //add delay
    sc_core::sc_time t(SDRAM_READ_DELAY, SC_NS);
    delay += t;
  }
  else if (cmd == 1) {
    //call 32 bit write function
    if (gp.get_streaming_width() == 32) {
      uint32_t data = *gp.get_data_ptr();
      write_sdram (gp.get_address(), data);
    }
    //call 64 bit write function
    else if (gp.get_streaming_width() == 64) {
      uint64_t data = *gp.get_data_ptr();
      write_sdram (gp.get_address(), data);
    }
    //add delay
    sc_core::sc_time t(SDRAM_WRITE_DELAY, SC_NS);
    delay += t;
  }
}

//-----------READ--AND--WRITE--FUNCTIONS-----------//

// ---ROM---

//scope: same template parameters as Mctrl
PRE_MCTRL uint32_t Generic_memory POST_MCTRL::
//read from ROM
read_rom(uint32_t address) {
  uint32_t mem_word = 0;

  //Independent from 8, 16, or 32 bit access, 4 adjacent 8 bit words
  //will be read. Possible burst return must be handled by MCTRL.
  for (int i=3; i>=0; i++) {
    mem_word = mem_word << 8;
    mem_word = rom[address + i];
  }
  return mem_word;
}

// ---IO---

//scope: same template parameters as Mctrl
PRE_MCTRL void Generic_memory POST_MCTRL::
//write into IO
write_io(uint32_t address, uint32_t data) {
  io[address] = data;
}

//scope: same template parameters as Mctrl
PRE_MCTRL uint32_t Generic_memory POST_MCTRL::
//read from IO
read_io(uint32_t address) {
  return io[address];
}

// ---SRAM---

//scope: same template parameters as Mctrl
PRE_MCTRL void Generic_memory POST_MCTRL::
//write into SRAM: 32 bit access
write_sram(uint32_t address, uint32_t data) {
  //split word into 4 bytes
  sram[address + 3] = static_cast<uint8_t> (data >> 24);
  sram[address + 2] = static_cast<uint8_t> (data >> 16);
  sram[address + 1] = static_cast<uint8_t> (data >> 8);
  sram[address] = static_cast<uint8_t> (data and 0x000000FF);
}

//scope: same template parameters as Mctrl
PRE_MCTRL void Generic_memory POST_MCTRL::
//write into SRAM: 16 bit access
write_sram(uint32_t address, uint16_t data) {
  //split halfword into 2 bytes
  sram[address + 1] = static_cast<uint8_t> (data >> 8);
  sram[address] = static_cast<uint8_t> (data and 0x00FF);
}

//scope: same template parameters as Mctrl
PRE_MCTRL void Generic_memory POST_MCTRL::
//write into SRAM: 8 bit access
write_sram(uint32_t address, uint8_t data) {
  sram[address] = data;
}

//scope: same template parameters as Mctrl
PRE_MCTRL uint32_t Generic_memory POST_MCTRL::
//Read from SRAM: Byte-/halfword-wise bursts (in case of 8 / 16 bit accesses) will only
//need to be modeled on the bus (i.e. between MCTRL and LEON)
read_sram(uint32_t address) {
  uint32_t mem_word = 0;

  //Independent from 8, 16, or 32 bit access, 4 adjacent 8 bit words
  //will be read. Possible burst return must be handled by MCTRL.
  for (int i=3; i>=0; i++) {
    mem_word = mem_word << 8;
    mem_word = sram[address + i];
  }
  return mem_word;
}

// ---SDRAM---

//scope: same template parameters as Mctrl
PRE_MCTRL void Generic_memory POST_MCTRL::
//write 64 bits into SDRAM
write_sdram(uint32_t address, uint64_t data) {
  uint32_t word1 = static_cast<uint32_t> (data >> 32);
  uint32_t word2 = static_cast<uint32_t> (data and 0x00000000FFFFFFFF);
  sdram[address + 4] = word1;
  sdram[address] = word2;
}

//scope: same template parameters as Mctrl
PRE_MCTRL void Generic_memory POST_MCTRL::
//write 32 bits into SDRAM
write_sdram(uint32_t address, uint32_t data) {
  sdram[address] = data;
}

//scope: same template parameters as Mctrl
PRE_MCTRL uint32_t Generic_memory POST_MCTRL::
//read from SDRAM, 32 bit access
read_sdram32(uint32_t address) {
  return (sdram[address]);
}

//scope: same template parameters as Mctrl
PRE_MCTRL uint64_t Generic_memory POST_MCTRL::
//read from SDRAM, 64 bit access
read_sdram64(uint32_t address) {
  uint64_t mem_dword = static_cast<uint64_t>(sdram[address + 4]);
  mem_dword = (mem_dword << 32) + sdram[address];
  return mem_dword;
}
