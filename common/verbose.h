// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file verbose.h
/// @brief Implements a unified output system for messages and debunging.
///
/// @details The operators defined in verbose.h can be used to filter output
/// messages respecting their severity. As explained in @ref install2 "Bulding the Library" the verbosity
/// level of the simulations must be defined during configuration of the library:
///
///     $ ./waf configure –verbosity=1..5
///
/// Five levels may be chosen: error,
/// warning, report, info and debug.
///
/// The operators are used in a similar way to C++ stdout:
///
/// ~~~{.cpp}
/// std::cout   << value << std::endl;  // Regular C++ stdout
///
/// v::error    << value << v::endl;    // Verbosity error stream
/// v::warn     << value << v::endl;    // Verbosity warn stream
/// v::report   << value << v::endl;    // Verbosity report stream
/// v::info     << value << v::endl;    // Verbosity info stream
/// v::debug    << value << v::endl;    // Verbosity debug stream
/// ~~~
/// Defining the verbosity at configuration time has the advantage that undesired output is optimized way (compared to runtime switching).
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef COMMON_VERBOSE_H_
#define COMMON_VERBOSE_H_

#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>  // NOLINT(readability/streams)
#include <streambuf>

#include "core/common/common.h"
#include "core/common/systemc.h"

namespace v {
// Import std istd::ostream operators
using std::dec;
using std::endl;
using std::flush;
using std::hex;
using std::setfill;
using std::setw;

/// Output colors
class Color {
  public:
    /// Constructor
    ///
    /// @param value The color code to determ the output.
    explicit Color(const char *value) :
      m_value(value) {
    }
  private:
    /// Stores the color code
    const char *m_value;

    /// Prints the color code
    friend std::ostream &operator<<(std::ostream &os, const Color &cl);
};

/// Display the color code on the std::ostream
inline std::ostream &operator<<(std::ostream &os, const Color &cl) {
  os << cl.m_value;
  return os;
}

/// Output colors
class Number {
  public:
    /// Constructor
    Number(const char *_prefix, char _fill, int _width, bool _hex) :
      prefix(_prefix), fill(_fill), width(_width), hex(_hex) {}
  private:
    const char *prefix;
    char fill;
    int width;
    bool hex;

    friend std::ostream &operator<<(std::ostream &os, const Number &n);
};

/// Display the color code on the std::ostream
inline std::ostream &operator<<(std::ostream &os, const Number &n) {
  os << (n.hex ? std::hex : std::dec) << std::setfill(n.fill) << n.prefix << std::setw(n.width);
  return os;
}

// Colors defined in the cpp file
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

extern Number uint64;
extern Number uint32;
extern Number uint16;
extern Number uint8;
extern Number noint;
extern Number uintptr;

/// Standard verbosity is 3.
/// All messages but debug.
#ifndef VERBOSITY
#ifdef GLOBALVERBOSITY
#define VERBOSITY GLOBALVERBOSITY
#else
#define VERBOSITY 4
#endif
#endif

/// Message stream.
/// This stream is used for output messages
template<int level>
class msgstream {
  public:
    explicit msgstream(std::streambuf *sb) :
      m_stream(sb) {
    }

    inline msgstream &operator<<(const std::stringstream &in) {
      if (level < VERBOSITY) {
        m_stream << in.str();
      }
      return *this;
    }

    template<class T>
    inline msgstream &operator<<(const T &in) {
      if (level < VERBOSITY) {
        m_stream << in;
      }
      return *this;
    }

    inline msgstream &operator<<(std::ostream & (*in)(std::ostream &)) {
      if (level < VERBOSITY) {
        m_stream << in;
      }
      return *this;
    }
  private:
    std::ostream m_stream;
};

/// This stream is used for an output line.
/// It defines the Header of a line and returns a msgstream for the message itself.
template<int level>
class logstream {
  public:
    explicit logstream(std::streambuf *sb) :
      m_stream(sb) {
    }

    template<class T>
    inline msgstream<level> &operator<<(const T &in) {
      if (level < VERBOSITY) {
        m_stream << "@" << sc_core::sc_time_stamp().to_string().c_str()
                 << " /" << std::dec
                 << (unsigned)sc_core::sc_delta_count() << " ("
                 << ::v::Blue << in << ::v::Normal << "): ";
        switch (level) {
        case 0:
          m_stream << v::Red << "Error: " << v::Normal;
          break;
        case 1:
          m_stream << v::Yellow << "Warning: " << v::Normal;
          break;
        case 2:
          m_stream << v::Green << "Report: " << v::Normal;
          break;
        case 3:
          m_stream << v::Cyan << "Info: " << v::Normal;
          break;
        case 4:
          m_stream << v::Blue << "Analysis: " << v::Normal;
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

    operator bool() const {
      return level < VERBOSITY;
    }

  private:
    msgstream<level> m_stream;
};

extern logstream<0> error;
extern logstream<1> warn;
extern logstream<2> report;
extern logstream<3> info;
extern logstream<4> analysis;
extern logstream<5> debug;
extern logstream<99> null_log;

/// This function can be used if you wish to log all verbose output in a file.
/// The logfile gets filled with data in parallel to the screen output.
/// If you want to end the logging to a file simply call the function with NULL as parameter.
///
/// @param filename The file name of the logfile.
void logFile(const char *filename);

/// This function is intended to create a logfile next to the executable simply callit like:
/// > v::logApplication(argv[0])
/// If argv[0] is "./build/test1/mytest" the logfile will be "./build/test1/mytest.log"
///
/// @param name Application name
void logApplication(const char *name);
}  // namespace v

#endif  // COMMON_VERBOSE_H_
/// @}
