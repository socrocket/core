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

#include "gaisler/apbuart/tcpio.h"

SR_HAS_UARTBACKEND(TcpIO);

/// Creates a connection
void TcpIO::makeConnection() {
  try {
    //srCommand("TcpIO")();
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), g_port));
    this->socket = new boost::asio::ip::tcp::socket(io_service);
    srInfo()
      ("port", g_port)
      (std::string("waiting for connection ") + g_port.toString());
    acceptor.accept(*this->socket);
    srInfo()
      ("port", g_port)
      ("connection accepted");
  } catch (...)    {
    srError()
      ("port", g_port)
      ("Error during creation of connection.");
  }
}

/// Opens a new socket connection on the specified port
TcpIO::TcpIO(ModuleName mn, unsigned int port, bool test) : 
  BaseModule<DefaultBase>(mn),
  g_port("port", port, m_generics) 
  {
  if (!test) {
    this->makeConnection();
  } else {
    // I have to create the connection in a boost thread
    boost::thread thrd(ConnectionThread(this));
  }
}

TcpIO::~TcpIO() {}

/// Receives a character; returns true if read the character is valid,
/// false in case the character is not valid (such as if we are communicating
/// on a socket and there are no available characters)
uint32_t TcpIO::receivedChars() {
  return this->socket->available();
}

/// Receives a character; returns true if read the character is valid,
/// false in case the character is not valid (such as if we are communicating
/// on a socket and there are no available characters)
void TcpIO::getReceivedChar(char *toRecv) {
  if (this->socket->available() < 1) {
    return;
  }

  boost::system::error_code asioError;
  this->socket->read_some(boost::asio::buffer(toRecv, 1), asioError);

  if (*toRecv == '\r') {
    *toRecv = '\0';
  }

  if (asioError == boost::asio::error::eof) {
    srError()
      ("Connection with the UART unexpectedly closed");
  }
}

/// Sends a character on the communication channel
void TcpIO::sendChar(char toSend) {
  boost::system::error_code asioError;
  boost::asio::write(*this->socket, boost::asio::buffer(&toSend, 1), boost::asio::transfer_all(), asioError);
}
/// @}
