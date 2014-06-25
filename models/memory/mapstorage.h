#ifndef MAPSTORAGE_H
#define MAPSTORAGE_h

#include "vmap.h"

#include "storage.h"

class MapStorage : public Storage {

  public:
    MapStorage(const uint32_t &size);
    
    ~MapStorage();

    void write(const uint32_t &addr, const uint8_t &byte);
  
    uint8_t read(const uint32_t &addr);

    void write_block(const uint32_t &addr, uint8_t *ptr, const uint32_t &len);
    
    void read_block(const uint32_t &addr, uint8_t *ptr, const uint32_t &len) const;
  
    void erase(const uint32_t &start, const uint32_t &end);

  private:
    typedef vmap<uint32_t, uint8_t> map_mem;
    map_mem data;


};

#endif
