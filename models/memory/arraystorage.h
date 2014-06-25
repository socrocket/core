#ifndef ARRAYSTORAGE_H
#define ARRAYSTORAGE_h

#include "storage.h"

class ArrayStorage : public Storage {

  public:
    ArrayStorage(const uint32_t &size);
    
    ~ArrayStorage();

    void write(const uint32_t &addr, const uint8_t &byte);
  
    uint8_t read(const uint32_t &addr);

    void write_block(const uint32_t &addr, uint8_t *ptr, const uint32_t &len);
    
    void read_block(const uint32_t &addr, uint8_t *ptr, const uint32_t &len) const;
  
    void erase(const uint32_t &start, const uint32_t &end);

  private:
    uint8_t *data;



};

#endif
