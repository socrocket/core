// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup ahbctrl AHBCtrl
/// @{
/// @file ahbctrl.h
/// AHB Controller / Bus model. The decoder collects all AHB request from the
/// masters and forwards them to the appropriate slave.
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef MODELS_AHBCTRL_AHBCTRL_H_
#define MODELS_AHBCTRL_AHBCTRL_H_

#include "core/common/amba.h"
#include "core/common/base.h"
#include "core/common/sr_param.h"
#include <tlm.h>
#include <map>

#include "core/common/ahbdevice.h"
#include "core/common/clkdevice.h"
#include "core/common/sr_signal.h"
#include "core/common/msclogger.h"
#include "core/common/socrocket.h"
#include "core/common/sr_param.h"

class AHBCtrl : public BaseModule<DefaultBase>, public CLKDevice {
  public:
    GC_HAS_CALLBACKS();
    SC_HAS_PROCESS(AHBCtrl);
    SR_HAS_SIGNALS(AHBCtrl);

    // AMBA sockets
    // ------------------

    /// AHB slave multi-socket
    amba::amba_slave_socket<32, 0>  ahbIN;
    /// AHB master multi-socket
    amba::amba_master_socket<32, 0> ahbOUT;
    /// Broadcast of master_id and write address for dcache snooping
    signal<t_snoop>::out snoop;

    // Public functions
    // ----------------

    /// TLM blocking transport method
    void b_transport(
        uint32_t id,
        tlm::tlm_generic_payload &gp,  // NOLINT(runtime/references)
        sc_core::sc_time &delay);      // NOLINT(runtime/references)

    /// TLM non-blocking transport forward (for AHB slave multi-sock)
    tlm::tlm_sync_enum nb_transport_fw(
        uint32_t id,
        tlm::tlm_generic_payload &gp,  // NOLINT(runtime/references)
        tlm::tlm_phase &phase,         // NOLINT(runtime/references)
        sc_core::sc_time &delay);      // NOLINT(runtime/references)

    /// TLM non-blocking transport backward (for AHB master multi-sock)
    tlm::tlm_sync_enum nb_transport_bw(
        uint32_t id, tlm::tlm_generic_payload &gp,  // NOLINT(runtime/references)
        tlm::tlm_phase &phase,                      // NOLINT(runtime/references)
        sc_core::sc_time &delay);                   // NOLINT(runtime/references)

    /// TLM debug interface
    unsigned int transport_dbg(uint32_t id, tlm::tlm_generic_payload &gp);  // NOLINT(runtime/references)

    /// DMI Pathes
    virtual bool get_direct_mem_ptr(unsigned int index, tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data);
    virtual void invalidate_direct_mem_ptr(unsigned int index, sc_dt::uint64 start_range, sc_dt::uint64 end_range);

    /// The arbiter thread. Responsible for arbitrating transactions in AT mode.
    void arbitrate();

    void AcceptThread();

    void RequestThread();

    void ResponseThread();

    void EndResponseThread();

    /// Collect common transport statistics.
    void transport_statistics(tlm::tlm_generic_payload &gp);  // NOLINT(runtime/references)

    /// Helper function - prints pending requests in arbiter
    void print_requests();

    /// Print common transport statistics.
    void print_transport_statistics(const char *name) const;

    /// Constructor
    // Omitted parameters:
    // -------------------
    // nahbm  - Number of AHB masters
    // nahbs  - Number of AHB slaves
    // It is checked that the number of binding does not raise above 16.
    // Apart from that the parameters are not required.
    // debug  - Print configuration
    // Not required. Use verbosity outputs instead.
    // icheck - Check bus index
    // Not required.
    // enbusmon - Enable AHB bus monitor
    // assertwarn - Enable assertions for AMBA recommendations.
    // asserterr - Enable assertion for AMBA requirements
    AHBCtrl(
      ModuleName nm,  ///< SystemC name
      uint32_t ioaddr = 0xFFF,     ///< The MSB address of the I/O area
      uint32_t iomask = 0xFFF,     ///< The I/O area address mask
      uint32_t cfgaddr = 0xFF0,    ///< The MSB address of the configuration area (PNP)
      uint32_t cfgmask = 0xFF0,    ///< The address mask of the configuration area
      bool rrobin = false,         ///< 1 - round robin, 0 - fixed priority arbitration (only AT)
      bool split = false,          ///< Enable support for AHB SPLIT response (only AT)
      uint32_t defmast = 0,        ///< ID of the default master
      bool ioen = true,            ///< AHB I/O area enable
      bool fixbrst = false,        ///< Enable support for fixed-length bursts
      bool fpnpen = true,          ///< Enable full decoding of PnP configuration records.
      bool mcheck = true,          ///< Check if there are any intersections between core memory regions.
      bool pow_mon = false,        ///< Enable power monitoring
      AbstractionLayer ambaLayer = amba::amba_LT) __attribute__ ((deprecated));

    AHBCtrl(
      ModuleName nm,               ///< SystemC name
      AbstractionLayer ambaLayer,
      uint32_t ioaddr = 0xFFF,     ///< The MSB address of the I/O area
      uint32_t iomask = 0xFFF,     ///< The I/O area address mask
      uint32_t cfgaddr = 0xFF0,    ///< The MSB address of the configuration area (PNP)
      uint32_t cfgmask = 0xFF0,    ///< The address mask of the configuration area
      bool rrobin = false,         ///< 1 - round robin, 0 - fixed priority arbitration (only AT)
      bool split = false,          ///< Enable support for AHB SPLIT response (only AT)
      uint32_t defmast = 0,        ///< ID of the default master
      bool ioen = true,            ///< AHB I/O area enable
      bool fixbrst = false,        ///< Enable support for fixed-length bursts
      bool fpnpen = true,          ///< Enable full decoding of PnP configuration records.
      bool mcheck = true,          ///< Check if there are any intersections between core memory regions.
      bool pow_mon = false         ///< Enable power monitoring
    );

    /// Reset Callback
    void dorst();

    /// Desctructor
    ~AHBCtrl();

    /// Initialisation function for model generics
    void init_generics();


  private:
    // Data Members
    // ------------
    /// The MSB address of the I/O area
    sr_param<uint32_t> g_ioaddr;

    /// The I/O area address mask
    sr_param<uint32_t> g_iomask;

    /// The MSB address of the configuration area (PNP)
    sr_param<uint32_t> g_cfgaddr;

    /// The address mask of the configuration area
    sr_param<uint32_t> g_cfgmask;

    /// 1 - round robin, 0 - fixed priority arbitration (only AT)
    sr_param<bool> g_rrobin;

    /// Enable support for AHB SPLIT response (only AT)
    sr_param<bool> g_split;

    /// ID of the default master
    sr_param<uint32_t> g_defmast;

    /// AHB I/O area enable
    sr_param<bool> g_ioen;

    /// Enable support for fixed-length bursts
    sr_param<bool> g_fixbrst;

    /// Enable support for fixed-length bursts
    sr_param<bool> g_fpnpen;

    /// Check if there are any intersections between core memory regions
    sr_param<bool> g_mcheck;

    /// Enable power monitoring (Only TLM)
    sr_param<bool> g_pow_mon;

    const sc_time arbiter_eval_delay;

    // Shows if bus is busy in LT mode
    bool busy;

    typedef tlm::tlm_generic_payload payload_t;
    typedef gs::socket::bindability_base<tlm::tlm_base_protocol_types> socket_t;

    struct slave_info_t {
      uint32_t hindex;
      uint32_t hmask;
      uint32_t binding;
    };

    /// The round robin pointer
    unsigned int robin;

    /// Address decoder table (slave index, (bar addr, mask))
    std::map<uint32_t, slave_info_t> slave_map;
    std::pair<uint32_t, slave_info_t> slave_map_cache;

    /// Connection state:
    //  -----------------
    //  PENDING - Waiting for arbitration
    //  APHASE  - AHB address phase. BEGIN_REQ was sent to slave.
    //  DPHASE  - AHB data phase. Slave has sent BEGIN_RESP.
    enum TransStateType {TRANS_INIT, TRANS_PENDING, TRANS_SCHEDULED};
    enum DbusStateType {IDLE, RESPONSE, WAITSTATES};

    /// Keeps track on where the transactions have been coming from
    typedef struct {
      unsigned int master_id;
      unsigned int slave_id;
      sc_time start_time;
      TransStateType state;
      payload_t *trans;
    } connection_t;

    connection_t request_map[16];
    connection_t response_map[16];

    std::map<payload_t *, connection_t> pending_map;
    std::map<payload_t *, connection_t>::iterator pm_itr;

    /// Array of slave device information (PNP)
    const uint32_t *mSlaves[64];

    /// Array of master device information (PNP)
    const uint32_t *mMasters[64];

    int32_t address_bus_owner;
    DbusStateType data_bus_state;

    /// PEQs for arbitration, request notification and responses
    tlm_utils::peq_with_get<payload_t> m_AcceptPEQ;
    tlm_utils::peq_with_get<payload_t> m_RequestPEQ;
    tlm_utils::peq_with_get<payload_t> m_ResponsePEQ;
    tlm_utils::peq_with_get<payload_t> m_EndResponsePEQ;

    /// The number of slaves in the system
    sr_param<unsigned int> num_of_slave_bindings;
    /// The number of masters in the system
    sr_param<unsigned int> num_of_master_bindings;

    /// Total waiting time in arbiter
    sr_param<sc_time> m_total_wait;

    /// Total number of arbitrated instructions
    sr_param<unsigned long long> m_arbitrated;  // NOLINT(runtime/int)

    /// Maximum waiting time in arbiter
    sr_param<sc_time> m_max_wait;

    /// ID of the master with the maximum waiting time
    sr_param<uint64_t> m_max_wait_master;  // NOLINT(runtime/int)

    /// Number of idle cycles
    sr_param<uint64_t> m_idle_count;  // NOLINT(runtime/int)

    /// Total number of transactions handled by the instance
    sr_param<uint64_t> m_total_transactions;  // NOLINT(runtime/int)

    /// Succeeded number of transaction handled by the instance
    sr_param<uint64_t> m_right_transactions;  // NOLINT(runtime/int)

    /// Counts bytes written to AHBCTRL from the master side
    sr_param<uint64_t> m_writes;  // NOLINT(runtime/int)

    /// Counts bytes read from AHBCTRL from the master side
    sr_param<uint64_t> m_reads;  // NOLINT(runtime/int)

    /// ID of the master which currently 'owns' the bus
    uint32_t current_master;

    uint32_t requests_pending;

    /// Bus locking payload extension
    amba::amba_lock *lock;
    /// True if bus is supposed to be locked
    bool is_lock;
    /// ID of the master that locked the bus
    uint32_t lock_master;

    /// The abstraction layer of the model
    AbstractionLayer m_ambaLayer;

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

    /// Dynamic power of module (activation independent)
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

    // Private functions
    // -----------------

    /// Set up slave map and collect plug & play information
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

    /// SystemC End of Simulation handler
    /// Prints out the Performance Counter
    void end_of_simulation();

    /// Helper function for creating slave map decoder entries
    void setAddressMap(const uint32_t binding, const uint32_t hindex, const uint32_t haddr, const uint32_t hmask);

    /// Get slave index for a given address
    int get_index(const uint32_t address);

    /// Returns a PNP register from the slave configuration area
    unsigned int getPNPReg(const uint32_t address);

    /// Keeps track of master-payload relation
    void addPendingTransaction(
        tlm::tlm_generic_payload &trans,  // NOLINT(runtime/references)
        connection_t connection);

    /// Check memory map for overlaps
    void checkMemMap();
};

#endif  // MODELS_AHBCTRL_AHBCTRL_H_
/// @}
