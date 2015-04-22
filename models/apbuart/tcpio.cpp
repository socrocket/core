// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup apbuart
/// @{
/// @file tcpio.cpp
///
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>

#include "core/common/base.h"
#include "core/common/verbose.h"
#include "core/common/sr_report.h"
#include "core/models/apbuart/tcpio.h"

/// Creates a connection
void TcpIo::makeConnection() {
  try {
    //srCommand("TcpIo")();
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
    this->socket = new boost::asio::ip::tcp::socket(io_service);
    v::info << "UART: waiting for connections on port " << port << v::endl;
    acceptor.accept(*this->socket);
    v::info << "UART: connection accepted on port " << port << v::endl;
  } catch (...)    {
    v::error << "TcpIo" << "Error during the creation of the connection on port " << port;
  }
}

/// Opens a new socket connection on the specified port
TcpIo::TcpIo(ModuleName mn, unsigned int port, bool test) : sc_core::sc_object(mn), port(port) {
  if (!test) {
    this->makeConnection();
  } else {
    // I have to create the connection in a boost thread
    boost::thread thrd(ConnectionThread(this));
  }
}

TcpIo::~TcpIo() {}

/// Receives a character; returns true if read the character is valid,
/// false in case the character is not valid (such as if we are communicating
/// on a socket and there are no available characters)
uint32_t TcpIo::receivedChars() {
  return this->socket->available();
}

/// Receives a character; returns true if read the character is valid,
/// false in case the character is not valid (such as if we are communicating
/// on a socket and there are no available characters)
void TcpIo::getReceivedChar(char *toRecv) {
  if (this->socket->available() < 1) {
    return;
  }

  boost::system::error_code asioError;
  this->socket->read_some(boost::asio::buffer(toRecv, 1), asioError);

  if (*toRecv == '\r') {
    *toRecv = '\0';
  }

  if (asioError == boost::asio::error::eof) {
    v::error << "TcpIo" << "Connection with the UART Unexpetedly closed";
  }
}

/// Sends a character on the communication channel
void TcpIo::sendChar(char toSend) {
  boost::system::error_code asioError;
  boost::asio::write(*this->socket, boost::asio::buffer(&toSend, 1), boost::asio::transfer_all(), asioError);
}
/// @}
