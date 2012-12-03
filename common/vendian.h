// ********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the
// European Space Agency or of TU-Braunschweig.
// ********************************************************************
// Title:      vendian.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
// ********************************************************************

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
