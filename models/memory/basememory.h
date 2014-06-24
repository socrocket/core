#ifndef BASEMEMORY_H
#define BASEMEMORY_H

#include "mapstorage.h"
#include "arraystorage.h"
#include "storage.h"

class BaseMemory {

  public:
    enum implementation_type {
      ARRAY = 0,
      MAP = 1
    };

    typedef implementation_type type;
    
    BaseMemory(implementation_type type, const uint32_t size);
    
    ~BaseMemory();

    uint8_t read(const uint32_t addr);

    uint8_t read_dbg(const uint32_t addr);

    void write(const uint32_t addr, const uint8_t byte);
    
    void write_dbg(const uint32_t addr, const uint8_t byte);

    void write_block(const uint32_t addr, uint8_t *data, uint32_t len);
    
    void write_block_dbg(const uint32_t addr, uint8_t *data, uint32_t len);
    
    void read_block(const uint32_t addr, uint8_t *data, uint32_t len);
    
    void read_block_dbg(const uint32_t addr, uint8_t *data, uint32_t len);

    void erase(uint32_t start, uint32_t end);

    void erase_dbg(uint32_t start, uint32_t end);

    // byte read count
    unsigned long long reads;

    // byte write count
    unsigned long long writes;

    // word (4 bytes) read count
    unsigned long long reads32;

    // word (4 bytes) write count
    unsigned long long writes32;

  private:
    Storage *storage;

};

#endif
