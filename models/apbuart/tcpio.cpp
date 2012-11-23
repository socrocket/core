#include <boost/asio.hpp>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include "tcpio.h"

#include <boost/thread/thread.hpp>
#include "verbose.h"

///Creates a connection
void TcpIo::makeConnection(){
   try{
      asio::io_service io_service;
      asio::ip::tcp::acceptor acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
      this->socket = new asio::ip::tcp::socket(io_service);
      std::cerr << "UART: waiting for connections on port " << port << std::endl;
      acceptor.accept(*this->socket);
      std::cerr << "UART: connection accepted on port " << port << std::endl;
   }
   catch(...){
     v::error << "TcpIo" << "Error during the creation of the connection on port " << port;
   }
}

///Opens a new socket connection on the specified port
TcpIo::TcpIo(unsigned int port, bool test) : port(port){
    if(!test)
        this->makeConnection();
    else{
         //I have to create the connection in a boost thread
         boost::thread thrd(ConnectionThread(this));
    }
}

TcpIo::~TcpIo(){}

///Receives a character; returns true if read the character is valid,
///false in case the character is not valid (such as if we are communicating
///on a socket and there are no available characters)
uint32_t TcpIo::receivedChars() {
   return this->socket->available();
}

///Receives a character; returns true if read the character is valid,
///false in case the character is not valid (such as if we are communicating
///on a socket and there are no available characters)
void TcpIo::getReceivedChar(char * toRecv){
   if(this->socket->available() < 1)
      return;

   boost::system::error_code asioError;
   this->socket->read_some(asio::buffer(toRecv, 1), asioError);

   //if(*toRecv == '\r') {
   //  *toRecv = '\0';
   //}

   if(asioError == asio::error::eof) {
     v::error << "TcpIo" << "Connection with the UART Unexpetedly closed";
   }
}

///Sends a character on the communication channel
void TcpIo::sendChar(char toSend){
   boost::system::error_code asioError;
   asio::write(*this->socket, asio::buffer(&toSend, 1), asio::transfer_all(), asioError);
}
