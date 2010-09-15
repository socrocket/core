#include "verbose.h"

namespace out {

template <typename char_type, typename traits = std::char_traits<char_type> >
class basic_teebuf : public std::basic_streambuf<char_type, traits> {
  public:
    typedef typename traits::int_type int_type;
    
    basic_teebuf(std::basic_streambuf<char_type, traits> *sb1,
                 std::basic_streambuf<char_type, traits> *sb2)
      : sb1(sb1)
      , sb2(sb2){}
    
    void set_tee(std::basic_streambuf<char_type, traits> *tee) {
      //this->flush();
      sb2 = tee;
      sync();
    }
    
private:    
    virtual int sync() {
        int const r1 = sb1->pubsync();
        int const r2 = (sb2? sb2->pubsync() : 0);
        return r1 == 0 && r2 == 0 ? 0 : -1;
    }
    
    virtual int_type overflow(int_type c) {
        int_type const eof = traits::eof();
        
        if (traits::eq_int_type(c, eof)) {
            return traits::not_eof(c);

        } else {
            char_type const ch = traits::to_char_type(c);
            int_type const r1 = sb1->sputc(ch);
            if(sb2) {
              sb2->sputc(ch);
              return traits::eq_int_type(r1, eof) || traits::eq_int_type(r1, eof) ? eof : c;
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

void logFile(char *name) {
  if(outfile.is_open()) {
    outfile.close();
  }
  if(name) {
    outfile.open(name);
    logbuf.set_tee(outfile.rdbuf());
  }
}

logstream<0> info(&logbuf);
logstream<1> warn(&logbuf);
logstream<2> error(&logbuf);
logstream<3> debug(&logbuf);

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

} // namespace
