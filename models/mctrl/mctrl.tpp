/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mctrl.tpp                                               */
/*             implementation of the mctrl module                      */
/*             is included by mctrl.h template header file             */
/*                                                                     */
/* Modified on $Date: 2010-06-09 10:30:16 +0200 (Wed, 09 Jun 2010) $   */
/*          at $Revision: 10 $                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef MCTRL_TPP
#define MCTRL_TPP

#define DECODING_DELAY 42

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

//scope
PRE_MCTRL Mctrl POST_MCTRL::
//constructor
Mctrl(sc_core::sc_module_name name) :
  gr_device(
            name,                      //sc_module name
            gs::reg::ALIGNED_ADDRESS,  //address mode (options: aligned / indexed)
            16,                        //dword size (of register file)
            NULL                       //parent module
           ),
  bus( //greenreg_socket
      "bus",            //name
      r,                //register container
      0x0,              // ?
      0xFFFFFFFF,       // ?
      ::amba::amba_APB, // ?bus type?
      ::amba::amba_LT,  // ?communication type / abstraction level?
      false             // ?
     ),
  mctrl_rom("mctrl_rom"),
  mctrl_io("mctrl_io"),
  mctrl_sram("mctrl_sram"),
  mctrl_sdram("mctrl_sdram")
  {

  // register transport functions to sockets
  bus.register_b_transport (this, &Mctrl::b_transport);

      // more to be added






#ifdef MONOLITHIC_MODULE
  //connect sockets to memory module
  mem.mem_rom(mctrl_rom);
  mem.mem_io(mctrl_io);
  mem.mem_sram(mctrl_sram);
  mem.mem_sdram(mctrl_sdram);
#endif

  // create register | name + description
  r.create_register( "MCFG1", "Memory Configuration Register 1",
                   // offset
                      0x00,
                   // config
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                   // init value (to be calculated from the generics for all 4 registers)
                      0x00000000,
                   // write mask
                      MCTRL_MCFG1_WRITE_MASK,
                   // reg width (maximum 32 bit)
                      32,
                   // lock mask: Not implementet, has to be zero.
                      0x00
                   );
  r.create_register( "MCFG2", "Memory Configuration Register 2",
                      0x04,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      0x00000000,
                      MCTRL_MCFG2_WRITE_MASK,
                      32,
                      0x00
                   );
  r.create_register( "MCFG3", "Memory Configuration Register 3", 
                      0x08,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      0x00000000, 
                      MCTRL_MCFG3_WRITE_MASK,
                      32, 
                      0x00
                   );
  r.create_register( "MCFG4", "Power-Saving Configuration Register", 
                      0x0C,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      0x00000000, 
                      MCTRL_MCFG4_WRITE_MASK,
                      32,
                      0x00
                   );

  //process registration
  SC_THREAD(initialize_mctrl);

}

//explicit declaration of standard destructor required for linking
PRE_MCTRL Mctrl POST_MCTRL::~Mctrl() {
}


//scope
PRE_MCTRL void Mctrl POST_MCTRL::
//function to initialize registers and memory address spece constants
initialize_mctrl() {

  // --- 1. set configuration registers




  // --- 2. calculate address spaces of the different memory banks

  //ROM 
  //FIXME: Size???
  rom_bk1_s = static_cast<uint32_t>(romaddr << 20);            // 's' --> 'start'
  rom_bk1_e = static_cast<uint32_t>((ioaddr + romaddr) << 19); // 'e' --> 'end'
  rom_bk2_s = rom_bk1_e + 1;
  rom_bk2_e = static_cast<uint32_t>(ioaddr << 20) - 1;

  //IO
  //FIXME: size???
  io_s = static_cast<uint32_t>(ioaddr << 20);
  io_e = static_cast<uint32_t>(ioaddr << 21);

  // ------- RAM -------

  //SRAM bank size can be 8KB, 16KB, 32KB, ... 256MB
  uint32_t sram_bank_size = (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_RAM_BANK_SIZE) >> 9;

  //SDRAM bank size can be 4 MB, 8MB, 16MB, ... 512MB
  uint32_t sdram_bank_size = (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SDRAM_BANKSZ) >> 23;

  //address spaces in case of SRAM only configuration
  if (r[MCTRL_MCFG2].bit_get(14) == false) {
    //bank 1
    sram_bk1_s = static_cast<uint32_t>(ramaddr << 20);
    //                                                      1Byte << 13 = 8KB
    sram_bk1_e = sram_bk1_s + (1 << (sram_bank_size + 13)) - 1;
    //bank 2 starts right after bank 1
    sram_bk2_s = sram_bk1_e + 1;
    sram_bk2_e = sram_bk2_s + (1 << (sram_bank_size + 13)) - 1;
    //bank 3 ...
    sram_bk3_s = sram_bk2_e + 1;
    sram_bk3_e = sram_bk3_s + (1 << (sram_bank_size + 13)) - 1;

    sram_bk4_s = sram_bk3_e + 1;
    sram_bk4_e = sram_bk4_s + (1 << (sram_bank_size + 13)) - 1;

    sram_bk5_s = sram_bk4_e + 1;
    sram_bk5_e = sram_bk5_s + (1 << (sram_bank_size + 13)) - 1;

    //unused banks, sdram1 and sdram2
    sdram_bk1_s = 0;
    sdram_bk1_e = 0;
    sdram_bk2_s = 0;
    sdram_bk2_e = 0;

  }
  //address spaces in case of SDRAM and SRAM (lower 4 SRAM banks only) configuration
  else if (r[MCTRL_MCFG2].bit_get(13) == false) {
    //bank 1
    sram_bk1_s = static_cast<uint32_t>(ramaddr << 20);
    //                                                      1Byte << 13 = 8KB
    sram_bk1_e = sram_bk1_s + (1 << (sram_bank_size + 13)) - 1;
    //bank 2 starts right after bank 1
    sram_bk2_s = sram_bk1_e + 1;
    sram_bk2_e = sram_bk2_s + (1 << (sram_bank_size + 13)) - 1;
    //bank 3 ...
    sram_bk3_s = sram_bk2_e + 1;
    sram_bk3_e = sram_bk3_s + (1 << (sram_bank_size + 13)) - 1;

    sram_bk4_s = sram_bk3_e + 1;
    sram_bk4_e = sram_bk4_s + (1 << (sram_bank_size + 13)) - 1;
    //SDRAM bank 1 starts at upper half of RAM area
    sdram_bk1_s = static_cast<uint32_t>(ramaddr << 21);
    sdram_bk1_e = sdram_bk1_s + (1 << (sdram_bank_size + 22)) - 1;
    //SDRAM bank 2
    sdram_bk2_s = sdram_bk1_e + 1;
    sdram_bk2_e = sdram_bk2_s + (1 << (sdram_bank_size + 22)) - 1;

    //unused bank sram5: constants need to be defined, but range must be empty
    sram_bk5_s = 0;
    sram_bk5_e = 0;
  }
  // SDRAM only (located in lower half of RAM address space)
  else {
    //SDRAM bank 1 starts at lower half of RAM area
    sdram_bk1_s = static_cast<uint32_t>(ramaddr << 20);
    sdram_bk1_e = sdram_bk1_s + (1 << (sdram_bank_size + 22)) - 1;
    //SDRAM bank 2
    sdram_bk2_s = sdram_bk1_e + 1;
    sdram_bk2_e = sdram_bk2_s + (1 << (sdram_bank_size + 22)) - 1;

    //unused banks, sram1 ... sram5
    sram_bk1_s = 0;
    sram_bk1_e = 0;
    sram_bk2_s = 0;
    sram_bk2_e = 0;
    sram_bk3_s = 0;
    sram_bk3_e = 0;
    sram_bk4_s = 0;
    sram_bk4_e = 0;
    sram_bk5_s = 0;
    sram_bk5_e = 0;
  }
}



//-----------TLM--TRANSPORT--FUNCTIONS-----------//

//scope
PRE_MCTRL void Mctrl POST_MCTRL::
//blocking transport function
b_transport(tlm::tlm_generic_payload& gp, sc_time& delay)  {
  sc_core::sc_time t(DECODING_DELAY, SC_NS);

  //access to ROM adress space
  if (Mctrl::rom_bk1_s <= gp.get_address() <= Mctrl::rom_bk1_e) {
    delay += t;
    if (r[MCTRL_MCFG1].bit_get(9)) {
      gp.set_streaming_width(32);
    }
    else if (r[MCTRL_MCFG1].bit_get(8)) {
      gp.set_streaming_width(16);
    }
    else {
      gp.set_streaming_width(8);
    }
    mctrl_rom->b_transport(gp,delay);
  }
  else if (Mctrl::rom_bk2_s <= gp.get_address() <= Mctrl::rom_bk2_e) {
    delay += t;
    if (r[MCTRL_MCFG1].bit_get(9)) {
      gp.set_streaming_width(32);
    }
    else if (r[MCTRL_MCFG1].bit_get(8)) {
      gp.set_streaming_width(16);
    }
    else {
      gp.set_streaming_width(8);
    }
    mctrl_rom->b_transport(gp,delay);
  }
  //access to IO adress space
  else if (Mctrl::io_s <= gp.get_address() <= Mctrl::io_e) {
    delay += t;
    mctrl_io->b_transport(gp,delay);
  }
  //access to SRAM adress space
  else if (Mctrl::sram_bk1_s <= gp.get_address() <= Mctrl::sram_bk1_e) {
    if (r[MCTRL_MCFG2].bit_get(5)) {
      gp.set_streaming_width(32);
    }
    else if (r[MCTRL_MCFG2].bit_get(4)) {
      gp.set_streaming_width(16);
    }
    else {
      gp.set_streaming_width(8);
    }
    delay += t;
    mctrl_sram->b_transport(gp,delay);
  }
  else if (Mctrl::sram_bk2_s <= gp.get_address() <= Mctrl::sram_bk2_e) {
    if (r[MCTRL_MCFG2].bit_get(5)) {
      gp.set_streaming_width(32);
    }
    else if (r[MCTRL_MCFG2].bit_get(4)) {
      gp.set_streaming_width(16);
    }
    else {
      gp.set_streaming_width(8);
    }
    delay += t;
    mctrl_sram->b_transport(gp,delay);
  }
  else if (Mctrl::sram_bk3_s <= gp.get_address() <= Mctrl::sram_bk3_e) {
    if (r[MCTRL_MCFG2].bit_get(5)) {
      gp.set_streaming_width(32);
    }
    else if (r[MCTRL_MCFG2].bit_get(4)) {
      gp.set_streaming_width(16);
    }
    else {
      gp.set_streaming_width(8);
    }
    delay += t;
    mctrl_sram->b_transport(gp,delay);
  }
  else if (Mctrl::sram_bk4_s <= gp.get_address() <= Mctrl::sram_bk4_e) {
    if (r[MCTRL_MCFG2].bit_get(5)) {
      gp.set_streaming_width(32);
    }
    else if (r[MCTRL_MCFG2].bit_get(4)) {
      gp.set_streaming_width(16);
    }
    else {
      gp.set_streaming_width(8);
    }
    delay += t;
    mctrl_sram->b_transport(gp,delay);
  }
  else if (Mctrl::sram_bk5_s <= gp.get_address() <= Mctrl::sram_bk5_e) {
    if (r[MCTRL_MCFG2].bit_get(5)) {
      gp.set_streaming_width(32);
    }
    else if (r[MCTRL_MCFG2].bit_get(4)) {
      gp.set_streaming_width(16);
    }
    else {
      gp.set_streaming_width(8);
    }
    delay += t;
    mctrl_sram->b_transport(gp,delay);
  }
  //access to SDRAM adress space
  else if (Mctrl::sdram_bk1_s <= gp.get_address() <= Mctrl::sdram_bk1_e) {
    if (r[MCTRL_MCFG2].bit_get(18)) {
      gp.set_streaming_width(64);
    }
    else {
      gp.set_streaming_width(32);
    }
    delay += t;
    mctrl_sdram->b_transport(gp,delay);
  }
  else if (Mctrl::sdram_bk2_s <= gp.get_address() <= Mctrl::sdram_bk2_e) {

    delay += t;
    mctrl_sdram->b_transport(gp,delay);
  }


//Address decoding depends on the address spaces of the different memories.
//The sizes and address spaces are summarized in the following.

//start of rom address space: romaddr << 20 (default: 0x0)
//start of io address space: ioaddr << 20   (default: 512 << 20 = 0x200 << 20 = 0x20000000)
//start of ram address space: ramaddr << 20 (default: 1024 --> 0x40000000)

//SRAM: 
//Bank size of lower 4 banks is determined by MCFG2[12..9]
// 0000 -->   8 KB
// 0001 -->  16 KB
// 0010 -->  32 KB
// 1111 --> 256 MB  --> max 4x 256 MB = 1GB 
//Bank 5 size is calculated by sdrasel generic (max 512MB for sdrasel = 29)
//Bank 5 address space size is 2^(sdrasel + 1)
//Bank 5 cannot be used simultaneously with sdram

//SDRAM
//Bank size is determined by SDRAM BANKSZ bits (25..23) of MCFG2 register
// 000 -->   4 MB
// 001 -->   8 MB
// 010 -->  16 MB
// 111 --> 512 MB  --> max 2x 512 MB = 1GB

//Analyze generic payload

//Call read / write functions according to generic payload

//Calculate timing

//Modify and return generic payload

//State Machine management? Transaction can only be initialized
//in idle state.

}




//scope
PRE_MCTRL void Mctrl POST_MCTRL::
//write data into SRAM, needs to be triggered by AMBA socket
write_sram(uint32_t addr, uint32_t data, uint32_t width) {
  sc_core::sc_time t;
  tlm::tlm_generic_payload gp;
    gp.set_command(tlm::TLM_WRITE_COMMAND);
    gp.set_address(addr);
    gp.set_data_length(width);
    gp.set_streaming_width(4);
    gp.set_byte_enable_ptr(NULL);
    gp.set_data_ptr((unsigned char*)&data);
  mctrl_sram->b_transport(gp,t);
}

//scope
PRE_MCTRL void Mctrl POST_MCTRL::
//read data from SRAM, needs to be triggered by AMBA socket
read_sram(uint32_t addr, uint32_t width) {
  sc_core::sc_time t;
  uint32_t data;
  tlm::tlm_generic_payload gp;
    gp.set_command(tlm::TLM_READ_COMMAND);
    gp.set_address(addr);
    gp.set_data_length(width);
    gp.set_streaming_width(4);
    gp.set_byte_enable_ptr(NULL);
    gp.set_data_ptr((unsigned char*)&data);
  mctrl_sram->b_transport(gp,t);
}

#endif
