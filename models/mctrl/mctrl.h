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
// Modified on $Date: 2011-06-09 08:49:53 +0200 (Thu, 09 Jun 2011) $
//          at $Revision: 452 $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*****************************************************************************

#ifndef MCTRL_H
#define MCTRL_H

#define DEBUG

#include <tlm.h>
#include <systemc.h>

#include "ahbslave.h"
#include "apbdevice.h"
#include "clkdevice.h"
#include "memdevice.h"

//#include <cmath.h>
#include <algorithm>
#include <iostream>
#include <boost/config.hpp>

#include <greencontrol/config.h>
#include <greensocket/initiator/multi_socket.h>
#include <greenreg_ambasockets.h>
#include <amba.h>

#include <signalkit.h>
#include "vendian.h"
#include "verbose.h"
#include "ext_erase.h"

/// @addtogroup mctrl MCtrl
/// @{

/// @brief This class is an TLM 2.0 Model of the Aeroflex Gaisler GRLIB mctrl.
/// Further informations to the original VHDL Modle are available in the GRLIB IP Core User's Manual Section 66.
class Mctrl : public AHBSlave<gs::reg::gr_device>,
              public APBDevice,
              public CLKDevice {
    public:
        SC_HAS_PROCESS(Mctrl);
        SK_HAS_SIGNALS(Mctrl);
        
        /// Creates a new Instance of an MCtrl.
        ///
        /// @param name The SystemC name of the component to be created.
        /// @param romasel
        /// @param sdrasel
        /// @param romaddr
        /// @param rommask
        /// @param ioaddr
        /// @param iomask
        /// @param ramaddr
        /// @param rammask
        /// @param paddr
        /// @param pmask
        /// @param wprot
        /// @param srbanks
        /// @param ram8
        /// @param ram16
        /// @param sepbus
        /// @param sdbits
        /// @param mobile
        /// @param sden
        /// @param hindex
        /// @param pindex
        /// @param powermon
        /// @param abstractionLayer
        ///
        /// All constructor parameter are directly related to an VHDL Generic in the original Model. 
        /// Therefore read the GRLIB IP Core User's Manual Section 66.15 for more information.  
        Mctrl(sc_module_name name, int romasel = 28, int sdrasel = 29,
              int romaddr = 0x0, int rommask = 0xE00, 
              int ioaddr = 0x200, int iomask = 0xE00, 
              int ramaddr = 0x400, int rammask = 0xC00,
              int paddr = 0x0, int pmask = 0xFFF, 
              int wprot = 0, int srbanks = 4, 
              int ram8 = 0, int ram16 = 0, int sepbus = 0, 
              int sdbits = 32, int mobile = 0, int sden = 0, 
	      unsigned int hindex = 0, unsigned int pindex = 0, 
	      bool powmon = false,
	      amba::amba_layer_ids ambaLayer = amba::amba_LT);

        /// Default destructor
        ~Mctrl();

        /// APB Slave Socket
        ///
        /// Connects mctrl config registers to APB
        gs::reg::greenreg_socket<gs::amba::amba_slave<32> > apb;

        /// Memory Master Socket
        ///
        /// Initiate communication with memory modules.
        gs::socket::initiator_multi_socket<32> mem;

        // SystemC Declarations
        /// proclamation of callbacks   
        GC_HAS_CALLBACKS();

        /// Execute the callback registering when systemc reaches the end of elaboration.
        void end_of_elaboration();

        /// Gathers information about the connected memory types when SystemC reaches the start of simulation.
        void start_of_simulation();

        /// SystemC end of simulation handler.
        /// It's needed to print performance counter
        void end_of_simulation();

        sc_core::sc_time get_clock();

        // Signal Callbacks
        /// Reset Handler
        ///
        /// Executed on rst state change.
        /// Performs a full reset of the Model.
        /// So it will set all registers to defaults, etc.
        ///
        /// @param value The new Value of the Signal.
        /// @param delay A possible delay. Which means the reset might be performed in the future (Not used for resets!).
        void dorst();

	// Register Callbacks
        /// Performing an SDRAM Command
        ///
        /// This function is executed by the SDRAM Command Field in the MCFG2 Register.
        /// It will perform the coresponding state change to the SDRAM Controler and RAM.
        /// For more information read the corresponding Secton from the GRLIB IP Core User's Manual: Section 66.9.
        void launch_sdram_command();

        /// Performing an Power-Mode change
        ///
        /// This function is executed by the pmode Field in the MCFG4 Register.
        /// It will perform the coresponding state change to the Memory Controler and RAM.
        /// For more information read the corresponding Secton from the GRLIB IP Core User's Manual: Section 66.9.
        void switch_power_mode();
        void mcfg1_write();
        void mcfg2_write();

	/// Encapsulation function for functional part of the model
	uint32_t exec_func(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay, bool debug = false);

    private:

        /// Indexer for Memmory models on the mem Socket.
        ///
        /// This class is used to store informations to the connected memmory devices on the bus localy.
        /// See start_of_simulation Function and get_port for usage information.
        class MEMPort {
            public:
                MEMPort(uint32_t id, MEMDevice *dev);
                MEMPort();
                uint32_t   id;
                MEMDevice *dev;
                uint32_t   addr;
                uint32_t   length;
        };

        /// Instanciations for each memory device, plus one for no matching memory found.
        MEMPort c_rom, c_io, c_sram, c_sdram, c_null;

        /// Return the Indexer for a memory at a specific address.
        Mctrl::MEMPort get_port(uint32_t address);

        /// Ready to accept new transaction (send END_REQ)
	sc_event unlock_event;

        // Control / timing variables
        
        /// Count time elapsing in callbacks (to be added in next transaction)
        sc_core::sc_time callback_delay;
        
        /// Capture end time of last transaction to calculate sdram idle time
        sc_core::sc_time start_idle; 
        
        /// Time to perform next refresh
        sc_core::sc_time next_refresh; 
        
        // Refresh can only be started in idle state, 
        // so it might be necessary to stall
        sc_core::sc_time refresh_stall;

        /// False - ready to accept new transaction
        bool busy;
        
        /// Length of refresh cycle
        uint8_t m_trfc; 
        
        /// Capture current state of power mode
        uint8_t m_pmode; 

        /// GreenControl API container
        gs::cnf::cnf_api *m_api;
        
        /// Open a namespace for performance counting in the greencontrol realm
        gs::gs_param_array m_performance_counters;
        
        /// The number of total transactions handled by the mctrl
        gs::gs_param<unsigned long long> m_total_transactions;
        
        /// The number of successfull ended transactions
        gs::gs_param<unsigned long long> m_right_transactions;

        /// Total time of power down mode
        gs::gs_param<sc_time> m_power_down_time;
        
        /// Last time switched to power down mode
        gs::gs_param<sc_time> m_power_down_start;

        /// Total time of deep power down mode
        gs::gs_param<sc_time> m_deep_power_down_time;
        
        /// Last time switched to deep power down mode
        gs::gs_param<sc_time> m_deep_power_down_start;

        /// Total time of auto self refresh mode
        gs::gs_param<sc_time> m_self_refresh_time;
        
        /// Last time switched to auto self refresh mode
        gs::gs_param<sc_time> m_self_refresh_start;

        // Constructor parameters (modeling VHDL generics)
        const int g_romasel;
        const int g_sdrasel;
        const int g_romaddr;
        const int g_rommask;
        const int g_ioaddr;
        const int g_iomask;
        const int g_ramaddr;
        const int g_rammask;
        const int g_paddr;
        const int g_pmask;
        const int g_wprot;
        const int g_srbanks;
        const int g_ram8;
        const int g_ram16;
        const int g_sepbus;
        const int g_sdbits;
        const int g_mobile;
        const int g_sden;

    public:

        //--- Constant bit masks for APB register access

        // Register address offset
        static const uint32_t MCFG1                       = 0x00;
        static const uint32_t MCFG2                       = 0x04;
        static const uint32_t MCFG3                       = 0x08;
        static const uint32_t MCFG4                       = 0x0C;

        // Memory configuration register 1
        static const uint32_t MCFG1_WRITE_MASK            = 0x1FE808FF;
        static const uint32_t MCFG1_IOBUSW                = 0x18000000;
        static const uint32_t MCFG1_IBRDY                 = 0x04000000;
        static const uint32_t MCFG1_BEXCN                 = 0x02000000;
        static const uint32_t MCFG1_IO_WAITSTATES         = 0x01E00000;
        static const uint32_t MCFG1_IOEN                  = 0x00080000;
        static const uint32_t MCFG1_PWEN                  = 0x00000800;
        static const uint32_t MCFG1_PROM_WIDTH            = 0x00000300;
        static const uint32_t MCFG1_IO_WIDTH              = 0x18000000;
        static const uint32_t MCFG1_PROM_WRITE_WS         = 0x000000F0;
        static const uint32_t MCFG1_PROM_READ_WS          = 0x0000000F;

        // Memory configuration register 2
        static const uint32_t MCFG2_WRITE_MASK            = 0xFFD07EFF;
        static const uint32_t MCFG2_SDRF                  = 0x80000000;
        static const uint32_t MCFG2_TRP                   = 0x40000000;
        static const uint32_t MCFG2_SDRAM_TRFC            = 0x38000000;
        static const uint32_t MCFG2_TCAS                  = 0x04000000;
        static const uint32_t MCFG2_SDRAM_BANKSZ          = 0x03800000;
        static const uint32_t MCFG2_SDRAM_COSZ            = 0x00600000;
        static const uint32_t MCFG2_SDRAM_CMD             = 0x00180000;
        static const uint32_t MCFG2_D64                   = 0x00040000;
        static const uint32_t MCFG2_MS                    = 0x00010000;
        static const uint32_t MCFG2_SE                    = 0x00004000;
        static const uint32_t MCFG2_SI                    = 0x00002000;
        static const uint32_t MCFG2_RAM_BANK_SIZE         = 0x00001E00;
        static const uint32_t MCFG2_RBRDY                 = 0x00000080;
        static const uint32_t MCFG2_RMW                   = 0x00000040;
        static const uint32_t MCFG2_RAM_WIDTH             = 0x00000030;
        static const uint32_t MCFG2_RAM_WRITE_WS          = 0x0000000C;
        static const uint32_t MCFG2_RAM_READ_WS           = 0x00000003;

        // Memory configuration register 3
        static const uint32_t MCFG3_WRITE_MASK            = 0x07FFF000;
        static const uint32_t MCFG3_SDRAM_RLD_VAL         = 0x07FFF000;

        // Memory configuration register 4
        static const uint32_t MCFG4_WRITE_MASK            = 0xE0F7007F;
        static const uint32_t MCFG4_ME                    = 0x80000000;
        static const uint32_t MCFG4_CE                    = 0x40000000;
        static const uint32_t MCFG4_EM                    = 0x20000000;
        static const uint32_t MCFG4_TXSR                  = 0x00F00000;
        static const uint32_t MCFG4_PMODE                 = 0x00070000;
        static const uint32_t MCFG4_DC                    = 0x00000060;
        static const uint32_t MCFG4_TCSR                  = 0x00000018;
        static const uint32_t MCFG4_PASR                  = 0x00000007;

        //--- Register default values

        // Memory configuration register 1
        static const uint32_t MCFG1_IOBUSW_DEFAULT        = 0x18000000;
        static const uint32_t MCFG1_IBRDY_DEFAULT         = 0x00000000;
        static const uint32_t MCFG1_BEXCN_DEFAULT         = 0x00000000;
        static const uint32_t MCFG1_IO_WAITSTATES_DEFAULT = 0x00000000;
        static const uint32_t MCFG1_IOEN_DEFAULT          = 0x00000000;
        static const uint32_t MCFG1_PWEN_DEFAULT          = 0x00000000;
        static const uint32_t MCFG1_PROM_WIDTH_DEFAULT    = 0x00000300;
        static const uint32_t MCFG1_PROM_WRITE_WS_DEFAULT = 0x000000F0;
        static const uint32_t MCFG1_PROM_READ_WS_DEFAULT  = 0x0000000F;
        //                                                +
        static const uint32_t MCFG1_DEFAULT               = 0x180003FF;

        // Memory configuration register 2
        static const uint32_t MCFG2_SDRF_DEFAULT          = 0x80000000;
        static const uint32_t MCFG2_TRP_DEFAULT           = 0x40000000;
        static const uint32_t MCFG2_SDRAM_TRFC_DEFAULT    = 0x38000000;
        static const uint32_t MCFG2_TCAS_DEFAULT          = 0x04000000;
        static const uint32_t MCFG2_SDRAM_BANKSZ_DEFAULT  = 0x03000000;
        static const uint32_t MCFG2_SDRAM_COSZ_DEFAULT    = 0x00600000;
        static const uint32_t MCFG2_SDRAM_CMD_DEFAULT     = 0x00000000;
        static const uint32_t MCFG2_D64_DEFAULT           = 0x00000000;
        static const uint32_t MCFG2_MS_DEFAULT            = 0x00000000;
        static const uint32_t MCFG2_SE_DEFAULT            = 0x00000000;
        static const uint32_t MCFG2_SI_DEFAULT            = 0x00000000;
        static const uint32_t MCFG2_RAM_BANK_SIZE_DEFAULT = 0x00001C00;
        static const uint32_t MCFG2_RBRDY_DEFAULT         = 0x00000000;
        static const uint32_t MCFG2_RMW_DEFAULT           = 0x00000000;
        static const uint32_t MCFG2_RAM_WIDTH_DEFAULT     = 0x00000030;
        static const uint32_t MCFG2_RAM_WRITE_WS_DEFAULT  = 0x0000000C;
        static const uint32_t MCFG2_RAM_READ_WS_DEFAULT   = 0x00000003;
        //                                                +
        static const uint32_t MCFG2_DEFAULT               = 0xFF601C3F;

        // Memory configuration register 3
        static const uint32_t MCFG3_DEFAULT               = 0x07FFF000;

        // Memory configuration register 4
        static const uint32_t MCFG4_ME_DEFAULT            = 0x00000000;
        static const uint32_t MCFG4_CE_DEFAULT            = 0x00000000;
        static const uint32_t MCFG4_EM_DEFAULT            = 0x00000000;
        static const uint32_t MCFG4_TXSR_DEFAULT          = 0x00F00000;
        static const uint32_t MCFG4_PMODE_DEFAULT         = 0x00000000;
        static const uint32_t MCFG4_DS_DEFAULT            = 0x00000000;
        static const uint32_t MCFG4_TCSR_DEFAULT          = 0x00000000;
        static const uint32_t MCFG4_PASR_DEFAULT          = 0x00000000;
        //                                                +
        static const uint32_t MCFG4_DEFAULT               = 0x00F00000;
};

#endif // MCTRL_H

