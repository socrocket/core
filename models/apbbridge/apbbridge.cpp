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
// Title:      apbbridge.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implements an LT/AT AHB APB Bridge
//
// Method:
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

#include "apbbridge.h"
#include "verbose.h"

apb_bridge::apb_bridge(sc_core::sc_module_name nm, uint32_t haddr_, 
                       uint32_t hmask_) :
      sc_module(nm),
      CGrlibDevice(0x04, // vendor_id: ESA
                   0, // device: TODO: get real device ID
                   0, //
                   0, // IRQ
                   GrlibBAR(CGrlibDevice::AHBMEM, hmask_, 0, 0, haddr_), // GrlibBAR 0
                   0, // GrlibBAR 1
                   0, // GrlibBAR 2
                   0  // GrlibBAR 3
      ),
      ahb("ahb", amba::amba_AHB, amba::amba_LT, false),
      apb("apb", amba::amba_APB, amba::amba_LT, false),
      HADDR(haddr_),
      HMASK(hmask_) {

    // Assert generics are withing allowed ranges
    assert(HMASK <= 0xfff);
    assert(HMASK <= 0xfff);

    // register tlm blocking transport function
    ahb.register_b_transport(this, &apb_bridge::b_transport);

    // Display AHB slave information
    v::info << name() << "AHB slave @0x" << hex << v::setw(8)
            << v::setfill('0') << get_base_addr() << " size: 0x" << hex
            << v::setw(8) << v::setfill('0') << get_size() << " byte" << endl;
}

apb_bridge::~apb_bridge() {
}

void apb_bridge::setAddressMap(uint32_t i, sc_dt::uint64 baseAddr,
                               sc_dt::uint64 size) {
    baseAddr &= APBADDRMASK;
    sc_dt::uint64 highAddr = baseAddr + size;
    slave_map.insert(std::pair<uint32_t, slave_info_t>(i, std::pair<
            sc_dt::uint64, sc_dt::uint64>(baseAddr, highAddr)));
}

uint32_t apb_bridge::get_index(uint32_t address) {
    std::map<uint32_t, slave_info_t>::iterator it;

    address &= APBADDRMASK;
    for (it = slave_map.begin(); it != slave_map.end(); it++) {
        slave_info_t info = it->second;
        if (address >= info.first && address < info.second) {
            return it->first;
        }
    }
    assert("address is not in this range");

    return 0;
}

void apb_bridge::b_transport(tlm::tlm_generic_payload& ahb_gp,
                             sc_time& delay) {
    uint32_t index = get_index(ahb_gp.get_address());
    payload_t *apb_gp = apb.get_transaction();
    std::map<uint32_t, slave_info_t>::iterator it;

    // Warn if access exceeds the selected slave's memory region
    for(it=slave_map.begin(); it->first!=index; it++) {}
    if( !(it->second.second >= (ahb_gp.get_address() & APBADDRMASK) + ahb_gp.get_data_length())) {
       v::warn << name() << "Transaction length exceeds slave region." << endl;
    }

    apb_gp->set_command(ahb_gp.get_command());
    apb_gp->set_address(ahb_gp.get_address() & APBADDRMASK);
    apb_gp->set_data_length(ahb_gp.get_data_length());
    apb_gp->set_streaming_width(ahb_gp.get_streaming_width());
    apb_gp->set_byte_enable_ptr(ahb_gp.get_byte_enable_ptr());
    apb_gp->set_data_ptr(ahb_gp.get_data_ptr());
    apb[index]->b_transport(*apb_gp, delay);
    ahb_gp.set_response_status(apb_gp->get_response_status());

    apb.release_transaction(apb_gp);
}

void apb_bridge::start_of_simulation() {
    uint32_t num_of_bindings = apb.size();

    v::info << name()
            << "Start_of_simulation,mapping the memory ranges of all slaves"
            << v::endl;

    for (uint32_t i = 0; i < num_of_bindings; i++) {
         uint32_t a = 0;

        socket_t *other_socket = apb.get_other_side(i, a);
        sc_core::sc_object *obj = other_socket->get_parent();
        amba_slave_base *slave = dynamic_cast<amba_slave_base *> (obj);
        if(slave) {
            sc_dt::uint64 addr = slave->get_base_addr();
            sc_dt::uint64 size = slave->get_size();
            setAddressMap(i, addr, size);
        } else {
            v::warn << name() << "Unexpected NULL object." << v::endl;
        }
    }
}
