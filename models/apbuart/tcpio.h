// ********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the
// European Space Agency or of TU-Braunschweig.
// ********************************************************************
// Title:      tcpio.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose: Class implementing the communication among the simulated UART component and the
//	     host environment; this communication takes place using sockets.
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
// ********************************************************************

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
