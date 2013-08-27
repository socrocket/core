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
// Title:      nullio.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
// ********************************************************************

#include <iostream>

#include "nullio.h"

#include "verbose.h"

///Creates a connection
void NullIO::makeConnection(){
}

///Opens a new socket connection on the specified port
NullIO::NullIO() {
}

///Receives a character; returns true if read the character is valid,
///false in case the character is not valid (such as if we are communicating
///on a socket and there are no available characters)
uint32_t NullIO::receivedChars() {
   return 0;
}

///Receives a character; returns true if read the character is valid,
///false in case the character is not valid (such as if we are communicating
///on a socket and there are no available characters)
void NullIO::getReceivedChar(char * toRecv){
   return;
}

///Sends a character on the communication channel
void NullIO::sendChar(char toSend){
}
