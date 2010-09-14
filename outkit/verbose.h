/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       output.h:                                               */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TU Braunschweig              */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef OUTPUT_H
#define OUTPUT_H

#include <systemc.h>
#include <iostream>
#include <streambuf>

/// @addtogroup utils
/// @{

namespace out {

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

//#ifndef DEBUG
#define DEBUG 3
//#endif

#define output_header \
               "@" \
            <<  sc_core::sc_time_stamp().to_string().c_str() \
            << " /" \
            << std::dec \
            << (unsigned)sc_core::sc_delta_count() \
            << " (" << out::Blue \
            << "Global" << out::Normal \
            << "): "

#define oinfo \
  out::info << output_header << "Info: "

#define owarn \
  out::warn << output_header << out::Yellow << "Warning" << out::Normal << ": "

#define oerror \
  out::warn << output_header << out::Red << "Error" << out::Normal << ": "

#define odebug \
  out::warn << output_header << out::Cyan << "Debug" << out::Normal << ": "

              
template<int level>
class logstream {
  public:
    logstream(std::streambuf *sb) : m_stream(sb) {}

    //inline void show() const {
    //}
 
    template <class T>
    inline
    logstream& operator<<(const T &in) {
      if(level<DEBUG) {
        m_stream << in;
      }
      return *this;
    }
 
    inline
    logstream& operator<<(std::ostream& (*in)(std::ostream&)) {
      if(level<DEBUG) {
        m_stream << in;
      }
      return *this;
    }
    
  private:
    std::ostream m_stream;
};

extern logstream<0> info;
extern logstream<1> warn;
extern logstream<2> error;
extern logstream<3> debug;

void logFile(char *);

} // namespace 

//char *name() {
//  return "Global";
//}

/// @}

#endif // OUTPUT_H
