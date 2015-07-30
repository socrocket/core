// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file vendian.h
///
/// @details The header vendian.h provides endianess conversion functions 
/// for data types of different lengths. If the host system is little endian, 
/// CPU and unit tests must swap byte order. The latter is defined by the 
/// macro LITTLE_ENDIAN_BO. 
///
/// It has to be kept in mind that the LEON processor is a big endian CPU. 
/// Hence, memory images generated with the SPARC compiler (e.g. BCC) are 
/// also big endian. If the host system simulates the CPU, or testbench, in 
/// little endian byte order, all data items going to/from memory must be 
/// reordered!
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
#ifndef COMMON_VENDIAN_H_
#define COMMON_VENDIAN_H_

#include <stdint.h>

inline void swap_Endianess(uint64_t &datum) {
#ifdef LITTLE_ENDIAN_BO
  uint8_t helperByte = 0;
  for (uint32_t i = 0; i < sizeof (uint64_t) / 2; i++) {
    helperByte = ((uint8_t *)&datum)[i];
    ((uint8_t *)&datum)[i] = ((uint8_t *)&datum)[sizeof (uint64_t) - 1 - i];
    ((uint8_t *)&datum)[sizeof (uint64_t) - 1 - i] = helperByte;
  }
#endif
}

inline void swap_Endianess(uint32_t &datum) {
#ifdef LITTLE_ENDIAN_BO
  uint8_t helperByte = 0;
  for (uint32_t i = 0; i < sizeof (uint32_t) / 2; i++) {
    helperByte = ((uint8_t *)&datum)[i];
    ((uint8_t *)&datum)[i] = ((uint8_t *)&datum)[sizeof (uint32_t) - 1 - i];
    ((uint8_t *)&datum)[sizeof (uint32_t) - 1 - i] = helperByte;
  }
#endif
}

inline void swap_Endianess(uint16_t &datum) {
#ifdef LITTLE_ENDIAN_BO
  uint8_t helperByte = 0;
  for (uint32_t i = 0; i < sizeof (uint16_t) / 2; i++) {
    helperByte = ((uint8_t *)&datum)[i];
    ((uint8_t *)&datum)[i] = ((uint8_t *)&datum)[sizeof (uint16_t) \
                                                 - 1 - i];
    ((uint8_t *)&datum)[sizeof (uint16_t) - 1 - i] = helperByte;
  }
#endif
}

inline void swap_Endianess(uint8_t &datum) {}

#endif  // COMMON_VENDIAN_H_
/// @}
