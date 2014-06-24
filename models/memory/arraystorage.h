#ifndef ARRAYSTORAGE_H
#define ARRAYSTORAGE_h

#include "storage.h"

class ArrayStorage : public Storage {

  public:
    ArrayStorage(const uint32_t size);
    
    ~ArrayStorage();

    virtual void write(const uint32_t addr, const uint8_t byte);
  
    virtual uint8_t read(const uint32_t addr);

    virtual void write_block(const uint32_t addr, uint8_t *ptr, uint32_t len);
    
    virtual void read_block(const uint32_t addr, uint8_t *ptr, uint32_t len);
  
    virtual void erase(const uint32_t start, const uint32_t end);

  private:
    uint8_t *data;



};

#endif
