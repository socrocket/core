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

//constructor
Mctrl::Mctrl(sc_core::sc_module_name name,  int _hindex,    int _pindex,   int _romaddr,
                             int _rommask,  int _ioaddr,    int _iomask,   int _ramaddr,
                             int _rammask,  int _paddr,     int _pmask,    int _wprot,
                             int _srbanks,  int _ram8,      int _ram16,
                             int _sepbus,   int _sdbits,    int _mobile) :
  gr_device(
            name,                      //sc_module name
            gs::reg::ALIGNED_ADDRESS,  //address mode (options: aligned / indexed)
            16,                        //dword size (of register file)
            NULL                       //parent module
           ),
  pnpahb(
         (uint8_t) 0x04, //vendor: ESA
         (uint16_t) 0x0F, //device: MCTRL
         (uint8_t) 0,
         (uint8_t) 0,    //irq
         GrlibBAR(AHBMEM, rommask, true, true, romaddr),
         GrlibBAR(AHBMEM, iomask, false, false, ioaddr),
         GrlibBAR(AHBMEM, rammask, true, true, ramaddr),
         (uint32_t) 0
         ),
  pnpapb(
         (uint8_t) 0x04, //vendor: ESA
         (uint16_t) 0x0F, //device: MCTRL
         (uint8_t) 0,
         (uint8_t) 0,    //irq
         GrlibBAR(APBIO, pmask, 0, 0, paddr),
         (uint32_t) 0,
         (uint32_t) 0,
         (uint32_t) 0
        ),
  apb( //greenreg_socket
      "apb",            //name
      r,                //register container
      0x0,              // ?
      0xFFFFFFFF,       // ?
      ::amba::amba_APB, // ?bus type?
      ::amba::amba_LT,  // ?communication type / abstraction level?
      false             // ?
     ),
  ahb("ahb",
      ::amba::amba_AHB, // ?bus type?
      ::amba::amba_LT,  // ?communication type / abstraction level?
      false             // ?
     ),
  mctrl_rom("mctrl_rom"),
  mctrl_io("mctrl_io"),
  mctrl_sram("mctrl_sram"),
  mctrl_sdram("mctrl_sdram"),
  pmode(0),
  hindex   (_hindex),
  pindex   (_pindex),
  romaddr  (_romaddr),
  rommask  (_rommask),
  ioaddr   (_ioaddr),
  iomask   (_iomask),
  ramaddr  (_ramaddr),
  rammask  (_rammask),
  paddr    (_paddr),
  pmask    (_pmask),
  wprot    (_wprot),
  srbanks  (_srbanks),
  ram8     (_ram8),
  ram16    (_ram16),
  sepbus   (_sepbus),
  sdbits   (_sdbits),
  mobile   (_mobile)
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

//destructor unregisters callbacks
Mctrl::~Mctrl() {
  GC_UNREGISTER_CALLBACKS();
}


//register GreenReg callback after elaboration
void Mctrl::end_of_elaboration() {
  // create bit accessors for green registers
  r[MCTRL_MCFG2].br.create("lmr", 26, 26);    // tcas needs LMR command
  r[MCTRL_MCFG4].br.create("emr", 0, 6);      // DS, TCSR, PASR need EMR command
  r[MCTRL_MCFG2].br.create("launch", 19, 20); // SDRAM command field
  r[MCTRL_MCFG4].br.create("pmode", 16, 18);  // SDRAM power saving mode field
  r[MCTRL_MCFG2].br.create("si", 13, 13);     // SRAM disable, address space calculation
  r[MCTRL_MCFG2].br.create("se", 14, 14);     // SDRAM enable, address space calculation
  r[MCTRL_MCFG2].br.create("sr_bk", 9, 12);   // SRAM bank size
  r[MCTRL_MCFG2].br.create("sdr_bk", 23, 25); // SDRAM bank size

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
                                        gs::reg::POST_WRITE,      // function to be called after register write
                                        "erase_sdram",            // function name
                                        gs::reg::NOTIFY));        // notification on every register access

  GR_FUNCTION(Mctrl, sram_disable);
  GR_SENSITIVE(r[MCTRL_MCFG2].br["si"].add_rule(
                                        gs::reg::POST_WRITE,      // function to be called after register write
                                        "sram_disable",           // function name
                                        gs::reg::NOTIFY));        // notification on every register access

  GR_FUNCTION(Mctrl, sdram_enable);
  GR_SENSITIVE(r[MCTRL_MCFG2].br["se"].add_rule(
                                        gs::reg::POST_WRITE,      // function to be called after register write
                                        "sdram_enable",           // function name
                                        gs::reg::NOTIFY));        // notification on every register access

  GR_FUNCTION(Mctrl, sram_change_bank_size);
  GR_SENSITIVE(r[MCTRL_MCFG2].br["sr_bk"].add_rule(
                                        gs::reg::POST_WRITE,      // function to be called after register write
                                        "sram_change_bank_size",  // function name
                                        gs::reg::NOTIFY));        // notification on every register access

  GR_FUNCTION(Mctrl, sdram_change_bank_size);
  GR_SENSITIVE(r[MCTRL_MCFG2].br["sdr_bk"].add_rule(
                                        gs::reg::POST_WRITE,      // function to be called after register write
                                        "sdram_change_bank_size", // function name
                                        gs::reg::NOTIFY));        // notification on every register access

}


//function to initialize memory address space constants
void Mctrl::initialize_mctrl() {

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

  // --- set register values according to generics
  unsigned int set;
  if (sepbus) {
    set = r[MCTRL_MCFG2].get() | sdbits << 18;
    r[MCTRL_MCFG2].set( set );
  }

  // --- calculate address spaces of the different memory banks

  //ROM 
  uint32_t size = (4096 - rommask) << 20;
  rom_bk1_s = static_cast<uint32_t>(romaddr << 20); //   's' --> 'start'
  rom_bk2_e = rom_bk1_s + size - 1; //add full rom size: 'e' --> 'end'
  rom_bk1_e = rom_bk2_e >> 1;       //bk1 ends at half of address space, rounding fits
  rom_bk2_s = rom_bk1_e + 1;

  //IO
  size = (4096 - iomask) << 20;
  io_s = static_cast<uint32_t>(ioaddr << 20);
  io_e = io_s + size - 1;

  // ------- RAM -------

  //SRAM bank size: lower half of RAM address space divided by #banks
  uint32_t sram_bank_size;
  if (srbanks == 5) {              //max 4 banks in lower half
    sram_bank_size = ((4096 - rammask) / 8) << 20;
  }
  else {
    sram_bank_size = ((4096 - rammask) / (2*srbanks)) << 20;
  }

  //SDRAM bank size: upper half of RAM address space divided by 2 banks
  uint32_t sdram_bank_size = ((4096 - rammask) / 4) << 20;

  //write calculated bank sizes into MCFG2
  uint32_t i_sr  = static_cast<uint32_t>( log( sram_bank_size)/log(2) + 7 );
  uint32_t i_sdr = static_cast<uint32_t>( log(sdram_bank_size)/log(2) - 3 );
  set = (MCTRL_MCFG2_DEFAULT & ~MCTRL_MCFG2_RAM_BANK_SIZE & ~MCTRL_MCFG2_SDRAM_BANKSZ)
        | (i_sr << 9 & MCTRL_MCFG2_RAM_BANK_SIZE) | (i_sdr << 23 & MCTRL_MCFG2_SDRAM_BANKSZ);
  r[MCTRL_MCFG2].set( set );

  //address spaces in case of SRAM only configuration
  if (!(r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SE)) {
    //potentially unused banks
    sram_bk2_s = 0;
    sram_bk2_e = 0;
    sram_bk3_s = 0;
    sram_bk3_e = 0;
    sram_bk4_s = 0;
    sram_bk4_e = 0;
    sram_bk5_s = 0;
    sram_bk5_e = 0;

    //call function that calculates the SRAM bank addresses
    //This (lengthy) operation is required several times in the code.
    sram_calculate_bank_addresses( sram_bank_size );

    //unused banks, sdram1 and sdram2: constants need to be defined, but range must be empty
    sdram_bk1_s = 0;
    sdram_bk1_e = 0;
    sdram_bk2_s = 0;
    sdram_bk2_e = 0;

  }
  //address spaces in case of SDRAM and SRAM (lower 4 SRAM banks only) configuration
  else if (!(r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SI)) {
    //potentially unused banks: constants need to be defined, but range must be empty
    sram_bk2_s = 0;
    sram_bk2_e = 0;
    sram_bk3_s = 0;
    sram_bk3_e = 0;
    sram_bk4_s = 0;
    sram_bk4_e = 0;

    //call function that calculates the SRAM bank addresses
    sram_calculate_bank_addresses( sram_bank_size );

    //calculate SDRAM bank addresses
    //SDRAM bank 1 starts at upper half of RAM area
    sdram_bk1_s = static_cast<uint32_t>( (ramaddr + (4096 - rammask) / 2) << 20 );
    sdram_bk1_e = sdram_bk1_s + sdram_bank_size - 1;
    //SDRAM bank 2
    sdram_bk2_s = sdram_bk1_e + 1;
    sdram_bk2_e = sdram_bk2_s + sdram_bank_size - 1;

    //unused bank sram5: constants need to be defined, but range must be empty
    sram_bk5_s = 0;
    sram_bk5_e = 0;
  }
  // SDRAM only (located in lower half of RAM address space)
  else {
    //SDRAM bank 1 starts at lower half of RAM area
    sdram_bk1_s = static_cast<uint32_t>(ramaddr << 20);
    sdram_bk1_e = sdram_bk1_s + sdram_bank_size - 1;
    //SDRAM bank 2
    sdram_bk2_s = sdram_bk1_e + 1;
    sdram_bk2_e = sdram_bk2_s + sdram_bank_size - 1;

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

//blocking transport function
void Mctrl::b_transport(tlm::tlm_generic_payload& gp, sc_time& delay)  {
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
    else if (r[MCTRL_MCFG1].get() & MCTRL_MCFG1_PROM_WIDTH && ram16) {
      gp.set_streaming_width(2);
    }
    else if (ram8){
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
        start_idle = t_trans + cycle_time * cycles;
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
      start_idle = t_trans + cycle_time * cycles;
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
        //FIXME: add dynamic waitstates for IO access here
      }
      //calculate delay for write command
      else if (cmd == tlm::TLM_WRITE_COMMAND) {
        cycles = DECODING_DELAY + MCTRL_IO_WRITE_DELAY(cycles) + 
                 data_length / gp.get_streaming_width() - 1;  //multiple data cycles, i.e. burst access
      }
      //add delay and forward transaction to memory
      start_idle = t_trans + cycle_time * cycles;
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
    else if (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_RAM_WIDTH && ram16) {
      gp.set_streaming_width(2);
      //data length must match streaming width unless read-modify-write is enabled
      if (data_length < 2 && !(r[MCTRL_MCFG2].get() & MCTRL_MCFG2_RMW) ) {
        gp.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
        return;
      }
    }
    else if (ram8) {
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
    //check for write protection
    if (cmd == tlm::TLM_WRITE_COMMAND && wprot) {
      gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
      cycles = DECODING_DELAY;
      start_idle = t_trans + cycle_time * cycles;
      delay += cycle_time * cycles;
    }
    //add delay and forward transaction to memory
    else {
      start_idle = t_trans + cycle_time * cycles;
      delay += cycle_time * cycles;
      mctrl_sram->b_transport(gp,delay);
    }
  }
  //access to SDRAM adress space
  else if (Mctrl::sdram_bk1_s <= gp.get_address() and gp.get_address() <= Mctrl::sdram_bk1_e ||
           Mctrl::sdram_bk2_s <= gp.get_address() and gp.get_address() <= Mctrl::sdram_bk2_e    ) {

    //deep power down: memory is inactive and cannot be accessed
    //self refresh: system is powered down and should not even try to access memory
    //write protection: well... write protection.
    if ( mobile && (r[MCTRL_MCFG4].get() & MCTRL_MCFG4_ME) && (pmode == 5 || pmode == 2)
       | cmd == tlm::TLM_WRITE_COMMAND && wprot ) {
      gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
      cycles = DECODING_DELAY;
      delay += cycle_time * cycles;
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
  if (mobile>0 && sc_core::sc_time_stamp() - start_idle >= (r[MCTRL_MCFG3].get() >> 11) * cycle_time) {
    start_idle = sc_core::sc_time_stamp();
  }
}


//--------------CALLBACK--FUNCTIONS--------------//

//write into SDRAM_CMD field of MCFG2
void Mctrl::launch_sdram_command() {
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

//change of TCAS, DS, TCSR, or PASR
void Mctrl::configure_sdram() {
  //The reaction to the changes to these register fields is a command with a functionality 
  //transparent to the TLM memory system. However, the delay induced by this command can be modeled here.

  //one cycle to write the register + tRP (2+0 or 2+1) to let the changes take effect
  callback_delay += sc_time(BUS_CLOCK_CYCLE * (3 + (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_TRP) >> 30), SC_NS);

} 

//change of PMODE
void Mctrl::erase_sdram() {
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
        //leaving self refresh: tXSR + Auto Refresh cycle (tRFC)
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
    {
      //leaving deep power down mode: Precharge, 2x Auto-Refresh, LMR, EMR
      uint8_t cycles = 0;
      if (pmode == 5) {
        cycles = 2 + (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_TRP) >> 30            +  //precharge (tRP)
                 2 * (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SDRAM_TRFC >> 27) + 3 +  //2 * tRFC
                 2 * (3 + (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_TRP) >> 30) ;       //LMR + EMR
      }
      callback_delay += sc_time(BUS_CLOCK_CYCLE * cycles, SC_NS);
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

}

// --------- Address space calculation --------- //

//recalculate start / end addresses of ram banks after sram enable / disable
void Mctrl::sram_disable() {
  //changes only take effect if sdram enable bit is set
  if (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SE) {
    //SRAM has been disabled --> SDRAM needs to be mapped into lower half of RAM area
    if (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SI) {
      sdram_bk1_s -= ((4096 - rammask) / 2) << 20;
      sdram_bk1_e -= ((4096 - rammask) / 2) << 20;
      sdram_bk2_s -= ((4096 - rammask) / 2) << 20;
      sdram_bk2_e -= ((4096 - rammask) / 2) << 20;
      //in addition, the SRAM ranges need to be deleted
      sram_bk1_s = sram_bk1_e = 0;
      sram_bk2_s = sram_bk2_e = 0;
      sram_bk3_s = sram_bk3_e = 0;
      sram_bk4_s = sram_bk4_e = 0;
      sram_bk5_s = sram_bk5_e = 0;
    }
    //SRAM has been enabled --> SDRAM needs to be mapped into upper half of RAM area
    else {
      sdram_bk1_s += ((4096 - rammask) / 2) << 20;
      sdram_bk1_e += ((4096 - rammask) / 2) << 20;
      sdram_bk2_s += ((4096 - rammask) / 2) << 20;
      sdram_bk2_e += ((4096 - rammask) / 2) << 20;

      //calculate SRAM bank addresses based on MCFG2 bank size field
      //the GR callback is somehow misused for this pupose
      sram_change_bank_size();
    }
  }
}

//recalculate start / end addresses of ram banks after sdram enable / disable
void Mctrl::sdram_enable() {
  //SDRAM has just been enabled (SRAM cannot be disabled) 
  if(r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SE) {
    sram_bk5_s = 0;
    sram_bk5_e = 0;

    //calculate SDRAM bank addresses based on MCFG2 bank size field
    //the GR callback is somehow misused for this pupose
    sdram_change_bank_size();
  }
  //SDRAM has just been disabled
  else {
    sdram_bk1_s = 0;
    sdram_bk1_e = 0;
    sdram_bk2_s = 0;
    sdram_bk2_e = 0;

    //enable SRAM if it is disabled
    if(r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SI) {
      unsigned int set = r[MCTRL_MCFG2].get() | MCTRL_MCFG2_SI;
      r[MCTRL_MCFG2].set( set );
      //calculate SRAM bank addresses based on MCFG2 bank size field
      sram_change_bank_size();
    }
  }
}

//recalculate start / end addresses of sram banks after change of sram bank size
void Mctrl::sram_change_bank_size() {
  //calculate sram bank size from MCFG2 register
  // 8KB = 1B << 13
  //16KB = 1B << 13 + b'0001
  //32KB = 1B << 13 + b'0010 ...
  uint32_t sram_bank_size = 1 << ( 13 + (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_RAM_BANK_SIZE >> 9) );
  //calculate new bank addresses
  sram_calculate_bank_addresses( sram_bank_size );
}

//recalculate start / end addresses of sdram banks after change of sdram bank size
void Mctrl::sdram_change_bank_size() {
  //calculate sdram bank size from MCFG2 register
  // 4MB = 1B << 22
  // 8MB = 1B << 22 + b'001
  //16MB = 1B << 22 + b'010 ...
  uint32_t sdram_bank_size = 1 << ( 22 + (r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SDRAM_BANKSZ >> 23) );

  //calculate new bank addresses
  sdram_bk1_s = sram_bk1_s + ((4096 - rammask) / 2) << 20;
  sdram_bk1_e = sdram_bk1_s + sdram_bank_size - 1;
  sdram_bk2_s = sdram_bk1_e + 1;
  sdram_bk2_e = sdram_bk2_s + sdram_bank_size - 1;
}

void Mctrl::sram_calculate_bank_addresses(uint32_t sram_bank_size) {
  //calculate bank 1 address from generics and bank size
  sram_bk1_s = static_cast<uint32_t>(ramaddr << 20);
  sram_bk1_e = sram_bk1_s + sram_bank_size - 1;

  //calculate remaining bank addresses if banks are present
  switch (srbanks) {
    //banks 2 - 4
    case 4:
      sram_bk4_s = sram_bk1_s + 3 * sram_bank_size;
      sram_bk4_e = sram_bk4_s + sram_bank_size - 1;
      sram_bk3_s = sram_bk1_s + 2 * sram_bank_size;
      sram_bk3_e = sram_bk3_s + sram_bank_size - 1;
      sram_bk2_s = sram_bk1_s + sram_bank_size;
      sram_bk2_e = sram_bk2_s + sram_bank_size - 1;
      break;
    //banks 2 and 3
    case 3:
      sram_bk3_s = sram_bk1_s + 2 * sram_bank_size;
      sram_bk3_e = sram_bk3_s + sram_bank_size - 1;
      sram_bk2_s = sram_bk1_s + sram_bank_size;
      sram_bk2_e = sram_bk2_s + sram_bank_size - 1;
      break;
    //bank 2
    case 2:
      sram_bk2_s = sram_bk1_s + sram_bank_size;
      sram_bk2_e = sram_bk2_s + sram_bank_size - 1;
  }

  //set bank 5 values only if SDRAM is disabled
  if(r[MCTRL_MCFG2].get() & MCTRL_MCFG2_SE == 0) {
    sram_bk5_s = sram_bk1_s + ((4096 - rammask) / 2) << 20;
    sram_bk5_s = sram_bk1_s - 1 + (4096 - rammask) << 20;
  }

}


#endif






