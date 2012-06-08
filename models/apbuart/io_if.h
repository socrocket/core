#ifndef IO_IF_H
#define IO_IF_H

#include <stdint.h>

class io_if {
  public:
    virtual uint32_t receivedChars() = 0;
    virtual void getReceivedChar(char *) = 0;
    virtual void sendChar(char) = 0;
};

#endif // IO_IF_H
