// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mctrl
/// @{
/// @file mctrl.h
/// header file defining the mctrl module template includes implementation file
/// mctrl.tpp at the bottom
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef MODELS_MCTRL_MCTRL_H_
#define MODELS_MCTRL_MCTRL_H_

#include "core/common/systemc.h"
#include <tlm.h>
#include "core/common/amba.h"
#include "core/common/sr_param.h"
#include <greensocket/initiator/multi_socket.h>
#include <boost/config.hpp>
#include <algorithm>

#include "core/common/ahbslave.h"
#include "core/common/apbdevice.h"
#include "core/common/clkdevice.h"
#include "core/common/memdevice.h"
#include "gaisler/memory/ext_erase.h"
#include "core/common/vendian.h"
#include "core/common/verbose.h"
#include "core/common/apbslave.h"
#include "core/common/sr_signal.h"

/// @addtogroup mctrl MCtrl
/// @{

/// @brief This class is an TLM 2.0 Model of the Aeroflex Gaisler GRLIB mctrl.
/// Further informations to the original VHDL Modle are available in the GRLIB IP Core User's Manual Section 66.
class Mctrl : public AHBSlave<APBSlave>,
              public CLKDevice {
  public:
    SC_HAS_PROCESS(Mctrl);
    SR_HAS_SIGNALS(Mctrl);

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
    AbstractionLayer ambaLayer = amba::amba_LT);

    /// Default destructor
    ~Mctrl();

    /// Initialize generics
    void init_generics();
    /// Initialize registers
    void init_registers();

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

    /// Calculate power/energy values from normalized input data
    void power_model();

    /// Static power callback
    gs::cnf::callback_return_type sta_power_cb(
        gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
        gs::cnf::callback_type reason);

    /// Dynamic/Internal power callback
    gs::cnf::callback_return_type int_power_cb(
        gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
        gs::cnf::callback_type reason);

    /// Dynamic/Switching power callback
    gs::cnf::callback_return_type swi_power_cb(
        gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
        gs::cnf::callback_type reason);

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
    uint32_t exec_func(
        tlm::tlm_generic_payload &trans,  // NOLINT(runtime/references)
        sc_core::sc_time &delay,          // NOLINT(runtime/references)
        bool debug = false);
    uint32_t transport_dbg(tlm_generic_payload &gp);  // NOLINT(runtime/references)
    bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data);
    void invalidate_direct_mem_ptr(unsigned int index, sc_dt::uint64 start_range, sc_dt::uint64 end_range);

  private:
    /// Indexer for Memmory models on the mem Socket.
    ///
    /// This class is used to store informations to the connected memmory devices on the bus localy.
    /// See start_of_simulation Function and get_port for usage information.
    class MEMPort {
      public:
        MEMPort(uint32_t id, MEMDevice *dev);
        MEMPort();
        uint32_t id;
        MEMDevice *dev;
        uint32_t base_addr;
        uint32_t addr;
        uint32_t length;
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

    /// The number of total transactions handled by the mctrl
    sr_param<uint64_t> m_total_transactions;  // NOLINT(runtime/int)

    /// The number of successfull ended transactions
    sr_param<uint64_t> m_right_transactions;  // NOLINT(runtime/int)

    /// Total time of power down mode
    sr_param<sc_time> m_power_down_time;

    /// Last time switched to power down mode
    sr_param<sc_time> m_power_down_start;

    /// Total time of deep power down mode
    sr_param<sc_time> m_deep_power_down_time;

    /// Last time switched to deep power down mode
    sr_param<sc_time> m_deep_power_down_start;

    /// Total time of auto self refresh mode
    sr_param<sc_time> m_self_refresh_time;

    /// Last time switched to auto self refresh mode
    sr_param<sc_time> m_self_refresh_start;

    // *****************************************************
    // Power Modeling Parameters

    /// Normalized static power of controller
    sr_param<double> sta_power_norm;

    /// Normalized internal power of controller
    sr_param<double> int_power_norm;

    /// Normalized read energy
    sr_param<double> dyn_read_energy_norm;

    /// Normalized write energy
    sr_param<double> dyn_write_energy_norm;

    /// Controller static power
    sr_param<double> sta_power;

    /// Controller internal power
    sr_param<double> int_power;

    /// Controller switching poer
    sr_param<double> swi_power;

    /// Power frame starting time
    sr_param<sc_core::sc_time> power_frame_starting_time;

    /// Dynamic energy per read access
    sr_param<double> dyn_read_energy;

    /// Dynamic energy per write access
    sr_param<double> dyn_write_energy;

    /// Number of reads from memory
    sr_param<uint64_t> dyn_reads;  // NOLINT(runtime/int)

    /// Number of writes from memory
    sr_param<uint64_t> dyn_writes;  // NOLINT(runtime/int)

    // Constructor parameters (modeling VHDL generics)
    sr_param<int> g_romasel;
    sr_param<int> g_sdrasel;
    sr_param<int> g_romaddr;
    sr_param<int> g_rommask;
    sr_param<int> g_ioaddr;
    sr_param<int> g_iomask;
    sr_param<int> g_ramaddr;
    sr_param<int> g_rammask;
    sr_param<int> g_wprot;
    sr_param<int> g_srbanks;
    sr_param<int> g_ram8;
    sr_param<int> g_ram16;
    sr_param<int> g_sepbus;
    sr_param<int> g_sdbits;
    sr_param<int> g_mobile;
    sr_param<int> g_sden;

    /// Power monitoring on/off
    sr_param<bool> g_pow_mon;

  public:
    // --- Constant bit masks for APB register access

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

    // --- Register default values

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

#endif  // MODELS_MCTRL_MCTRL_H_

/// @}
