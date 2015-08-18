// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup apbuart
/// @{
/// @file tcpio.h
/// Class implementing the communication among the simulated UART component and
/// the
///
/// @date 2010-2015
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

#include "gaisler/apbuart/io_if.h"
#include "core/common/base.h"
#include "core/common/verbose.h"
#include "core/common/sr_report.h"

class TcpIO : public BaseModule<DefaultBase>, public io_if {
  private:
    /// Represents the currently open connection
    boost::asio::ip::tcp::socket *socket;

    /// The port on which the connection takes place;
    sr_param<unsigned int> g_port;

  public:
    /// Opens a new socket connection on the specified port
    TcpIO(ModuleName mn, unsigned int port = 2000, bool test = false);

    ~TcpIO();

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
  TcpIO *sock;
  explicit ConnectionThread(TcpIO *sock) : sock(sock) {}
  void operator()() {
    sock->makeConnection();
  }
};

#endif  // MODELS_APBUART_TCPIO_H_
/// @}
