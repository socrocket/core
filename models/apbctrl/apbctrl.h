// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup apbctrl
/// @{
/// @file apbctrl.h
/// Class definition of the AHB/APB bridge
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef MODELS_APBCTRL_APBCTRL_H_
#define MODELS_APBCTRL_APBCTRL_H_

#include "ahbslave.h"
#include "apbdevice.h"
#include "clkdevice.h"

#include <greencontrol/config.h>
#include <amba.h>
#include <systemc>

#include "vmap.h"

/// @addtogroup apbctrl APBCtrl
/// @{

class APBCtrl : public AHBSlave<>, public CLKDevice {
 public:

  GC_HAS_CALLBACKS();
  SC_HAS_PROCESS(APBCtrl);

  /// APB master multi-socket
  amba::amba_master_socket<32, 0> apb;

  /// Encapsulation function for functional part of the model (decoder)
  uint32_t exec_func(tlm::tlm_generic_payload &ahb_gp, sc_core::sc_time &delay, bool debug);

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
  APBCtrl(sc_core::sc_module_name nm,    ///< SystemC name
          uint32_t haddr_ = 0xfff,    ///< The MSB address of the AHB area. Sets the 12 MSBs in the AHB address
          uint32_t hmask_ = 0,        ///< The 12bit AHB area address mask
          bool mcheck = 0,            ///< Check if there are any intersections between APB slave memory regions
          uint32_t hindex = 0,        ///< AHB bus index
          bool pow_mon = 0,           ///< Enables power monitoring
          amba::amba_layer_ids ambaLayer = amba::amba_LT);

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

  /// Set up slave map and collect plug & play information
  void start_of_simulation();

  /// SystemC end of simulation hook
  void end_of_simulation();

  /// Calculate power/energy values from normalized input data
  void power_model();

  /// Static power callback
  gs::cnf::callback_return_type sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Dynamic/Internal power callback
  gs::cnf::callback_return_type int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Dynamic/Switching power callback
  gs::cnf::callback_return_type swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

 private:

  typedef tlm::tlm_generic_payload payload_t;
  typedef gs::socket::bindability_base<tlm::tlm_base_protocol_types> socket_t;

  /// Array of slave device information (PNP)
  const uint32_t *mSlaves[16];

  typedef struct {

    uint32_t pindex;
    uint32_t paddr;
    uint32_t pmask;

  } slave_info_t;

  /// Address decoder table (slave index, (bar addr, mask))
  vmap<uint32_t, slave_info_t> slave_map;
  /// iterator for slave map
  vmap<uint32_t, slave_info_t>::iterator it;
  /// iterator for slave map
  typedef vmap<uint32_t, slave_info_t>::iterator slave_iter;

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

  /// The MSB address of the AHB area. Sets the 12 MSBs in the AHB address
  unsigned int m_haddr;

  /// The 12bit AHB area address mask
  unsigned int m_hmask;

  /// Check if there are any intersections between APB slave memory regions
  bool m_mcheck;

  /// Enable power monitoring (Only TLM)
  bool m_pow_mon;

   /// Abstraction Layer
  amba::amba_layer_ids m_ambaLayer;

  /// Number of slaves bound at the APB side
  uint32_t num_of_bindings;

  // *****************************************************
  // Performance Counters

  /// Total number of transactions
  gs::gs_param<unsigned long long> m_total_transactions;

  /// Successful number of transactions
  gs::gs_param<unsigned long long> m_right_transactions;

  // *****************************************************
  // Power Modeling Parameters

  /// Normalized static power input
  gs::gs_param<double> sta_power_norm;

  /// Normalized internal power input (activation independent)
  gs::gs_param<double> int_power_norm;

  /// Normalized read access energy
  gs::gs_param<double> dyn_read_energy_norm;

  /// Normalized write access energy
  gs::gs_param<double> dyn_write_energy_norm;

  /// Parameter array for power data output
  gs::gs_param_array power;

  /// Static power of module
  gs::gs_param<double> sta_power;

  /// Internal power of module (activation independent)
  gs::gs_param<double> int_power;

  /// Switching power of module
  gs::gs_param<double> swi_power;

  /// Power frame starting time
  gs::gs_param<sc_core::sc_time> power_frame_starting_time;

  /// Dynamic energy per read access
  gs::gs_param<double> dyn_read_energy;

  /// Dynamic energy per write access
  gs::gs_param<double> dyn_write_energy;

  /// Number of reads from memory (read & reset by monitor)
  gs::gs_param<unsigned long long> dyn_reads;

  /// Number of writes to memory (read & reset by monitor)
  gs::gs_param<unsigned long long> dyn_writes;

};

/// @}

#endif // MODELS_APBCTRL_APBCTRL_H_
/// @}