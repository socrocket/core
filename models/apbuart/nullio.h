// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup apbuart
/// @{
/// @file nullio.h
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

#ifndef MODELS_APBUART_NULLIO_H_
#define MODELS_APBUART_NULLIO_H_

#include "models/apbuart/io_if.h"

class NullIO : public io_if {
  public:
    NullIO();

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

#endif  // MODELS_APBUART_NULLIO_H_
/// @}
