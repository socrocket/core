// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup apbuart
/// @{
/// @file reportio.h
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

#ifndef MODELS_APBUART_REPORTIO_H_
#define MODELS_APBUART_REPORTIO_H_

#include "gaisler/apbuart/io_if.h"
#include "core/common/systemc.h"
#include "core/common/base.h"
#include "core/common/sr_report.h"

class ReportIO : public BaseModule<DefaultBase>, public io_if {
  public:
    ReportIO(sc_core::sc_module_name nm);

    /// Receives a character; returns true if read the character is valid,
    /// false in case the character is not valid (such as if we are communicating
    /// on a socket and there are no available characters)
    uint32_t receivedChars();

    void getReceivedChar(char *toRecv);

    /// Sends a character on the communication channel
    void sendChar(char toSend);

    /// Creates a connection
    void makeConnection();

    /// Defines if single characters are send or only complete lines
    sr_param<bool> g_lines;

  private:
    std::string line;
};

#endif  // MODELS_APBUART_REPORTIO_H_
/// @}
