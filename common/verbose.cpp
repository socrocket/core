// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file verbose.cpp
/// Implements a unified output system for messages and debunging.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#include <string>
#include "core/common/verbose.h"

namespace v {
template<typename char_type, typename traits = std::char_traits<char_type> >
class basic_teebuf : public std::basic_streambuf<char_type, traits> {
  public:
    typedef typename traits::int_type int_type;

    basic_teebuf(std::basic_streambuf<char_type, traits> *sb1,
    std::basic_streambuf<char_type, traits> *sb2) :
      sb1(sb1), sb2(sb2) {
    }

    void set_tee(std::basic_streambuf<char_type, traits> *tee) {
      // this->flush();
      sb2 = tee;
      sync();
    }

  private:
    virtual int sync() {
      int const r1 = sb1->pubsync();
      int const r2 = (sb2 ? sb2->pubsync() : 0);
      return r1 == 0 && r2 == 0 ? 0 : -1;
    }

    virtual int_type overflow(int_type c) {
      int_type const eof = traits::eof();

      if (traits::eq_int_type(c, eof)) {
        return traits::not_eof(c);
      } else {
        char_type const ch = traits::to_char_type(c);
        int_type const r1 = sb1->sputc(ch);
        if (sb2) {
          sb2->sputc(ch);
          return traits::eq_int_type(r1, eof) || traits::eq_int_type(
            r1, eof) ? eof : c;
        } else {
          return traits::eq_int_type(r1, eof) ? eof : c;
        }
      }
    }

  private:
    std::basic_streambuf<char_type, traits> *sb1;
    std::basic_streambuf<char_type, traits> *sb2;
};

typedef basic_teebuf<char> teebuf;
std::ofstream outfile;
teebuf logbuf(std::cout.rdbuf(), NULL);

void logFile(const char *name) {
  if (outfile.is_open()) {
    outfile.close();
  }
  if (name) {
    outfile.open(name);
    logbuf.set_tee(outfile.rdbuf());
  }
}

void logApplication(const char *name) {
  // Create a logfile next to the binary
  char logfile[1024];
  logfile[0] = 0;
  snprintf(logfile, sizeof(logfile), "%s%s", name, ".log");
  v::logFile(logfile);
}

logstream<0> error(&logbuf);
logstream<1> warn(&logbuf);
logstream<2> report(&logbuf);
logstream<3> info(&logbuf);
logstream<4> analysis(&logbuf);
logstream<5> debug(&logbuf);
logstream<99> null_log(&logbuf);

/** Linux internal consol color pattern */
Color bgBlack("\e[40m");
Color bgWhite("\e[47m");
Color bgYellow("\e[43m");
Color bgRed("\e[41m");
Color bgGreen("\e[42m");
Color bgBlue("\e[44m");
Color bgMagenta("\e[45m");
Color bgCyan("\e[46m");

Color Black("\e[30m");
Color White("\e[37m");
Color Yellow("\e[33m");
Color Red("\e[31m");
Color Green("\e[32m");
Color Blue("\e[34m");
Color Magenta("\e[35m");
Color Cyan("\e[36m");

Color Normal("\e[0m");
Color Bold("\033[1m");
Color Blink("\e[36m");
Color Beep("\e[36m");

Number uint64("0x", '0', 16, true);  // NOLINT(readability/casting)
Number uint32("0x", '0', 8, true);   // NOLINT(readability/casting)
Number uintptr("0x", '0', 8, true); 
Number uint16("0x", '0', 4, true);   // NOLINT(readability/casting)
Number uint8("0x", '0', 2, true);    // NOLINT(readability/casting)
Number noint("", ' ', 0, false);     // NOLINT(readability/casting)
}  // namespace v

/// @}
