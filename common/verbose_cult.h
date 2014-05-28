// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file verbose_cult.h
/// Implements a unified output system for messages and debunging.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef VERBOSE_H
#define VERBOSE_H

#define CULT_ENABLE
#define CULT_WITH_SYSTEMC
#ifdef USE_SYSTEMC_2_3
#define CULT_USE_SYSTEMC_2_3
#endif
//#define CULT_WITH_TLM
#define CULT_SUBLEVELS 10

#ifdef CULT_ENABLE
#include "cult.h"
#endif
#include "common.h"

#include <iostream>
#include <string>
#include <systemc.h>

namespace v {

// Import std iostream operators
using std::endl;
using std::hex;
using std::dec;
using std::flush;
using std::setw;
using std::setfill;

/// Output colors
class Color {
    public:
        /// Constructor
        ///
        /// @param value The color code to determ the output.
        Color(const char *value) :
            m_value(value) {
        }
    private:
        /// Stores the color code
        const char *m_value;

        /// Prints the color code
        friend ostream &operator <<(ostream &os, const Color &cl);
};

/// Display the color code on the ostream
inline ostream &operator <<(ostream &os, const Color &cl) {
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

        friend ostream &operator <<(ostream &os, const Number &n);
};

/// Display the color code on the ostream
inline ostream &operator <<(ostream &os, const Number &n) {
    os << (n.hex? std::hex : std::dec) << std::setfill(n.fill) << n.prefix << std::setw(n.width);
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

/// Standard verbosity is 3.
/// All messages but debug.
#ifndef VERBOSITY
#  ifdef GLOBALVERBOSITY
#    define VERBOSITY GLOBALVERBOSITY
#  else
#    define VERBOSITY 3
#  endif
#endif

/// Message stream.
/// This stream is used for output messages
template<int level>
class msgstream /*: std::ostream*/ {
    public:
        msgstream() :
            messagestream(this) {
            switch (level) {
                case 0:
                    cultloglevel = CULT_ERROR;
                    break;
                case 1:
                    cultloglevel = CULT_WARNING;
                    break;
                case 2:
                    cultloglevel = CULT_INFO_(2);
                    break;
                case 3:
                    cultloglevel = CULT_INFO;
                    break;
                default:
                    cultloglevel = CULT_DEBUG;
            }
        }

        template<class T>
        //inline msgstream<level>& operator<<(const T &in) {
        inline std::ostream& operator<<(const T &in) {
			if(level < VERBOSITY) {
			  return messagestream << in;
			}
            return messagestream;
        }

        template<class T>
//        inline msgstream<level>& operator<<(std::ostream& (*in)(std::ostream&)) {
        inline std::ostream& operator<<(std::ostream& (*in)(std::ostream&)) {
		  if(level < VERBOSITY) {
            return messagestream << in;
          }
          return messagestream;
        }

        std::string module;
        class stringstream : public std::stringstream {
          public:
            stringstream(msgstream *parent) : std::stringstream(), parent(parent) {}
            int sync() {
              std::string msg = str();
              std::cout << "Sync" << endl;
              std::cout << msg;
              CULT_LOG_MESSAGE_SC(parent->module, cultloglevel, msg);
              // Empty messagestream
              str("");
              return std::stringstream::sync();
            }
          private:
            msgstream *parent;
        } messagestream;

    private:
        cult::LogLevel cultloglevel;
};

/// This stream is used for an output line.
/// It defines the Header of a line and returns a msgstream for the message itself.
template<int level>
class logstream {
    public:
        template<class T>
        inline msgstream<level>& operator<<(const T &in) {
			if ( level < VERBOSITY ) {
			  m_stream.module = in;
			}
            return m_stream;
        }

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
extern logstream<4> debug;

void logApplication(char *name);

} // namespace

#endif // VERBOSE_H
/// @}
