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
// Title:      ahbdecoder.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    AHB address decoder.
//             The decoder collects all AHB request from the masters and
//             forwards them to the appropriate slave.
//
// Method:
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Soeren Brinkmann
// Reviewed:
// ********************************************************************

#include "ahbdecoder.h"
#include "verbose.h"

CAHBDecoder::CAHBDecoder(sc_core::sc_module_name nm) :
      sc_module(nm),
      ahbIN("ahbIN", amba::amba_AHB, amba::amba_LT, false),    // TODO Both sockets require is_arbiter=true if bug in ambasockets is fixed
      ahbOUT("ahbOUT", amba::amba_AHB, amba::amba_LT, false) {

    // register tlm blocking transport function
    ahbIN.register_b_transport(this, &CAHBDecoder::b_transport);
}

CAHBDecoder::~CAHBDecoder() {
}

void CAHBDecoder::setAddressMap(const uint32_t i, const uint32_t baseAddr,
                                const uint32_t size) {
    uint32_t highAddr = baseAddr + size;
    slave_map.insert(std::pair<uint32_t, slave_info_t>
                        (i, slave_info_t(baseAddr, highAddr)));
}

int CAHBDecoder::get_index(const uint32_t address) {
    std::map<uint32_t, slave_info_t>::iterator it;

    for (it = slave_map.begin(); it != slave_map.end(); it++) {
        slave_info_t info = it->second;
        if (address >= info.first && address < info.second) {
            return it->first;
        }
    }
    v::warn << name() << "No address -> slave mapping found." << endl;

    return -1;
}

void CAHBDecoder::b_transport(uint32_t id,
                              tlm::tlm_generic_payload& ahb_gp,
                              sc_time& delay) {
    std::map<uint32_t, slave_info_t>::iterator it;

    uint32_t index = get_index(ahb_gp.get_address());

    // check for a valid index
    if(index >= 0) {
       // *** DEBUG
       uint32_t a = 0;
       socket_t *other_socket = ahbOUT.get_other_side(index, a);
       sc_core::sc_object *obj = other_socket->get_parent();

       other_socket = ahbIN.get_other_side(id, a);
       sc_core::sc_object *mstobj = other_socket->get_parent();


       v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
                << v::setw(8) << ahb_gp.get_address() << ", from master:"
                << mstobj->name() << ", forwarded to slave:" << obj->name() << endl;
       // ************

       // At this point arbitration and address decoding takes place

       // Forward request to the appropriate slave
       ahbOUT[index]->b_transport(ahb_gp, delay);
    } else {
       // Invalid index
       // TODO set response status to what?
       // Is access to unmapped memory illegal?
       // Is it successfully ignored?
       v::warn << name() << "Access to unmapped AHB address space." << endl;
    }

}

void CAHBDecoder::start_of_simulation() {
    uint32_t num_of_bindings = ahbOUT.size();

    v::info << name()
            << "Start_of_simulation,mapping the memory ranges of all slaves"
            << v::endl;

    for (uint32_t i = 0; i < num_of_bindings; i++) {
         uint32_t a = 0;

        socket_t *other_socket = ahbOUT.get_other_side(i, a);
        sc_core::sc_object *obj = other_socket->get_parent();
        amba_slave_base *slave = dynamic_cast<amba_slave_base *> (obj);
        if(slave) {
            uint32_t addr = slave->get_base_addr();
            uint32_t size = slave->get_size();
            setAddressMap(i, addr, size);
            v::info << name() << "Found AHB slave " << obj->name() << "@0x"
                    << hex << v::setw(8) << v::setfill('0') << addr
                    << ", size:" << hex << "0x" << size << endl;
        } else {
            v::warn << name() << "Unexpected NULL object." << v::endl;
        }
    }
    checkMemMap();
}

void CAHBDecoder::checkMemMap() {
   std::map<uint32_t, slave_info_t>::iterator it;
   std::map<uint32_t, slave_info_t>::iterator it2;

   for(it=slave_map.begin(), it2=slave_map.begin(); it!=slave_map.end(); it++, it2++) {
      for(it2++; it2!=slave_map.end(); it2++) {
         if(((it2->second.first >= it->second.first) &&
            (it2->second.first < it->second.second)) ||
            (((it2->second.second - 1) >= it->second.first) &&
             ((it2->second.second - 1)< it->second.second))) {
            // Memory regions overlap output warning
             uint32_t a = 0;
             socket_t *other_socket = ahbOUT.get_other_side(it->first, a);
             sc_core::sc_object *obj = other_socket->get_parent();

             other_socket = ahbOUT.get_other_side(it2->first, a);
             sc_core::sc_object *obj2 = other_socket->get_parent();

             v::error << name() << "Overlap in AHB memory mapping." << endl;
             v::debug << name() << obj->name() << "@0x" << hex << v::setw(8)
                      << v::setfill('0') << it->second.first << ":0x" << hex
                      << v::setw(8) << v::setfill('0') << (it->second.second - 1) << endl;
             v::debug << name() << obj2->name() << "@0x" << hex << v::setw(8)
                      << v::setfill('0') << it2->second.first << ":0x" << hex
                      << v::setw(8) << v::setfill('0') << (it2->second.second - 1)
                      << endl;

         }
      }
   }
}
