//*********************************************************************
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
//*********************************************************************
// Title:      input_device.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Class definition of a cache-subsystem.
//             The cache-subsystem envelopes an instruction cache,
//             a data cache and a memory management unit.
//             The input_device class provides two TLM slave interfaces
//             for connecting the cpu to the caches and an AHB master
//             interface for connection to the main memory.
//
// Modified on $Date: 2012-02-28 11:45:20 +0100 (Tue, 28 Feb 2012) $
//          at $Revision: 785 $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#ifndef __INPUT_DEVICE_H__
#define __INPUT_DEVICE_H__

#include "ahbdevice.h"
#include "clkdevice.h"

#include <math.h>
#include <greencontrol/config.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <amba.h>

#include "socrocket.h"
#include "signalkit.h"
#include "power_monitor.h"

#include "verbose.h"

/// @addtogroup input_device Input_Device
/// @{

/// Top-level class of the memory sub-system for the TrapGen LEON3 simulator
class input_device : public sc_core::sc_module, public AHBDevice, public CLKDevice {

    public:
	SC_HAS_PROCESS(input_device);
        SK_HAS_SIGNALS(input_device);

        // TLM sockets
        // -----------

        // AHB master socket
        amba::amba_master_socket<32> ahb;

        // SignalKit interrupt output
        signal<bool>::out irq;

        // Constructor
        input_device(sc_core::sc_module_name name, 
                     unsigned int hindex,
                     unsigned int hirq,
                     unsigned int framesize,
                     unsigned int frameaddr,
                     sc_core::sc_time interval,
                     bool pow_mon,
                     amba::amba_layer_ids ambaLayer);

	
	/// TLM non-blocking backward transport function for ahb master socket
	tlm::tlm_sync_enum ahb_nb_transport_bw(tlm::tlm_generic_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &delay);

	/// Thread for response synchronization (AT only: sync and send END_RESP)
	void ResponseThread();

	/// Thread for data phase processing in write operations (AT only: sends BEGIN_DATA)
	void DataThread();

	/// Delayed release of transactions (AT only)
	void cleanUP();

        /// Thread for triggering gen_frame (generates new_frame event)
        void frame_trigger();

        /// Thread for generating the data frame
        void gen_frame();

        /// MemIF implementation - writes data to AHB master
        void mem_write(unsigned int addr, unsigned int asi, unsigned char * data,
                               unsigned int length, sc_core::sc_time * t,
                               unsigned int * debug, bool is_dbg);
	/// MemIF implementation - reads data from AHB master
        void mem_read(unsigned int addr, unsigned int asi, unsigned char * data,
                              unsigned int length, sc_core::sc_time * t,
                              unsigned int * debug, bool is_dbg);

	/// Called at end of simulation to print execution statistics
	void end_of_simulation();

	/// Reset function
	void dorst();

	/// Deal with clock changes
	void clkcng();

        // data members
        // ------------


    private:

        /// IRQ number
        uint32_t m_irq;

        /// Frame size in bytes
        uint32_t m_framesize;

        /// Target address for data frame
        uint32_t m_frameaddr;

        /// Time interval for creating data frame
        sc_core::sc_time m_interval;

        /// ID of the AHB master
        uint32_t m_master_id;

        /// Event for activating the gen_frame thread
        sc_event new_frame;

        /// Pointer to the data frame
        uint32_t * frame;

        /// GreenControl API container
        gs::cnf::cnf_api *m_api;
        /// GreenControl Parameter array
        gs::gs_param_array m_performance_counters;

        /// Total number of successful transactions for execution statistics 
        gs::gs_param<unsigned long long> m_right_transactions;
	/// Total number of transactions for execution statistics
        gs::gs_param<unsigned long long> m_total_transactions;

	/// Power monitoring enabled/disable
	bool m_pow_mon;

	/// amba abstraction layer
	amba::amba_layer_ids m_abstractionLayer;

	/// Event for unblocking mem_read and mem_write from ahb_nb_transport_bw (on END_REQ)
	sc_event  mEndRequestEvent;
	/// Event for unblocking mem_read from ahb_nb_transport_bw (on BEGIN_RESP)
	sc_event  mBeginResponseEvent;

	/// PEQ for transfer of payload from ahb_nb_transport_bw to ResponseThread (on BEGIN_RESP)
	tlm_utils::peq_with_get<tlm::tlm_generic_payload> mResponsePEQ;
	/// PEQ for transfer of payload from ahb_nb_transport_bw or mem_write to DataThread (after END_REQ)
	tlm_utils::peq_with_get<tlm::tlm_generic_payload> mDataPEQ;
	/// PEQ for transfer of expired payload to cleanUP thread.
	tlm_utils::peq_with_get<tlm::tlm_generic_payload> mEndTransactionPEQ;
};

/// @}

#endif //__INPUT_DEVICE_H__
