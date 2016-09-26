/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     sparse_memory_at.hpp
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

#ifndef TRAP_SPARSE_MEMORY_AT_H

#define TRAP_SPARSE_MEMORY_AT_H


#include "common/report.hpp"

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/peq_with_cb_and_phase.h>

#include <boost/lexical_cast.hpp>
#include <string>
#include <cstring>
#include <map>

DECLARE_EXTENDED_PHASE(internal_ph);

namespace trap {

/**
 * @brief SparseMemoryAT
 */
template<unsigned NUM_INITIATORS, unsigned sock_size>
class SparseMemoryAT : public sc_module {
  public:
  SparseMemoryAT(sc_module_name name, unsigned size, sc_time latency = SC_ZERO_TIME) :
    sc_module(name), size(size), latency(latency), trans_id(0),
    trans_in_progress(false), m_peq(this, &SparseMemoryAT::peq_cb) {
    for (unsigned i = 0; i < NUM_INITIATORS; ++i) {
      this->target_socket[i] =
        new tlm_utils::simple_target_socket_tagged<SparseMemoryAT,
          sock_size>(("mem_socket_" + boost::lexical_cast<std::string>(i)).c_str());
      this->target_socket[i]->register_nb_transport_fw(this, &SparseMemoryAT::nb_transport_fw, i);
      this->target_socket[i]->register_transport_dbg(this, &SparseMemoryAT::transport_dbg, i);
    }

    end_module();
  } // SparseMemoryAT()

  /// ..........................................................................

  ~SparseMemoryAT() {
    for (unsigned i = 0; i < NUM_INITIATORS; ++i) {
      delete this->target_socket[i];
    }
  } // ~SparseMemoryAT()

  /// ..........................................................................

  // TLM-2 non-blocking transport method.
  tlm::tlm_sync_enum nb_transport_fw(int tag, tlm::tlm_generic_payload& trans,
  tlm::tlm_phase& phase, sc_time& delay) {
    sc_dt::uint64 adr = trans.get_address();
    unsigned len = trans.get_data_length();
    unsigned char* byt = trans.get_byte_enable_ptr();
    unsigned wid = trans.get_streaming_width();

    // Obliged to check the transaction attributes for unsupported features
    // and to generate the appropriate error response.
    if (byt != 0) {
      trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
      return tlm::TLM_COMPLETED;
    }
    // Queue the transaction until the annotated time has elapsed.
    if (phase == tlm::BEGIN_REQ) {
      while (this->trans_in_progress) {
        //std::cerr << "Waiting for trans_in_progress.\n";
        wait(this->trans_completed);
      }
      //std::cerr << "There are no trans_in_progress.\n";
      this->trans_in_progress = true;
    }
    this->trans_id = tag;
    m_peq.notify(trans, phase, delay);
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
    return tlm::TLM_ACCEPTED;
  } // nb_transport_fw()

  /// ..........................................................................

  void peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase) {
    tlm::tlm_sync_enum status;
    sc_time delay;

    switch (phase) {
    case tlm::BEGIN_REQ:
      status = send_end_req(trans);
    break;

    case tlm::END_RESP:
      //std::cerr << "tlm::END_RESP in memory peq_cb.\n";
      this->trans_in_progress = false;
      this->trans_completed.notify();
    break;

    case tlm::END_REQ:
    case tlm::BEGIN_RESP:
      SC_REPORT_FATAL("TLM-2", "Illegal transaction phase received by target.");
    break;

    default:
      if (phase == internal_ph) {
        // Execute the read or write commands.
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

        trans.set_response_status(tlm::TLM_OK_RESPONSE);

        // Target must honor BEGIN_RESP/END_RESP exclusion rule, i.e. it must
        // not send BEGIN_RESP until receiving previous END_RESP or BEGIN_REQ.
        send_response(trans);
        //std::cerr << "Memory reading address in memory " << std::hex << std::showbase << adr << ".\n";
      }
    break;
    }
  } // peq_cb()

  /// ..........................................................................

  tlm::tlm_sync_enum send_end_req(tlm::tlm_generic_payload& trans) {
    tlm::tlm_sync_enum status;
    tlm::tlm_phase bw_phase;
    tlm::tlm_phase int_phase = internal_ph;

    // Queue the acceptance and the response with the appropriate latency.
    bw_phase = tlm::END_REQ;
    sc_time zero_delay = SC_ZERO_TIME;
    status = (*(this->target_socket[trans_id]))->nb_transport_bw(trans, bw_phase, zero_delay);
    if (status == tlm::TLM_COMPLETED) {
      // Transaction aborted by the initiator (TLM_UPDATED cannot occur at this
      // point in the base protocol, so no need to check it.).
      trans.release();
      return status;
    }
    // Queue internal event to mark beginning of response.
    m_peq.notify(trans, int_phase, this->latency);
    return status;
  } // send_end_req()

  /// ..........................................................................

  void send_response(tlm::tlm_generic_payload& trans) {
    tlm::tlm_sync_enum status;
    tlm::tlm_phase bw_phase;

    bw_phase = tlm::BEGIN_RESP;
    sc_time zero_delay = SC_ZERO_TIME;
    status = (*(this->target_socket[trans_id]))->nb_transport_bw(trans, bw_phase, zero_delay);

    //std::cerr << "Response status " << status << ".\n";
    if (status == tlm::TLM_UPDATED) {
      // The timing annotation must be honored.
      m_peq.notify(trans, bw_phase, SC_ZERO_TIME);
    } else if (status == tlm::TLM_COMPLETED) {
      // The initiator has terminated the transaction.
      trans.release();
    }
  } // send_response()

  /// ..........................................................................

  // TLM-2 debug transaction method.
  unsigned transport_dbg(int tag, tlm::tlm_generic_payload& trans) {
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
  tlm_utils::simple_target_socket_tagged<SparseMemoryAT, sock_size>* target_socket[NUM_INITIATORS];
  private:
  std::map<unsigned, unsigned char> mem;
  const sc_time latency;
  unsigned size;
  int trans_id;
  bool trans_in_progress;
  sc_event trans_completed;
  tlm_utils::peq_with_cb_and_phase<SparseMemoryAT> m_peq;
}; // class SparseMemoryAT

} // namespace trap

/// ****************************************************************************
#endif // TRAP_SPARSE_MEMORY_AT_H

