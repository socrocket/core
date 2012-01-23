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

#include <tlm.h>

#include "amba.h"
#include "socrocket.h"
#include "power_monitor.h"

#include "ahbdevice.h"
#include "clkdevice.h"
#include "signalkit.h"

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

	/// The arbiter thread
	void arbitrate_me();
	
	/// The data thread
	void DataThread();

	void EndData();

	/// The request thread (for com. with nb_trans_fw of slaves)
	void RequestThread();

	/// The response thread (for com. with nb_trans_bw of masters)
	void ResponseThread();

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

	payload_t * selected_transaction;

	/// The internal state of the bus controller (concerning arbitration)
	enum TransStateType {IDLE, PENDING, BUSY};

	/// Keeps track on where the transactions have been coming from
	typedef struct {

	  unsigned int master_id;
	  unsigned int slave_id;
	  sc_time start_time;
	  TransStateType state;

	} connection_t;

	std::map<payload_t*, connection_t> pending_map;
	std::map<payload_t*, connection_t>::iterator pm_itr;

	/// Array of slave device information (PNP)
	const uint32_t *mSlaves[64];

	/// Array of master device information (PNP)
	const uint32_t *mMasters[64];

	/// PEQs for arbitration, request notification and responses
	tlm_utils::peq_with_get<payload_t> mRequestPEQ;
	tlm_utils::peq_with_get<payload_t> mDataPEQ;
	tlm_utils::peq_with_get<payload_t> mEndDataPEQ;
	tlm_utils::peq_with_get<payload_t> mResponsePEQ;	

	/// Event triggered by transport_fw to notify response thread about END_RESP
	sc_event mEndResponseEvent;
	/// Event triggered by transport_bw to notify request thread about END_REQ
	sc_event mEndRequestEvent;

	/// The number of slaves in the system
	unsigned int num_of_slave_bindings;
	/// The number of masters in the system
	unsigned int num_of_master_bindings;

	/// Total waiting time in arbiter
	sc_time m_total_wait;

	/// Total number of arbitrated instructions
	uint64_t m_arbitrated;

	/// Maximum waiting time in arbiter
	sc_time m_max_wait;

	/// ID of the master with the maximum waiting time
	uint32_t m_max_wait_master;

	/// Number of idle cycles
	uint64_t m_idle_count;

	/// Total number of transactions handled by the instance
	uint64_t m_total_transactions;

	/// Succeeded number of transaction handled by the instance
	uint64_t m_right_transactions;

	/// The abstraction layer of the model
	amba::amba_layer_ids m_ambaLayer;

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
