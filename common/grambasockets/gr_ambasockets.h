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
// Title:      gr_ambasocket.h
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

#ifndef GR_AMBASOCKETS_H
#define GR_AMBASOCKETS_H

#include <greenreg/greenreg.h>
#include <greensocket/utils/greensocketaddress_base.h>
#include <greenreg/greenreg_socket/transactor_if.h>
#include <greenreg/framework/core/gr_common.h>
#include "core/common/amba.h"
#include "core/common/vendian.h"

#include <systemc>
#include <iostream>

namespace gs {
namespace amba {

class generic_slave_base {
    public:
        generic_slave_base(
                           gs::reg::I_register_container_bus_access * _registers,
                           gr_uint_t _base_addr, gr_uint_t _decode_size) :
            m_registers(_registers), m_decode_size(_decode_size), m_base_addr(
                    _base_addr), m_delay_enabled(true) // enabled by default
        {
        }

        virtual ~generic_slave_base() {
        }

        inline gr_uint_t get_decode_size() {
            return m_decode_size;
        }

        /// Disables the delay for all notification rule callbacks caused by this socket
        void disable_delay() {
            m_delay_enabled = false;
        }

        /// Enables the delay for all notification rule callbacks caused by this socket
        void enable_delay() {
            m_delay_enabled = true;
        }

        /// Returns if the delay is enabled
        bool delay_enabled() {
            return m_delay_enabled;
        }

    protected:
        bool write_to_module(unsigned int _data, unsigned int _address,
                             unsigned int _byte_enable,
                             gs::reg::transaction_type* _transaction,
                             bool _delayed) {
            return (m_registers->bus_write(_data, _address, _byte_enable,
                    _transaction, _delayed));
        }

        unsigned int read_from_module(unsigned int& _data,
                                      unsigned int _address,
                                      unsigned int _byte_enable,
                                      gs::reg::transaction_type* _transaction,
                                      bool _delayed) {
            return (m_registers->bus_read(_data, _address, _byte_enable,
                    _transaction, _delayed));
        }

        gs::reg::I_register_container_bus_access * m_registers;
        gr_uint_t m_decode_size;

    protected:
        gr_uint_t m_base_addr;

        /// If this socket let the caused notification rule callbacks be delayed
        bool m_delay_enabled;

};

template<unsigned int BUSWIDTH = 32>
class amba_slave : public generic_slave_base,
                   public ::amba_slave_base,
                   public ::amba::amba_slave_socket<BUSWIDTH>,
                   public gs::socket::GreenSocketAddress_base {
    public:

        amba_slave(sc_core::sc_module_name _name,
                   gs::reg::I_register_container_bus_access & _reg_bind,
                   gr_uint_t _base_address, gr_uint_t _decode_size,
                   ::amba::amba_bus_type _type, ::amba::amba_layer_ids _layer,
                   bool _arbiter) :
            generic_slave_base(&_reg_bind, _base_address, _decode_size),
                    ::amba::amba_slave_socket<BUSWIDTH>(_name, _type, _layer,
                            _arbiter), ::gs::socket::GreenSocketAddress_base(
                            (const char *)_name), m_base(_base_address),
                    m_high(_base_address + _decode_size) {

            // Bind amba blocking ...
            amba_slave::register_b_transport(this, &amba_slave::b_transport);
            // Register debug transport
            amba_slave::register_transport_dbg(this, &amba_slave::transport_dbg);

            gs::socket::GreenSocketAddress_base::base_addr = _base_address;
            gs::socket::GreenSocketAddress_base::high_addr = _base_address
                    + _decode_size;

        }

        virtual ~amba_slave() {
        }

    public:
       unsigned int transport_dbg(tlm::tlm_generic_payload &trans) {
           sc_time delay;
           b_transport(trans, delay);
           return trans.get_data_length();
       }

        void b_transport(tlm::tlm_generic_payload& gp, sc_core::sc_time&) {
            unsigned int address = gp.get_address() - m_base;
            unsigned int byteaddr = address & 0x3;
            address = address & ~0x3;
            unsigned int length;
            unsigned int mask;
            unsigned int data;

            gs::reg::transaction_type trans(this, &gp);

            if (gp.is_write()) {
                //for (unsigned int i=0; i<gp.get_data_length(); i+=4) {  // Works only in nonburst mode max 4byte atime
                length = gp.get_data_length();
                data = 0;

                memcpy(&data, &(gp.get_data_ptr()[0]), length);
                switch (length) {
                    case 1:
                        mask = 0x1;
                        data <<= (byteaddr << 3);
                        mask <<= byteaddr;
                        break;
                    case 2:
                        mask = 0x3;
                        printf("GreenReg AMBA Sockets: 2byte access is not implementet correctly\n");
                        break;
                    case 3:
                        mask = 0x7;
                        printf("GreenReg AMBA Sockets: 3byte access is not implementet correctly\n");
                        break;
                    default:
                        mask = 0xF;
                        #ifdef LITTLE_ENDIAN_BO
                        swap_Endianess(data);
                        #endif
                        break;
                }

                //  std::cout<<"    "<<address<<": "<<std::hex<<"0x"<<((gp.get_data_ptr()[address]<16)?"0":"")<<((unsigned short)gp.get_data_ptr()[address])<<std::endl;
                m_registers->bus_write(data, address, mask, &trans, m_delay_enabled);
            }
            if (gp.is_read()) {
                m_bus_read_data = 0;
                m_registers->bus_read(m_bus_read_data, address, 0xF, &trans, m_delay_enabled);

                #ifdef LITTLE_ENDIAN_BO
                swap_Endianess(m_bus_read_data);
                #endif
                m_bus_read_data >>= (byteaddr << 3);

                memcpy(&(gp.get_data_ptr()[0]), &m_bus_read_data, gp.get_data_length());
            }
            gp.set_response_status(tlm::TLM_OK_RESPONSE);
        }

        // tlm_slave_if implementation
        virtual void setAddress(sc_dt::uint64 base, sc_dt::uint64 high) {
            m_base = base;
            m_high = high;

            gs::socket::GreenSocketAddress_base::base_addr = base;
            gs::socket::GreenSocketAddress_base::high_addr = high;
        }

        sc_dt::uint64 get_base_addr() {
            return m_base;
        }

        sc_dt::uint64 get_size() {
            return m_high - m_base;
        }

        //virtual sc_core::sc_object *get_parent() {
        //    std::cout << "my get_parent" << std::endl;
        //    return this;
        //}

        inline void set_delay() {
        }

    protected:
        sc_dt::uint64 m_base;
        sc_dt::uint64 m_high;

    private:
        unsigned int m_bus_read_data;
};

template<unsigned int BUSWIDTH = 32>
class amba_master : public tlm_components::transactor_if,
                    public ::amba::amba_master_socket<BUSWIDTH> {
        //                  , public sc_core::sc_module {
    public:
        typedef ::amba::amba_master_socket<BUSWIDTH> socket;

        amba_master(sc_core::sc_module_name _name, ::amba::amba_bus_type _type,
                    ::amba::amba_layer_ids _layer, bool _arbiter) :
            socket(_name, _type, _layer, _arbiter) {
        }

        virtual ~amba_master() {
        }
        virtual void random_read() {
        }
        virtual void random_write() {
        }

    protected:
        typedef tlm::tlm_generic_payload payload_t;

        // transactor methods
        virtual void _read(unsigned _address, unsigned _length,
                           unsigned int* _db, bool _bytes_enabled,
                           unsigned int* _be) {
            // fix because greenbus does not overwrite data block cleanly (even though the back end does)
            // Payload immer vom socket hohlen damit der interne pool benutzt wird, ist schneller und sauberer.
            payload_t *gp = socket::get_transaction();
            *_db = 0;
            sc_core::sc_time t;
            gp->set_command(tlm::TLM_READ_COMMAND);
            gp->set_address(_address);
            gp->set_data_length(_length);
            gp->set_streaming_width(4);
            gp->set_byte_enable_ptr(NULL);
            gp->set_data_ptr(reinterpret_cast<unsigned char *> (_db));
            socket::m_if_wrapper.b_transport(*gp, t);
            //wait(t); // LT -> always 0
            socket::release_transaction(gp);
        }

        virtual void _write(unsigned _address, unsigned _length,
                            unsigned int* _db, bool _bytes_enabled,
                            unsigned int* _be) {
            sc_core::sc_time t;
            payload_t *gp = socket::get_transaction();
            gp->set_command(tlm::TLM_WRITE_COMMAND);
            gp->set_address(_address);
            gp->set_data_length(_length);
            gp->set_streaming_width(4);
            gp->set_byte_enable_ptr(NULL);
            gp->set_data_ptr(reinterpret_cast<unsigned char *> (_db));
            socket::m_if_wrapper.b_transport(*gp, t);
            //wait(t); // LT -> always 0
            socket::release_transaction(gp);
        }

    private:
};

} // end namespace amba
} // end namespace gs

#endif /*_GR_AMBASOCKET_H_*/
