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
  mctrl_sdram("mctrl_sdram"),
  pmode(0)
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
//register GreenReg callback after elaboration
end_of_elaboration() {
  // create bit accessors for green registers
  r[MCTRL_MCFG2].br.create("lmr", 26, 26);    // tcas needs LMR command
  r[MCTRL_MCFG4].br.create("emr", 0, 6);      // DS, TCSR, PASR need EMR command
  r[MCTRL_MCFG2].br.create("launch", 19, 20); // SDRAM command field
  r[MCTRL_MCFG4].br.create("pmode", 16, 18);  // SDRAM power saving mode field

  GR_FUNCTION(Mctrl, configure_sdram);                  // args: module name, callback function name
  GR_SENSITIVE(r[MCTRL_MCFG2].br["lmr"].add_rule(
                                        gs::reg::POST_WRITE,  // function to be called after register write
                                        "configure_sdram",    // function name
                                        gs::reg::NOTIFY));    // notification on every register access
  GR_SENSITIVE(r[MCTRL_MCFG4].br["emr"].add_rule(
                                        gs::reg::POST_WRITE,
                                        "configure_sdram",
                                        gs::reg::NOTIFY));

  GR_FUNCTION(Mctrl, launch_sdram_command);
  GR_SENSITIVE(r[MCTRL_MCFG2].br["launch"].add_rule(
                                        gs::reg::POST_WRITE,     // function to be called after register write
                                        "launch_sdram_command",  // function name
                                        gs::reg::NOTIFY));       // notification on every register access

  GR_FUNCTION(Mctrl, erase_sdram);
  GR_SENSITIVE(r[MCTRL_MCFG4].br["pmode"].add_rule(
                                        gs::reg::POST_WRITE,  // function to be called after register write
                                        "erase_sdram",        // function name
                                        gs::reg::NOTIFY));    // notification on every register access
}


//scope
PRE_MCTRL void Mctrl POST_MCTRL::
//function to initialize memory address space constants
initialize_mctrl() {

  //reset callback delay
  callback_delay = SC_ZERO_TIME;

  //set default values of mobile SDRAM
  unsigned int mcfg;
  switch (mobile) {
    //case 0 is default value (set by initialization)
    case 1:
      //enable mobile SDRAM support
      mcfg = static_cast<unsigned int> (r[MCTRL_MCFG2].get() | MCTRL_MCFG2_MS);
      r[MCTRL_MCFG2].set( mcfg );
      break;
    case 2:
      //enable mobile SDRAM support
      mcfg = static_cast<unsigned int> (r[MCTRL_MCFG2].get() | MCTRL_MCFG2_MS);
      r[MCTRL_MCFG2].set( mcfg );
      //enable mobile SDRAM
      mcfg = static_cast<unsigned int> (r[MCTRL_MCFG4].get() | MCTRL_MCFG4_ME);
      r[MCTRL_MCFG4].set( mcfg );
    //Case 3 would be the same as 2 here, the difference being that 3 disables std SDRAM,
    //i.e. mobile cannot be disabled. This will be implemented wherever someone tries to
    //disable mobile SDRAM.
  }

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
  //add delay from previously activated callbacks
  delay += callback_delay;
  callback_delay = sc_core::SC_ZERO_TIME;

  //capture current system time for idle time calculation (required in power down mode)
  sc_core::sc_time t_trans = sc_core::sc_time_stamp();

  //prepare further delay calculation
  sc_core::sc_time cycle_time(BUS_CLOCK_CYCLE, SC_NS);
  uint8_t cycles = 0;

  //gp parameters required for delay calculation
  tlm::tlm_command cmd = gp.get_command();
  uint8_t data_length = data_length;

  //access to ROM adress space
  if (Mctrl::rom_bk1_s <= gp.get_address() and gp.get_address() <= Mctrl::rom_bk1_e ||
      Mctrl::rom_bk2_s <= gp.get_address() and gp.get_address() <= Mctrl::rom_bk2_e    ) {
    //determine streaming width by MCFG1[9..8]
    if ( (r[MCTRL_MCFG1].get() & MCTRL_MCFG1_PROM_WIDTH) >> 9) {
      gp.set_streaming_width(4);
    }
    else if (r[MCTRL_MCFG1].get() & MCTRL_MCFG1_PROM_WIDTH) {
      gp.set_streaming_width(2);
    }
    else {
      gp.set_streaming_width(1);
    }
    if (cmd == tlm::TLM_WRITE_COMMAND) {
      //PROM write access must be explicitly allowed
      if ( !(r[MCTRL_MCFG1].get() & MCTRL_MCFG1_PWEN) ) {
        //issue error message / failure
        gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
        //address decoding delay only
        delay += cycle_time;
      }
      //calculate delay for write command
      else {
        cycles = (r[MCTRL_MCFG1].get() & MCTRL_MCFG1_PROM_WRITE_WS) >> 4;
        cycles = DECODING_DELAY + MCTRL_ROM_WRITE_DELAY(cycles) + 
                 data_length / gp.get_streaming_width() - 1;  //multiple data cycles, i.e. burst access
        //add delay and forward transaction to memory
        delay += cycle_time * cycles;
        mctrl_rom->b_transport(gp,delay);
      }
    }
    //calculate delay for read command
    else if (cmd == tlm::TLM_READ_COMMAND) {
      cycles = (r[MCTRL_MCFG1].get() & MCTRL_MCFG1_PROM_READ_WS);
      cycles = DECODING_DELAY + MCTRL_ROM_READ_DELAY(cycles) + 
               data_length / gp.get_streaming_width() - 1;  //multiple data cycles, i.e. burst access
      //add delay and forward transaction to memory
      delay += cycle_time * cycles;
      mctrl_rom->b_transport(gp,delay);
    }
  }
  //access to IO adress space
  else if (Mctrl::io_s <= gp.get_address() and gp.get_address() <= Mctrl::io_e) {
    //IO enable bit set?
    if (r[MCTRL_MCFG1].get() & MCTRL_MCFG1_IOEN) {
      cycles = (r[MCTRL_MCFG1].get() & MCTRL_MCFG1_IO_WAITSTATES) >> 20;
      //calculate delay for read command
      if (cmd == tlm::TLM_READ_COMMAND) {
        cycles = DECODING_DELAY + MCTRL_IO_READ_DELAY(cycles) + 
                 data_length / gp.get_streaming_width() - 1;  //multiple data cycles, i.e. burst access
      }
      //calculate delay for write command
      else if (cmd == tlm::TLM_WRITE_COMMAND) {
        cycles = DECODING_DELAY + MCTRL_IO_WRITE_DELAY(cycles) + 
                 data_length / gp.get_streaming_width() - 1;  //multiple data cycles, i.e. burst access
      }
      //add delay and forward transaction to memory
      delay += cycle_time * cycles;
      gp.set_streaming_width(4);
      mctrl_io->b_transport(gp,delay);

    }
    else {
      //IO enable not set: issue error message / failure
      gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
      //address decoding delay only
      delay += cycle_time;
    }
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
      //data length must match streaming width unless read-modify-write is enabled
      if (data_length < 4 && !(r[MCTRL_MCFG2].get() & MCTRL_MCFG2_RMW) ) {
        gp.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
        return;
      }
    }
    else if (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_RAM_WIDTH) {
      gp.set_streaming_width(2);
      //data length must match streaming width unless read-modify-write is enabled
      if (data_length < 2 && !(r[MCTRL_MCFG2].get() & MCTRL_MCFG2_RMW) ) {
        gp.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
        return;
      }
    }
    else {
      gp.set_streaming_width(1);
    }
    //calculate delay for read command
    if (cmd == tlm::TLM_READ_COMMAND) {
      cycles = (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_RAM_READ_WS);
      cycles = DECODING_DELAY + MCTRL_SRAM_READ_DELAY(cycles) + 
               data_length / gp.get_streaming_width() - 1;  //multiple data cycles, i.e. burst access
    }
    //calculate delay for write command
    else if (cmd == tlm::TLM_WRITE_COMMAND) {
      cycles = (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_RAM_WRITE_WS >> 2);
      cycles = DECODING_DELAY + MCTRL_SRAM_WRITE_DELAY(cycles) + 
               data_length / gp.get_streaming_width() - 1;  //multiple data cycles, i.e. burst access
    }
    //add delay and forward transaction to memory
    delay += cycle_time * cycles;
    mctrl_sram->b_transport(gp,delay);
  }
  //access to SDRAM adress space
  else if (Mctrl::sdram_bk1_s <= gp.get_address() and gp.get_address() <= Mctrl::sdram_bk1_e ||
           Mctrl::sdram_bk2_s <= gp.get_address() and gp.get_address() <= Mctrl::sdram_bk2_e    ) {

    //deep power down: memory is inactive and cannot be accessed
    //self refresh: system is powered down and should not even try to access memory
    if ( mobile && (r[MCTRL_MCFG4].get() & MCTRL_MCFG4_ME) && (pmode == 5 || pmode == 2) ) {
      gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    }
    //no deep power down status, so regular access is possible
    else {
      if (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_D64) {
        gp.set_streaming_width(8);
      }
      else {
        gp.set_streaming_width(4);
      }
      //calculate read delay: trcd, tcas, and trp can all be either 2 or 3
      if (cmd == tlm::TLM_READ_COMMAND) {
        cycles = 6;
        if (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_TCAS) {
          cycles += 2; //trcd = tcas = 3
        }
        if (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_TRP) {
          cycles++; //trp = 3
        }
        //calculate number of activated rows during burst access:
        // 1. calculate row length from bank size and 'column size' field (which in 
        //    fact determines the number of column address bits, i.e. the row length)
        uint32_t sdram_row_length;
                //bank size of 512MB activates col-sz field of MCFG2; any other bank size causes default col-sz of 2048
        switch ( sdram_bk1_e - sdram_bk1_s + 1 ) {
          case 0x20000000:
            sdram_row_length = 256 << (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SDRAM_COSZ >> 21);
            sdram_row_length *= (sdram_row_length == 2048) ? (2 * gp.get_streaming_width()) : (gp.get_streaming_width());
            break;
          default:
            sdram_row_length = 2048 * gp.get_streaming_width();
        }
        // 2. get number of activated rows from data address, data length, and row length
        uint8_t additional_rows = (gp.get_address() + data_length / sdram_row_length) - (gp.get_address() / sdram_row_length);
        cycles += cycles * (additional_rows);
        //In addition to opening and closing rows, the words must be transmitted. Word 1 was transmitted within CAS delay.
        cycles += data_length / gp.get_streaming_width() - 1;
      }
      //calculate write delay (bus write burst is transformed into burst of writes)
      else if (cmd == tlm::TLM_WRITE_COMMAND) {
        cycles = 6;
        if (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_TCAS) {
          cycles += 2; //trcd = tcas = 3
        }
        if (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_TRP) {
          cycles++; //trp = 3
        }
        //every write transaction needs the entire write access time
        cycles *= data_length / gp.get_streaming_width();
      }
      //if in power down mode, each access will take +1 clock cycle
      if (mobile>0 && sc_core::sc_time_stamp() - start_idle >=16 * cycle_time) {
                    //is mobile SDRAM enabled?
        cycles += ( (r[MCTRL_MCFG4].get() & MCTRL_MCFG4_ME >> 15) & 
                   //is mobile SDRAM allowed?
                   r[MCTRL_MCFG2].get() & 
                   //power down mode?    --> shift to LSB (=1)
                   r[MCTRL_MCFG4].get() ) >> 16;
      }
      //add delay and forward transaction to memory
      cycles += DECODING_DELAY;
      delay += cycle_time * cycles;
      start_idle = t_trans + cycle_time * cycles;
      mctrl_sdram->b_transport(gp,delay);
    }
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


//--------------CALLBACK--FUNCTIONS--------------//

//scope
PRE_MCTRL void Mctrl POST_MCTRL::
//write into SDRAM_CMD field of MCFG2
launch_sdram_command() {
  uint8_t cmd = ( (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SDRAM_CMD >> 19) & 0x000000FF);
  switch (cmd) {
    // LMR / EMR
   case 3:
      // LMR / EMR commands are assumed to be issued right after changes of TCAS, DS, TCSR, PASR.
      // The delay has already been added in the according callback (configure_sdram).
      break;
    // Auto-Refresh: Forces a refresh, which needs idle state! How can that be guaranteed?
    //               Refresh asap, i.e. right after termination of active command? Always send a precharge before Refresh?
    //               Whatever, for LT it's as simple as waiting for exactly one refresh cycle:
    //           --> The previous transaction will always have finished before the Sim Kernel takes note of this callback.
    case 2:
        callback_delay += sc_time(BUS_CLOCK_CYCLE * (3 + MCTRL_MCFG2_SDRAM_TRFC_DEFAULT >> 30), SC_NS);
      break;
    // Precharge: Terminate current burst transaction (no effect in LT) --> wait for tRP
    case 1:
      callback_delay += sc_time(BUS_CLOCK_CYCLE * (2 + MCTRL_MCFG2_TRP_DEFAULT >> 29), SC_NS);
      break;
  }
  //clear command bits
  unsigned int set = static_cast<unsigned int> (r[MCTRL_MCFG2].get() & ~MCTRL_MCFG2_SDRAM_CMD);
  r[MCTRL_MCFG2].set( set );
}

//scope
PRE_MCTRL void Mctrl POST_MCTRL::
//change of TCAS, DS, TCSR, or PASR
configure_sdram() {
  //The reaction to the changes to these register fields is a command with a functionality 
  //transparent to the TLM memory system. However, the delay induced by this command can be modeled here.

  //one cycle to write the register + tRP (2+0 or 2+1) to let the changes take effect
  callback_delay += sc_time(BUS_CLOCK_CYCLE * (3 + (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_TRP) >> 30), SC_NS);

} 

//scope
PRE_MCTRL void Mctrl POST_MCTRL::
//change of PMODE
erase_sdram() {
  //prepare transaction, including erase extension
  sc_core::sc_time t;
  uint32_t data = sdram_bk1_e;
  ext_erase* erase = new ext_erase;
  tlm::tlm_generic_payload gp;
    gp.set_command(tlm::TLM_WRITE_COMMAND);
    gp.set_streaming_width(4);
    gp.set_data_length(4);
    gp.set_data_ptr((unsigned char*) &data);
    gp.set_extension (erase);

  switch (r[MCTRL_MCFG4].get() & MCTRL_MCFG4_PMODE) {
    case 0x00000000:
    { 
      //check previous power down mode
      uint8_t cycles = 0;
      switch (pmode) {
        //leaving self refresh: tXSR + Auto Refresh period (tRFC)
        case 2:
          cycles = (r[MCTRL_MCFG4].get() & MCTRL_MCFG4_TXSR >> 20) +          //tXSR
                   (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SDRAM_TRFC >> 27) + 3; //tRFC
          break;
        //leaving deep power down mode: Precharge, 2x Auto-Refresh, LMR, EMR
        case 5:
          cycles = 2 + (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_TRP) >> 30            +  //precharge (tRP)
                   2 * (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SDRAM_TRFC >> 27) + 3 +  //2 * tRFC
                   2 * (3 + (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_TRP) >> 30) ;       //LMR + EMR
      }
      callback_delay += sc_time(BUS_CLOCK_CYCLE * cycles, SC_NS);
      pmode = 0;
      break;
    }
    //deep power down: erase entire SDRAM
    case 0x00050000:
      pmode = 5;
      //erase bank 1
      gp.set_address(sdram_bk1_s);
      mctrl_sdram->b_transport(gp,t);
      //erase bank 2
      gp.set_address(sdram_bk2_s);
      data = sdram_bk2_e;
      mctrl_sdram->b_transport(gp,t);
      break;
    //partial array self refresh: partially erase SDRAM
    case 0x00020000:
      pmode = 2;
      uint8_t pasr = r[MCTRL_MCFG4].get() & MCTRL_MCFG4_PASR;
      if (pasr) {
        //pasr enabled --> half array max --> always erase bank 2
        gp.set_address(sdram_bk2_s);
        data = sdram_bk2_e;
        mctrl_sdram->b_transport(gp,t);
        //partially erase lower bank according to PASR bits 
        gp.set_address(sdram_bk1_s);
        data = Mctrl::sdram_bk1_e >> (pasr-1);
        mctrl_sdram->b_transport(gp,t);
      }
  }

}

#endif
