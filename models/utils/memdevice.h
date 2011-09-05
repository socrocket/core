//*********************************************************************
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
//*********************************************************************
// Title:      memdevice.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file containing the definition of a baseclass
//             for all memory tlm models. It implements the the device
//             information register needed for the plug and play
//             interface.
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef MEM_DEVICE_H
#define MEM_DEVICE_H

#include <stdint.h>

/// @addtogroup utils
/// @{

/// This class is a base class for memory models. It implements the device plug and play informations.
/// @see mctrl
class MEMDevice {
    public:
        /// Device type
        enum device_type {
            ROM   = 0,
            IO    = 1,
            SRAM  = 2,
            SDRAM = 3
        };

        /// All device informations are needed while constructing a device.
        /// The register content is formed here.
        MEMDevice(device_type type, uint32_t banks = 0, uint32_t bsize = 0, uint32_t bits = 32, uint32_t cols = 0);

        MEMDevice();
        /// Empty destructor
        virtual ~MEMDevice();

        /// Returns the memory configuration.
        virtual const char *get_device_info() const;

        /// Returns the device type.
        virtual const device_type get_type() const;
        
        /// Returns the number of banks of the memory (sram or sdram bank if needed)
        virtual const uint32_t get_banks() const;

        /// Returns the bank size of the memory bank.
        /// All memory banks of a type should have the same size.
        virtual const uint32_t get_bsize() const;
        
        /// Returns the bit width of a memory word.
        /// 8, 16, 32 or 64 bits are allowed.
        virtual const uint32_t get_bits() const;

        /// Returns the column size of the memory (sdram)
        virtual const uint32_t get_cols() const;
    private:
        device_type m_type;
        uint32_t m_banks;
        uint32_t m_bsize;
        uint32_t m_bits;
        uint32_t m_cols;
};

#endif
