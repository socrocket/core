#ifndef AHBSLAVE_H
#define AHBSLAVE_H 

#include <systemc.h>
#include <tlm.h>
#include <amba.h>
#include <stdint.h>

#include "ahbdevice.h"

class AHBSlave : public sc_module, public AHBDevice {
    public:
        SC_HAS_PROCESS(AHBSlave);

        AHBSlave(sc_core::sc_module_name mn, int32_t hindex, uint8_t vendor, uint8_t device, uint8_t version, uint8_t irq, amba::amba_layer_ids ambaLayer, uint32_t bar0 = 0, uint32_t bar1 = 0, uint32_t bar2 = 0, uint32_t bar3 = 0);
        ~AHBSlave();

        /// AHB Slave Socket
        ///
        /// Receives instructions (mem access) from CPU
        ::amba::amba_slave_socket<32> ahb;

        /// Interface to functional part of the model
        virtual uint32_t exec_transport(tlm::tlm_generic_payload &gp, sc_time &delay) = 0;

        /// TLM blocking transport functions
        virtual void b_transport(tlm::tlm_generic_payload& gp, sc_time& delay);

        /// TLM debug transport function
        virtual uint32_t transport_dbg(tlm::tlm_generic_payload& gp);

        /// TLM non-blocking transport function
        virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay);

        /// Accept new transaction (busy or not)
        void acceptTXN();

        /// Thread for interfacing functional part of the model in AT mode
        void processTXN();
    private: 
        /// Ready to accept new transaction (send END_REQ)
        sc_event unlock_event;

        /// Event queues for AT mode
        tlm_utils::peq_with_get<tlm::tlm_generic_payload> mAcceptPEQ;
        tlm_utils::peq_with_get<tlm::tlm_generic_payload> mTransactionPEQ;

        bool busy;
};

/* vim: set expandtab noai ts=4 sw=4: */
/* -*- mode: c-mode; tab-width: 4; indent-tabs-mode: nil; -*- */

#endif // AHBSLAVE_H
