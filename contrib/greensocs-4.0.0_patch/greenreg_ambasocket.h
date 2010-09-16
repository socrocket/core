/*
Copyright (c) 2008, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// ChangeLog GreenSocs
// 2009-01-21 Christian Schroeder: changed namespace
// 2009-01-29 Christian Schroeder: renamings
// 2009-01-29 FURTHER CHANGES SEE SVN LOG!


/*
*  @file greenreg_socket.hpp
*   The key concept behind greenreg_socket is that it provides TLM independence
*   for register file accesses. Unlike standard SystemC greenreg_socket, which needs
*   to be parameterised over an interface, the tlm greenreg_socket is a complex object that
*   can be parameterised over a protocol. This class provides a module with a
*   means for connection and communication with its surroundings.
*
*/


#ifndef __GREENREG_AMBASOCKET_H__
#define __GREENREG_AMBASOCKET_H__

// [greebus protocol Header file]
//#include "greenbus.h"

#include "gr_ambasocket.h"
#include "greenreg.h"


namespace gs {
namespace reg {

typedef  gs::reg::I_register_container_bus_access memory_space_type;
  
template< typename PROTOCOL>
class greenreg_socket;
  
/// greenreg_socket: A greenreg port class
template<>
class greenreg_socket<gs::amba::amba_slave<32> >: public gs::amba::amba_slave<32> {
  public:
    
    greenreg_socket(sc_core::sc_module_name _name, gs::reg::I_register_container_bus_access & _reg_bind, gr_uint_t _base_address, gr_uint_t _decode_size, ::amba::amba_bus_type _type = ::amba::amba_APB, ::amba::amba_layer_ids _layer = ::amba::amba_LT, bool _arbiter = false)
      : gs::amba::amba_slave<32>( _name, _reg_bind, _base_address, _decode_size, _type, _layer, _arbiter) {}
    
    gs::amba::amba_slave<32>& operator()() {
      return *this;
    }
    
    gs::amba::amba_slave<32>& get_bus_port() {
      return *this;
    }
     
};
  
template<>
class greenreg_socket<gs::amba::amba_master<32> >: public gs::amba::amba_master<32> {
  public:
    
    greenreg_socket(sc_core::sc_module_name _name, ::amba::amba_bus_type _type = ::amba::amba_APB, ::amba::amba_layer_ids _layer = ::amba::amba_LT, bool _arbiter = false)
      : gs::amba::amba_master<32>( _name, _type, _layer, _arbiter) {}
    
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

#endif // __GREENREG_AMBASOCKET_H__
