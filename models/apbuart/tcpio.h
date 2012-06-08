/**
 * Class implementing the communication among the simulated UART component and the
 * host environment; this communication takes place using sockets.
 */

#ifndef TCPIO_H
#define TCPIO_H

#include <string>
#include <iostream>
#include <boost/asio.hpp>

#include "io_if.h"

using namespace boost;

class TcpIo : public io_if {
  private:

   ///Represents the currently open connection
   asio::ip::tcp::socket * socket;

   ///The port on which the connection takes place;
   unsigned int port;

  public:

   ///Opens a new socket connection on the specified port
   TcpIo(unsigned int port = 2000, bool test = false);

   ~TcpIo();

   ///Receives a character; returns true if read the character is valid,
   ///false in case the character is not valid (such as if we are communicating
   ///on a socket and there are no available characters)

   uint32_t receivedChars();
   void getReceivedChar(char * toRecv);

   ///Sends a character on the communication channel
   void sendChar(char toSend);

   ///Creates a connection
   void makeConnection();
};

struct ConnectionThread{
    TcpIo *sock;
    ConnectionThread(TcpIo *sock) : sock(sock){}
    void operator()(){
        sock->makeConnection();
    }
};

#endif // TCPIO_H
