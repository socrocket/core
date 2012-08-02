// ********************************************************************
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
// ********************************************************************
// Title:      apbctrl.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Class definition of the AHB2APB bridge
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
// ********************************************************************

#ifndef APBCTRL_H
#define APBCTRL_H

#include "ahbslave.h"
#include "apbdevice.h"
#include "clkdevice.h"

#include <greencontrol/config.h>
#include <amba.h>
#include <systemc>

#include "power_monitor.h"
#include "vmap.h"

/// @addtogroup apbctrl APBctrl
/// @{

class APBCtrl : public AHBSlave<>, public CLKDevice {
 public:

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

  SC_HAS_PROCESS(APBCtrl);

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

  /// Normalized dynamic power input (activation independent)
  gs::gs_param<double> dyn_power_norm;

  /// Normalized read access energy
  gs::gs_param<double> dyn_read_energy_norm;

  /// Normalized write access energy
  gs::gs_param<double> dyn_write_energy_norm;  

  /// Parameter array for power data output
  gs::gs_param_array power;

  /// Static power of module
  gs::gs_param<double> sta_power;

  /// Dynamic power of module (activation independent)
  gs::gs_param<double> dyn_power;  

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

#endif // APBCTRL_H
