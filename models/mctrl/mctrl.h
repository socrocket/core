//*****************************************************************************
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
//*****************************************************************************
// Title:      mctrl.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file defining the mctrl module template
//             includes implementation file mctrl.tpp at the bottom
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Dennis Bode
// Reviewed:
//*****************************************************************************

#ifndef MCTRL_H
#define MCTRL_H

#define DEBUG

//#include <cmath.h>
#include <algorithm>
#include <iostream>
#include <boost/config.hpp>
#include <systemc.h>
#include <tlm.h>
#include "tlm_utils/simple_initiator_socket.h"
#include <greenreg_ambasockets.h>
#include "greencontrol/all.h"
#include "amba.h"
#include "generic_memory.h"
#include "ahbdevice.h"
#include "apbdevice.h"
#include "signalkit.h"
#include "verbose.h"
#include "ext_erase.h"

class Mctrl : public gs::reg::gr_device,
              public AHBDevice,
              public APBDevice,
              public signalkit::signal_module<Mctrl> {
    public:
        //constructor / destructor
        Mctrl(sc_module_name name, int _romasel = 28, int _sdrasel = 29,
              int _romaddr = 0x0, int _rommask = 0xE00, 
              int _ioaddr = 0x200, int _iomask = 0xE00, 
              int _ramaddr = 0x400, int _rammask = 0xC00,
              int _paddr = 0x0, int _pmask = 0xFFF, 
              int _wprot = 0, int _srbanks = 4, 
              int _ram8 = 0, int _ram16 = 0, int _sepbus = 0, 
              int _sdbits = 32, int _mobile = 0, int _sden = 0, 
	      unsigned int hindex = 0, unsigned int pindex = 0);
        ~Mctrl();

        //APB slave socket: connects mctrl config registers to apb
        gs::reg::greenreg_socket<gs::amba::amba_slave<32> > apb;

        //AHB slave socket: receives instructions (mem access) from CPU
        ::amba::amba_slave_socket<32> ahb;

        //Master sockets: Initiate communication with memory modules
        tlm_utils::simple_initiator_socket<Mctrl> mctrl_rom;
        tlm_utils::simple_initiator_socket<Mctrl> mctrl_io;
        tlm_utils::simple_initiator_socket<Mctrl> mctrl_sram;
        tlm_utils::simple_initiator_socket<Mctrl> mctrl_sdram;

        //reset signal
        signal<bool>::in rst;

        //device identification on AHB bus
        inline sc_dt::uint64 get_size() {
            //get start address of memory area
            uint64_t base = std::min(romaddr, ioaddr);
            base = std::min(static_cast<uint64_t> (ramaddr), base);

            //get end address of memory area
            uint64_t end = std::max(romaddr, ioaddr);
            end = std::max(static_cast<uint64_t> (ramaddr), end);
            //base of highest mem area + size of that area
            if (end == static_cast<uint64_t> (ramaddr)) {
                end += ((~rammask) & 0xfff) + 1;
            } else if (end == static_cast<uint64_t> (ioaddr)) {
                end += ((~iomask) & 0xfff) + 1;
            } else if (end == static_cast<uint64_t> (romaddr)) {
                end += ((~rommask) & 0xfff) + 1;
            }

            //size is given in MB, so << 20
            return ((end - base) << 20);
        }
        inline sc_dt::uint64 get_base_addr() {
            //get start address of memory area
            uint64_t base = std::min(romaddr, ioaddr);
            base = std::min(static_cast<uint64_t> (ramaddr), base);

            return base << 20;
        }

        // proclamation of callbacks
        GC_HAS_CALLBACKS();

        // function prototypes
        void end_of_elaboration();
        void sram_calculate_bank_addresses(uint32_t sram_bank_size);

        // thread process to initialize MCTRL 
        // (set registers, define address spaces, etc.)
        void reset_mctrl(const bool &value, const sc_core::sc_time &time);

        //callbacks reacting on register access
        void launch_sdram_command();
        void configure_sdram();
        void erase_sdram();
        void sram_disable();
        void sdram_enable();
        void sram_change_bank_size();
        void sdram_change_bank_size();
        void sdram_change_refresh_cycle();

        // define TLM transport functions
        virtual void b_transport(tlm::tlm_generic_payload& gp, sc_time& delay);
        // TLM debug interface
        uint32_t transport_dbg(tlm::tlm_generic_payload& gp);

        // management of the clock cycle length, required for delay calculation
        sc_core::sc_time cycle_time; //variable to store the clock period
        
        // Three functions to set the clockcycle length.
        /// gets the clock period from an sc_clk instance
        void clk(sc_core::sc_clock &clk);
        
        // gets the clock period from an sc_time variable
        void clk(sc_core::sc_time &period); 

        // directly gets the clock period and time unit
        void clk(double period, sc_core::sc_time_unit base);

    private:
        // address space variables
        uint32_t rom_bk1_s, rom_bk1_e, rom_bk2_s, rom_bk2_e, io_s, io_e,
                sram_bk1_s, sram_bk1_e, sram_bk2_s, sram_bk2_e, sram_bk3_s,
                sram_bk3_e, sram_bk4_s, sram_bk4_e, sram_bk5_s, sram_bk5_e,
                sdram_bk1_s, sdram_bk1_e, sdram_bk2_s, sdram_bk2_e;

        // control / timing variables
        
        // count time elapsing in callbacks (to be added in next transaction)
        sc_core::sc_time callback_delay;
        
        // capture end time of last transaction to calculate sdram idle time
        sc_core::sc_time start_idle; 
        
        // time to perform next refresh
        sc_core::sc_time next_refresh; 
        
        // refresh can only be started in idle state, 
        // so it might be necessary to stall
        sc_core::sc_time refresh_stall; 
        
        //length of refresh cycle
        uint8_t trfc; 
        
        //capture current state of power mode
        uint8_t pmode; 

        //constructor parameters (modeling VHDL generics)
        const int romasel;
        const int sdrasel;
        const int romaddr;
        const int rommask;
        const int ioaddr;
        const int iomask;
        const int ramaddr;
        const int rammask;
        const int paddr;
        const int pmask;
        const int wprot;
        const int srbanks;
        const int ram8;
        const int ram16;
        const int sepbus;
        const int sdbits;
        const int mobile;
        const int sden;
    public:
        //delay definitions (in clock cycles)
        static const uint32_t DECODING_DELAY = 1;
        static const uint32_t ROM_READ_DELAY(uint8_t wstates) {
           //data1, data2, lead-out
            return(3+wstates);
        }
        static const uint32_t ROM_WRITE_DELAY(uint8_t wstates) {
           //lead-in, data, lead-out
            return(3+wstates);
        }
        static const uint32_t IO_READ_DELAY(uint8_t wstates) {
            //lead-in, data1, data2, lead-out
            return(4+wstates);
        }
        static const uint32_t IO_WRITE_DELAY(uint8_t wstates) {
            //lead-in, data, lead-out
            return(3+wstates);
        }
        static const uint32_t SRAM_READ_DELAY(uint8_t wstates) {
            //data1, data2, lead-out
            return(3+wstates);
        }
        static const uint32_t SRAM_WRITE_DELAY(uint8_t wstates) {
            //lead-in, data, lead-out
            return(3+wstates);
        }

        //---constant bit masks for APB register access

        //register address offset
        static const uint32_t MCFG1 = 0x00;
        static const uint32_t MCFG2 = 0x04;
        static const uint32_t MCFG3 = 0x08;
        static const uint32_t MCFG4 = 0x0C;

        //memory configuration register 1
        static const uint32_t MCFG1_WRITE_MASK =    0x1FE808FF;
        static const uint32_t MCFG1_IOBUSW =        0x18000000;
        static const uint32_t MCFG1_IBRDY =         0x04000000;
        static const uint32_t MCFG1_BEXCN =         0x02000000;
        static const uint32_t MCFG1_IO_WAITSTATES = 0x01E00000;
        static const uint32_t MCFG1_IOEN =          0x00080000;
        static const uint32_t MCFG1_PWEN =          0x00000800;
        static const uint32_t MCFG1_PROM_WIDTH =    0x00000300;
        static const uint32_t MCFG1_PROM_WRITE_WS = 0x000000F0;
        static const uint32_t MCFG1_PROM_READ_WS =  0x0000000F;

    //memory configuration register 2
    static const uint32_t MCFG2_WRITE_MASK      = 0xFFD07EFF;
    static const uint32_t MCFG2_SDRF            = 0x80000000;
    static const uint32_t MCFG2_TRP             = 0x40000000;
    static const uint32_t MCFG2_SDRAM_TRFC      = 0x38000000;
    static const uint32_t MCFG2_TCAS            = 0x04000000;
    static const uint32_t MCFG2_SDRAM_BANKSZ    = 0x03800000;
    static const uint32_t MCFG2_SDRAM_COSZ      = 0x00600000;
    static const uint32_t MCFG2_SDRAM_CMD       = 0x00180000;
    static const uint32_t MCFG2_D64             = 0x00040000;
    static const uint32_t MCFG2_MS              = 0x00010000;
    static const uint32_t MCFG2_SE              = 0x00004000;
    static const uint32_t MCFG2_SI              = 0x00002000;
    static const uint32_t MCFG2_RAM_BANK_SIZE   = 0x00001E00;
    static const uint32_t MCFG2_RBRDY           = 0x00000080;
    static const uint32_t MCFG2_RMW             = 0x00000040;
    static const uint32_t MCFG2_RAM_WIDTH       = 0x00000030;
    static const uint32_t MCFG2_RAM_WRITE_WS    = 0x0000000C;
    static const uint32_t MCFG2_RAM_READ_WS     = 0x00000003;

        //memory configuration register 3
        static const uint32_t MCFG3_WRITE_MASK = 0x07FFF000;
        static const uint32_t MCFG3_SDRAM_RLD_VAL = 0x07FFF000;

        //memory configuration register 4
        static const uint32_t MCFG4_WRITE_MASK = 0xE0F7007F;
        static const uint32_t MCFG4_ME =         0x80000000;
        static const uint32_t MCFG4_CE =         0x40000000;
        static const uint32_t MCFG4_EM =         0x20000000;
        static const uint32_t MCFG4_TXSR =       0x00F00000;
        static const uint32_t MCFG4_PMODE =      0x00070000;
        static const uint32_t MCFG4_DC =         0x00000060;
        static const uint32_t MCFG4_TCSR =       0x00000018;
        static const uint32_t MCFG4_PASR =       0x00000007;

        //---register default values

        //memory configuration register 1
        static const uint32_t MCFG1_IOBUSW_DEFAULT =        0x00000000;
        static const uint32_t MCFG1_IBRDY_DEFAULT =         0x00000000;
        static const uint32_t MCFG1_BEXCN_DEFAULT =         0x00000000;
        static const uint32_t MCFG1_IO_WAITSTATES_DEFAULT = 0x00000000;
        static const uint32_t MCFG1_IOEN_DEFAULT =          0x00000000;
        static const uint32_t MCFG1_PWEN_DEFAULT =          0x00000000;
        static const uint32_t MCFG1_PROM_WIDTH_DEFAULT =    0x00000000;
        static const uint32_t MCFG1_PROM_WRITE_WS_DEFAULT = 0x000000F0;
        static const uint32_t MCFG1_PROM_READ_WS_DEFAULT =  0x0000000F;
        //                                                      +
        static const uint32_t MCFG1_DEFAULT =               0x000000FF;

    //memory configuration register 2
    static const uint32_t MCFG2_SDRF_DEFAULT            = 0x80000000;
    static const uint32_t MCFG2_TRP_DEFAULT             = 0x40000000;
    static const uint32_t MCFG2_SDRAM_TRFC_DEFAULT      = 0x38000000;
    static const uint32_t MCFG2_TCAS_DEFAULT            = 0x04000000;
    static const uint32_t MCFG2_SDRAM_BANKSZ_DEFAULT    = 0x03000000;
    static const uint32_t MCFG2_SDRAM_COSZ_DEFAULT      = 0x00600000;
    static const uint32_t MCFG2_SDRAM_CMD_DEFAULT       = 0x00000000;
    static const uint32_t MCFG2_D64_DEFAULT             = 0x00000000;
    static const uint32_t MCFG2_MS_DEFAULT              = 0x00000000;
    static const uint32_t MCFG2_SE_DEFAULT              = 0x00000000;
    static const uint32_t MCFG2_SI_DEFAULT              = 0x00000000;
    static const uint32_t MCFG2_RAM_BANK_SIZE_DEFAULT   = 0x00001C00;
    static const uint32_t MCFG2_RBRDY_DEFAULT           = 0x00000000;
    static const uint32_t MCFG2_RMW_DEFAULT             = 0x00000000;
    static const uint32_t MCFG2_RAM_WIDTH_DEFAULT       = 0x00000030;
    static const uint32_t MCFG2_RAM_WRITE_WS_DEFAULT    = 0x0000000C;
    static const uint32_t MCFG2_RAM_READ_WS_DEFAULT     = 0x00000003;
    //                                                      +
    static const uint32_t MCFG2_DEFAULT                 = 0xFF601C3F;

        //memory configuration register 3
        static const uint32_t MCFG3_DEFAULT = 0x07FFF000;

        //memory configuration register 4
        static const uint32_t MCFG4_ME_DEFAULT = 0x00000000;
        static const uint32_t MCFG4_CE_DEFAULT = 0x00000000;
        static const uint32_t MCFG4_EM_DEFAULT = 0x00000000;
        static const uint32_t MCFG4_TXSR_DEFAULT = 0x00F00000;
        static const uint32_t MCFG4_PMODE_DEFAULT = 0x00000000;
        static const uint32_t MCFG4_DS_DEFAULT = 0x00000000;
        static const uint32_t MCFG4_TCSR_DEFAULT = 0x00000000;
        static const uint32_t MCFG4_PASR_DEFAULT = 0x00000000;
        //                                                      +
        static const uint32_t MCFG4_DEFAULT = 0x00F00000;
};

#endif // MCTRL_H

