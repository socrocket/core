// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file vendian.h
/// 
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef VENDIAN_H
#define VENDIAN_H

#include <stdint.h>

inline void swap_Endianess(uint64_t &datum) {
    #ifdef LITTLE_ENDIAN_BO
    uint8_t helperByte = 0;
    for(uint32_t i = 0; i < sizeof(uint64_t)/2; i++){
        helperByte = ((uint8_t *)&datum)[i];
        ((uint8_t *)&datum)[i] = ((uint8_t *)&datum)[sizeof(uint64_t) -1 -i];
        ((uint8_t *)&datum)[sizeof(uint64_t) -1 -i] = helperByte;
    }
    #endif
}

inline void swap_Endianess(uint32_t &datum) {
    #ifdef LITTLE_ENDIAN_BO
    uint8_t helperByte = 0;
    for(uint32_t i = 0; i < sizeof(uint32_t)/2; i++){
        helperByte = ((uint8_t *)&datum)[i];
        ((uint8_t *)&datum)[i] = ((uint8_t *)&datum)[sizeof(uint32_t) -1 -i];
        ((uint8_t *)&datum)[sizeof(uint32_t) -1 -i] = helperByte;
    }
    #endif
}

inline void swap_Endianess(uint16_t &datum) {
    #ifdef LITTLE_ENDIAN_BO
    uint8_t helperByte = 0;
    for(uint32_t i = 0; i < sizeof(uint16_t)/2; i++){
        helperByte = ((uint8_t *)&datum)[i];
        ((uint8_t *)&datum)[i] = ((uint8_t *)&datum)[sizeof(uint16_t) \
            -1 -i];
        ((uint8_t *)&datum)[sizeof(uint16_t) -1 -i] = helperByte;
    }
    #endif
}

inline void swap_Endianess(uint8_t &datum) {}

#endif
/// @}