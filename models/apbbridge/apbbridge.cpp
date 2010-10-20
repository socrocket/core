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
//*********************************************************************

#include "apbbridge.h"

apb_bridge::apb_bridge(sc_core::sc_module_name nm, sc_dt::uint64 bAddr, 
                       sc_dt::uint64 mSize) :
    sc_module(nm), ahb("ahb", amba::amba_AHB, amba::amba_LT, false), apb("apb",
            amba::amba_APB, amba::amba_LT, false), baseAddr(bAddr), mem_size(
            mSize) {

    assert(
            mem_size % 4 == 0
                    && "invalid size given in bytes, give as a multiple of 4 (or maybe BUSWIDTH)");
    ahb.register_b_transport(this, &apb_bridge::b_transport);
}

apb_bridge::~apb_bridge() {
}

sc_dt::uint64 apb_bridge::get_size() {
    return mem_size;
}

sc_dt::uint64 apb_bridge::get_base_addr() {
    return baseAddr;
}

void apb_bridge::setAddressMap(uint32_t i, sc_dt::uint64 baseAddr,
                               sc_dt::uint64 size) {
    sc_dt::uint64 highAddr = baseAddr + size;
    slave_map.insert(std::pair<uint32_t, slave_info_t>(i, std::pair<
            sc_dt::uint64, sc_dt::uint64>(baseAddr, highAddr)));
}

uint32_t apb_bridge::get_index(uint32_t address) {
    std::map<uint32_t, slave_info_t>::iterator it;

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

    apb_gp->set_command(ahb_gp.get_command());
    apb_gp->set_address(ahb_gp.get_address());
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
        sc_core::sc_object* obj = other_socket->get_parent();
        amba_slave_base * slave = dynamic_cast<amba_slave_base *> (obj);
        sc_dt::uint64 addr = slave->get_base_addr();
        sc_dt::uint64 size = slave->get_size();
        setAddressMap(i, addr, size);
    }
}

