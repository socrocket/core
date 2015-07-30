// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup apbuart
/// @{
/// @file reportio.cpp
///
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include "gaisler/apbuart/reportio.h"

SR_HAS_UARTBACKEND(ReportIO);

/// Creates a connection
void ReportIO::makeConnection() {
}

/// Opens a new socket connection on the specified port
ReportIO::ReportIO(sc_core::sc_module_name nm) : 
  BaseModule<DefaultBase> (nm),
  g_lines("lines", false, m_generics) {
    line = "";
  }

/// Receives a character; returns true if read the character is valid,
/// false in case the character is not valid (such as if we are communicating
/// on a socket and there are no available characters)
uint32_t ReportIO::receivedChars() {
  return 0;
}

/// Receives a character; returns true if read the character is valid,
/// false in case the character is not valid (such as if we are communicating
/// on a socket and there are no available characters)
void ReportIO::getReceivedChar(char *toRecv) {
}

/// Sends a character on the communication channel
void ReportIO::sendChar(char toSend) {
  if (g_lines) {
    line += toSend;
    if (toSend == '\n') {
      srInfo()
        ("line", line)
        ("line sent");
      line = "";
    } 
  } else {
    srInfo()
      ("character", toSend)
      ("character sent");
  }
}
/// @}
