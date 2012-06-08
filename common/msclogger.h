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
// Title:      msclogger.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Utility class for timing verification.
//             Keeps track of TLM communication by
//             writing a MSCGEN control file. 
//             
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


#ifndef MSCLOGGER_H
#define MSCLOGGER_H

#include <tlm.h>
#include <amba.h>
#include <iostream>
#include <fstream>

#include "verbose.h"


extern std::ofstream msc;
extern sc_core::sc_time msclogger_start;
extern sc_core::sc_time msclogger_end;

#ifdef MSCLOGGER
static const uint32_t msclogger_enable = 1;
#else
static const uint32_t msclogger_enable = 1;
#endif

class msclogger {

 public:

  typedef tlm::tlm_generic_payload payload_t;
  typedef gs::socket::bindability_base<tlm::tlm_base_protocol_types> socket_t;

  static void msc_start(const char * filename, const char * nodes) {

    if (msclogger_enable) {

      msc.open(filename);
  
      msc << "msc {\n";
      msc << "  hscale=\"2\";\n\n";
      msc << "  " << nodes << ";\n\n";

    }
  }

  static void msc_end() {

    if (msclogger_enable) {

      msc << "\n}\n";
      msc.close();

    }
  }


  static void forward(sc_core::sc_object * from, amba::amba_master_socket<32> * ahb, tlm::tlm_generic_payload * trans, tlm::tlm_phase phase, sc_core::sc_time delay = SC_ZERO_TIME) {
    if (msclogger_enable) {

      uint32_t a = 0;
      socket_t *socket_to = ahb->get_other_side(0, a);
      sc_core::sc_object * to = socket_to->get_parent();
  
      msc << "  " << from->name() << "=>" << to->name() << " [ label = \"" << phase << "(" << trans << "/" << sc_time_stamp() \
          << "/" << delay << ")\", linecolour = \"#" << hex << reinterpret_cast<uint32_t>(trans) << "\", textcolour = \"#" << hex << (uint32_t)trans << "\"];\n";

    }
  }

  static void forward(sc_core::sc_object * from, amba::amba_master_socket<32,0> * ahb, tlm::tlm_generic_payload * trans, tlm::tlm_phase phase, sc_core::sc_time delay = SC_ZERO_TIME, uint32_t binding = 0) {
    if (msclogger_enable) {

      uint32_t a = 0;
      socket_t *socket_to = ahb->get_other_side(binding, a);
      sc_core::sc_object * to = socket_to->get_parent();
  
      msc << "  " << from->name() << "=>" << to->name() << " [ label = \"" << phase << "(" << trans << "/"  << sc_time_stamp() \
          << "/" << delay << ")\", linecolour = \"#" << hex << reinterpret_cast<uint32_t>(trans) << "\", textcolour = \"#" << hex << (uint32_t)trans << "\"];\n";

    }
  }

  static void backward(sc_core::sc_object * from, amba::amba_slave_socket<32> * ahb, tlm::tlm_generic_payload * trans, tlm::tlm_phase phase, sc_core::sc_time delay = SC_ZERO_TIME) {

    if (msclogger_enable) {

      uint32_t a = 0;
      socket_t *socket_to = ahb->get_other_side(0, a);
      sc_core::sc_object * to = socket_to->get_parent();
    
      msc << "  " << from->name() << "=>" << to->name() << " [ label = \"" << phase << "(" << trans << "/" << sc_time_stamp() \
          << "/" << delay << ")\", linecolour = \"#" << hex << reinterpret_cast<uint32_t>(trans) << "\", textcolour = \"#" << hex << (uint32_t)trans << "\"];\n";

    }
  }

  static void backward(sc_core::sc_object * from, amba::amba_slave_socket<32,0> * ahb, tlm::tlm_generic_payload * trans, tlm::tlm_phase phase, sc_core::sc_time delay = SC_ZERO_TIME, uint32_t binding = 0) {

    if (msclogger_enable) {

      uint32_t a = 0;
      socket_t *socket_to = ahb->get_other_side(binding, a);
      sc_core::sc_object * to = socket_to->get_parent();
    
      msc << "  " << from->name() << "=>" << to->name() << " [ label = \"" << phase << "(" << trans << "/" << sc_time_stamp() \
          << "/" << delay << ")\", linecolour = \"#" << hex << reinterpret_cast<uint32_t>(trans) << "\", textcolour = \"#" << hex << (uint32_t)trans << "\"];\n";
    }
  }

  static void return_forward(sc_core::sc_object * from, amba::amba_master_socket<32> * ahb, tlm::tlm_generic_payload * trans, tlm::tlm_sync_enum status, sc_core::sc_time delay = SC_ZERO_TIME) {

    if (msclogger_enable) {

      uint32_t a = 0;
      socket_t *socket_to = ahb->get_other_side(0, a);
      sc_core::sc_object * to = socket_to->get_parent();

      msc << "  " << from->name() << ">>" << to->name() << " [ label = \"";
        
      switch (status) {

      case 0:
          
        msc << "TLM_ACCEPTED";
        break;
      case 1:

        msc << "TLM_UPDATED";
        break;
      default:

        msc << "TLM_COMPLETED";
      }

      msc << "(" << trans << "/" << sc_time_stamp() << "/" << delay << ")\", linecolour = \"#" << hex << reinterpret_cast<uint32_t>(trans) << "\", textcolour = \"#" << hex << (uint32_t)trans << "\"];\n";

    }
  }

  static void return_forward(sc_core::sc_object * from, amba::amba_master_socket<32,0> * ahb, tlm::tlm_generic_payload * trans, tlm::tlm_sync_enum status, sc_core::sc_time delay = SC_ZERO_TIME, uint32_t binding = 0) {

    if (msclogger_enable) {

      uint32_t a = 0;
      socket_t *socket_to = ahb->get_other_side(binding, a);
      sc_core::sc_object * to = socket_to->get_parent();

      msc << "  " << from->name() << ">>" << to->name() << " [ label = \"";
      
      switch (status) {

      case 0:
          
        msc << "TLM_ACCEPTED";
        break;
      case 1:

        msc << "TLM_UPDATED";
        break;
      default:

        msc << "TLM_COMPLETED";
      }

      msc << "(" << trans << "/" << sc_time_stamp() << "/" << delay << ")\", linecolour = \"#" << hex << reinterpret_cast<uint32_t>(trans) << "\", textcolour = \"#" << hex << (uint32_t)trans << "\"];\n";
    }
  }

  static void return_backward(sc_core::sc_object * from, amba::amba_slave_socket<32> * ahb, tlm::tlm_generic_payload * trans, tlm::tlm_sync_enum status, sc_core::sc_time delay = SC_ZERO_TIME) {

    if (msclogger_enable) {

      uint32_t a = 0;
      socket_t *socket_to = ahb->get_other_side(0, a);
      sc_core::sc_object * to = socket_to->get_parent();

      msc << "  " << from->name() << ">>" << to->name() << " [ label = \"";
      
      switch (status) {

      case 0:
          
        msc << "TLM_ACCEPTED";
        break;
      case 1:
            
        msc << "TLM_UPDATED";
        break;
      default:

        msc << "TLM_COMPLETED";
      }

      msc << "(" << trans << "/" << sc_time_stamp() << "/" << delay << ")\", linecolour = \"#" << hex << reinterpret_cast<uint32_t>(trans) << "\", textcolour = \"#" << hex << (uint32_t)trans << "\"];\n";
    }
  }

  static void return_backward(sc_core::sc_object * from, amba::amba_slave_socket<32,0> * ahb, tlm::tlm_generic_payload * trans, tlm::tlm_sync_enum status, sc_core::sc_time delay = SC_ZERO_TIME, uint32_t binding = 0) {

    if (msclogger_enable) {

      uint32_t a = 0;
      socket_t *socket_to = ahb->get_other_side(binding, a);
      sc_core::sc_object * to = socket_to->get_parent();

      msc << "  " << from->name() << ">>" << to->name() << " [ label = \"";
      
      switch (status) {

      case 0:
          
        msc << "TLM_ACCEPTED";
        break;
      case 1:

        msc << "TLM_UPDATED";
        break;
      default:

        msc << "TLM_COMPLETED";
      }

      msc << "(" << trans << "/" << sc_time_stamp() << "/" << delay << ")\", linecolour = \"#" << hex << reinterpret_cast<uint32_t>(trans) << "\", textcolour = \"#" << hex << (uint32_t)trans << "\"];\n";
    }
  }
};

#endif // MSCLOGGER
