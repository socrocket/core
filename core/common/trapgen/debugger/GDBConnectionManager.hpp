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
\ ***************************************************************************/

/**
 * Class used to manage connections with the GDB server.
 */

#ifndef GDBCONNECTIONMANAGER_HPP
#define GDBCONNECTIONMANAGER_HPP

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
 * High level repesentation of a GDB request packet
 */
struct GDBRequest {
  enum Type {QUEST_req = 0, EXCL_req, c_req, C_req, D_req, g_req, G_req, H_req, i_req, I_req, k_req, m_req, M_req,
             p_req, P_req, q_req, s_req, S_req, t_req, T_req, v_req, X_req, z_req, Z_req, UNK_req, ERROR_req, INTR_req};
  Type type;
  unsigned int address;
  unsigned int length;
  unsigned int reg;
  unsigned int signal;
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
};

/**
 * High level repesentation of a GDB response packet
 */
struct GDBResponse {
  enum Type {S_rsp = 0, T_rsp, W_rsp, X_rsp, OUTPUT_rsp, OK_rsp, ERROR_rsp, MEM_READ_rsp, REG_READ_rsp, CONT_rsp,
             NOT_SUPPORTED_rsp};
  Type type;
  unsigned int payload;
  std::string message;
  unsigned int size;
  std::vector<char> data;
  std::vector<std::pair<std::string, unsigned int> > info;
  GDBResponse() {
    payload = 0;
    message = "";
    size = 0;
  }
};

/**
 * Manages the connection among the GDB debugger and the stub which communicates with
 * the processor; it is responsible of handling socket connection and
 * of the coding and decoding of the messages.
 * For more details on the internal workings of GDB look at http://sourceware.org/gdb/current/onlinedocs/gdb_33.html
 */
class GDBConnectionManager {
  private:
    ///Represents the currently open connection
    boost::asio::ip::tcp::socket *socket;
    boost::asio::io_service io_service;
    ///Specifies the endianess of the current processor; true means
    ///that it is the same endianess of the host, false, otherwise
    bool endianess;
    ///Computes the checksum for the data
    unsigned char computeChecksum(std::string &data);
    ///Checks that the checksum included in the packet is correct
    bool checkChecksum(std::string & data, char checkSum[2]);
    ///Converts a generic numeric value into a string of hex numbers;
    ///each hex number of the string is in the same order of the endianess
    ///of the processor linked to this stub
    std::string toHexString(unsigned int value, int numChars = -1);
    ///Converts an hexadecimal number expressed with a string
    ///into its correspondent integer number
    ///each hex number of the string is in the same order of the endianess
    ///of the processor linked to this stub
    unsigned int toIntNum(std::string &toConvert);
    ///Converts a hexadecimal number into the corresponding character string
    std::string toStr(std::string &toConvert);
    ///Converts a hex character to an int representing it
    int chToHex(unsigned char ch);
    ///Converts and integer hex to a char representing it
    unsigned char hexToInt(unsigned int num);
    ///Map used to convert hex strings in integers
    std::map<char, unsigned int> HexMap;
    ///Specifis whether communication has been killed by the other endpoint or not
    bool killed;
    ///List of characters received from the GDB stub
    std::list<unsigned char> recvdChars;
    ///Mutex and condition variables managing access to the
    ///queue of received characters
    boost::mutex queueMutex;
    boost::condition emptyQueueCond;

    ///Reads a character from the queue of ready characters
    unsigned char readQueueChar();
  public:
    GDBConnectionManager(bool endianess);
    ~GDBConnectionManager();
    ///Creates a socket connection waiting on the specified port;
    ///this will be later used to communicate with GDB
    void initialize(unsigned int port);
    ///Sends the response to the GDB debugger connected
    void sendResponse(GDBResponse &response);
    ///Waits for the sending of a packet from GDB; it then parses it and
    ///translates it into the correct request
    GDBRequest processRequest();
    ///Closes the connection with the GDB debugger
    void disconnect();
    ///Keeps waiting for a character on the channel with the GDB
    ///debugger
    bool checkInterrupt();
    ///Sends and interrupt message to the GDB debugger signaling that
    ///the execution of the program halted: this way the GDB
    ///debugger becomes responsive and it is possible to debug the
    ///program under test
    void sendInterrupt();
};
}

#endif
