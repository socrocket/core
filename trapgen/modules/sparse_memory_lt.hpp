/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     sparse_memory_lt.hpp
* @brief    This file is part of the TRAP runtime library.
* @details
* @author   Luca Fossati
* @author   Lillian Tadros (Technische Universitaet Dortmund)
* @date     2008-2013 Luca Fossati
*           2015-2016 Technische Universitaet Dortmund
* @copyright
*
* This file is part of TRAP.
*
* TRAP is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
* or see <http://www.gnu.org/licenses/>.
*
* (c) Luca Fossati, fossati@elet.polimi.it, fossati.l@gmail.com
*
*******************************************************************************/

#ifndef TRAP_SPARSE_MEMORY_LT_H

#define TRAP_SPARSE_MEMORY_LT_H


#include "common/report.hpp"

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

#include <boost/lexical_cast.hpp>
#include <string>
#include <map>

namespace trap {

/**
 * @brief SparseMemoryLT
 */
template<unsigned NUM_INITIATORS, unsigned sock_size>
class SparseMemoryLT : public sc_module {
  public:
  SparseMemoryLT(sc_module_name name, unsigned size, sc_time latency = SC_ZERO_TIME) :
    sc_module(name), latency(latency) {
    for (unsigned i = 0; i < NUM_INITIATORS; ++i) {
      this->target_socket[i] =
        new tlm_utils::simple_target_socket<SparseMemoryLT,
          sock_size>(("mem_socket_" + boost::lexical_cast<std::string>(i)).c_str());
      this->target_socket[i]->register_b_transport(this, &SparseMemoryLT::b_transport);
      this->target_socket[i]->register_get_direct_mem_ptr(this, &SparseMemoryLT::get_direct_mem_ptr);
      this->target_socket[i]->register_transport_dbg(this, &SparseMemoryLT::transport_dbg);
    }

    end_module();
  } // SparseMemoryLT()

  /// ..........................................................................

  ~SparseMemoryLT() {
    for (unsigned i = 0; i < NUM_INITIATORS; ++i) {
      delete this->target_socket[i];
    }
  } // ~SparseMemoryLT()

  /// ..........................................................................

  // TLM-2 blocking transport method.
  void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64 adr = trans.get_address();
    unsigned char* ptr = trans.get_data_ptr();
    unsigned len = trans.get_data_length();
    unsigned char* byt = trans.get_byte_enable_ptr();
    unsigned wid = trans.get_streaming_width();

    if (byt != 0) {
      trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
      return;
    }

    if (cmd == tlm::TLM_READ_COMMAND) {
      for (unsigned i = 0; i < len; ++i, ++ptr) {
        *ptr = this->mem[adr + i];
      }
    } else if (cmd == tlm::TLM_WRITE_COMMAND) {
      for (unsigned i = 0; i < len; ++i, ++ptr) {
        this->mem[adr + i] = *ptr;
      }
    }

    // Use temporal decoupling: Add memory latency to delay argument.
    delay += this->latency;

    trans.set_dmi_allowed(false);
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
  } // b_transport()

  /// ..........................................................................

  // TLM-2 DMI method.
  bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data) {
    // Deny read and write access.
    dmi_data.allow_read_write();
    dmi_data.set_start_address(0);
    dmi_data.set_end_address((sc_dt::uint64)-1);

    return false;
  } // get_direct_mem_ptr()

  /// ..........................................................................

  // TLM-2 debug transaction method.
  unsigned transport_dbg(tlm::tlm_generic_payload& trans) {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64 adr = trans.get_address();
    unsigned char* ptr = trans.get_data_ptr();
    unsigned len = trans.get_data_length();

    if (cmd == tlm::TLM_READ_COMMAND) {
      for (unsigned i = 0; i < len; ++i, ++ptr) {
        *ptr = this->mem[adr + i];
      }
    } else if (cmd == tlm::TLM_WRITE_COMMAND) {
      for (unsigned i = 0; i < len; ++i, ++ptr) {
        this->mem[adr + i] = *ptr;
      }
    }

    return len;
  } // transport_dbg()

  /// ..........................................................................

  // Used to directly write a word into memory. It is mainly used to load the
  // application program into memory.
  inline void write_byte_dbg(const unsigned& address, const unsigned char& datum) throw() {
    this->mem[address] = datum;
  } // write_byte_dbg()

  /// ..........................................................................

  public:
  tlm_utils::simple_target_socket<SparseMemoryLT, sock_size>* target_socket[NUM_INITIATORS];
  private:
  std::map<unsigned, unsigned char> mem;
  const sc_time latency;
}; // class SparseMemoryLT

} // namespace trap

/// ****************************************************************************
#endif // TRAP_SPARSE_MEMORY_LT_H

