//*********************************************************************
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
//*********************************************************************
// Title:      verbose.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implements a unified output system for messages
//             and debunging.
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TU Braunschweig
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef VERBOSE_H
#define VERBOSE_H

#include <systemc.h>
#include <iostream>
#include <streambuf>
#include <iomanip>

/// @addtogroup utils
/// @{

namespace v {

using std::endl;
using std::hex;
using std::dec;
using std::flush;
using std::setw;
using std::setfill;


class Color {
  public:
    Color(const char *value) : m_value(value) {}
  private:
    const char *m_value;
  friend ostream &operator << (ostream &os, const Color &cl);
};

inline ostream &operator << (ostream &os, const Color &cl) {
  os << cl.m_value;
  return os;
}

extern Color bgBlack;
extern Color bgWhite;
extern Color bgYellow;
extern Color bgRed;
extern Color bgGreen;
extern Color bgBlue;
extern Color bgMagenta;
extern Color bgCyan;

extern Color Black;
extern Color White;
extern Color Yellow;
extern Color Red;
extern Color Green;
extern Color Blue;
extern Color Magenta;
extern Color Cyan;

extern Color Normal;
extern Color Bold;
extern Color Blink;
extern Color Beep;

#ifndef VERBOSITY
#define VERBOSITY 1
#endif

template<int level>
class msgstream {
  public:
    msgstream(std::streambuf *sb) : m_stream(sb) {}

    template <class T>
    inline
    msgstream& operator<<(const T &in) {
      if(level<VERBOSITY) {
        m_stream << in;
      }
      return *this;
    }

    inline
    msgstream& operator<<(std::ostream& (*in)(std::ostream&)) {
      if(level<VERBOSITY) {
        m_stream << in;
      }
      return *this;
    }

  private:
    std::ostream m_stream;
};

template<int level>
class logstream {
  public:
    logstream(std::streambuf *sb) : m_stream(sb) {}

    template <class T>
    inline
    msgstream<level>& operator<<(const T &in) {
      if(level<VERBOSITY) {
        m_stream << "@"
                 <<  sc_core::sc_time_stamp().to_string().c_str()
                 << " /"
                 << std::dec
                 << (unsigned)sc_core::sc_delta_count()
                 << " (" << ::v::Blue
                 << in << ::v::Normal
                 << "): ";
        switch(level) {
          case 0:
            m_stream << v::Red << "Error: " << v::Normal;
            break;
          case 1:
            m_stream << v::Yellow << "Warning: " << v::Normal;
            break;
          case 2:
            m_stream << v::Cyan << "Info: " << v::Normal;
            break;
          default:
            m_stream << v::Magenta << "Debug: " << v::Normal;
        }
      }
      return m_stream;
    }

    /*inline
    msgstream<level>& operator<<(std::ostream& (*in)(std::ostream&)) {
      if(level<VERBOSITY) {
        m_stream << in;
      }
      return *this;
    }*/

  private:
    msgstream<level> m_stream;
};

extern logstream<0> error;
extern logstream<1> warn;
extern logstream<2> info;
extern logstream<3> debug;

void logFile(char *);

} // namespace

/// @}

#endif // VERBOSE_H
