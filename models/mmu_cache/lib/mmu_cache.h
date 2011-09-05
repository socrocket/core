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
// Title:      mmu_cache.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Class definition of a cache-subsystem.
//             The cache-subsystem envelopes an instruction cache,
//             a data cache and a memory management unit.
//             The mmu_cache class provides two TLM slave interfaces
//             for connecting the cpu to the caches and an AHB master
//             interface for connection to the main memory.
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#ifndef __MMU_CACHE_H__
#define __MMU_CACHE_H__

#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

#include <math.h>

#include "icio_payload_extension.h"
#include "dcio_payload_extension.h"

#include "amba.h"
#include "socrocket.h"
#include "signalkit.h"
#include "ahbdevice.h"
#include "power_monitor.h"

#include "verbose.h"
#include "cache_if.h"
#include "ivectorcache.h"
#include "dvectorcache.h"
#include "nocache.h"
#include "mmu_cache_if.h"
#include "mmu.h"
#include "localram.h"

/// @addtogroup mmu_cache MMU_Cache
/// @{

/// Top-level class of the memory sub-system for the TrapGen LEON3 simulator
class mmu_cache : public sc_core::sc_module, public mmu_cache_if, public AHBDevice, public signalkit::signal_module<mmu_cache> {

    public:

        // TLM sockets
        // -----------

        // iu3 instruction cache in/out
        tlm_utils::simple_target_socket<mmu_cache> icio;

        // iu3 data cache in/out
        tlm_utils::simple_target_socket<mmu_cache> dcio;

        // amba master socket
        amba::amba_master_socket<32> ahb;

	// snooping port
	signal<t_snoop>::in snoop;

	SC_HAS_PROCESS(mmu_cache);

        /// @brief Constructor of the top-level class of the memory sub-system (caches and mmu).
        /// @icen          instruction cache enable
        /// @irepl         instruction cache replacement strategy
        /// @isets         number of instruction cache sets
        /// @ilinesize     instruction cache line size (in bytes)
        /// @isetsize      size of an instruction cache set (in kbytes)
        /// @isetlock      enable instruction cache locking
        /// @dcen          data cache enable
        /// @drepl         data cache replacement strategy
        /// @dsets         number of data cache sets
        /// @dlinesize     data cache line size (in bytes)
        /// @dsetsize      size of a data cache set (in kbytes)
        /// @dsetlock      enable data cache locking
        /// @dsnoop        enable data cache snooping
        /// @ilram         enable instruction scratch pad
        /// @ilramsize     size of the instruction scratch pad (in kbytes)
        /// @ilramstart    start address of the instruction scratch pad
        /// @dlram         enable data scratch pad
        /// @dlramsize     size of the data scratch pad (in kbytes)
        /// @dlramstart    start address of the data scratch pad
        /// @cached        fixed cacheability mask
        /// @mmu_en        mmu enable
        /// @itlb_num      number of instruction TLBs
        /// @dtlb_num      number of data TLBs
        /// @tlb_type      split or shared instruction and data TLBs
        /// @tlb_rep       TLB replacement strategy
        /// @mmupgsz       MMU page size
        /// @name                               SystemC module name
        /// @id                                 ID of the AHB master
	/// @abstractionLayer                   Select LT or AT abstraction
        /// @icache_hit_read_response_delay     Delay on an instruction cache hit
        /// @icache_miss_read_response_delay    Delay on an instruction cache miss
        /// @dcache_hit_read_response_delay     Delay on a data cache read hit
        /// @dcache_miss_read_response_delay    Delay on a data cache read miss
        /// @dcache_write_response_delay        Delay on a data cache write (hit/miss)
        /// @itlb_hit_response_delay            Delay on an instruction TLB hit
        /// @itlb_miss_response_delay           Delay on an instruction TLB miss
        /// @dtlb_hit_response_delay            Delay on a data TLB hit
        /// @dtlb_miss_response_delay           Delay on a data TLB miss
        mmu_cache(unsigned int icen, unsigned int irepl, unsigned int isets,
                  unsigned int ilinesize, unsigned int isetsize,
                  unsigned int isetlock, unsigned int dcen, unsigned int drepl,
                  unsigned int dsets, unsigned int dlinesize,
                  unsigned int dsetsize, unsigned int dsetlock,
                  unsigned int dsnoop, unsigned int ilram,
                  unsigned int ilramsize, unsigned int ilramstart,
                  unsigned int dlram, unsigned int dlramsize,
                  unsigned int dlramstart, unsigned int cached,
                  unsigned int mmu_en, unsigned int itlb_num,
                  unsigned int dtlb_num, unsigned int tlb_type,
                  unsigned int tlb_rep, unsigned int mmupgsz,
                  sc_core::sc_module_name name, unsigned int id,
		  bool pow_mon,
		  amba::amba_layer_ids abstractionLayer);

        // member functions
        // ----------------
        // TLM blocking forward transport function for icio socket
        void icio_custom_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay_time);
        // TLM blocking forward transport function for dcio socket
        void dcio_custom_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay_time);

	// TLM non-blocking forward transport function for icio socket
	tlm::tlm_sync_enum icio_custom_nb_transport_fw(tlm::tlm_generic_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &delay_time);
	// TLM non-blocking forward transport function for dcio socket
	tlm::tlm_sync_enum dcio_custom_nb_transport_fw(tlm::tlm_generic_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &delay_time);
	
	// TLM non-blocking backward transport function for ahb master socket
	tlm::tlm_sync_enum ahb_custom_nb_transport_bw(tlm::tlm_generic_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &delay_time);

	// Instruction service thread for AT
	void icio_service_thread();

	// Data service thread for AT
	void dcio_service_thread();

	void ResponseThread();

	void DataThread();

	void cleanUP();

        // interface to AMBA master socket (impl. mem_if)
        virtual void mem_write(unsigned int addr, unsigned char * data,
                               unsigned int length, sc_core::sc_time * t,
                               unsigned int * debug);
        virtual bool mem_read(unsigned int addr, unsigned char * data,
                              unsigned int length, sc_core::sc_time * t,
                              unsigned int * debug);

        // read/write cache control register
        void write_ccr(unsigned char * data, unsigned int len, sc_core::sc_time *delay);
        virtual unsigned int read_ccr();

	// Snooping function (For calling dcache->snoop_invalidate)
	void snoopingCallBack(const t_snoop& snoop, const sc_core::sc_time& delay);

	/// Helper functions for definition of clock cycle
	void clk(sc_core::sc_clock &clk);
	void clk(sc_core::sc_time &period);
	void clk(double period, sc_core::sc_time_unit base);

        // data members
        // ------------
	
	// icio payload event queue (for AT)
	tlm_utils::peq_with_get<tlm::tlm_generic_payload> icio_PEQ;

	// dcio payload event queue (for AT)
	tlm_utils::peq_with_get<tlm::tlm_generic_payload> dcio_PEQ;

        /// instruction cache pointer
        cache_if * icache;
        /// data cache pointer
        cache_if * dcache;
        /// mmu poiner
        mmu * m_mmu;
        /// instruction scratchpad pointer
        localram * ilocalram;
        /// data scratchpad pointer
        localram * dlocalram;

    private:

        // CACHE CONTROL REGISTER
        // ======================
        // [1:0] instruction cache state (ICS) - indicates the current instruction cache state
        // (X0 - disabled, 01 - frozen, 11 - enabled)
        // [3:2] data cache state (DCS) - indicates the current data cache state
        // (X0 - disabled, 01 - frozen, 11 - enabled)
        // [4] instruction cache freeze on interrupt (IF) - if set the instruction cache will automatically be frozen when an asynchronous interrupt is taken
        // [5] data cache freeze on interrupt (DF) - if set the data cache will automatically be frozen when an asynchronous interrupt is taken
        // [14] - data cache flush pending (DP) - This bit is set when an data cache flush operation is in progress
        // [15] - instruction cache flush pending (IP) - This bis is set when an instruction cache flush operation is in progress
        // [16] - instruction burst fetch (IB) - This bit enables burst fill during instruction fetch
        // [21] - Flush Instruction cache (FI) - If set, will flush the instruction cache. Always reads as zero.
        // [22] - Flush data cache (FD)        - If set, will flush the data cache. Always reads as zero.
        // [23] - Data cache snoop enable (DS) - If set, will enable data cache snooping.
        unsigned int CACHE_CONTROL_REG;

	// icache enabled
        unsigned int m_icen;
	// dcache enabled
        unsigned int m_dcen;

	// dcache snooping enabled
	unsigned int m_dsnoop;

	// instruction scratchpad settings
        unsigned int m_ilram;
        unsigned int m_ilramstart;

	// data scratchpad settings
        unsigned int m_dlram;
        unsigned int m_dlramstart;

	// fixed cacheability mask
	unsigned int m_cached;

	// mmu enable
        unsigned int m_mmu_en;

        // amba master id
        unsigned int m_master_id;

	// power monitoring enabled
	bool m_pow_mon;

	// amba abstraction layer
	amba::amba_layer_ids m_abstractionLayer;

        unsigned int m_txn_count;
        unsigned int m_data_count;

        bool m_bus_granted;
        tlm::tlm_generic_payload *current_trans;
        bool m_request_pending;
        bool m_data_pending;
        bool m_bus_req_pending;
        bool m_restart_pending_req;

	// events
	sc_event ahb_transaction_response;

	sc_event  mEndRequestEvent;

	tlm_utils::peq_with_get<tlm::tlm_generic_payload> mResponsePEQ;
	tlm_utils::peq_with_get<tlm::tlm_generic_payload> mDataPEQ;
	tlm_utils::peq_with_get<tlm::tlm_generic_payload> mEndTransactionPEQ;

	/// Clock cycle time
	sc_core::sc_time clockcycle;

};

/// @}

#endif //__MMU_CACHE_H__
