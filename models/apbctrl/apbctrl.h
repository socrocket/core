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
// Purpose:    Implements an LT/AT AHB APB Bridge
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

#include <systemc>
#include <amba.h>
#include <greencontrol/config.h>

#include "ahbdevice.h"
#include "apbdevice.h"
#include "clkdevice.h"
#include "power_monitor.h"
#include "vmap.h"

/// @addtogroup apbctrl APBctrl
/// @{

class APBCtrl : public sc_core::sc_module, public AHBDevice, public CLKDevice {
    public:

        /// AHB slave socket
        amba::amba_slave_socket<32> ahb;
	/// APB master multi-socket
        amba::amba_master_socket<32, 0> apb;

	// Encapsulation function for functional part of the model (decoder)
	void exec_decoder(tlm::tlm_generic_payload &ahb_gp, sc_core::sc_time &delay, bool debug);

	/// Thread for modeling the AHB pipeline delay + busy or not
	void acceptTXN();

	// Thread for interfacing with the functional part of the model in AT mode
	void processTXN();

	/// TLM blocking transport function
        void ahb_b_transport(tlm::tlm_generic_payload& gp, sc_time& delay);	

	/// TLM non-blocking transport forward (for AHB slave sock)
	tlm::tlm_sync_enum ahb_nb_transport_fw(tlm::tlm_generic_payload& gp,
					   tlm::tlm_phase& phase, sc_core::sc_time& delay);

	/// TLM debug interface
	unsigned int transport_dbg(uint32_t id, tlm::tlm_generic_payload& gp);

	/// Helper function for creating slave map decoder entries
        void setAddressMap(const uint32_t binding, const uint32_t pindex, const uint32_t paddr, const uint32_t pmask);

	/// Get slave index for a given address
        int get_index(const uint32_t address);

	/// Returns a PNP register from the APB configuration area (upper 4kb of address space)
	unsigned int getPNPReg(const uint32_t address);

	/// Reset Callback
	void dorst();
  
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

    private:

	/// The MSB address of the AHB area. Sets the 12 MSBs in the AHB address
	unsigned int mhaddr;
	/// The 12bit AHB area address mask
	unsigned int mhmask;
	/// Check if there are any intersections between APB slave memory regions
	bool mmcheck;
	/// Enable power monitoring (Only TLM)
	bool m_pow_mon;

	// Event queue for AT mode
	tlm_utils::peq_with_get<tlm::tlm_generic_payload> mAcceptPEQ;
	tlm_utils::peq_with_get<tlm::tlm_generic_payload> mTransactionPEQ;	

	/// Abstraction Layer
	amba::amba_layer_ids mambaLayer;

	// Ready to accept new transaction (send END_REQ)
	sc_event unlock_event;

	// false - ready to accept new transaction
	bool busy;

	/// The base address of the PNP APB device records
	/// 0xFF000
	const uint32_t m_pnpbase;

        typedef tlm::tlm_generic_payload payload_t;
        typedef gs::socket::bindability_base<tlm::tlm_base_protocol_types> socket_t;

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

	/// Array of slave device information (PNP)
	const uint32_t *mSlaves[16];

  /// GreenControle api instance
  gs::cnf::cnf_api *m_api;

  /// Open a namespace for performance counting in the greencontrol realm
  gs::gs_param_array m_performance_counters;

  /// Total number of transactions
  gs::gs_param<uint64_t> m_total_transactions;

  /// Successful number of transactions
  gs::gs_param<uint64_t> m_right_transactions;

    /// Set up slave map and collect plug & play information
    void start_of_simulation();
        
    /// SystemC end of simulation hook
    void end_of_simulation();

};

/// @}

#endif // APBCTRL_H
