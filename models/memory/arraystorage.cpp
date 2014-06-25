#include "arraystorage.h"

ArrayStorage::ArrayStorage(const uint32_t &size) {
  data = new uint8_t[size];
  erase(0, size);
}

ArrayStorage::~ArrayStorage() {
  delete[] data;
}

void ArrayStorage::write(const uint32_t &addr, const uint8_t &byte) {
  data[addr] = byte;
}

uint8_t ArrayStorage::read(const uint32_t &addr) {
  return data[addr];
}

void ArrayStorage::erase(const uint32_t &start, const uint32_t &end) {
  memset(&data[start], 0, end-start);
}

void ArrayStorage::write_block(const uint32_t &addr, uint8_t *ptr, const uint32_t &len) {
  memcpy(&data[addr], ptr, len);
}

void ArrayStorage::read_block(const uint32_t &addr, uint8_t *ptr, const uint32_t &len) const {
  memcpy(ptr, &data[addr], len);
}
