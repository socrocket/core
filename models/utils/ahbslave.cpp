#include "ahbslave.h"
#include "verbose.h"

using namespace std;
using namespace sc_core;
using namespace tlm;

AHBSlave::AHBSlave(sc_module_name mn, int32_t hindex, uint8_t vendor, uint8_t device, uint8_t version, uint8_t irq, amba::amba_layer_ids ambaLayer, uint32_t bar0, uint32_t bar1, uint32_t bar2, uint32_t bar3) :
            sc_module(mn),
            AHBDevice( hindex, vendor, device, version, irq, bar0, bar1, bar2, bar3),
            ahb("ahb", ::amba::amba_AHB, ambaLayer, false /* Abitration */ ), 
            mAcceptPEQ("mAcceptPEQ"), mTransactionPEQ("TransactionPEQ"), busy(false) {

    // Register transport functions to sockets
    ahb.register_b_transport(this, &AHBSlave::b_transport);
    ahb.register_transport_dbg(this, &AHBSlave::transport_dbg);

    // Register non-blocking transport for AT
    if(ambaLayer == amba::amba_AT) {
        ahb.register_nb_transport_fw(this, &AHBSlave::nb_transport_fw);
        
        // Thread for modeling AHB pipeline delay
        SC_THREAD(acceptTXN);
        
        // Thread for interfacing functional part of the model
        // in AT mode.
        SC_THREAD(processTXN);
    }
}

//destructor unregisters callbacks
AHBSlave::~AHBSlave() {
}

// TLM non-blocking forward transport function
tlm::tlm_sync_enum AHBSlave::nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay) {

    v::debug << name() << "nb_transport_fw received transaction " << hex << &trans << " with phase: " << phase << v::endl;

    // The master has sent BEGIN_REQ
    if(phase == tlm::BEGIN_REQ) {
        mAcceptPEQ.notify(trans, delay);
        delay = SC_ZERO_TIME;

    } else if(phase == amba::BEGIN_DATA) {
        mTransactionPEQ.notify(trans, delay);
        delay = SC_ZERO_TIME;

    } else if(phase == tlm::END_RESP) {
        // nothing to do

    } else {
        v::error << name() << "Invalid phase in call to nb_transport_fw!" << v::endl;
        trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    }
    return(tlm::TLM_ACCEPTED);
}

// Thread for modeling the AHB pipeline delay
void AHBSlave::acceptTXN() {

    tlm::tlm_phase phase;
    sc_core::sc_time delay;
    tlm::tlm_sync_enum status;

    tlm::tlm_generic_payload * trans;

    while(1) {
        wait(mAcceptPEQ.get_event());

        while((trans = mAcceptPEQ.get_next_transaction())) {

            // Read transaction will be processed directly.
            // For write we wait for BEGIN_DATA (see nb_transport_fw)
            if(trans->get_command() == TLM_READ_COMMAND) {
                mTransactionPEQ.notify(*trans);
            }

            // Check if new transaction can be accepted
            if(busy == true) {
                wait(unlock_event);
            }

            // Send END_REQ
            phase = tlm::END_REQ;
            delay = SC_ZERO_TIME;

            v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << phase << v::endl;

            // Call to backward transport
            status = ahb->nb_transport_bw(*trans, phase, delay);
            assert(status==tlm::TLM_ACCEPTED);
        }
    }
}

// Process for interfacing the functional part of the model in AT mode
void AHBSlave::processTXN() {
    tlm::tlm_phase phase;
    sc_core::sc_time delay;
    tlm::tlm_sync_enum status;
    tlm::tlm_generic_payload *trans;

    while(1) {
        wait(mTransactionPEQ.get_event());

        while((trans = mTransactionPEQ.get_next_transaction())) {
            v::debug << name() << "Process transaction " << hex << trans << v::endl;

            // Reset delay
            delay = SC_ZERO_TIME;

            // Call the functional part of the model
            exec_transport(*trans, delay);

            // Device busy (can not accept new transaction anymore)
            busy = true;

            // Consume component delay
            wait(delay);

            // Device idle
            busy = false;

            // Ready to accept new transaction
            unlock_event.notify();

            if(trans->get_command() == tlm::TLM_WRITE_COMMAND) {
                // For write commands send END_DATA.
                // Transaction has been delayed until begin of Data Phase (see transport_fw)
                phase = amba::END_DATA;

                v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << phase << " (delay: " << delay << ")" << v::endl;

                // Call backward transport
                status = ahb->nb_transport_bw(*trans, phase, delay);

                assert((status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_COMPLETED));

            } else {
                // Read command - send BEGIN_RESP
                phase = tlm::BEGIN_RESP;

                v::debug << name() << "Transaction " << hex << trans << " call to nb_transport_bw with phase " << phase << " (delay: " << delay << ")" << v::endl;

                // Call backward transport
                status = ahb->nb_transport_bw(*trans, phase, delay);
                assert(status==tlm::TLM_ACCEPTED);
            }
        }
    }
}
       

// TLM blocking transport function
void AHBSlave::b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
    // Call the functional part of the model
    // -------------------------------------
    exec_transport(trans, delay);
}

// TLM blocking transport function
uint32_t AHBSlave::transport_dbg(tlm::tlm_generic_payload& trans) {
    // Call the functional part of the model
    // -------------------------------------
    sc_time delay;
    return exec_transport(trans, delay);
}


/* vim: set expandtab noai ts=4 sw=4: */
/* -*- mode: c-mode; tab-width: 4; indent-tabs-mode: nil; -*- */

