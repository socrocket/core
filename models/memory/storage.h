// Interface for adressable storage, eg. map or array based

#ifndef  STORAGE_H
#define STORAGE_H
#include "systemc.h"

class Storage {
  public:
    virtual void write(const uint32_t addr, const uint8_t byte) = 0;

    virtual uint8_t read(const uint32_t addr) = 0;

    virtual void write_block(const uint32_t addr, uint8_t *data, uint32_t len) = 0;
    
    virtual void read_block(const uint32_t addr, uint8_t *data, uint32_t len) = 0;

    virtual void erase(const uint32_t start, const uint32_t end) = 0;

};

#endif
