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
// Title:      ahbctrl.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    AHB Controller / Bus model.
//             The decoder collects all AHB request from the masters and
//             forwards them to the appropriate slave.
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

#ifndef AHBCTRL_H
#define AHBCTRL_H

#include "socrocket.h"

#include "ahbdevice.h"
#include "clkdevice.h"
#include "signalkit.h"

#include <greencontrol/config.h>
#include <amba.h>
#include <tlm.h>

#include "msclogger.h"

/// @addtogroup ahbctrl AHBctrl
/// @{

class AHBCtrl : public sc_core::sc_module, public CLKDevice {

 public:
 
  SC_HAS_PROCESS(AHBCtrl);
  SK_HAS_SIGNALS(AHBCtrl);

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
  void b_transport(uint32_t id, tlm::tlm_generic_payload& gp, sc_core::sc_time& delay);

  /// TLM non-blocking transport forward (for AHB slave multi-sock)
  tlm::tlm_sync_enum nb_transport_fw(uint32_t id, tlm::tlm_generic_payload& gp,
                                     tlm::tlm_phase& phase, sc_core::sc_time& delay);

  /// TLM non-blocking transport backward (for AHB master multi-sock)
  tlm::tlm_sync_enum nb_transport_bw(uint32_t id, tlm::tlm_generic_payload& gp,
                                      tlm::tlm_phase& phase, sc_core::sc_time& delay);

  /// TLM debug interface
  unsigned int transport_dbg(uint32_t id, tlm::tlm_generic_payload& gp);	

  /// The arbiter thread. Responsible for arbitrating transactions in AT mode.
  void arbitrate();
	
  void AcceptThread();

  void RequestThread();

  void ResponseThread();

  void EndResponseThread();

  /// Collect common transport statistics.
  void transport_statistics(tlm::tlm_generic_payload &gp);

  /// Helper function - prints pending requests in arbiter
  void print_requests();

  /// Print common transport statistics.
  void print_transport_statistics(const char *name) const;

  /// Constructor
  AHBCtrl(sc_core::sc_module_name nm, ///< SystemC name
          unsigned int ioaddr,  ///< The MSB address of the I/O area
          unsigned int iomask,  ///< The I/O area address mask
          unsigned int cfgaddr, ///< The MSB address of the configuration area (PNP)
          unsigned int cfgmask, ///< The address mask of the configuration area
          bool rrobin,          ///< 1 - round robin, 0 - fixed priority arbitration (only AT)
          bool split,           ///< Enable support for AHB SPLIT response (only AT)
          unsigned int defmast, ///< ID of the default master
          bool ioen,            ///< AHB I/O area enable
          bool fixbrst,         ///< Enable support for fixed-length bursts
          bool fpnpen,          ///< Enable full decoding of PnP configuration records.
          bool mcheck,          ///< Check if there are any intersections between core memory regions.
          bool pow_mon,         ///< Enable power monitoring
          amba::amba_layer_ids ambaLayer);		 
		 
  /// Reset Callback
  void dorst(); 
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
  
  /// Desctructor
  ~AHBCtrl();


 private:

  // Data Members
  // ------------

  /// The MSB address of the I/O area
  unsigned int mioaddr;
  /// The I/O area address mask
  unsigned int miomask;
  /// The MSB address of the configuration area (PNP)
  unsigned int mcfgaddr;
  /// The address mask of the configuration area
  unsigned int mcfgmask;
  /// 1 - round robin, 0 - fixed priority arbitration (only AT)
  bool mrrobin;
  /// Enable support for AHB SPLIT response (only AT)
  bool msplit;
  /// ID of the default master
  unsigned int mdefmast;
  /// AHB I/O area enable
  bool mioen;
  /// Enable support for fixed-length bursts
  bool mfixbrst;
  /// Enable support for fixed-length bursts
  bool mfpnpen;
  /// Check if there are any intersections between core memory regions
  bool mmcheck;

  const sc_time arbiter_eval_delay;

  // Shows if bus is busy in LT mode
  bool busy;

  /// Enable power monitoring (Only TLM)
  bool m_pow_mon;
  
  typedef tlm::tlm_generic_payload payload_t;
  typedef gs::socket::bindability_base<tlm::tlm_base_protocol_types> socket_t;

  struct slave_info_t {
    uint32_t hindex;
    uint32_t haddr;
    uint32_t hmask;
  };

  /// The round robin pointer
  unsigned int robin;

  /// Address decoder table (slave index, (bar addr, mask))
  std::map<uint32_t, slave_info_t> slave_map;
  /// Iterator for slave map
  std::map<uint32_t, slave_info_t>::iterator it;
  typedef std::map<uint32_t, slave_info_t>::iterator slave_iter;

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
    payload_t * trans;
    
  } connection_t;

  connection_t request_map[16];
  connection_t response_map[16];

  std::map<payload_t*, connection_t> pending_map;
  std::map<payload_t*, connection_t>::iterator pm_itr;

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
  unsigned int num_of_slave_bindings;
  /// The number of masters in the system
  unsigned int num_of_master_bindings;

  /// GreenControl API container
  gs::cnf::cnf_api *m_api;
        
  /// Open a namespace for performance counting in the greencontrol realm
  gs::gs_param_array m_performance_counters;

  /// Total waiting time in arbiter
  gs::gs_param<sc_time> m_total_wait;

  /// Total number of arbitrated instructions
  gs::gs_param<unsigned long long> m_arbitrated;

  /// Maximum waiting time in arbiter
  gs::gs_param<sc_time> m_max_wait;

  /// ID of the master with the maximum waiting time
  gs::gs_param<unsigned long long> m_max_wait_master;

  /// Number of idle cycles
  gs::gs_param<unsigned long long> m_idle_count;

  /// Total number of transactions handled by the instance
  gs::gs_param<unsigned long long> m_total_transactions;

  /// Succeeded number of transaction handled by the instance
  gs::gs_param<unsigned long long> m_right_transactions;

  /// Counts bytes written to AHBCTRL from the master side
  gs::gs_param<unsigned long long> m_writes;

  /// Counts bytes read from AHBCTRL from the master side
  gs::gs_param<unsigned long long> m_reads;

  /// ID of the master which currently 'owns' the bus
  uint32_t current_master;

  uint32_t requests_pending;

  /// Bus locking payload extension
  amba::amba_lock * lock;
  /// True if bus is supposed to be locked
  bool is_lock;
  /// ID of the master that locked the bus
  uint32_t lock_master;

  /// The abstraction layer of the model
  amba::amba_layer_ids m_ambaLayer;

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

  // Private functions
  // -----------------
  
  /// Set up slave map and collect plug & play information
  void start_of_simulation();
    
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
  void addPendingTransaction(tlm::tlm_generic_payload& trans, connection_t connection);

  /// Check memory map for overlaps
  void checkMemMap();

};

/// @}

#endif // AHBCTRL_H
