/***************************************************************************\
*
*   This file is part of TRAP.
*
*   TRAP is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*   or see <http://www.gnu.org/licenses/>.
*
*
*
*   (c) Luca Fossati, fossati@elet.polimi.it, fossati.l@gmail.com
*
\***************************************************************************/

#ifndef PINTARGET_HPP
#define PINTARGET_HPP

#include <boost/lexical_cast.hpp>
#include <string>
#include "core/common/systemc.h"
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

#include "core/common/trapgen/utils/trap_utils.hpp"

namespace trap {
template<unsigned int sockSize>
class PINTarget : public sc_module {
  public:
    tlm_utils::simple_target_socket<PINTarget, sockSize> socket;

    PINTarget(sc_module_name name) : sc_module(name), socket(("pin_target_" + boost::lexical_cast<std::string>(
                                                                name)).c_str()) {
      this->socket.register_b_transport(this, &PINTarget::b_transport);
      end_module();
    }

    void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay) {
      tlm::tlm_command cmd = trans.get_command();
      sc_dt::uint64 adr = trans.get_address();
      unsigned char *ptr = trans.get_data_ptr();
      unsigned int len = trans.get_data_length();
      unsigned char *byt = trans.get_byte_enable_ptr();
      unsigned int wid = trans.get_streaming_width();

      if (trans.get_command() == tlm::TLM_READ_COMMAND) {
        THROW_EXCEPTION("Error, the read request is not currently supported by external PINs");
      } else if (cmd == tlm::TLM_WRITE_COMMAND) {
        this->values[(unsigned int)adr] = *((unsigned int *)ptr);
      }

      trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    // Method used to read the value of the just assigned PIN
    unsigned int readPIN(unsigned int address) {
      if (this->values.find(address) == this->values.end()) {
        THROW_EXCEPTION("Address " << std::hex << std::showbase << address << " not yet written by PIN port");
      }
      return this->values[address];
    }
  private:
    std::map<unsigned int, unsigned int> values;
};
}

#endif
