// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup apbctrl
/// @{
/// @file apbctrl.h
/// Class definition of the AHB/APB bridge
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef MODELS_APBCTRL_APBCTRL_H_
#define MODELS_APBCTRL_APBCTRL_H_

#include "core/common/amba.h"
#include "core/common/sr_param.h"
#include "core/common/systemc.h"
#include "core/common/base.h"

#include "core/common/ahbslave.h"
#include "core/common/ahbdevice.h"
#include "core/common/apbdevice.h"
#include "core/common/clkdevice.h"
#include "core/common/vmap.h"

/// @addtogroup apbctrl APBCtrl
/// @{

class APBCtrl : public AHBSlave<>, public CLKDevice {
  public:
    GC_HAS_CALLBACKS();
    SC_HAS_PROCESS(APBCtrl);

    /// APB master multi-socket
    amba::amba_master_socket<32, 0> apb;

    /// Encapsulation function for functional part of the model (decoder)
    uint32_t exec_func(
        tlm::tlm_generic_payload &ahb_gp,  // NOLINT(runtime/references)
        sc_core::sc_time &delay,           // NOLINT(runtime/references)
        bool debug);

    /// Helper function for creating slave map decoder entries
    void setAddressMap(const uint32_t binding, const uint32_t pindex, const uint32_t paddr, const uint32_t pmask);

    /// Get slave index for a given address
    int get_index(const uint32_t address);

    /// Returns a PNP register from the APB configuration area (upper 4kb of address space)
    unsigned int getPNPReg(const uint32_t address);

    /// Reset Callback
    void dorst();

    /// Return clock cycle for ahb interface
    sc_core::sc_time get_clock();

    /// Check memory map for overlaps
    void checkMemMap();

    /// Constructor
    APBCtrl(
      ModuleName nm,  ///< SystemC name
      uint32_t haddr = 0xfff,      ///< The MSB address of the AHB area. Sets the 12 MSBs in the AHB address
      uint32_t hmask = 0,          ///< The 12bit AHB area address mask
      bool mcheck = 0,             ///< Check if there are any intersections between APB slave memory regions
      uint32_t hindex = 0,         ///< AHB bus index
      bool pow_mon = 0,            ///< Enables power monitoring
      AbstractionLayer ambaLayer = amba::amba_LT);

    // Omitted parameters:
    // -------------------
    // nslaves    - Number of APB slaves
    // debug      - Print debug information during simulation
    // Not required. Use verbosity outputs instead.
    // icheck     - Check bus index
    // Not required.
    // enbusmon   - Enable APB bus monitoring
    // Not required
    // asserterr  - Enable assertions for AMBA requirements
    // assertwarn - Enable assertions for AMBA recommendations
    // ccheck     - Sanity checks on PnP configuration records

    /// Desctructor
    ~APBCtrl();

    /// Initialisation function for model generics
    void init_generics();

    /// Systemc end of elaboration hook. Enables power monitoring.
    void end_of_elaboration();

    /// Set up slave map and collect plug & play information
    void start_of_simulation();

    /// SystemC end of simulation hook
    void end_of_simulation();

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

  private:
    typedef tlm::tlm_generic_payload payload_t;
    typedef gs::socket::bindability_base<tlm::tlm_base_protocol_types> socket_t;

    /// Array of slave device information (PNP)
    const uint32_t *mSlaves[16];

    typedef struct {
      uint32_t pindex;
      uint32_t pmask;
      uint32_t binding;
    } slave_info_t;

    /// Address decoder table (slave index, (bar addr, mask))
    std::map<uint32_t, slave_info_t> slave_map;
    std::pair<uint32_t, slave_info_t> slave_map_cache;

    // Event queue for AT mode
    tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_AcceptPEQ;
    tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_TransactionPEQ;

    // Ready to accept new transaction (send END_REQ)
    sc_event unlock_event;

    // false - ready to accept new transaction
    bool busy;

    /// The base address of the PNP APB device records
    /// 0xFF000
    const uint32_t m_pnpbase;
    
    //sr_param<uint32_t> g_haddr;
    //sr_param<uint32_t> g_hmask;
    /// Check if there are any intersections between APB slave memory regions
    sr_param<bool> g_mcheck;

    /// Enable power monitoring (Only TLM)
    sr_param<bool> g_pow_mon;

    /// Abstraction Layer
    AbstractionLayer m_ambaLayer;

    /// Number of slaves bound at the APB side
    uint32_t num_of_bindings;

    // *****************************************************
    // Performance Counters

    /// Total number of transactions
    sr_param<uint64_t> m_total_transactions;  // NOLINT(runtime/int)

    /// Successful number of transactions
    sr_param<uint64_t> m_right_transactions;  // NOLINT(runtime/int)

    // *****************************************************
    // Power Modeling Parameters

    /// Normalized static power input
    sr_param<double> sta_power_norm;

    /// Normalized internal power input (activation independent)
    sr_param<double> int_power_norm;

    /// Normalized read access energy
    sr_param<double> dyn_read_energy_norm;

    /// Normalized write access energy
    sr_param<double> dyn_write_energy_norm;

    /// Static power of module
    sr_param<double> sta_power;

    /// Internal power of module (activation independent)
    sr_param<double> int_power;

    /// Switching power of module
    sr_param<double> swi_power;

    /// Power frame starting time
    sr_param<sc_core::sc_time> power_frame_starting_time;

    /// Dynamic energy per read access
    sr_param<double> dyn_read_energy;

    /// Dynamic energy per write access
    sr_param<double> dyn_write_energy;

    /// Number of reads from memory (read & reset by monitor)
    sr_param<uint64_t> dyn_reads;  // NOLINT(runtime/int)

    /// Number of writes to memory (read & reset by monitor)
    sr_param<uint64_t> dyn_writes;  // NOLINT(runtime/int)
};

/// @}

#endif  // MODELS_APBCTRL_APBCTRL_H_
/// @}
