/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     gdb_connection_manager.cpp
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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include "gdb_connection_manager.hpp"
#include "common/report.hpp"

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

#include <csignal>
#ifndef SIGTRAP
#define SIGTRAP 5
#endif

#ifndef __PRETTY_FUNCTION__
#ifdef __FUNCDNAME__
#define __PRETTY_FUNCTION__ __FUNCDNAME__
#else
#define __PRETTY_FUNCTION__ "NONAME"
#endif
#endif

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

trap::GDBConnectionManager::GDBConnectionManager(bool endianess) : endianess(endianess), killed(false) {
  this->socket = NULL;
} // GDBConnectionManager::GDBConnectionManager()

/// ----------------------------------------------------------------------------

trap::GDBConnectionManager::~GDBConnectionManager() {
  if (this->socket != NULL && !this->killed) {
    delete this->socket;
  }
} // GDBConnectionManager::~GDBConnectionManager()

/// ----------------------------------------------------------------------------

/// Creates a socket connection waiting on the specified port. This will be used
/// later to communicate with GDB.
void trap::GDBConnectionManager::initialize(unsigned port) {
  try {
    boost::asio::ip::tcp::acceptor acceptor(this->io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

    this->socket = new boost::asio::ip::tcp::socket(this->io_service);
    std::cerr << "GDB: waiting for connections on port " << port  << ".\n";
    acceptor.accept(*this->socket);
    std::cerr << "GDB: connection accepted on port " << port  << ".\n\n";
  }
  catch(...) {
    this->killed = true;
    THROW_ERROR("Cannot create connection on port " + boost::lexical_cast<std::string>(port) + '.');
  }
} // GDBConnectionManager::initialize()

/// ----------------------------------------------------------------------------

/// Sends the response to the connected GDB debugger.
void trap::GDBConnectionManager::send_response(GDBResponse& response) {
  //All the response packets are in the form $<packet info>#<checksum>
  std::string payload;
  //First of all I compute the payload; it depends on the particular packet
  //sent by GDB
  switch (response.type) {
    case GDBResponse::S_RSP: {
      //S request: informs GDB that a signal interrupted program execution
      payload = 'S' + this->itohstr((unsigned char)response.payload, 2);
    break;}
    case GDBResponse::T_RSP: {
      //T request: informs GDB that a signal interrupted program execution;
      //more datailed information is provided
      payload = 'T' + this->itohstr((unsigned char)response.payload, 2);
      std::vector<std::pair<std::string, unsigned> >::iterator pairs_it, pairs_end;
      for (pairs_it = response.info.begin(), pairs_end = response.info.end();
      pairs_it != pairs_end; pairs_it++) {
        if (pairs_it->first == "thread" || pairs_it->first == "watch" ||
        pairs_it->first == "rwatch" || pairs_it->first == "awatch" || pairs_it->first == "library") {
            //it is a hex number representing a register and the second part
            //represents the value of that register
            payload += pairs_it->first + ':' + this->itohstr(pairs_it->second, response.size*2);
        } else {
            //it is a hex number representing a register and the second part
            //represents the value of that register; TODO we should check that
            //this is really a number
            payload += pairs_it->first + ':' + this->itohstr(pairs_it->second, response.size*2);
        }
        payload += ';';
      }
    break;}
    case GDBResponse::W_RSP: {
      //Process Exited
      payload = 'W' + this->itohstr((unsigned char)response.payload, 2);
      this->killed = true;
    break;}
    case GDBResponse::X_RSP: {
      //Process Exited
      payload = 'X' + this->itohstr((unsigned char)response.payload);
    break;}
    case GDBResponse::OUTPUT_RSP: {
      //Sending output message to the GDB debugger console
      payload = 'O';
      std::string::iterator msg_it, msg_end;
      for (msg_it = response.message.begin(), msg_end = response.message.end();
      msg_it != msg_end; msg_it++) {
        payload += this->itohstr((unsigned char)*msg_it, 2);
      }
    break;}
    case GDBResponse::OK_RSP: {
      payload = "OK";
    break;}
    case GDBResponse::ERROR_RSP: {
      payload = 'E' + this->itohstr((unsigned char)response.payload, 2);
    break;}
    case GDBResponse::REG_READ_RSP:
    case GDBResponse::MEM_READ_RSP: {
    std::vector<char>::iterator data_it, data_end;
    for (data_it = response.data.begin(), data_end = response.data.end(); data_it != data_end; data_it++)
      payload += this->itohstr((unsigned char)*data_it, 2);
    break;}
    case GDBResponse::CONT_RSP: {
      std::vector<char>::iterator data_it, data_end;
      for (data_it = response.data.begin(), data_end = response.data.end(); data_it != data_end; data_it++) {
        payload += ';' + *data_it;
      }
    break;}
    default: {
    break;}
  }

  unsigned char ack = '\x0';
  bool retry = false;
  do {
    //Now I complete the packet with the checksum
    std::string packet = '$' + payload + '#' + this->itohstr(this->compute_checksum(payload), 2);

    //Finally I can send the packet on the network
    boost::system::error_code asio_error;
    boost::asio::write(*this->socket, boost::asio::buffer(packet.c_str(), packet.size()), boost::asio::transfer_all(), asio_error);

#ifndef NDEBUG
    if (asio_error) {
      std::cerr << __PRETTY_FUNCTION__ << ": Write error " << asio_error.message() << ".\n";
    }
#endif

    if (this->killed)
      break;
    //Now I have to check that the packet was correctly received, otherwise I
    //retransmitt it
    int num_retries = 0;
    retry = false;
    do {
      num_retries = 0;
      do {
        ack = this->read_queue_char();
        if (ack == '\x0') {
          std::cerr << "\nConnection closed unexpectedly by the GDB debugger.\n\n";
          this->killed = true;
          return;
        }
        num_retries++;
      } while((ack & 0x7f) != '+' && (ack & 0x7f) != '-');
      if (num_retries > 1) {
        //Some random characters were received,  I signal an error
        packet = "$E00#a5";
        boost::asio::write(*this->socket, boost::asio::buffer(packet.c_str(), packet.size()), boost::asio::transfer_all(), asio_error);
        retry = true;
      }
    } while(num_retries > 1);
  } while((ack & 0x7f) == '-' || retry);
} // GDBConnectionManager::send_response()

/// ----------------------------------------------------------------------------

/// Waits for the sending of a packet from GDB. It then parses it and translates
/// it into the correct request.
trap::GDBRequest trap::GDBConnectionManager::process_request() {
    //Ok, we have to read the request and create its high level representation;
  //Note how this operation is repeated until the packet is correctly received
  std::string payload;
  bool correctly_received = false;
  GDBRequest req;

  do {
    unsigned char received_char = '\x0';
    boost::system::error_code asio_error;

    //Reading the starting character
    while((received_char & 0x7f) != '$') {
      received_char = this->read_queue_char();
      if (received_char == '\x0') {
        std::cerr << "\nConnection closed unexpectedly the GDB debugger before sending a request.\n";
        std::cerr << "\nThere might be a problem with your GDB client.\n\n";
        std::cerr << "\nDetaching from GDB and restarting simulation...\n\n";
        req.type = GDBRequest::ERROR_REQ;
        this->killed = true;
        return req;
      }
    }
    if (received_char == 0x03) {
      //It means that I received an interrupt from GDB, I stop the simulation and
      //become responsive
      req.type = GDBRequest::INTR_REQ;
      return req;
    }

    //Now I have to start reading the payload: I go on until # is enocuntered;
    payload = "";
    while((received_char & 0x7f) != '#') {
      received_char = this->read_queue_char();
      if (received_char == '\x0') {
        std::cerr << "\nConnection closed unexpectedly the GDB debugger.\n\n";
        std::cerr << "\nDetaching from GDB and restarting simulation...\n\n";
        req.type = GDBRequest::ERROR_REQ;
        this->killed = true;
        return req;
      }
      if ((received_char & 0x7f) != '#')
        payload += (char)(received_char & 0x7f);
    }

    //Finally I read the checksum: it should be composed of two characters
    char checksum[2];
    checksum[0] = this->read_queue_char();
    if (checksum[0] == '\x0') {
        std::cerr << "\nConnection closed unexpectedly the GDB debugger.\n\n";
        std::cerr << "\nDetaching from GDB and restarting simulation...\n\n";
      req.type = GDBRequest::ERROR_REQ;
      this->killed = true;
      return req;
    }
    checksum[1] = this->read_queue_char();
    if (checksum[1] == '\x0') {
        std::cerr << "\nConnection closed unexpectedly the GDB debugger.\n\n";
        std::cerr << "\nDetaching from GDB and restarting simulation...\n\n";
      req.type = GDBRequest::ERROR_REQ;
      this->killed = true;
      return req;
    }

    //Now I have to check the checksum...
    correctly_received = this->check_checksum(payload, checksum);

    //...and communicate the result of the checking to GDB server
    char check_result;
    if (correctly_received) {
      check_result = '+';
    } else {
      check_result = '-';
    }
    boost::asio::write(*this->socket, boost::asio::buffer(&check_result, 1), boost::asio::transfer_all(), asio_error);
    if (asio_error) {
      std::cerr << __PRETTY_FUNCTION__ << ": Write Error:" << asio_error.message() << std::endl;
      req.type = GDBRequest::ERROR_REQ;
      return req;
    }
  } while(!correctly_received);

  //Now I have do decode the payload and transform it into the real packet
  char payload_type = payload[0];
  payload = payload.substr(1);
  switch (payload_type) {
    case '!': {
      req.type = GDBRequest::EXCL_REQ;
    break;}
    case '?': {
      req.type = GDBRequest::QUEST_REQ;
    break;}
    case 'c': {
      req.type = GDBRequest::c_REQ;
      if (payload.size() > 0) {
        req.address = (unsigned)std::strtoul(payload.c_str(), NULL, 0);
      } else
        req.address = 0;
    break;}
    case 'C': {
      req.type = GDBRequest::C_REQ;
      std::string::size_type sep_index = payload.find(';');
      if (sep_index == std::string::npos) {
        req.signal = (unsigned)std::strtoul(payload.c_str(), NULL, 0);
      } else {
        std::string temp = payload.substr(0, sep_index);
        req.signal = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
        temp = payload.substr(sep_index + 1);
        req.address = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
      }
    break;}
    case 'D': {
      req.type = GDBRequest::D_REQ;
    break;}
    case 'g': {
      req.type = GDBRequest::g_REQ;
    break;}
    case 'G': {
      req.type = GDBRequest::G_REQ;
      std::string::iterator pay_it, pay_end;
      for (pay_it = payload.begin(), pay_end = payload.end();
                    pay_it != pay_end; pay_it++) {
        std::string buf(1, *pay_it);
        pay_it++;
        buf += *pay_it;
        req.data.push_back((unsigned char)std::strtoul(buf.c_str(), NULL, 0));
      }
    break;}
    case 'H': {
      req.type = GDBRequest::H_REQ;
      req.data.push_back(payload[0]);
      payload = payload.substr(1);
      req.value = boost::lexical_cast<int>(payload);
    break;}
    case 'i': {
      req.type = GDBRequest::i_REQ;
      if (payload.size() > 0) {
        std::string::size_type sep_index = payload.find(',');
        if (sep_index == std::string::npos) {
          req.value = 1;
          req.address = (unsigned)std::strtoul(payload.c_str(), NULL, 0);
        } else {
          std::string temp = payload.substr(0, sep_index);
          req.address = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
          temp = payload.substr(sep_index + 1);
          req.value = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
        }
      } else {
        req.address = 0;
        req.value = 1;
      }
    break;}
    case 'I': {
      req.type = GDBRequest::I_REQ;
    break;}
    case 'k': {
      this->killed = true;
      req.type = GDBRequest::k_REQ;
    break;}
    case 'm': {
      req.type = GDBRequest::m_REQ;
      std::string::size_type sep_index = payload.find(',');
      std::string temp = payload.substr(0, sep_index);
      req.address = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
      temp = payload.substr(sep_index + 1);
      req.length = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
    break;}
    case 'M': {
      req.type = GDBRequest::M_REQ;
      std::string::size_type sep_index = payload.find(',');
      std::string::size_type sep_index2 = payload.find(':');
      std::string temp = payload.substr(0, sep_index);
      req.address = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
      temp = payload.substr(sep_index + 1, sep_index2 - sep_index - 1);
      req.length = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
      temp = payload.substr(sep_index2 + 1);
      //Now it is time to read the content of memory
      std::string::iterator data_it, data_end;
      for (data_it = temp.begin(), data_end = temp.end(); data_it != data_end; data_it++) {
        std::string buf(1, *data_it);
        data_it++;
        buf += *data_it;
        req.data.push_back((unsigned char)std::strtoul(buf.c_str(), NULL, 0));
      }
      //Now I check that the length of the buffer is the specified one
      if (req.data.size() != req.length) {
        std::cerr << __PRETTY_FUNCTION__ << ": Error in the M message: Data size mistmatch.\n";
      }
    break;}
    case 'p': {
      req.type = GDBRequest::p_REQ;
      req.reg = (unsigned)std::strtoul(payload.c_str(), NULL, 0);
    break;}
    case 'P': {
      req.type = GDBRequest::P_REQ;
      std::string::size_type sep_index = payload.find('=');
      if (sep_index == std::string::npos) {
        std::cerr << __PRETTY_FUNCTION__ << ": Error in the P message: No arguments given.\n";
      }
      std::string temp = payload.substr(0, sep_index);
      req.reg = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
      temp = payload.substr(sep_index + 1);
      req.value = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
    break;}
    case 'q': {
      req.type = GDBRequest::q_REQ;
      std::string::size_type sep_index = payload.find(',');
      if (sep_index == std::string::npos) {
        req.type = GDBRequest::UNK_REQ;
        break;
      }
      std::string temp = payload.substr(0, sep_index);
      req.command = temp;
      temp = payload.substr(sep_index + 1);
      req.extension = this->hstrtocstr(temp);
    break;}
    case 's': {
      req.type = GDBRequest::s_REQ;
      if (payload.size() > 0) {
        req.address = (unsigned)std::strtoul(payload.c_str(), NULL, 0);
      } else {
        req.address = 0;
      }
    break;}
    case 'S': {
      req.type = GDBRequest::S_REQ;
      std::string::size_type sep_index = payload.find(';');
      if (sep_index == std::string::npos) {
        req.signal = (unsigned)std::strtoul(payload.c_str(), NULL, 0);
      } else {
        std::string temp = payload.substr(0, sep_index);
        req.signal = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
        temp = payload.substr(sep_index + 1);
        req.address = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
      }
    break;}
    case 't': {
      req.type = GDBRequest::t_REQ;
      std::string::size_type sep_index = payload.find(':');
      std::string::size_type sep_index2 = payload.find(',');
      if (sep_index == std::string::npos || sep_index2 == std::string::npos)
        std::cerr << __PRETTY_FUNCTION__ << ": Error in the t message: No arguments given.\n";
      std::string temp = payload.substr(0, sep_index);
      req.address = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
      temp = payload.substr(sep_index + 1, sep_index2 - sep_index - 1);
      req.value = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
      temp = payload.substr(sep_index2 + 1);
      req.length = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
    break;}
    case 'T': {
      req.type = GDBRequest::T_REQ;
      req.value = (unsigned)std::strtoul(payload.c_str(), NULL, 0);
    break;}
    case 'v': {
      req.type = GDBRequest::v_REQ;
      req.command = payload;
    break;}
    /*case 'X': {
      req.type = GDBRequest::X_REQ;
      std::string::size_type sep_index = payload.find(',');
      std::string::size_type sep_index2 = payload.find(':');
      if (sep_index == std::string::npos || sep_index2 == std::string::npos)
        std::cerr << __PRETTY_FUNCTION__ << ": error in the M message: no arguments given" << std::endl;
      std::string temp = payload.substr(0, sep_index);
      req.address = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
      temp = payload.substr(sep_index + 1, sep_index2 - sep_index - 1);
      req.length = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
      temp = payload.substr(sep_index2 + 1);
      //Now it is time to read the content of memory
      std::string::iterator data_it, data_end;
      for (data_it = temp.begin(), data_end = temp.end(); data_it != data_end; data_it++) {
        req.data.push_back(*data_it);
      }
      //Now I check that the length of the buffer is the specified one
      if (req.data.size() != req.length)
        std::cerr << __PRETTY_FUNCTION__ << ": error in the X message: different length of bytes" << std::endl;
    break;}*/
    case 'z': {
      req.type = GDBRequest::z_REQ;
      std::string::size_type sep_index = payload.find(',');
      std::string::size_type sep_index2 = payload.find_last_of(',');
      if (sep_index == std::string::npos || sep_index2 == std::string::npos)
        std::cerr << __PRETTY_FUNCTION__ << ": Error in the z message: No arguments given.\n";
      std::string temp = payload.substr(0, sep_index);
      req.value = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
      temp = payload.substr(sep_index + 1, sep_index2 - sep_index - 1);
      req.address = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
      temp = payload.substr(sep_index2 + 1);
      req.length = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
    break;}
    case 'Z': {
      req.type = GDBRequest::Z_REQ;
      std::string::size_type sep_index = payload.find(',');
      std::string::size_type sep_index2 = payload.find_last_of(',');
      if (sep_index == std::string::npos || sep_index2 == std::string::npos)
        std::cerr << __PRETTY_FUNCTION__ << ": Error in the Z message: No arguments given.\n";
      std::string temp = payload.substr(0, sep_index);
      req.value = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
      temp = payload.substr(sep_index + 1, sep_index2 - sep_index - 1);
      req.address = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
      temp = payload.substr(sep_index2 + 1);
      req.length = (unsigned)std::strtoul(temp.c_str(), NULL, 0);
    break;}
    default: {
      req.type = GDBRequest::UNK_REQ;
    break;}
  }

  return req;
} // GDBConnectionManager::process_request()

/// ----------------------------------------------------------------------------

 /// Keeps waiting for a character on the channel to the GDB debugger.
bool trap::GDBConnectionManager::check_interrupt() {
  unsigned char received_char = '\x0';
  boost::system::error_code asio_error;
  //Reading the starting character
  do {
    this->socket->read_some(boost::asio::buffer(&received_char, 1), asio_error);
    if (asio_error == boost::asio::error::eof) {
      boost::mutex::scoped_lock lock(this->queue_mutex);
      this->received_chars.push_back('\x0');
      this->empty_queue_condition.notify_all();
      this->killed = true;
      return false;
    }
    if ((received_char & 0x7f) != 0x03 && !this->killed) {
      boost::mutex::scoped_lock lock(this->queue_mutex);
      this->received_chars.push_back(received_char);
      this->empty_queue_condition.notify_all();
    }
    //std::cerr << received_char << "- hex form -" << std::hex << std::showbase << (unsigned)received_char << std::endl;
  } while((received_char & 0x7f) != 0x03 && !this->killed);
  //std::cerr << "returned since an interrupt was encountered" << std::endl;
  if (this->killed) {
    return false;
  } else {
    return true;
  }
} // GDBConnectionManager::check_interrupt()

/// ----------------------------------------------------------------------------

/// Sends and interrupt message to the GDB debugger signaling that the
/// execution of the program has halted. This way the GDB debugger becomes
/// responsive and it is possible to debug the program under test.
void trap::GDBConnectionManager::send_interrupt() {
  GDBResponse response;
  response.type = GDBResponse::S_RSP;
  response.payload = SIGTRAP;

  this->send_response(response);
} // GDBConnectionManager::send_interrupt()

/// ----------------------------------------------------------------------------

///Closes the connection with the GDB debugger
void trap::GDBConnectionManager::disconnect() {
  if (this->socket != NULL) {
    if (this->socket->is_open()) {
      this->socket->close();
    }
    delete this->socket;
    this->socket = NULL;
  }
} // GDBConnectionManager::disconnect()

/// ----------------------------------------------------------------------------

/// Reads a character from the queue of ready characters.
unsigned char trap::GDBConnectionManager::read_queue_char() {
  boost::mutex::scoped_lock lock(this->queue_mutex);
  while(this->received_chars.empty()) {
    this->empty_queue_condition.wait(lock);
  }
  unsigned char recvd = this->received_chars.front();
  this->received_chars.pop_front();
  return recvd;
} // GDBConnectionManager::read_queue_char()

/// ----------------------------------------------------------------------------

/// Computes the checksum for the data.
unsigned char trap::GDBConnectionManager::compute_checksum(std::string& data) {
  unsigned char sum = 0;
  std::string::iterator data_it, data_end;
  for (data_it = data.begin(), data_end = data.end(); data_it != data_end; data_it++) {
    sum += (unsigned char)*data_it;
  }
  return sum;
} // GDBConnectionManager::compute_checksum()

/// ----------------------------------------------------------------------------

/// Checks that the checksum included in the packet is correct.
bool trap::GDBConnectionManager::check_checksum(std::string& data, char checksum[2]) {
  unsigned char comp_checksum = this->compute_checksum(data);
  unsigned char recv_checksum = ((this->hctoi(checksum[0]) & 0x0f) << 4) | (this->hctoi(checksum[1]) & 0x0f);
  return comp_checksum == recv_checksum;
} // GDBConnectionManager::check_checksum()

/// ----------------------------------------------------------------------------

/// Converts a hex character to an int representing it.
int trap::GDBConnectionManager::hctoi(unsigned char ch) {
  if (ch >= 'a' && ch <= 'f')
    return ch-'a'+10;
  if (ch >= '0' && ch <= '9')
    return ch-'0';
  if (ch >= 'A' && ch <= 'F')
    return ch-'A'+10;
  return -1;
} // GDBConnectionManager::hctoi()

/// ----------------------------------------------------------------------------

/// Converts a generic numeric value into a string of hex numbers. Each hex
/// number of the string is in the same order of the endianess of the processor
/// linked to this stub.
std::string trap::GDBConnectionManager::itohstr(unsigned value, int num_chars) {
  std::ostringstream oss;

  if (!this->endianess && ((value & 0xFFFFFF00) != 0)) {
    //I have to flip the bytes of value so that the endianess is correct
    value = ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) |
            ((value & 0x00FF0000) >> 8) | ((value & 0xFF000000) >> 24);
  }

  //Conversion to hex
  if (num_chars == -1) {
    oss << std::hex << value;
  } else {
    oss << std::hex << std::setw(num_chars) << std::setfill('0') << value;
  }

  return oss.str();
} // GDBConnectionManager::itohstr()

/// ----------------------------------------------------------------------------

/// Converts a hexadecimal string into the corresponding character string.
std::string trap::GDBConnectionManager::hstrtocstr(std::string& to_convert) {
  //What I do is to read the string element in couples; then
  //I convert each couple to its integer representation:
  //that is one string character
  if ((to_convert.size() % 2) != 0) {
    std::cerr << __PRETTY_FUNCTION__ << ": Odd number of characters in hexadecimal string " << to_convert << ".\n";
    return "";
  }
  std::string outstr = "";
  for (unsigned i = 0; i < to_convert.size()/2; i++) {
    std::string temp = to_convert.substr(i*2,  2);
    outstr += (char)std::strtoul(temp.c_str(), NULL, 16);
  }

  return outstr;
} // GDBConnectionManager::hstrtocstr()

/// ****************************************************************************
