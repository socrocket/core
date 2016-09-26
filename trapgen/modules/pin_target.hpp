/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     pin_target.hpp
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

#ifndef TRAP_PIN_TARGET_H
#define TRAP_PIN_TARGET_H

#include "common/report.hpp"

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

#include <boost/lexical_cast.hpp>
#include <string>

namespace trap {

/**
 * @brief PINTarget
 */
template<unsigned SOCK_SIZE>
class PINTarget : public sc_module {
  public:
  PINTarget(sc_module_name name) :
    sc_module(name),
    target_socket(("pin_target_" + boost::lexical_cast<std::string>(name)).c_str()) {
    this->target_socket.register_b_transport(this, &PINTarget::b_transport);
    end_module();
  } // PINTarget()

  /// ..........................................................................

  void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64 adr = trans.get_address();
    unsigned char* ptr = trans.get_data_ptr();
    unsigned len = trans.get_data_length();
    unsigned char* byt = trans.get_byte_enable_ptr();
    unsigned wid = trans.get_streaming_width();

    if (trans.get_command() == tlm::TLM_READ_COMMAND) {
      THROW_EXCEPTION("External pins do not yet support read requests.");
    } else if (cmd == tlm::TLM_WRITE_COMMAND) {
      this->values[(unsigned)adr] = *((unsigned*)ptr);
    }

    trans.set_response_status(tlm::TLM_OK_RESPONSE);
  } // b_transport()

  /// ..........................................................................

  // Method used to read the value of the PIN that has just been assigned.
  unsigned read_pin(unsigned address) {
    if (this->values.find(address) == this->values.end()) {
      THROW_EXCEPTION("Address " << std::hex << std::showbase << address << " not yet written by PIN port.");
    }
    return this->values[address];
  } // read_pin()

  /// ..........................................................................

  public:
  tlm_utils::simple_target_socket<PINTarget, SOCK_SIZE> target_socket;
  private:
  std::map<unsigned, unsigned> values;
}; // class PINTarget

} // namespace trap

/// ****************************************************************************
#endif // TRAP_PIN_TARGET_H
