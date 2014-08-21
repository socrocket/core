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
// Title:      greenreg_ambasocket.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implements a greenreg socket for the ambakit
//
// Method:
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TU Braunschweig
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef GREENREG_AMBASOCKETS_H_
#define GREENREG_AMBASOCKETS_H_

#include <greenreg/greenreg.h>
#include "gr_ambasockets.h"

namespace gs {
namespace reg {

typedef gs::reg::I_register_container_bus_access memory_space_type;

template<typename PROTOCOL>
class greenreg_socket;

/// greenreg_socket: A greenreg port class
template<>
class greenreg_socket<gs::amba::amba_slave<32> > : public gs::amba::amba_slave<32> {
    public:
        greenreg_socket(
            sc_core::sc_module_name _name,
            gs::reg::I_register_container_bus_access & _reg_bind,
            gr_uint_t _base_address,
            gr_uint_t _decode_size,
            ::amba::amba_bus_type _type = ::amba::amba_APB,
            ::amba::amba_layer_ids _layer = ::amba::amba_LT,
            bool _arbiter = false) :
            gs::amba::amba_slave<32>(_name, _reg_bind, _base_address,
                    _decode_size, _type, _layer, _arbiter) {
        }

        gs::amba::amba_slave<32>& operator()() {
            return *this;
        }

        gs::amba::amba_slave<32>& get_bus_port() {
            return *this;
        }

};

template<>
class greenreg_socket<gs::amba::amba_master<32> > : public gs::amba::amba_master<32> {
    public:

        greenreg_socket(sc_core::sc_module_name _name,
                        ::amba::amba_bus_type _type = ::amba::amba_APB,
                        ::amba::amba_layer_ids _layer = ::amba::amba_LT,
                        bool _arbiter = false) :
            gs::amba::amba_master<32>(_name, _type, _layer, _arbiter) {
        }

        //! get_bus_port {enables port binding to bus}
        gs::amba::amba_master<32>& get_bus_port() {
            return *this;
        }

        //! operator ->() {enables high level API invokeaction}
        sc_core::sc_export<tlm_components::transactor_if>& operator->() {
            return ::gs::amba::amba_master<32>::m_in_port;
        }
};

} // end namespace gs:reg
} // end namespace gs:reg

#endif  // GREENREG_AMBASOCKET_H_
