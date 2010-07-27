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
  //call write (1) or read (0) function according to TLM COMMAND field
  if (cmd == 0) {
    unsigned char* data_ptr = gp.get_data_ptr();
    *data_ptr = read_rom( gp.get_address(), gp.get_streaming_width() );
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
    unsigned char* data_ptr = gp.get_data_ptr();
    *data_ptr = read_io( gp.get_address() );
    sc_core::sc_time t(IO_READ_DELAY, SC_NS);
    delay += t;
  }
  else if (cmd == 1) {
    write_io (gp.get_address(), *gp.get_data_ptr() );
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
    unsigned char* data_ptr = gp.get_data_ptr();
    *data_ptr = read_sram( gp.get_address(), gp.get_streaming_width() );
    sc_core::sc_time t(SRAM_READ_DELAY, SC_NS);
    delay += t;
  }
  else if (cmd == 1) {
    write_sram (gp.get_address(), *gp.get_data_ptr(), gp.get_streaming_width() );
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
    unsigned char* data_ptr = gp.get_data_ptr();
    if (gp.get_streaming_width() == 32) {
      uint32_t mem_word = read_sdram32( gp.get_address() );
      *data_ptr = mem_word;
    }
    else if (gp.get_streaming_width() == 64) {
      uint64_t mem_dword = read_sdram64( gp.get_address() );
      *data_ptr = mem_dword;
    }
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
    sc_core::sc_time t(SDRAM_WRITE_DELAY, SC_NS);
    delay += t;
  }
}

//-----------READ--AND--WRITE--FUNCTIONS-----------//

//scope: same template parameters as Mctrl
PRE_MCTRL uint32_t Generic_memory POST_MCTRL::
//read from ROM
read_rom(uint32_t address, uint8_t width) {
  //in case of 8 or 16 bit access, a 32 bit word is sent in a burst of 4 / 2 words
  uint32_t mem_word;

  //hash keys might address 8, 16, or 32 bits
  //MCFG1 bits 9 and 8 determine rom width (1- --> 32 bits)
  if (width == 32) {
    mem_word = rom[address];
  }
  else if (width == 16) {
    //01 --> 16 bits: 32 bit word is spread over two hash map positions (address keys)
    mem_word = (rom[address + 2] << 16) + rom[address];
  }
  else if (width == 8) {
    //00 --> 8 bits: 32 bit word is spread over four hash map positions (address keys)
    mem_word = (rom[address + 3] << 24) +
               (rom[address + 2] << 16) +
               (rom[address + 1] <<  8) +
                rom[address];
  }
  return mem_word;
}

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

//scope: same template parameters as Mctrl
PRE_MCTRL void Generic_memory POST_MCTRL::
//write into SRAM
write_sram(uint32_t address, uint32_t data, uint8_t width) {
  uint32_t mem_word;

  //access can address 8, 16, or 32 bits
  if (width == 32) {
    sram[address] = data;
  }
  else if (width == 16) {
    //upper half word (HW_TWO)
    if (address % 4) {
      //fetch full word and delete upper half
      mem_word = sram[address - 2] and HW_TWO_MASK_N;
      //overwrite upper half with new data and write back
      sram[address - 2] = mem_word or (data << 16);
    }
    //lower half word (HW_ONE)
    else {
      mem_word = sram[address] and HW_ONE_MASK_N;
      sram[address] = mem_word or (data);
    }
  }
  //same procedure for byte address
  else if (width == 8) {
    if (address % 4 == 3) {
      mem_word = sram[address - 3] and BYTE_FOUR_MASK_N;
      sram[address - 3] = mem_word or (data << 24);
    }
    else if (address % 4 == 2) {
      mem_word = sram[address - 2] and BYTE_THREE_MASK_N;
      sram[address - 2] = mem_word or (data << 16);
    }
    else if (address % 4 == 1) {
      mem_word = sram[address - 1] and BYTE_TWO_MASK_N;
      sram[address - 1] = mem_word or (data << 8);
    }
    else  {
      mem_word = sram[address] and HW_ONE_MASK_N;
      sram[address] = mem_word or (data);
    }
  }
}

//scope: same template parameters as Mctrl
PRE_MCTRL uint32_t Generic_memory POST_MCTRL::
//Read from SRAM: Byte-/halfword-wise bursts (in case of 8 / 16 bit accesses) will only
//need to be modeled on the bus (i.e. between MCTRL and LEON)
read_sram(uint32_t address, uint8_t width) {
  uint32_t mem_word;

  //hash keys might address 8, 16, or 32 bits
  if (width == 32) {
    mem_word = sram[address];
  }
  else if (width == 16) {
    //32 bit word is spread over two hash map positions (address keys)
    mem_word = (sram[address + 2] << 16) + sram[address];
  }
  else if (width == 8) {
    //32 bit word is spread over four hash map positions (address keys)
    mem_word = (sram[address + 3] << 24) +
               (sram[address + 2] << 16) +
               (sram[address + 1] <<  8) +
                sram[address];
  }
  return mem_word;
}

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
//read from SDRAM
read_sdram32(uint32_t address) {
  return (sdram[address]);
}

//scope: same template parameters as Mctrl
PRE_MCTRL uint64_t Generic_memory POST_MCTRL::
//read from SDRAM
read_sdram64(uint32_t address) {
  uint64_t mem_dword = static_cast<uint64_t>(sdram[address + 4]);
  mem_dword = (mem_dword << 32) + sdram[address];
  return mem_dword;
}
