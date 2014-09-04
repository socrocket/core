// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup apbuart
/// @{
/// @file nullio.cpp
///
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include "core/models/apbuart/nullio.h"
#include "core/common/verbose.h"

/// Creates a connection
void NullIO::makeConnection() {
}

/// Opens a new socket connection on the specified port
NullIO::NullIO() {
}

/// Receives a character; returns true if read the character is valid,
/// false in case the character is not valid (such as if we are communicating
/// on a socket and there are no available characters)
uint32_t NullIO::receivedChars() {
  return 0;
}

/// Receives a character; returns true if read the character is valid,
/// false in case the character is not valid (such as if we are communicating
/// on a socket and there are no available characters)
void NullIO::getReceivedChar(char *toRecv) {
}

/// Sends a character on the communication channel
void NullIO::sendChar(char toSend) {
}
/// @}
