/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     gdb_connection_manager.hpp
* @brief    This file is part of the TRAP runtime library.
* @details  Class used to manage connections with the GDB server.
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

#ifndef TRAP_GDB_CONNECTION_MANAGER_H
#define TRAP_GDB_CONNECTION_MANAGER_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <boost/asio.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace trap {

/**
 * @brief GDBRequest
 *
 * High level repesentation of a GDB request packet.
 */
struct GDBRequest {
  enum Type {QUEST_REQ = 0, EXCL_REQ, c_REQ, C_REQ, D_REQ, g_REQ, G_REQ, H_REQ,
             i_REQ, I_REQ, k_REQ, m_REQ, M_REQ, p_REQ, P_REQ, q_REQ, s_REQ,
             S_REQ, t_REQ, T_REQ, v_REQ, X_REQ, z_REQ, Z_REQ,
             UNK_REQ, ERROR_REQ, INTR_REQ};
  Type type;
  unsigned address;
  unsigned length;
  unsigned reg;
  unsigned signal;
  int value;
  std::string command;
  std::string extension;
  std::vector<unsigned char> data;
  GDBRequest() {
    address = 0;
    length = 0;
    reg = 0;
    signal = 0;
    value = 0;
    command = "";
    extension = "";
  }
}; // struct GDBRequest

/// ****************************************************************************

/**
 * @brief GDBResponse
 *
 * High level repesentation of a GDB response packet.
 */
struct GDBResponse {
  enum Type {S_RSP = 0, T_RSP, W_RSP, X_RSP, OUTPUT_RSP, OK_RSP, ERROR_RSP,
             MEM_READ_RSP, REG_READ_RSP, CONT_RSP, UNSUPPORTED_RSP};
  Type type;
  unsigned payload;
  std::string message;
  unsigned size;
  std::vector<char> data;
  std::vector<std::pair<std::string, unsigned> > info;
  GDBResponse() {
    payload = 0;
    message = "";
    size = 0;
  }
}; // struct GDBResponse

/// ****************************************************************************

/**
 * @brief GDBResponse
 *
 * Manages the connection between the GDB debugger and the stub which
 * communicates with the processor. It is responsible for handling socket
 * connection and for the coding and decoding of the messages.
 *
 * @sa https://sourceware.org/gdb/current/onlinedocs/gdb/Remote-Protocol.html
 */
class GDBConnectionManager {
  /// @name Constructors and Destructors
  /// @{

  public:
  GDBConnectionManager(bool endianess);
  ~GDBConnectionManager();

  /// @} Constructors and Destructors
  /// ------------------------------------------------------------------------
  /// @name Interface Methods
  /// @{

  public:
  /// Creates a socket connection waiting on the specified port. This will be
  /// used later to communicate with GDB.
  void initialize(unsigned port);

  /// Sends the response to the connected GDB debugger.
  void send_response(GDBResponse& response);

  /// Waits for the sending of a packet from GDB. It then parses it and
  /// translates it into the correct request.
  GDBRequest process_request();

  /// Keeps waiting for a character on the channel to the GDB debugger.
  bool check_interrupt();

  /// Sends and interrupt message to the GDB debugger signaling that the
  /// execution of the program has halted. This way the GDB debugger becomes
  /// responsive and it is possible to debug the program under test.
  void send_interrupt();

  /// Closes the connection with the GDB debugger.
  void disconnect();

  /// @} Interface Methods
  /// ------------------------------------------------------------------------
  /// @name Internal Methods
  /// @{

  private:
  /// Reads a character from the queue of ready characters.
  unsigned char read_queue_char();

  /// Computes the checksum for the data.
  unsigned char compute_checksum(std::string& data);

  /// Checks that the checksum included in the packet is correct.
  bool check_checksum(std::string& data, char checksum[2]);

  /// Converts a hex character to an int representing it.
  int hctoi(unsigned char ch);

  /// Converts a generic numeric value into a string of hex numbers. Each hex
  /// number of the string is in the same order of the endianess of the processor
  /// linked to this stub.
  std::string itohstr(unsigned value, int num_chars = -1);

  /// Converts a hexadecimal string into the corresponding character string.
  std::string hstrtocstr(std::string& to_convert);

  /// @} Internal Methods
  /// ------------------------------------------------------------------------
  /// @name Data
  /// @{

  private:
  /// The currently open connection.
  boost::asio::ip::tcp::socket* socket;
  boost::asio::io_service io_service;
  /// Endianess of the current processor: true - same as host endianess, false
  /// - otherwise.
  bool endianess;
  /// Specifies whether communication has been killed by the other endpoint.
  bool killed;
  /// List of characters received from the GDB stub.
  std::list<unsigned char> received_chars;
  /// Mutex and condition variables managing access to the queue of received
  /// characters.
  boost::mutex queue_mutex;
  boost::condition empty_queue_condition;

  /// @} Data
}; // class GDBConnectionManager

} // namespace trap

/// ****************************************************************************
#endif // TRAP_GDB_CONNECTION_MANAGER_H
