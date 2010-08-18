/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mctrl.tpp                                               */
/*             implementation of the mctrl module                      */
/*             is included by mctrl.h template header file             */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef MCTRL_TPP
#define MCTRL_TPP

//should be defined somewhere else
#define BUS_CLOCK_CYCLE 10

//delay definitions (in clock cycles)
#define DECODING_DELAY 1
#define MCTRL_ROM_READ_DELAY(wstates) 2+wstates
#define MCTRL_ROM_WRITE_DELAY(wstates) 3+wstates
#define MCTRL_IO_READ_DELAY(wstates) 2+wstates
#define MCTRL_IO_WRITE_DELAY(wstates) 3+wstates
#define MCTRL_SRAM_READ_DELAY(wstates) 2+wstates
#define MCTRL_SRAM_WRITE_DELAY(wstates) 3+wstates

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
  apb( //greenreg_socket
      "bus",            //name
      r,                //register container
      0x0,              // ?
      0xFFFFFFFF,       // ?
      ::amba::amba_APB, // ?bus type?
      ::amba::amba_LT,  // ?communication type / abstraction level?
      false             // ?
     ),
  ahb("ahb"),
  mctrl_rom("mctrl_rom"),
  mctrl_io("mctrl_io"),
  mctrl_sram("mctrl_sram"),
  mctrl_sdram("mctrl_sdram")
  {

  // register transport functions to sockets
  ahb.register_b_transport (this, &Mctrl::b_transport);

      // nb_transport to be added 


  // create register | name + description
  r.create_register( "MCFG1", "Memory Configuration Register 1",
                   // offset
                      0x00,
                   // config
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                   // init value (to be calculated from the generics for all 4 registers)
                      MCTRL_MCFG1_DEFAULT,
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
                      MCTRL_MCFG2_DEFAULT,
                      MCTRL_MCFG2_WRITE_MASK,
                      32,
                      0x00
                   );
  r.create_register( "MCFG3", "Memory Configuration Register 3", 
                      0x08,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      MCTRL_MCFG3_DEFAULT, 
                      MCTRL_MCFG3_WRITE_MASK,
                      32, 
                      0x00
                   );
  r.create_register( "MCFG4", "Power-Saving Configuration Register", 
                      0x0C,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      MCTRL_MCFG4_DEFAULT, 
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
//function to initialize memory address space constants
initialize_mctrl() {

  // --- calculate address spaces of the different memory banks

  //ROM 
  //FIXME: Size???
  rom_bk1_s = static_cast<uint32_t>(romaddr << 20);            // 's' --> 'start'
  rom_bk1_e = static_cast<uint32_t>((ioaddr + romaddr) << 19); // 'e' --> 'end'
  rom_bk2_s = rom_bk1_e + 1;
  rom_bk2_e = static_cast<uint32_t>(ioaddr << 20) - 1;

  //IO
  //FIXME: size???
  io_s = static_cast<uint32_t>(ioaddr << 20);
  io_e = static_cast<uint32_t>(ioaddr << 21) - 1;

  // ------- RAM -------

  //SRAM bank size can be 8KB, 16KB, 32KB, ... 256MB
  uint32_t sram_bank_size = (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_RAM_BANK_SIZE) >> 9;

  //SDRAM bank size can be 4 MB, 8MB, 16MB, ... 512MB
  uint32_t sdram_bank_size = (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SDRAM_BANKSZ) >> 23;

  //address spaces in case of SRAM only configuration
  if (!(r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SE)) {
    //bank 1
    sram_bk1_s = static_cast<uint32_t>(ramaddr << 20);
    //                                       1Byte << 13 = 8KB
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
  else if (!(r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SI)) {
    //bank 1
    sram_bk1_s = static_cast<uint32_t>(ramaddr << 20);
    //                                       1Byte << 13 = 8KB
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

#ifdef DEBUG
  cout << endl << hex << "--- address space borders ---" << endl
       << "ROM_1:   " <<   rom_bk1_s << " - " <<   rom_bk1_e << endl
       << "ROM_2:   " <<   rom_bk2_s << " - " <<   rom_bk2_e << endl
       << "IO:      " <<        io_s << " - " <<        io_e << endl
       << "SRAM_1:  " <<  sram_bk1_s << " - " <<  sram_bk1_e << endl
       << "SRAM_2:  " <<  sram_bk2_s << " - " <<  sram_bk2_e << endl
       << "SRAM_3:  " <<  sram_bk3_s << " - " <<  sram_bk3_e << endl
       << "SRAM_4:  " <<  sram_bk4_s << " - " <<  sram_bk4_e << endl
       << "SRAM_5:  " <<  sram_bk5_s << " - " <<  sram_bk5_e << endl
       << "SDRAM_1: " << sdram_bk1_s << " - " << sdram_bk1_e << endl
       << "SDRAM_2: " << sdram_bk2_s << " - " << sdram_bk2_e << endl;
#endif

}



//-----------TLM--TRANSPORT--FUNCTIONS-----------//

//scope
PRE_MCTRL void Mctrl POST_MCTRL::
//blocking transport function
b_transport(tlm::tlm_generic_payload& gp, sc_time& delay)  {
  sc_core::sc_time cycle_time(BUS_CLOCK_CYCLE, SC_NS);
  uint8_t cycles = 0;

  //timing depends on command
  tlm::tlm_command cmd = gp.get_command();

  //access to ROM adress space
  if (Mctrl::rom_bk1_s <= gp.get_address() and gp.get_address() <= Mctrl::rom_bk1_e ||
      Mctrl::rom_bk2_s <= gp.get_address() and gp.get_address() <= Mctrl::rom_bk2_e    ) {
    //determine streaming width by MCFG1[9..8]
//  if (r[MCTRL_MCFG1].bit_get(9)) {
    if ( (r[MCTRL_MCFG1].get() & MCTRL_MCFG1_PROM_WIDTH) >> 9) {
      gp.set_streaming_width(4);
    }
//  else if (r[MCTRL_MCFG1].bit_get(8)) {
    else if (r[MCTRL_MCFG1].get() & MCTRL_MCFG1_PROM_WIDTH) {
      gp.set_streaming_width(2);
    }
    else {
      gp.set_streaming_width(1);
    }
    if (cmd == tlm::TLM_WRITE_COMMAND) {
      //PROM write access must be explicitly allowed
//    if (!r[MCTRL_MCFG1].bit_get(11)) {
      if ( !(r[MCTRL_MCFG1].get() & MCTRL_MCFG1_PWEN) ) {
        //issue error message / failure
        gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
        //address decoding delay only
        cycles = DECODING_DELAY;
      }
      //calculate delay for write command
      else {
        cycles = (r[MCTRL_MCFG1].get() & MCTRL_MCFG1_PROM_WRITE_WS) >> 4;
        cycles = DECODING_DELAY + MCTRL_ROM_WRITE_DELAY(cycles) + 
                 gp.get_data_length() / gp.get_streaming_width() - 1;  //multiple data cycles, i.e. burst access
        //add delay and forward transaction to memory
        delay += cycle_time * cycles;
        mctrl_rom->b_transport(gp,delay);
      }
    }
    //calculate delay for read command
    else if (cmd == tlm::TLM_READ_COMMAND) {
      cycles = (r[MCTRL_MCFG1].get() & MCTRL_MCFG1_PROM_READ_WS);
      cycles = DECODING_DELAY + MCTRL_ROM_READ_DELAY(cycles) + 
               gp.get_data_length() / gp.get_streaming_width() - 1;  //multiple data cycles, i.e. burst access
      //add delay and forward transaction to memory
      delay += cycle_time * cycles;
      mctrl_rom->b_transport(gp,delay);
    }
  }
  //access to IO adress space
  else if (Mctrl::io_s <= gp.get_address() and gp.get_address() <= Mctrl::io_e) {
    cycles = (r[MCTRL_MCFG1].get() & MCTRL_MCFG1_IO_WAITSTATES) >> 20;
    //calculate delay for read command
    if (cmd == tlm::TLM_READ_COMMAND) {
      cycles = DECODING_DELAY + MCTRL_IO_READ_DELAY(cycles) + 
               gp.get_data_length() / gp.get_streaming_width() - 1;  //multiple data cycles, i.e. burst access
    }
    //calculate delay for write command
    else if (cmd == tlm::TLM_WRITE_COMMAND) {
      cycles = DECODING_DELAY + MCTRL_IO_WRITE_DELAY(cycles) + 
               gp.get_data_length() / gp.get_streaming_width() - 1;  //multiple data cycles, i.e. burst access
    }
    //add delay and forward transaction to memory
    delay += cycle_time * cycles;
    gp.set_streaming_width(4);
    mctrl_io->b_transport(gp,delay);
  }
  //access to SRAM adress space
  else if (Mctrl::sram_bk1_s <= gp.get_address() and gp.get_address() <= Mctrl::sram_bk1_e ||
           Mctrl::sram_bk2_s <= gp.get_address() and gp.get_address() <= Mctrl::sram_bk2_e ||
           Mctrl::sram_bk3_s <= gp.get_address() and gp.get_address() <= Mctrl::sram_bk3_e ||
           Mctrl::sram_bk4_s <= gp.get_address() and gp.get_address() <= Mctrl::sram_bk4_e ||
           Mctrl::sram_bk5_s <= gp.get_address() and gp.get_address() <= Mctrl::sram_bk5_e    ) {
    //determine streaming width (below bit mask contains bits 5 and 4, so shift is required)
    if ( (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_RAM_WIDTH) >> 5 ) {
      gp.set_streaming_width(4);
    }
    else if (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_RAM_WIDTH) {
      gp.set_streaming_width(2);
    }
    else {
      gp.set_streaming_width(1);
    }
    //calculate delay for read command
    if (cmd == tlm::TLM_READ_COMMAND) {
      cycles = (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_RAM_READ_WS);
      cycles = DECODING_DELAY + MCTRL_SRAM_READ_DELAY(cycles) + 
               gp.get_data_length() / gp.get_streaming_width() - 1;  //multiple data cycles, i.e. burst access
    }
    //calculate delay for write command
    else if (cmd == tlm::TLM_WRITE_COMMAND) {
      cycles = (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_RAM_WRITE_WS >> 2);
      cycles = DECODING_DELAY + MCTRL_SRAM_WRITE_DELAY(cycles) + 
               gp.get_data_length() / gp.get_streaming_width() - 1;  //multiple data cycles, i.e. burst access
    }
    //add delay and forward transaction to memory
    delay += cycle_time * cycles;
    mctrl_sram->b_transport(gp,delay);
  }
  //access to SDRAM adress space
  else if (Mctrl::sdram_bk1_s <= gp.get_address() and gp.get_address() <= Mctrl::sdram_bk1_e ||
           Mctrl::sdram_bk2_s <= gp.get_address() and gp.get_address() <= Mctrl::sdram_bk2_e    ) {
    if (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_D64) {
      gp.set_streaming_width(8);
    }
    else {
      gp.set_streaming_width(4);
    }
    //read delay = write delay: trcd, tcas, and trp can all be either 2 or 3
    cycles = 6 + 
             gp.get_data_length() / gp.get_streaming_width() - 1;  //multiple data cycles, i.e. burst access;
    if (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_TCAS) {
      cycles += 2; //trcd = tcas = 3
    }
    if (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_TRP) {
      cycles++; //trp = 3
    }
    //add delay and forward transaction to memory
    delay += cycle_time * cycles;
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

//Modify and return generic payload

//State Machine management? Transaction can only be initialized
//in idle state.

}


#endif
