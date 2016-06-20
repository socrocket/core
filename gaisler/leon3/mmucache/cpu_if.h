#ifndef GAISLER_LEON3_MMU_CACHE_CPU_IF_H
#define GAISLER_LEON3_MMU_CACHE_CPU_IF_H

#include "core/common/trapgen/ToolsIf.hpp"
#include "core/common/trapgen/utils/trap_utils.hpp"
#include "core/common/systemc.h"

class cpu_if {
  public:
    virtual sc_dt::uint64 read_dword(const uint32_t &address, const uint32_t asi = 0xA, const uint32_t flush = 0, const uint32_t lock = 0) throw() = 0;
    virtual uint32_t read_word(const uint32_t &address , const uint32_t asi = 0xA, const uint32_t flush = 0, const uint32_t lock = 0) throw() = 0;
    virtual uint16_t read_half(const uint32_t &address, const uint32_t asi = 0xA, const uint32_t flush = 0, const uint32_t lock = 0) throw() = 0;
    virtual uint8_t read_byte(const uint32_t &address, const uint32_t asi = 0xA, const uint32_t flush = 0, const uint32_t lock = 0) throw() = 0;
    virtual uint32_t read_instr(const uint32_t &address, const uint32_t asi = 0xA, const uint32_t flush = 0) throw() = 0;
    virtual sc_dt::uint64 read_dword_dbg(const uint32_t &address) {
        return this->read_dword(address, 0x8, 0, 0);
    }

    virtual uint32_t read_word_dbg(const uint32_t &address) {
        return this->read_word(address, 0x8, 0, 0);
    }

    virtual uint16_t read_half_dbg(const uint32_t &address) {
        return this->read_half(address, 0x8, 0, 0);
    }

    virtual uint8_t read_byte_dbg(const uint32_t &address) {
        return this->read_byte(address, 0x8, 0, 0);
    }

    virtual void write_dword(const uint32_t &address, sc_dt::uint64 datum, const uint32_t asi = 0xA, const uint32_t flush = 0, const uint32_t lock = 0) throw() = 0;
    virtual void write_word(const uint32_t &address, uint32_t datum, const uint32_t asi = 0xA, const uint32_t flush = 0, const uint32_t lock = 0) throw() = 0;
    virtual void write_half(const uint32_t &address, uint16_t datum, const uint32_t asi = 0xA, const uint32_t flush = 0, const uint32_t lock = 0) throw() = 0;
    virtual void write_byte(const uint32_t &address, uint8_t datum, const uint32_t asi = 0xA, const uint32_t flush = 0, const uint32_t lock = 0) throw() = 0;
    virtual void write_dword_dbg(const uint32_t &address, sc_dt::uint64 datum) {
        this->write_dword(address, datum, 0x8, 0, 0);
    }

    virtual void write_word_dbg(const uint32_t &address, uint32_t datum) {
        this->write_word(address, datum, 0x8, 0, 0);
    }

    virtual void write_half_dbg(const uint32_t &address, uint16_t datum) {
        this->write_half(address, datum, 0x8, 0, 0);
    }

    virtual void write_byte_dbg(const uint32_t &address, uint8_t datum) {
        this->write_byte(address, datum, 0x8, 0, 0);
    }

    virtual void lock() = 0;
    virtual void unlock() = 0;
    inline void swapEndianess(uint32_t & datum) const throw() {
        uint8_t helperByte = 0;
        for(uint32_t i = 0; i < sizeof(uint32_t)/2; i++){
            helperByte = ((uint8_t *)&datum)[i];
            ((uint8_t *)&datum)[i] = ((uint8_t *)&datum)[sizeof(uint32_t) -1 -i];
            ((uint8_t *)&datum)[sizeof(uint32_t) -1 -i] = helperByte;
        }
    }
    inline void swapEndianess(uint16_t & datum) const throw() {
        uint8_t helperByte = 0;
        for(uint32_t i = 0; i < sizeof(uint16_t)/2; i++){
            helperByte = ((uint8_t *)&datum)[i];
            ((uint8_t *)&datum)[i] = ((uint8_t *)&datum)[sizeof(uint16_t) \
                -1 -i];
            ((uint8_t *)&datum)[sizeof(uint16_t) -1 -i] = helperByte;
        }
    }
    virtual ~cpu_if() {};
};

#endif  // GAISLER_LEON3_MMU_CACHE_CPU_IF_H
