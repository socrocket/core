// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup apbuart
/// @{
/// @file io_if.h
///
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef MODELS_APBUART_IO_IF_H_
#define MODELS_APBUART_IO_IF_H_

#include <stdint.h>

class io_if {
  public:
    virtual uint32_t receivedChars() = 0;
    virtual void getReceivedChar(char *toRecv) = 0;
    virtual void sendChar(char toSend) = 0;
};

#endif  // MODELS_APBUART_IO_IF_H_
/// @}
