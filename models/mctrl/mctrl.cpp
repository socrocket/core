//*********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is 
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      mctrl.tpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    implementation of the mctrl module
//             is included by mctrl.h template header file
//
// Method:
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Dennis Bode
// Reviewed:
//*********************************************************************

#include "mctrl.h"

using namespace sc_core;
using namespace tlm;

//constructor
Mctrl::Mctrl(sc_module_name name, int _romasel, int _sdrasel,
             int _romaddr, int _rommask, int _ioaddr, int _iomask,
             int _ramaddr, int _rammask, int _paddr, int _pmask, int _wprot,
             int _srbanks, int _ram8, int _ram16, int _sepbus, int _sdbits,
             int _mobile, int _sden, unsigned int hindex, unsigned int pindex,
             bool _powermon) :
            gr_device(name,
                    gs::reg::ALIGNED_ADDRESS,
                    16,
                    NULL
            ),
            AHBDevice(
                    hindex,
                    0x04, //vendor: ESA
                    0x0F, //device: MCTRL
                    0,
                    0,    //irq
                    BAR(AHBDevice::AHBMEM, _rommask, true, true, _romaddr),
                    BAR(AHBDevice::AHBMEM, _iomask, false, false, _ioaddr),
                    BAR(AHBDevice::AHBMEM, _rammask, true, true, _ramaddr),
                    0),
            APBDevice(
                    pindex,
                    0x04, //vendor: ESA
                    0x0F, //device: MCTRL
                    0,
                    0,    //irq
                    APBDevice::APBIO, _pmask, 0, 0, _paddr),
            apb( //greenreg_socket
                    "apb", //name
                    r, //register container
                    APBDevice::get_base_addr_(), //apb base address
                    APBDevice::get_size_(),      //apb address space size
                    ::amba::amba_APB,            //bus type
                    ::amba::amba_LT,             //abstraction level
                    false //socket is not used for arbitration
            ),
            ahb("ahb", ::amba::amba_AHB, //bus type
                    ::amba::amba_LT, //abstraction level
                    false //socket is not used for arbitration
            ), 
            mctrl_rom("mctrl_rom"), mctrl_io("mctrl_io"), 
            mctrl_sram("mctrl_sram"), mctrl_sdram("mctrl_sdram"), 
            rst(&Mctrl::reset_mctrl, "RESET"), pmode(0), romasel(_romasel),
            sdrasel(_sdrasel), romaddr(_romaddr), rommask(_rommask), 
            ioaddr(_ioaddr), iomask(_iomask), ramaddr(_ramaddr), 
            rammask(_rammask), paddr(_paddr), pmask(_pmask), wprot(_wprot),
            srbanks(_srbanks), ram8(_ram8), ram16(_ram16), sepbus(_sepbus),
            sdbits(_sdbits), mobile(_mobile), sden(_sden), powermon(_powermon) {
    PM::registerIP(this, "mctrl", powermon);
    // Display APB slave information 
    v::info << this->name() << "APB slave @" << v::uint32 << apb.get_base_addr() 
            << " size: " << v::uint32 << apb.get_size() << " byte" << v::endl;

    //check consistency of address space generics
    //rom space in MByte: 4GB - masked area (rommask)
    //rom space in Byte: 2^(romasel + 1)
    //same for ram and sdrasel
    if((4096 - _rommask) != (1 << (_romasel - 19))) {
        v::error << this->name() << "Inconsistent address space parameters. " 
                 << "Check romasel and rom-addr/-mask parameter." << v::endl;
    }
    if((4096 - _rammask) != (1 << (_sdrasel - 19))) {
        v::error << this->name() << "Inconsistent address space parameters. " 
                 << "Check sdrasel vs. ram-addr/-mask parameter." << v::endl;
    }
    if(_romaddr < _ioaddr  && (_romaddr + 4096 - _rommask) > _ioaddr) {
        v::error << this->name() << "Inconsistent address space parameters. "
                 << "Check rom/io address and mask for overlaps." << v::endl;
    }

    if(_romaddr < _ramaddr && (_romaddr + 4096 - _rommask) > _ramaddr) {
        v::error << this->name() << "Inconsistent address space parameters. "
                 << "Check rom/ram address and mask for overlaps." << v::endl;
    }
    
    if(_ioaddr  < _romaddr && (_ioaddr  + 4096 - _iomask)  > _romaddr) {
        v::error << this->name() << "Inconsistent address space parameters. "
                 << "Check io/rom address and mask for overlaps." << v::endl;
    }
    
    if(_ioaddr  < _ramaddr && (_ioaddr  + 4096 - _iomask)  > _ramaddr) {
        v::error << this->name() << "Inconsistent address space parameters. "
                 << "Check io/ram address and mask for overlaps." << v::endl;
    }
    
    if(_ramaddr < _romaddr && (_ramaddr + 4096 - _rammask) > _romaddr) {
        v::error << this->name() << "Inconsistent address space parameters. "
                 << "Check *ram/rom address and mask for overlaps." << v::endl;
    }
    
    if(_ramaddr < _ioaddr  && (_ramaddr + 4096 - _rammask) > _ioaddr) {
        v::error << this->name() << "Inconsistent address space parameters. "
                 << "Check ram/io address and mask for overlaps." << v::endl;
    }


    // register transport functions to sockets
    ahb.register_b_transport(this, &Mctrl::b_transport);
    ahb.register_transport_dbg(this, &Mctrl::transport_dbg);

    // nb_transport to be added


  // create register | name + description
  r.create_register( "MCFG1", "Memory Configuration Register 1",
                   // offset
                      0x00,
                   // config
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | 
                      gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                   // init value (to be calculated from the generics for all 4 registers)
                      MCFG1_DEFAULT,
                   // write mask             
                   // FIXME: check consistency of ram8, 
                   //        ram 16 generics vs. default and 
                   //        mask of PROM WIDTH field
                      MCFG1_WRITE_MASK | _ram16 << 9 | 
                      _ram16 << 8 | _ram8 << 9,
                   // reg width (maximum 32 bit)
                      32,
                   // lock mask: Not implementet, has to be zero.
                      0x00
                   );
  r.create_register( "MCFG2", "Memory Configuration Register 2",
                      0x04,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | 
                      gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      MCFG2_DEFAULT,  
                      //FIXME: check consistency of ram8, 
                      //       ram 16 generics vs. default and 
                      //       mask of RAM WIDTH field
                      MCFG2_WRITE_MASK | _ram16 << 5 | 
                      _ram16 << 4 | _ram8 << 5,
                      32,
                      0x00
                   );
  r.create_register( "MCFG3", "Memory Configuration Register 3", 
                      0x08,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | 
                      gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      MCFG3_DEFAULT, 
                      MCFG3_WRITE_MASK,
                      32, 
                      0x00
                   );
  r.create_register( "MCFG4", "Power-Saving Configuration Register", 
                      0x0C,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | 
                      gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      MCFG4_DEFAULT, 
                      MCFG4_WRITE_MASK,
                      32,
                      0x00
                   );
}

//destructor unregisters callbacks
Mctrl::~Mctrl() {
    GC_UNREGISTER_CALLBACKS();
}

//register GreenReg callback after elaboration
void Mctrl::end_of_elaboration() {
  // create bit accessors for green registers
  // callbacks can then be registers on the defined bits only instead 
  // of the entire register
  // arguments: br.create(name, start bit, end bit)
  r[MCFG2].br.create("lmr", 26, 26);      // tcas needs LMR command
  r[MCFG4].br.create("emr", 0, 6);        // DS, TCSR, PASR need EMR command
  r[MCFG2].br.create("launch", 19, 20);   // SDRAM command field
  r[MCFG4].br.create("pmode", 16, 18);    // SDRAM power saving mode field
  r[MCFG2].br.create("si", 13, 13);       // SRAM disable, address space calculation
  r[MCFG2].br.create("se", 14, 14);       // SDRAM enable, address space calculation
  r[MCFG2].br.create("sr_bk", 9, 12);     // SRAM bank size
  r[MCFG2].br.create("sdr_bk", 23, 25);   // SDRAM bank size
  r[MCFG2].br.create("sdr_trfc", 27, 29); // SDRAM refresh cycle

    //register callbacks
    GR_FUNCTION(Mctrl, sram_disable);
    GR_SENSITIVE(r[MCFG2].br["si"].add_rule(gs::reg::POST_WRITE, // function to be called after register write
          "sram_disable", gs::reg::BIT_STATE_CHANGE)); // notification on bit state change only

    GR_FUNCTION(Mctrl, sram_change_bank_size);
    GR_SENSITIVE(r[MCFG2].br["sr_bk"].add_rule(gs::reg::POST_WRITE,
          "sram_change_bank_size", gs::reg::BIT_STATE_CHANGE));


    //The following callbacks affect SDRAM only and are therefore not required if SDRAM is disabled
    if (sden) {
        GR_FUNCTION(Mctrl, configure_sdram);
        GR_SENSITIVE(r[MCFG2].br["lmr"].add_rule(gs::reg::POST_WRITE, 
              "configure_sdram", gs::reg::NOTIFY));
        GR_SENSITIVE(r[MCFG4].br["emr"].add_rule(gs::reg::POST_WRITE,
              "configure_sdram", gs::reg::NOTIFY));

        GR_FUNCTION(Mctrl, launch_sdram_command);
        GR_SENSITIVE(r[MCFG2].br["launch"].add_rule(gs::reg::POST_WRITE,
              "launch_sdram_command", gs::reg::BIT_STATE_CHANGE));

        GR_FUNCTION(Mctrl, erase_sdram);
        GR_SENSITIVE(r[MCFG4].br["pmode"].add_rule(gs::reg::POST_WRITE,
              "erase_sdram", gs::reg::BIT_STATE_CHANGE));

        GR_FUNCTION(Mctrl, sdram_enable);
        GR_SENSITIVE(r[MCFG2].br["se"].add_rule(gs::reg::POST_WRITE,
              "sdram_enable", gs::reg::BIT_STATE_CHANGE));

        GR_FUNCTION(Mctrl, sdram_change_bank_size);
        GR_SENSITIVE(r[MCFG2].br["sdr_bk"].add_rule(gs::reg::POST_WRITE,
              "sdram_change_bank_size",
              gs::reg::BIT_STATE_CHANGE));

        GR_FUNCTION(Mctrl, sdram_change_refresh_cycle);
        GR_SENSITIVE(r[MCFG2].br["sdr_trfc"].add_rule(gs::reg::POST_WRITE, 
              "sdram_change_refresh_cycle", gs::reg::BIT_STATE_CHANGE));
    } //if sden

    //initialize mctrl according to generics
    reset_mctrl(false, SC_ZERO_TIME);
}

//function to initialize and reset memory address space constants
void Mctrl::reset_mctrl(const bool &value, const sc_time &time) {

    //low active reset
    if(!value) {
        //reset callback delay
        callback_delay = SC_ZERO_TIME;

        //set refresh cycle and schedule first refresh
        trfc = 3 + (r[MCFG2] & MCFG2_SDRAM_TRFC) >> 27;
        next_refresh = cycle_time * (r[MCFG3] >> 11);

        //set default values of mobile SDRAM
        if (sden) {
            uint32_t mcfg;
            switch (mobile) {
                //case 0 is default value (set by initialization)
                case 1:
                    //enable mobile SDRAM support
                    mcfg = static_cast<uint32_t> (r[MCFG2] | MCFG2_MS);
                    r[MCFG2].set(mcfg);
                    break;
                case 2:
                    //enable mobile SDRAM support
                    mcfg = static_cast<uint32_t> (r[MCFG2] | MCFG2_MS);
                    r[MCFG2].set(mcfg);
                    //enable mobile SDRAM
                    mcfg = static_cast<uint32_t> (r[MCFG4] | MCFG4_ME);
                    r[MCFG4].set(mcfg);
                    break;
                    // Case 3 would be the same as 2 here, 
                    // the difference being that 3 disables std SDRAM,
                    //i.e. mobile cannot be disabled. 
                    //This will be implemented wherever someone tries to
                    //disable mobile SDRAM.
                default:;
		  break;
            }
        }

        // --- set register values according to generics
        uint32_t set;
        if(sden) {
            set = r[MCFG2] | MCFG2_SDRF | MCFG2_SE;
            if (sepbus) {
                set |= sdbits << 18;
                r[MCFG2] = set;
            }
        }

        // --- calculate address spaces of the different memory banks

        //ROM
        uint32_t size = (4096 - rommask) << 20;
        rom_bk1_s = static_cast<uint32_t> (romaddr << 20); // 's' --> 'start'
        rom_bk2_e = rom_bk1_s + size - 1; //add full rom size: 'e' --> 'end'
        rom_bk1_e = (rom_bk1_s + rom_bk2_e) >> 1; 
        //bk1 ends at half of address space, rounding fits
        
        rom_bk2_s = rom_bk1_e + 1;

        //IO
        size = (4096 - iomask) << 20;
        io_s = static_cast<uint32_t> (ioaddr << 20);
        io_e = io_s + size - 1;

        // ------- RAM -------

        //SRAM bank size: lower half of RAM address space divided by #banks
        uint32_t sram_bank_size;
        if(srbanks == 5) { //max 4 banks in lower half
            sram_bank_size = ((0x1000 - rammask) / 8) << 20;
        } else {
            sram_bank_size = ((4096 - rammask) / (2 * srbanks)) << 20;
        }
        v::debug << name() << "SRAM bank size calculated from rammask and "
                 << "srbanks: " << v::uint32 << sram_bank_size << v::endl;
        //SDRAM bank size: upper half of RAM address space divided by 2 banks
        uint32_t sdram_bank_size = ((4096 - rammask) / 4) << 20;

        //write calculated bank sizes into MCFG2
        //SRAM: "0000" --> 8KByte <-- 2^13Byte
        uint32_t i_sr = static_cast<uint32_t>(log(sram_bank_size+1) / log(2) - 13);
        //SDRAM: "000" --> 4MByte <-- 2^22Bte
        uint32_t i_sdr = static_cast<uint32_t>(log(sdram_bank_size) / log(2) - 22);
        set = (MCFG2_DEFAULT & ~MCFG2_RAM_BANK_SIZE & ~MCFG2_SDRAM_BANKSZ) | 
              ((i_sr << 9) & MCFG2_RAM_BANK_SIZE) | 
              ((i_sdr << 23) & MCFG2_SDRAM_BANKSZ);
        r[MCFG2].set(set);
        v::debug << name() << "Value stored to MCFG2 field 'SRAM Bank Size': " 
                 << v::uint32 << i_sr << std::endl;

        // SRAM bank size has to be a power of two, 
        // which is not necessarily the case here.
        // Certain combinations of rammask and srbanks
        // might have caused odd numbers here, so
        // the SRAM bank size is re-calculated from the register values.
        sram_bank_size = 1 << (13 + ((r[MCFG2] & MCFG2_RAM_BANK_SIZE) >> 9));
        v::debug << name() << "SRAM bank size resulting from MCFG2: "
                 << v::uint32 << sram_bank_size << std::endl;

        // same for SDRAM bank size (in case a strange rammask value has 
        // been given)
        sdram_bank_size = 1 << (22 + ((r[MCFG2] & MCFG2_SDRAM_BANKSZ) >> 23));

        //address spaces in case of SRAM only configuration
        if(!sden || !(r[MCFG2] & MCFG2_SE)) {
            //potentially unused banks
            sram_bk2_s = 0;
            sram_bk2_e = 0;
            sram_bk3_s = 0;
            sram_bk3_e = 0;
            sram_bk4_s = 0;
            sram_bk4_e = 0;
            sram_bk5_s = 0;
            sram_bk5_e = 0;

            // call function that calculates the SRAM bank addresses
            // This (lengthy) operation is required several times in the code.
            sram_calculate_bank_addresses(sram_bank_size);

            // unused banks, sdram1 and sdram2: constants need to be defined, 
            // but range must be empty
            sdram_bk1_s = 0;
            sdram_bk1_e = 0;
            sdram_bk2_s = 0;
            sdram_bk2_e = 0;

        } else if(!(r[MCFG2] & MCFG2_SI)) {
        // address spaces in case of SDRAM and SRAM (lower 4 SRAM banks only) 
        // configuration
            // potentially unused banks: constants need to be defined, 
            // but range must be empty
            sram_bk2_s = 0;
            sram_bk2_e = 0;
            sram_bk3_s = 0;
            sram_bk3_e = 0;
            sram_bk4_s = 0;
            sram_bk4_e = 0;

            // call function that calculates the SRAM bank addresses
            sram_calculate_bank_addresses(sram_bank_size);

            // calculate SDRAM bank addresses
            // SDRAM bank 1 starts at upper half of RAM area
            sdram_bk1_s = static_cast<uint32_t> ((ramaddr + (4096 - rammask)
                    / 2) << 20);
            sdram_bk1_e = sdram_bk1_s + sdram_bank_size - 1;
            // SDRAM bank 2
            sdram_bk2_s = sdram_bk1_e + 1;
            sdram_bk2_e = sdram_bk2_s + sdram_bank_size - 1;

            // unused bank sram5: constants need to be defined, 
            // but range must be empty
            sram_bk5_s = 0;
            sram_bk5_e = 0;
        } else {
        // SDRAM only (located in lower half of RAM address space)
            //SDRAM bank 1 starts at lower half of RAM area
            sdram_bk1_s = static_cast<uint32_t> (ramaddr << 20);
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
    }

    v::info << name() << v::endl << "--- address space borders ---" << v::endl
            << "ROM_1:   " << v::uint32 <<   rom_bk1_s 
                  << " - " << v::uint32 <<   rom_bk1_e << v::endl
            << "ROM_2:   " << v::uint32 <<   rom_bk2_s 
                  << " - " << v::uint32 <<   rom_bk2_e << v::endl
            << "IO:      " << v::uint32 <<        io_s 
                  << " - " << v::uint32 <<        io_e << v::endl
            << "SRAM_1:  " << v::uint32 <<  sram_bk1_s 
                  << " - " << v::uint32 <<  sram_bk1_e << v::endl
            << "SRAM_2:  " << v::uint32 <<  sram_bk2_s 
                  << " - " << v::uint32 <<  sram_bk2_e << v::endl
            << "SRAM_3:  " << v::uint32 <<  sram_bk3_s 
                  << " - " << v::uint32 <<  sram_bk3_e << v::endl
            << "SRAM_4:  " << v::uint32 <<  sram_bk4_s 
                  << " - " << v::uint32 <<  sram_bk4_e << v::endl
            << "SRAM_5:  " << v::uint32 <<  sram_bk5_s 
                  << " - " << v::uint32 <<  sram_bk5_e << v::endl
            << "SDRAM_1: " << v::uint32 << sdram_bk1_s 
                  << " - " << v::uint32 << sdram_bk1_e << v::endl
            << "SDRAM_2: " << v::uint32 << sdram_bk2_s 
                  << " - " << v::uint32 << sdram_bk2_e << v::endl;
}

//-----------TLM--TRANSPORT--FUNCTIONS-----------//

//blocking transport function
void Mctrl::b_transport(tlm_generic_payload& gp, sc_time& delay) {
    char *transaction = NULL;

    uint32_t addr = gp.get_address();

    //add delay from previously activated callbacks
    delay += callback_delay;

    //variable to count clock cycles for delay calculation
    uint8_t cycles = 0;

    //start of transaction (required for refresh delay and in power down mode)
    sc_time t_trans = sc_time_stamp() + callback_delay;

    //refresh handling required for sdram only (deactivated by default)
    //refresh handling can be deactivated by SDRF bit of MCFG2
    if (sden && (r[MCFG2] & MCFG2_SDRF)) {
        //next refresh must end in the future
        while (t_trans > next_refresh + cycle_time * trfc) {
            // start of sdram idle period must be later than end 
            // of last refresh
            if (start_idle < next_refresh + cycle_time * trfc) {
                start_idle = next_refresh + cycle_time * trfc;
            }
            //last refresh may have been stalled by an sdram access (see below)
            next_refresh += cycle_time * (r[MCFG3] >> 11) - refresh_stall;
            refresh_stall = SC_ZERO_TIME;
        }
    }

    //gp parameters required for delay calculation
    tlm_command cmd = gp.get_command();
    uint8_t data_length = gp.get_data_length();

    //get burst_size extension for later checks of consistency of burst_size, 
    //streaming_width, and data length 
    amba::amba_burst_size * amba_burst_size;
    if (!(ahb.get_extension<amba::amba_burst_size>(amba_burst_size, gp))) {
        amba_burst_size->value = 0;
    }
    // If present, the burst size extension of the gp is now stored 
    // in amba_burst_size.
    // Else, the burst size is set to zero, always indicating that 
    // the extension is not present.

    if (amba_burst_size->value) {
        //data length must be multiple of burst size
        if (data_length % amba_burst_size->value) {
            v::error << name() << "Data length (" << data_length 
                     << ") does not match burst_size (" 
                     << amba_burst_size->value << ")." << v::endl;
            //FIXME: add delay
            gp.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
            return;
        }
        //burst size must be 1, 2, or 4
        if(amba_burst_size->value > 4 || amba_burst_size->value == 3) {
            v::error << name() << "invalid value to amba_burst_size extension: "
                     << amba_burst_size->value << v::endl;
            //FIXME: add delay
            gp.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
            return;
        }
        //subword burst access does not make sense, so issue a warning
        if (amba_burst_size->value < 4 && amba_burst_size->value < data_length) {
            v::warn << name() << "subword burst access detected" << v::endl;
        }
    }
    //access to ROM adress space
    if(((rom_bk1_s <= addr) && (addr <= rom_bk1_e)) ||
       ((rom_bk2_s <= addr) && (addr <= rom_bk2_e))) {
        //determine streaming width by MCFG1[9..8]
        if(((r[MCFG1] & MCFG1_PROM_WIDTH)>>8)==0x2) {
            gp.set_streaming_width(4);
        } else if ((r[MCFG1] & MCFG1_PROM_WIDTH) && ram16) {
            gp.set_streaming_width(2);
        } else if (ram8){
            gp.set_streaming_width(1);
        } else {
            v::error << name() << "Attempted disallowed sub-word access to PROM" 
                     << v::endl;
            gp.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
            //FIXME: add delay?
            return;
        }
        //data length must be multiple of streaming width
        if(data_length % gp.get_streaming_width()) {
            v::error << name() << "PROM access: data length (" << data_length 
                     << ") does not match streaming width (" 
                     << gp.get_streaming_width() << ")." << v::endl;
            gp.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
            //FIXME: add delay?
            return;
        }
        if(cmd == TLM_WRITE_COMMAND) {
            //PROM write access must be explicitly allowed
            if(!(r[MCFG1] & MCFG1_PWEN)) {
                PM::send(this, "romwrite", 1, sc_time_stamp(), 0, powermon);
                //issue error message / failure
                gp.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
                //address decoding delay only
                delay += DECODING_DELAY * cycle_time;
                PM::send(this, "romwrite", 0, (sc_time_stamp()+delay), 0, powermon);
            } else {
            //calculate delay for write command
                PM::send(this, "romwrite", 1, sc_time_stamp(), 0, powermon);
                cycles = (r[MCFG1] & MCFG1_PROM_WRITE_WS) >> 4;
                cycles = DECODING_DELAY + ROM_WRITE_DELAY(cycles) + 
                         (data_length / gp.get_streaming_width() - 1);
                //multiple data cycles, i.e. burst access
                //add delay and forward transaction to memory
                start_idle = t_trans + cycle_time * cycles;
                delay += cycle_time * cycles;
                PM::send(this, "romwrite", 0, (sc_time_stamp()+delay), 0, powermon);
                mctrl_rom->b_transport(gp,delay);
            }
        } else if (cmd == TLM_READ_COMMAND) {
        //calculate delay for read command
            PM::send(this, "romread", 1, sc_time_stamp(), 0, powermon);
            cycles = (r[MCFG1] & MCFG1_PROM_READ_WS);
            cycles = DECODING_DELAY + ROM_READ_DELAY(cycles) + 
                     2 * (data_length / gp.get_streaming_width() - 1);  
            //multiple data1 / data2 cycles, i.e. burst access
            //add delay and forward transaction to memory
            start_idle = t_trans + cycle_time * cycles;
            delay += cycle_time * cycles;
            PM::send(this, "romread", 0, (sc_time_stamp()+delay), 0, powermon);
            mctrl_rom->b_transport(gp,delay);
            //set cacheable_access extension
            ahb.validate_extension<amba::amba_cacheable> (gp);
        }
    } else if ((io_s <= addr) && (addr <= io_e)) {
    //access to IO adress space
        //IO enable bit set?
        if (r[MCFG1] & MCFG1_IOEN) {
            //determine streaming width by MCFG1[9..8]
            if ( (r[MCFG1] & MCFG1_IOBUSW) >> 28) {
                gp.set_streaming_width(4);
            } else if (r[MCFG1] & MCFG1_IOBUSW) {
                gp.set_streaming_width(2);
            } else {
                gp.set_streaming_width(1);
            }
            cycles = (r[MCFG1] & MCFG1_IO_WAITSTATES) >> 20;
            //data length must match (multiple of) streaming width
            if (data_length % gp.get_streaming_width()) {
                v::error << name() << "I/O access: data length (" 
                         << data_length << ") does not match streaming width ("
                         << gp.get_streaming_width() << ")." << v::endl;
                gp.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
                //FIXME: add delay?
                return;
            }
            //calculate delay for read command
            if(cmd == TLM_READ_COMMAND) {
                transaction = "ioread";
                cycles = DECODING_DELAY + IO_READ_DELAY(cycles) + 
                         2 * (data_length / gp.get_streaming_width() - 1);
                //multiple data cycles, i.e. burst access
            } else if (cmd == TLM_WRITE_COMMAND) {
                transaction = "iowrite";
            //calculate delay for write command
                cycles = DECODING_DELAY + IO_WRITE_DELAY(cycles) + 
                         data_length / gp.get_streaming_width() - 1;
                //multiple data cycles, i.e. burst access
            }
            PM::send(this, transaction, 1, sc_time_stamp(), 0, powermon);
            //add delay and forward transaction to memory
            start_idle = t_trans + cycle_time * cycles;
            delay += cycle_time * cycles;
            gp.set_streaming_width(4);
            sc_time tmp = sc_time(delay);
            PM::send(this, transaction, 0, (sc_time_stamp()+delay), 0, powermon);
            mctrl_io->b_transport(gp,delay);
            if(tmp != delay && r[MCFG1] & MCFG1_IBRDY) {
                v::error << name() << "IO devices changed delay value, "
                         << "but IBRDY = 0. The change is undone by Mctrl, "
                         << "data is probably invalid." << v::endl;
                delay = tmp;
            }
        } else {
            //IO enable not set: issue error message / failure
            gp.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
            //address decoding delay only
            delay += DECODING_DELAY * cycle_time;
        }
    } else if(((sram_bk1_s <= addr) && (addr <= sram_bk1_e)) ||
              ((sram_bk2_s <= addr) && (addr <= sram_bk2_e)) ||
              ((sram_bk3_s <= addr) && (addr <= sram_bk3_e)) ||
              ((sram_bk4_s <= addr) && (addr <= sram_bk4_e)) ||
              ((sram_bk5_s <= addr) && (addr <= sram_bk5_e))) {
    //access to SRAM adress space
        //determine streaming width (below bit mask contains bits 5 and 4,
        // so shift is required)
        if((r[MCFG2] & MCFG2_RAM_WIDTH) >> 5) {
            gp.set_streaming_width(4);
            //data length must match streaming width unless 
            //read-modify-write is enabled
            if((data_length % 4) && !(r[MCFG2] & MCFG2_RMW)) {
                v::error << name() << "Attempted disallowed sub-word access "
                         << "to SRAM. Data length is " << data_length 
                         << ". RMW is disabled." << v::endl;
                gp.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
                //FIXME: add delay?
                return;
            }
        } else if((r[MCFG2] & MCFG2_RAM_WIDTH) && ram16) {
            gp.set_streaming_width(2);
            //data length must match streaming width unless 
            //read-modify-write is enabled
            if (data_length % 2 && !(r[MCFG2] & MCFG2_RMW) ) {
                v::error << name() << "Attempted disallowed byte access "
                         << "to SRAM. Data length is " << data_length
                         << ". RMW is disabled." << v::endl;
                gp.set_response_status(TLM_BYTE_ENABLE_ERROR_RESPONSE);
                //FIXME: add delay?
                return;
            }
        } else if (ram8) {
            gp.set_streaming_width(1);
        } else {
            v::error << name() << "Attempted disallowed byte access "
                     << "to SRAM. MCFG2 does not match ram8 / ram16 generics. "
                     << "This error should not occur." << std::endl;
        }
        //calculate delay for read command
        if (cmd == TLM_READ_COMMAND) {
            transaction = "sramread";
            cycles = (r[MCFG2] & MCFG2_RAM_READ_WS);
            cycles = DECODING_DELAY + SRAM_READ_DELAY(cycles) + 
                     2 * (data_length / gp.get_streaming_width() - 1) + 3;  
            //multiple data1 / data2 cycles, i.e. burst access
            
            //set cacheable_access extension (only required for read commands)
            ahb.validate_extension<amba::amba_cacheable> (gp);
        }else if(cmd == TLM_WRITE_COMMAND) {
        //calculate delay for write command
            transaction = "sramwrite";
            cycles = (r[MCFG2] & MCFG2_RAM_WRITE_WS) >> 2;
            cycles = DECODING_DELAY + SRAM_WRITE_DELAY(cycles) + 
                     data_length / gp.get_streaming_width() - 1 + 6;
            //multiple data cycles, i.e. burst access
        }
        //check for write protection
        if(cmd == TLM_WRITE_COMMAND && wprot) {
            transaction = "sramwrite";
            gp.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
            cycles = DECODING_DELAY;
            start_idle = t_trans + cycle_time * cycles;
            delay += cycle_time * cycles;
        }else {
        //add delay and forward transaction to memory
            PM::send(this, transaction, 1, sc_time_stamp(), 0, powermon);
            start_idle = t_trans + cycle_time * cycles;
            delay += cycle_time * cycles;
            sc_time tmp = sc_time(delay);
            PM::send(this, transaction, 0, (sc_time_stamp()+delay), 0, powermon);
            mctrl_sram->b_transport(gp,delay);
            if((tmp != delay) && (r[MCFG1] & MCFG2_RBRDY)) {
                v::error << name() << "RAM devices changed delay value, "
                         << "but RBRDY = 0. The change is undone by Mctrl, "
                         << "data is probably invalid." << v::endl;
                delay = tmp;
            }
        }
    } else if(((sdram_bk1_s <= addr) && (addr <= sdram_bk1_e)) ||
              ((sdram_bk2_s <= addr) && (addr <= sdram_bk2_e))) {
    //access to SDRAM adress space

        //deep power down: memory is inactive and cannot be accessed
        //self refresh: system is powered down and should not even try 
        //to access memory
        //write protection: well... write protection.
        if (mobile && (r[MCFG4] & MCFG4_ME) && (pmode == 5 || pmode == 2)
           || cmd == TLM_WRITE_COMMAND && wprot) {
            gp.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
            cycles = DECODING_DELAY;
            delay += cycle_time * cycles;
        } else {
        //no deep power down status, so regular access is possible
            //check 64 vs. 32 bit access
            if (r[MCFG2] & MCFG2_D64) {
                gp.set_streaming_width(8);
                //check for disallowed sub-word access
                if((data_length % 8) && !(r[MCFG2] & MCFG2_RMW)
                                     && cmd == TLM_WRITE_COMMAND) {
                    v::error << name() << "Attempted dis-allowed sub-word "
                             << "access to SDRAM in 64 bit mode." << v::endl;
                }
            } else {
                gp.set_streaming_width(4);
                //check for disallowed sub-word access
                if((data_length % 4) && !(r[MCFG2] & MCFG2_RMW)
                                     && cmd == TLM_WRITE_COMMAND) {
                    v::error << name() << "Attempted dis-allowed sub-word "
                             << "access to SDRAM in 32 bit mode. "
                             << "Data length is " << data_length 
                             << ", RMW is disabled." << v::endl;
                }
            }
            //calculate read delay: trcd, tcas, and trp can all 
            //be either 2 or 3
            cycles = 6;
            //cycles += 2 if trcd=tcas=3
            cycles += 2 * ((r[MCFG2] & MCFG2_TCAS) >> 26);
            //cycles += 1 if trp=3
            cycles += ((r[MCFG2] & MCFG2_TRP) >> 30);
            //calculate delay for read command
            if(cmd == TLM_READ_COMMAND) {
                transaction = "sdramread";
                cycles += (data_length / gp.get_streaming_width() - 1); 
                //multiple data cycles, i.e. burst access

                //calculate number of activated rows during burst access:
                // 1. calculate row length from bank size and 'column size' 
                // field (which in fact determines the number of column 
                // address bits, i.e. the row length)
                uint32_t sdram_row_length;
                        //bank size of 512MB activates col-sz field of MCFG2; 
                        //any other bank size causes default col-sz of 2048
                switch(sdram_bk1_e - sdram_bk1_s + 1) {
                    case 0x20000000:
                        sdram_row_length = 256 << 
                            ((r[MCFG2] & MCFG2_SDRAM_COSZ) >> 21);
                        sdram_row_length *= (sdram_row_length == 2048)
                                          ? (2 * gp.get_streaming_width()) 
                                          : (gp.get_streaming_width());
                        break;
                    default:
                        sdram_row_length = 2048 * gp.get_streaming_width();
                }
                //set cacheable_access extension 
                //(only required for read commands)
                ahb.validate_extension<amba::amba_cacheable> (gp);
            } else if(cmd == TLM_WRITE_COMMAND) {
            //every write transaction needs the entire write access time 
            //(burst of writes)
                transaction = "sdramwrite";
                cycles *= data_length / gp.get_streaming_width();
            }
            //if in power down mode, each access will take +1 clock cycle
            if((mobile>0) && ((sc_time_stamp()-start_idle)>=(16*cycle_time))) {
                            //is mobile SDRAM enabled?
                cycles += (((r[MCFG4] & MCFG4_ME) >> 15) & 
                            //is mobile SDRAM allowed?
                            r[MCFG2] & 
                            //power down mode?    --> shift to LSB (=1)
                            r[MCFG4]) >> 16;
            }
            //add decoding delay and complete calculation of transaction delay
            cycles += DECODING_DELAY;
            delay += cycle_time * cycles;
            //add refresh delay after each refresh period
            if(t_trans < next_refresh + (trfc * cycle_time) && 
               t_trans >= next_refresh) {
            // (a) transactions starts during refresh cycle, i.e. 
            // transaction is stalled
                delay += next_refresh + trfc * cycle_time - t_trans;
                next_refresh += cycle_time * (r[MCFG3] >> 11);
            } else if(t_trans < next_refresh && 
                      t_trans + delay - callback_delay > next_refresh) {
            // (b) transaction starts before and ends after next
            // scheduled refresh command, i.e. refresh is stalled
            // note: 'callback_delay' must not be counted twice, 
            // but it has been added to 'delay' and to 't_trans'
                refresh_stall = t_trans + delay 
                              - callback_delay - next_refresh;
                next_refresh += refresh_stall;
            }
            //capture end of transaction and forward transaction to memory

            PM::send(this, transaction, 1, sc_time_stamp(), 0, powermon);
            start_idle = t_trans + delay;
            PM::send(this, transaction, 0, (sc_time_stamp()+delay), 0, powermon);
            mctrl_sdram->b_transport(gp,delay);
        }
    } else {
    //no memory device at given address
        v::error << name() << "Invalid memory acces: No device at address" 
                 << v::uint32 << addr << "." << v::endl;
        gp.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
    }
    //end of transaction: reset callback delay variable
    callback_delay = SC_ZERO_TIME;
}


//debug transport function
uint32_t Mctrl::transport_dbg(tlm_generic_payload& gp) {
    //access to ROM adress space
    uint32_t addr = gp.get_address();
    if(((rom_bk1_s <= addr) && (addr <= rom_bk1_e)) ||
       ((rom_bk2_s <= addr) && (addr <= rom_bk2_e))) {
        return mctrl_rom->transport_dbg(gp);
    } else if((io_s <= addr) && (addr <= io_e)) {
    //access to IO adress space
        return mctrl_io->transport_dbg(gp);
    } else if(((sram_bk1_s <= addr) && (addr <= sram_bk1_e)) ||
              ((sram_bk2_s <= addr) && (addr <= sram_bk2_e)) ||
              ((sram_bk3_s <= addr) && (addr <= sram_bk3_e)) ||
              ((sram_bk4_s <= addr) && (addr <= sram_bk4_e)) ||
              ((sram_bk5_s <= addr) && (addr <= sram_bk5_e))) {
    //access to SRAM adress space
        return mctrl_sram->transport_dbg(gp);
    } else if(((sdram_bk1_s <= addr) && (addr <= sdram_bk1_e)) ||
              ((sdram_bk2_s <= addr) && (addr <= sdram_bk2_e))) {
    //access to SDRAM adress space

        return mctrl_sdram->transport_dbg(gp);
    } else {
    //no memory device at given address
        v::error << name() << "Invalid memory acces: No device at address"
                 << v::uint32 << addr << "." << v::endl;
        gp.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
        return 0;
    }
}

//--------------CALLBACK--FUNCTIONS--------------//

//write into SDRAM_CMD field of MCFG2
void Mctrl::launch_sdram_command() {
    if (!sden) {
        return;
    }
    uint8_t cmd = (((r[MCFG2] & MCFG2_SDRAM_CMD) >> 19) & 0x000000FF);
    switch(cmd) {
        // LMR / EMR
        case 3:
            // LMR / EMR commands are assumed to be issued right after 
            // changes of TCAS, DS, TCSR, PASR.
            // The delay has already been added in the according callback 
            // (configure_sdram).
            break;
        // Auto-Refresh: Forces a refresh, which needs idle state!
        // How can that be guaranteed?
        // Refresh asap, i.e. right after termination of active command? 
        // Always send a precharge before Refresh?
        // Whatever, for LT it's as simple as waiting for exactly 
        // one refresh cycle:
        // --> The previous transaction will always have finished 
        // before the Sim Kernel takes note of this callback.
        case 2:
            callback_delay += cycle_time *(3 + MCFG2_SDRAM_TRFC_DEFAULT >> 30);
            break;
        // Precharge: Terminate current burst transaction 
        // (no effect in LT) --> wait for tRP
        case 1:
            callback_delay += cycle_time * (2 + MCFG2_TRP_DEFAULT >> 29);
            break;
    }
    //clear command bits
    uint32_t set = static_cast<uint32_t> (r[MCFG2] & ~MCFG2_SDRAM_CMD);
    r[MCFG2].set(set);
}

//change of TCAS, DS, TCSR, or PASR
void Mctrl::configure_sdram() {
    // The reaction to the changes to these register fields is a command 
    // with a functionality transparent to the TLM memory system. 
    // However, the delay induced by this command can be modeled here.

    // one cycle to write the register + tRP (2+0 or 2+1) 
    // to let the changes take effect
    if(sden) {
        callback_delay += cycle_time * (3 + (r[MCFG2] & MCFG2_TRP) >> 30);
    }
}

//change of PMODE
void Mctrl::erase_sdram() {
    //prepare transaction, including erase extension
    sc_time t;
    uint32_t data = sdram_bk1_e;
    ext_erase* erase = new ext_erase;
    tlm_generic_payload gp;
    gp.set_command(TLM_WRITE_COMMAND);
    gp.set_streaming_width(4);
    gp.set_data_length(4);
    gp.set_data_ptr((unsigned char*)&data);
    gp.set_extension(erase);

    switch(r[MCFG4] & MCFG4_PMODE) {
        case 0x00000000: { 
          //check previous power down mode
          uint8_t cycles = 0;
          uint8_t cycles_after_refresh = 0;
          switch(pmode) {
              //leaving self refresh: tXSR + Auto Refresh cycle (tRFC)
              case 2:
                  cycles = ((r[MCFG4] & MCFG4_TXSR) >> 20) +
                  //tXSR
                           ((r[MCFG2] & MCFG2_SDRAM_TRFC) >> 27) + 3;
                  //tRFC
                  break;
              // leaving deep power down mode: 
              // Precharge, 2x Auto-Refresh, LMR, EMR
              case 5:
                  cycles = 2 + (r[MCFG2] & MCFG2_TRP) >> 30 +
                           //precharge (tRP)
                           2 * ((r[MCFG2] & MCFG2_SDRAM_TRFC) >> 27) + 3 +
                           //2 * tRFC
                           2 * (3 + (r[MCFG2] & MCFG2_TRP) >> 30);
                  //LMR + EMR
                  cycles_after_refresh = 2 * (3 + (r[MCFG2] & MCFG2_TRP) >> 30);
                  //LMR + EMR
            }
            callback_delay += cycle_time * cycles;
            next_refresh = sc_time_stamp() + cycle_time * 
                           (cycles - cycles_after_refresh + trfc);
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
        case 0x00020000: {
            // leaving deep power down mode: 
            // Precharge, 2x Auto-Refresh, LMR, EMR
            uint8_t cycles = 0;
            if(pmode == 5) {
                cycles = 2 + (r[MCFG2] & MCFG2_TRP) >> 30 +
                         //precharge (tRP)
                         2 * ((r[MCFG2] & MCFG2_SDRAM_TRFC) >> 27) + 3 +
                         //2 * tRFC
                         2 * (3 + (r[MCFG2] & MCFG2_TRP) >> 30);
                //LMR + EMR
            }
            callback_delay += cycle_time * cycles;
            pmode = 2;
            uint8_t pasr = r[MCFG4] & MCFG4_PASR;
            if(pasr) {
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
    if(r[MCFG2] & MCFG2_SE) {
        if(r[MCFG2] & MCFG2_SI) {
            // SRAM has been disabled 
            // --> SDRAM needs to be mapped into lower half of RAM area
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
        } else {
            // SRAM has been enabled 
            // --> SDRAM needs to be mapped into upper half of RAM area
            sdram_bk1_s += ((4096 - rammask) / 2) << 20;
            sdram_bk1_e += ((4096 - rammask) / 2) << 20;
            sdram_bk2_s += ((4096 - rammask) / 2) << 20;
            sdram_bk2_e += ((4096 - rammask) / 2) << 20;

            //calculate SRAM bank addresses based on MCFG2 bank size field
            //the GR callback is somehow misused for this pupose
            sram_change_bank_size();
        }
        v::warn << name() << "address ranges of RAM banks have just "
                << "been changed to" << std::endl
                << "SRAM_1:  " << v::uint32 <<  sram_bk1_s 
                      << " - " << v::uint32 <<  sram_bk1_e << std::endl
                << "SRAM_2:  " << v::uint32 <<  sram_bk2_s 
                      << " - " << v::uint32 <<  sram_bk2_e << std::endl
                << "SRAM_3:  " << v::uint32 <<  sram_bk3_s 
                      << " - " << v::uint32 <<  sram_bk3_e << std::endl
                << "SRAM_4:  " << v::uint32 <<  sram_bk4_s 
                      << " - " << v::uint32 <<  sram_bk4_e << std::endl
                << "SRAM_5:  " << v::uint32 <<  sram_bk5_s 
                      << " - " << v::uint32 <<  sram_bk5_e << std::endl
                << "SDRAM_1: " << v::uint32 << sdram_bk1_s 
                      << " - " << v::uint32 << sdram_bk1_e << std::endl
                << "SDRAM_2: " << v::uint32 << sdram_bk2_s 
                      << " - " << v::uint32 << sdram_bk2_e << std::endl;
    }
}

// recalculate start / end addresses of ram banks after 
// sdram enable / disable
void Mctrl::sdram_enable() {
    //SDRAM has just been enabled (SRAM cannot be disabled)
    if(r[MCFG2] & MCFG2_SE) {
        sram_bk5_s = 0;
        sram_bk5_e = 0;

        //calculate SDRAM bank addresses based on MCFG2 bank size field
        sdram_change_bank_size();
    } else {
    //SDRAM has just been disabled
        sdram_bk1_s = 0;
        sdram_bk1_e = 0;
        sdram_bk2_s = 0;
        sdram_bk2_e = 0;

        //enable SRAM if it is disabled
        if(r[MCFG2] & MCFG2_SI) {
            uint32_t set = r[MCFG2] | MCFG2_SI;
            r[MCFG2].set(set);
            //calculate SRAM bank addresses based on MCFG2 bank size field
            sram_change_bank_size();
        }
    }
    v::warn << name() << "address ranges of RAM banks have just been "
            << "changed to" << std::endl
            << "SRAM_1:  " << v::uint32 <<  sram_bk1_s 
                  << " - " << v::uint32 <<  sram_bk1_e << std::endl
            << "SRAM_2:  " << v::uint32 <<  sram_bk2_s 
                  << " - " << v::uint32 <<  sram_bk2_e << std::endl
            << "SRAM_3:  " << v::uint32 <<  sram_bk3_s 
                  << " - " << v::uint32 <<  sram_bk3_e << std::endl
            << "SRAM_4:  " << v::uint32 <<  sram_bk4_s 
                  << " - " << v::uint32 <<  sram_bk4_e << std::endl
            << "SRAM_5:  " << v::uint32 <<  sram_bk5_s 
                  << " - " << v::uint32 <<  sram_bk5_e << std::endl
            << "SDRAM_1: " << v::uint32 << sdram_bk1_s 
                  << " - " << v::uint32 << sdram_bk1_e << std::endl
            << "SDRAM_2: " << v::uint32 << sdram_bk2_s 
                  << " - " << v::uint32 << sdram_bk2_e << std::endl;
}

// recalculate start / end addresses of sram banks after change 
// of sram bank size
void Mctrl::sram_change_bank_size() {
    //calculate sram bank size from MCFG2 register
    // 8KB = 1B << 13
    //16KB = 1B << 13 + b'0001
    //32KB = 1B << 13 + b'0010 ...
    uint32_t sram_bank_size = 1 << 
        (13 + ((r[MCFG2] & MCFG2_RAM_BANK_SIZE) >> 9));

    //check for conflicts: 1-4 banks must fit into half of RAM address space
    if((srbanks * sram_bank_size) > 
          static_cast<uint32_t>(((4096 - rammask) / 2) << 20)) {
        v::error << name() << "SRAM bank size has been changed. "
                 << "The extended memory banks exceed SRAM addrres space. ["
                 << srbanks << " * " << v::uint32 << sram_bank_size << " > " 
                 << v::uint32 << (4096 - rammask) * 1024 * 512 << "]" 
                 << v::endl;
    }

    // calculate new bank addresses
    sram_calculate_bank_addresses(sram_bank_size);
}

// recalculate start / end addresses of sdram banks after change 
// of sdram bank size
void Mctrl::sdram_change_bank_size() {
  //calculate sdram bank size from MCFG2 register
  // 4MB = 1B << 22
  // 8MB = 1B << 22 + b'001
  //16MB = 1B << 22 + b'010 ...
  uint32_t sdram_bank_size = 1 << 
      ( 22 + ((r[MCFG2] & MCFG2_SDRAM_BANKSZ) >> 23) );

  //check for conflicts: 2 banks must fit into half of RAM address space
  if (2*sdram_bank_size > 
        static_cast<uint32_t>( ((4096 - rammask) / 2) << 20 )) {
    v::error << name() << "SDRAM bank size has been changed. "
             << "The extended memory banks exceed SDRAM addrres space." 
             << v::endl;
  }

    //calculate new bank addresses
    sdram_bk1_s = sram_bk1_s + ((4096 - rammask) / 2) << 20;
    sdram_bk1_e = sdram_bk1_s + sdram_bank_size - 1;
    sdram_bk2_s = sdram_bk1_e + 1;
    sdram_bk2_e = sdram_bk2_s + sdram_bank_size - 1;
}

void Mctrl::sram_calculate_bank_addresses(uint32_t sram_bank_size) {
    //calculate bank 1 address from generics and bank size
    sram_bk1_s = static_cast<uint32_t> (ramaddr << 20);
    sram_bk1_e = sram_bk1_s + sram_bank_size - 1;

    //calculate remaining bank addresses if banks are present
    switch (srbanks) {
        case 5: // banks 5, 4, 3 and 2
            //set bank 5 values only if SDRAM is disabled
            if ((r[MCFG2] & MCFG2_SE) == 0) {
                sram_bk5_s = sram_bk1_s + ((4096 - rammask) / 2) << 20;
                sram_bk5_e = sram_bk1_s + ((4096 - rammask) << 20) - 1;
            }
        case 4: // banks 4, 3 and 2
            sram_bk4_s = sram_bk1_s + 3 * sram_bank_size;
            sram_bk4_e = sram_bk4_s + sram_bank_size - 1;
        case 3: // banks 3 and 2
            sram_bk3_s = sram_bk1_s + 2 * sram_bank_size;
            sram_bk3_e = sram_bk3_s + sram_bank_size - 1;
        case 2: // bank 2
            sram_bk2_s = sram_bk1_s + sram_bank_size;
            sram_bk2_e = sram_bk2_s + sram_bank_size - 1;
    }
}

// Function to change the refresh period
void Mctrl::sdram_change_refresh_cycle() {
    trfc = 3 + (r[MCFG2] & MCFG2_SDRAM_TRFC) >> 27;
}

// Extract basic cycle rate from a sc_clock
void Mctrl::clk(sc_clock &clk) {
    cycle_time = clk.period();
}

// Extract basic cycle rate from a clock period
void Mctrl::clk(sc_time &period) {
    cycle_time = period;
}

// Extract basic cycle rate from a clock period in double
void Mctrl::clk(double period, sc_time_unit base) {
    cycle_time = sc_time(period, base);
}
