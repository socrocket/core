// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup apbuart
/// @{
/// @file tcpio.h
/// Class implementing the communication among the simulated UART component and
/// the
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef MODELS_APBUART_TCPIO_H_
#define MODELS_APBUART_TCPIO_H_

#include <boost/asio.hpp>
#include <string>

#include "core/models/apbuart/io_if.h"

class TcpIo : public sc_core::sc_object, public io_if {
  private:
    /// Represents the currently open connection
    boost::asio::ip::tcp::socket *socket;

    /// The port on which the connection takes place;
    unsigned int port;

  public:
    /// Opens a new socket connection on the specified port
    TcpIo(ModuleName mn, unsigned int port = 2000, bool test = false);

    ~TcpIo();

    /// Receives a character; returns true if read the character is valid,
    /// false in case the character is not valid (such as if we are communicating
    /// on a socket and there are no available characters)

    uint32_t receivedChars();
    void getReceivedChar(char *toRecv);

    /// Sends a character on the communication channel
    void sendChar(char toSend);

    /// Creates a connection
    void makeConnection();
};

struct ConnectionThread {
  TcpIo *sock;
  explicit ConnectionThread(TcpIo *sock) : sock(sock) {}
  void operator()() {
    sock->makeConnection();
  }
};

#endif  // MODELS_APBUART_TCPIO_H_
/// @}
