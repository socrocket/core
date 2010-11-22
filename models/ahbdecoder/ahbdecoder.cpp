// *****************************************************************************
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
// *****************************************************************************
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
// *****************************************************************************

#include "ahbdecoder.h"
#include "verbose.h"

CAHBDecoder::CAHBDecoder(sc_core::sc_module_name nm,
                         amba::amba_layer_ids ambaLayer) :
      sc_module(nm),
      ahbIN("ahbIN", amba::amba_AHB, amba::amba_LT, false),    // TODO Both sockets require is_arbiter=true if bug in ambasockets is fixed
      ahbOUT("ahbOUT", amba::amba_AHB, amba::amba_LT, false) {

    if(ambaLayer==amba::amba_LT) {
      // register tlm blocking transport function
      ahbIN.register_b_transport(this, &CAHBDecoder::b_transport);
    }

    // Register non blocking transport calls
    if(ambaLayer==amba::amba_AT) {
      // register tlm non blocking transport forward path
      ahbIN.register_nb_transport_fw(this, &CAHBDecoder::nb_transport_fw, 1);

      // register tlm non blocking transport backward path
      ahbOUT.register_nb_transport_bw(this, &CAHBDecoder::nb_transport_bw, 1);
    }
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

int CAHBDecoder::getMaster2Slave(const uint32_t slaveID) {
   std::map<uint32_t, int32_t>::iterator it;

   it = MstSlvMap.find(slaveID);
   return it->second;
}

void CAHBDecoder::b_transport(uint32_t id,
                              tlm::tlm_generic_payload& ahb_gp,
                              sc_time& delay) {
    std::map<uint32_t, slave_info_t>::iterator it;

    uint32_t a = 0;
    socket_t* other_socket = ahbIN.get_other_side(id, a);
    sc_core::sc_object *mstobj = other_socket->get_parent();

    int index = get_index(ahb_gp.get_address());

    // check for a valid index
    if(index >= 0) {
       // *** DEBUG
       other_socket = ahbOUT.get_other_side(index, a);
       sc_core::sc_object *obj = other_socket->get_parent();

       v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
                << v::setw(8) << ahb_gp.get_address() << ", from master:"
                << mstobj->name() << ", forwarded to slave:" << obj->name() << endl;
       // ************

       // At this point arbitration and address decoding takes place

       // Wait for semaphore
       SlvSemaphore.find(index)->second->wait();
       // Forward request to the appropriate slave
       ahbOUT[index]->b_transport(ahb_gp, delay);
       // Post to semaphore
       SlvSemaphore.find(index)->second->post();
    } else {
       // Invalid index
       ahb_gp.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
       v::warn << name() << "AHB Request@0x" << hex << v::setfill('0')
               << v::setw(8) << ahb_gp.get_address() << ", from master:"
               << mstobj->name() << ": Unmapped address space." << endl;
    }
}

// TLM non blocking transport call forward path (from masters to slaves)
tlm::tlm_sync_enum CAHBDecoder::nb_transport_fw(uint32_t id, tlm::tlm_generic_payload& gp,
                                                tlm::tlm_phase& phase, sc_core::sc_time& delay) {

   // Obtain slave index for the requested address
   int index = get_index(gp.get_address());

   uint32_t a = 0;
   socket_t* other_socket = ahbIN.get_other_side(id, a);
   sc_core::sc_object *mstobj = other_socket->get_parent();

   if(index>=0) {
      other_socket = ahbOUT.get_other_side(index,a);
      sc_core::sc_object *slvobj = other_socket->get_parent();

      if((getMaster2Slave(index)==static_cast<int>(id)) || (getMaster2Slave(index)==-1)) {
         tlm::tlm_sync_enum returnValue;
         MstSlvMap[index] = id;

         v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
                  << v::setw(8) << gp.get_address() << ", from master:"
                  << mstobj->name() << " forwared to slave:" << slvobj->name()
                  << ", phase:" << phase << endl;

         // Forward request
         returnValue = ahbOUT[index]->nb_transport_fw(gp, phase, delay);

         // Clear transaction mapping if transaction finishes
         if((returnValue==tlm::TLM_COMPLETED) ||
            ((phase==tlm::END_RESP) && (returnValue==tlm::TLM_ACCEPTED))) {
            MstSlvMap[index] = -1;
         }

         // return to initiator
         return returnValue;
      } else {
         // Requested slave is busy. Don't execute command and end
         // transaction.
         v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
                  << v::setw(8) << gp.get_address() << ", from master:"
                  << mstobj->name() << " to slave:" << slvobj->name()
                  << ": Slave busy." << endl;
         return tlm::TLM_COMPLETED;
      }
    } else {
       // Invalid index
       gp.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);

       v::warn << name() << "AHB Request@0x" << hex << v::setfill('0')
               << v::setw(8) << gp.get_address() << ", from master:"
               << mstobj->name() << ": Unmapped address space." << endl;

       return tlm::TLM_COMPLETED;
    }
}  // tlm::tlm_sync_enum CAHBDecoder::nb_transport_fw()

// TLM non blocking transport call backward path (from slaves to masters)
tlm::tlm_sync_enum CAHBDecoder::nb_transport_bw(uint32_t id, tlm::tlm_generic_payload& gp,
                                                tlm::tlm_phase& phase, sc_core::sc_time& delay) {
   int index = getMaster2Slave(id);

   uint32_t a = 0;
   socket_t* other_socket = ahbOUT.get_other_side(id, a);
   sc_core::sc_object *slvobj = other_socket->get_parent();

   if(index >= 0) {
      tlm::tlm_sync_enum returnValue;

      other_socket = ahbIN.get_other_side(index,a);
      sc_core::sc_object *mstobj = other_socket->get_parent();

      v::debug << name() << "AHB Request@0x" << hex << v::setfill('0')
               << v::setw(8) << gp.get_address() << ", from slave:"
               << slvobj->name() << " forwarded to master:" << mstobj->name()
               << ", phase:" << phase << endl;

      // Forward request
      returnValue = ahbIN[index]->nb_transport_bw(gp, phase, delay);
      // Reset MstSlvMap if transaction finishes
      if((returnValue==tlm::TLM_COMPLETED) ||
         ((phase==tlm::END_RESP) && (returnValue==tlm::TLM_UPDATED))) {
         MstSlvMap[id] = -1;
      }

      // Return to initiator
      return returnValue;
   } else {
      v::warn << name() << "Backward path by slave:" << slvobj->name()
              << ": No active connection found." << endl;
      return tlm::TLM_COMPLETED;
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
            sc_core::sc_semaphore* newSema = new sc_core::sc_semaphore(1);
            uint32_t addr = slave->get_base_addr();
            uint32_t size = slave->get_size();
            setAddressMap(i, addr, size); // Insert slave into memory map
            MstSlvMap.insert(std::pair<uint32_t, int32_t>(i, -1));
            SlvSemaphore.insert(std::pair<uint32_t, sc_core::sc_semaphore*>(i, newSema));
            v::info << name() << "Found AHB slave " << obj->name() << "@0x"
                    << hex << v::setw(8) << v::setfill('0') << addr
                    << ", size:" << hex << "0x" << size << endl;
        } else {
            v::warn << name() << "Unexpected NULL object." << v::endl;
        }
    }
    // Check memory map for overlaps
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
