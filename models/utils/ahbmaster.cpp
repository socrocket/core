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
// Title:      ahbmaster.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
// ********************************************************************

#include "ahbmaster.h"

#ifndef MTI_SYSTEMC
//#include <greensocket/initiator/multi_socket.h>
#include <greenreg_ambasockets.h>
#endif

using namespace std;
using namespace sc_core;
using namespace tlm;

// Constructor (sc_module version)
template<>
AHBMaster<sc_module>::AHBMaster(sc_module_name nm, uint8_t hindex, uint8_t vendor, uint8_t device, uint8_t version, uint8_t irq, 
                                amba::amba_layer_ids ambaLayer, uint32_t bar0, uint32_t bar1, uint32_t bar2, uint32_t bar3) :
  sc_module(nm),
  AHBDevice(hindex, vendor, device, version, irq, bar0, bar1, bar2, bar3),
  ahb("ahb", ::amba::amba_AHB, ambaLayer, false),
  m_ResponsePEQ("ResponsePEQ"),
  m_ambaLayer(ambaLayer),
  m_performance_counters("performance_counters"),
  m_reads("bytes_read", 0llu, m_performance_counters), m_writes("bytes_written", 0llu, m_performance_counters),
  response_error(false) { 

    if(ambaLayer == amba::amba_AT) {
        // Register backward transport function
        ahb.register_nb_transport_bw(this, &AHBMaster::nb_transport_bw);

        // Thread for response processing (read)
        SC_THREAD(ResponseThread);
    }

    sc_module *self = dynamic_cast<sc_module *>(this);
    if(self) {
        m_api = gs::cnf::GCnf_Api::getApiInstance(self);
    } else {
        v::error << name() << "A AHBDevice instance must also inherit from sc_module when it gets instantiated. "
                           << "To ensure the performance counter will work correctly" << v::endl;
    }
}

#ifndef MTI_SYSTEMC

// Constructor (gr_device version)
template<>
AHBMaster<gs::reg::gr_device>::AHBMaster(sc_module_name nm, uint8_t hindex, uint8_t vendor, uint8_t device, uint8_t version, uint8_t irq, 
                                         amba::amba_layer_ids ambaLayer, uint32_t bar0, uint32_t bar1, uint32_t bar2, uint32_t bar3) :
  gr_device(nm, gs::reg::ALIGNED_ADDRESS, 16, NULL),
  AHBDevice(hindex, vendor, device, version, irq, bar0, bar1, bar2, bar3),
  ahb("ahb", ::amba::amba_AHB, ambaLayer, false),
  m_ResponsePEQ("ResponsePEQ"),
  m_ambaLayer(ambaLayer),
  m_performance_counters("performance_counters"),
  m_reads("bytes_read", 0llu, m_performance_counters), m_writes("bytes_written", 0llu, m_performance_counters) {

     if (ambaLayer == amba::amba_AT) {
        // Register backward transport function
        ahb.register_nb_transport_bw(this, &AHBMaster::nb_transport_bw);

        // Thread for response processing (read)
        SC_THREAD(ResponseThread);

      }

      sc_module *self = dynamic_cast<sc_module *>(this);
      if(self) {
          m_api = gs::cnf::GCnf_Api::getApiInstance(self);
      } else {
          v::error << name() << "A AHBDevice instance must also inherit from sc_module when it gets instantiated. "
                             << "To ensure the performance counter will work correctly" << v::endl;
      }
}

#endif
