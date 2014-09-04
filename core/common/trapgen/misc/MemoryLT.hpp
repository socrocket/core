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
#include <string>
#include "core/common/systemc.h"
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

#include "core/common/trapgen/utils/trap_utils.hpp"

namespace trap {
template<unsigned int N_INITIATORS, unsigned int sockSize>
class MemoryLT : public sc_module {
  public:
    tlm_utils::simple_target_socket<MemoryLT, sockSize> *socket[N_INITIATORS];

    MemoryLT(sc_module_name name, unsigned int size, sc_time latency = SC_ZERO_TIME) :
      sc_module(name), size(size), latency(latency) {
      for (int i = 0; i < N_INITIATORS; i++) {
        this->socket[i] =
          new tlm_utils::simple_target_socket<MemoryLT, sockSize>(("mem_socket_" + boost::lexical_cast<std::string>(
                                                                     i)).c_str());
        this->socket[i]->register_b_transport(this, &MemoryLT::b_transport);
        this->socket[i]->register_get_direct_mem_ptr(this, &MemoryLT::get_direct_mem_ptr);
        this->socket[i]->register_transport_dbg(this, &MemoryLT::transport_dbg);
      }

      // Reset memory
      this->mem = new unsigned char[this->size];
      memset(this->mem, 0, size);
      end_module();
    }

    ~MemoryLT() {
      delete this->mem;
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

      if (adr > this->size) {
        trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        std::cerr << "Error requesting address " << std::showbase << std::hex << adr << std::dec << std::endl;
        return;
      }
      if (byt != 0) {
        trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
        return;
      }

      if (cmd == tlm::TLM_READ_COMMAND) {
        memcpy(ptr, &this->mem[adr], len);
      } else if (cmd == tlm::TLM_WRITE_COMMAND) {
        memcpy(&this->mem[adr], ptr, len);
      }

      // Use temporal decoupling: add memory latency to delay argument
      delay += this->latency;

      trans.set_dmi_allowed(true);
      trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    // TLM-2 DMI method
    bool get_direct_mem_ptr(tlm::tlm_generic_payload &trans, tlm::tlm_dmi &dmi_data) {
      // Allow read and write access
      dmi_data.allow_read_write();

      // Set other details of DMI region
      dmi_data.set_dmi_ptr(this->mem);
      dmi_data.set_start_address(0);
      dmi_data.set_end_address(this->size);
      dmi_data.set_read_latency(this->latency);
      dmi_data.set_write_latency(this->latency);

      return true;
    }

    // TLM-2 debug transaction method
    unsigned int transport_dbg(tlm::tlm_generic_payload &trans) {
      tlm::tlm_command cmd = trans.get_command();
      sc_dt::uint64 adr = trans.get_address();
      unsigned char *ptr = trans.get_data_ptr();
      unsigned int len = trans.get_data_length();

      // Calculate the number of bytes to be actually copied
      unsigned int num_bytes = (len < this->size - adr) ? len : this->size - adr;

      if (cmd == tlm::TLM_READ_COMMAND) {
        memcpy(ptr, &this->mem[adr], num_bytes);
      } else if (cmd == tlm::TLM_WRITE_COMMAND) {
        memcpy(&this->mem[adr], ptr, num_bytes);
      }

      return num_bytes;
    }

    // Method used to directly write a word into memory; it is mainly used to load the
    // application program into memory
    inline void write_byte_dbg(const unsigned int &address, const unsigned char &datum) throw() {
      if (address >= this->size) {
        THROW_ERROR("Address " << std::hex << std::showbase << address << " out of memory");
      }
      this->mem[address] = datum;
    }
  private:
    const sc_time latency;
    unsigned int size;
    unsigned char *mem;
};
}

#endif
