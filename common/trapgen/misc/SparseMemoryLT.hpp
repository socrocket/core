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

#ifndef MEMORYLT_HPP
#define MEMORYLT_HPP

#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include "core/common/systemc.h"
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

#include "core/common/trapgen/utils/trap_utils.hpp"

namespace trap {
template<unsigned int N_INITIATORS, unsigned int sockSize>
class SparseMemoryLT : public sc_module {
  public:
    tlm_utils::simple_target_socket<SparseMemoryLT, sockSize> *socket[N_INITIATORS];

    SparseMemoryLT(sc_module_name name, unsigned int size, sc_time latency = SC_ZERO_TIME) :
      sc_module(name), latency(latency) {
      for (int i = 0; i < N_INITIATORS; i++) {
        this->socket[i] =
          new tlm_utils::simple_target_socket<SparseMemoryLT,
            sockSize>(("mem_socket_" + boost::lexical_cast<std::string>(i)).c_str());
        this->socket[i]->register_b_transport(this, &SparseMemoryLT::b_transport);
        this->socket[i]->register_get_direct_mem_ptr(this, &SparseMemoryLT::get_direct_mem_ptr);
        this->socket[i]->register_transport_dbg(this, &SparseMemoryLT::transport_dbg);
      }

      end_module();
    }

    ~SparseMemoryLT() {
      for (int i = 0; i < N_INITIATORS; i++) {
        delete this->socket[i];
      }
    }

    void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay) {
      tlm::tlm_command cmd = trans.get_command();
      sc_dt::uint64 adr = trans.get_address();
      unsigned char *ptr = trans.get_data_ptr();
      unsigned int len = trans.get_data_length();
      unsigned char *byt = trans.get_byte_enable_ptr();
      unsigned int wid = trans.get_streaming_width();

      if (byt != 0) {
        trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
        return;
      }

      if (cmd == tlm::TLM_READ_COMMAND) {
        for (int i = 0; i < len; i++, ptr++) {
          *ptr = this->mem[adr + i];
        }
      } else if (cmd == tlm::TLM_WRITE_COMMAND) {
        for (int i = 0; i < len; i++, ptr++) {
          this->mem[adr + i] = *ptr;
        }
      }

      // Use temporal decoupling: add memory latency to delay argument
      delay += this->latency;

      trans.set_dmi_allowed(false);
      trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    // TLM-2 DMI method
    bool get_direct_mem_ptr(tlm::tlm_generic_payload &trans, tlm::tlm_dmi &dmi_data) {
      // Deny read and write access
      dmi_data.allow_read_write();
      dmi_data.set_start_address(0);
      dmi_data.set_end_address((sc_dt::uint64)-1);

      return false;
    }

    // TLM-2 debug transaction method
    unsigned int transport_dbg(tlm::tlm_generic_payload &trans) {
      tlm::tlm_command cmd = trans.get_command();
      sc_dt::uint64 adr = trans.get_address();
      unsigned char *ptr = trans.get_data_ptr();
      unsigned int len = trans.get_data_length();

      if (cmd == tlm::TLM_READ_COMMAND) {
        for (int i = 0; i < len; i++, ptr++) {
          *ptr = this->mem[adr + i];
        }
      } else if (cmd == tlm::TLM_WRITE_COMMAND) {
        for (int i = 0; i < len; i++, ptr++) {
          this->mem[adr + i] = *ptr;
        }
      }

      return len;
    }

    // Method used to directly write a word into memory; it is mainly used to load the
    // application program into memory
    inline void write_byte_dbg(const unsigned int &address, const unsigned char &datum) throw() {
      this->mem[address] = datum;
    }
  private:
    const sc_time latency;
    std::map<unsigned int, unsigned char> mem;
};
}

#endif
