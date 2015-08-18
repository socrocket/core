// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file msclogger.h
/// Utility class for timing verification. Keeps track of TLM communication by
/// writing a MSCGEN control file.
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef COMMON_MSCLOGGER_H_
#define COMMON_MSCLOGGER_H_

#include <tlm.h>
#include "core/common/amba.h"
#include <fstream>   // NOLINT(readability/streams)
#include <iostream>  // NOLINT(readability/streams)

#include "core/common/verbose.h"

extern std::ofstream msc;
extern sc_core::sc_time msclogger_start;
extern sc_core::sc_time msclogger_end;

#ifdef MSCLOGGER
static const uint32_t msclogger_enable = 1;
#else
static const uint32_t msclogger_enable = 0;
#endif

class msclogger {
  public:
    typedef tlm::tlm_generic_payload payload_t;
    typedef gs::socket::bindability_base<tlm::tlm_base_protocol_types> socket_t;

    static void msc_start(const char *filename, const char *nodes) {
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

    static void forward(sc_core::sc_object *from,
    amba::amba_master_socket<32> *ahb,
    tlm::tlm_generic_payload *trans,
    tlm::tlm_phase phase,
    sc_core::sc_time delay = SC_ZERO_TIME) {
      if (msclogger_enable) {
        uint32_t a = 0;
        socket_t *socket_to = ahb->get_other_side(0, a);
        sc_core::sc_object *to = socket_to->get_parent();

        size_t addr = reinterpret_cast<size_t>(trans);
        uint32_t id = addr & 0xFFFFFF;
        msc << "  " << from->name() << "=>" << to->name() << " [ label = \"" << phase << "(" << trans << "/" <<
      sc_time_stamp() \
            << "/" << delay << ")\", linecolour = \"#" << hex << id << "\", textcolour = \"#" << hex << id << "\"];\n";
      }
    }

    static void forward(sc_core::sc_object *from,
    amba::amba_master_socket<32, 0> *ahb,
    tlm::tlm_generic_payload *trans,
    tlm::tlm_phase phase,
    sc_core::sc_time delay = SC_ZERO_TIME,
    uint32_t binding = 0) {
      if (msclogger_enable) {
        uint32_t a = 0;
        socket_t *socket_to = ahb->get_other_side(binding, a);
        sc_core::sc_object *to = socket_to->get_parent();

        size_t addr = reinterpret_cast<size_t>(trans);
        uint32_t id = addr & 0xFFFFFF;
        msc << "  " << from->name() << "=>" << to->name() << " [ label = \"" << phase << "(" << trans << "/"  <<
      sc_time_stamp() \
            << "/" << delay << ")\", linecolour = \"#" << hex << id << "\", textcolour = \"#" << hex << id << "\"];\n";
      }
    }

    static void backward(sc_core::sc_object *from,
    amba::amba_slave_socket<32> *ahb,
    tlm::tlm_generic_payload *trans,
    tlm::tlm_phase phase,
    sc_core::sc_time delay = SC_ZERO_TIME) {
      if (msclogger_enable) {
        uint32_t a = 0;
        socket_t *socket_to = ahb->get_other_side(0, a);
        sc_core::sc_object *to = socket_to->get_parent();
        size_t addr = reinterpret_cast<size_t>(trans);
        uint32_t id = addr & 0xFFFFFF;

        msc << "  " << from->name() << "=>" << to->name() << " [ label = \"" << phase << "(" << trans << "/" <<
      sc_time_stamp() \
            << "/" << delay << ")\", linecolour = \"#" << hex << id << "\", textcolour = \"#" << hex << id << "\"];\n";
      }
    }

    static void backward(sc_core::sc_object *from,
    amba::amba_slave_socket<32, 0> *ahb,
    tlm::tlm_generic_payload *trans,
    tlm::tlm_phase phase,
    sc_core::sc_time delay = SC_ZERO_TIME,
    uint32_t binding = 0) {
      if (msclogger_enable) {
        uint32_t a = 0;
        socket_t *socket_to = ahb->get_other_side(binding, a);
        sc_core::sc_object *to = socket_to->get_parent();
        size_t addr = reinterpret_cast<size_t>(trans);
        uint32_t id = addr & 0xFFFFFF;

        msc << "  " << from->name() << "=>" << to->name() << " [ label = \"" << phase << "(" << trans << "/" <<
      sc_time_stamp() \
            << "/" << delay << ")\", linecolour = \"#" << hex << id << "\", textcolour = \"#" << hex << id << "\"];\n";
      }
    }

    static void return_forward(sc_core::sc_object *from,
    amba::amba_master_socket<32> *ahb,
    tlm::tlm_generic_payload *trans,
    tlm::tlm_sync_enum status,
    sc_core::sc_time delay = SC_ZERO_TIME) {
      if (msclogger_enable) {
        uint32_t a = 0;
        socket_t *socket_to = ahb->get_other_side(0, a);
        sc_core::sc_object *to = socket_to->get_parent();
        size_t addr = reinterpret_cast<size_t>(trans);
        uint32_t id = addr & 0xFFFFFF;

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

        msc << "(" << trans << "/" << sc_time_stamp() << "/" << delay << ")\", linecolour = \"#" << hex << id <<
      "\", textcolour = \"#" << hex << id << "\"];\n";
      }
    }

    static void return_forward(sc_core::sc_object *from,
    amba::amba_master_socket<32, 0> *ahb,
    tlm::tlm_generic_payload *trans,
    tlm::tlm_sync_enum status,
    sc_core::sc_time delay = SC_ZERO_TIME,
    uint32_t binding = 0) {
      if (msclogger_enable) {
        uint32_t a = 0;
        socket_t *socket_to = ahb->get_other_side(binding, a);
        sc_core::sc_object *to = socket_to->get_parent();
        size_t addr = reinterpret_cast<size_t>(trans);
        uint32_t id = addr & 0xFFFFFF;

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

        msc << "(" << trans << "/" << sc_time_stamp() << "/" << delay << ")\", linecolour = \"#" << hex << id <<
      "\", textcolour = \"#" << hex << id << "\"];\n";
      }
    }

    static void return_backward(sc_core::sc_object *from,
    amba::amba_slave_socket<32> *ahb,
    tlm::tlm_generic_payload *trans,
    tlm::tlm_sync_enum status,
    sc_core::sc_time delay = SC_ZERO_TIME) {
      if (msclogger_enable) {
        uint32_t a = 0;
        socket_t *socket_to = ahb->get_other_side(0, a);
        sc_core::sc_object *to = socket_to->get_parent();
        size_t addr = reinterpret_cast<size_t>(trans);
        uint32_t id = addr & 0xFFFFFF;

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

        msc << "(" << trans << "/" << sc_time_stamp() << "/" << delay << ")\", linecolour = \"#" << hex << id <<
      "\", textcolour = \"#" << hex << id << "\"];\n";
      }
    }

    static void return_backward(sc_core::sc_object *from,
    amba::amba_slave_socket<32, 0> *ahb,
    tlm::tlm_generic_payload *trans,
    tlm::tlm_sync_enum status,
    sc_core::sc_time delay = SC_ZERO_TIME,
    uint32_t binding = 0) {
      if (msclogger_enable) {
        uint32_t a = 0;
        socket_t *socket_to = ahb->get_other_side(binding, a);
        sc_core::sc_object *to = socket_to->get_parent();
        size_t addr = reinterpret_cast<size_t>(trans);
        uint32_t id = addr & 0xFFFFFF;

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

        msc << "(" << trans << "/" << sc_time_stamp() << "/" << delay << ")\", linecolour = \"#" << hex << id <<
      "\", textcolour = \"#" << hex << id << "\"];\n";
      }
    }
};

#endif  // COMMON_MSCLOGGER_H_
/// @}
