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
// Title:      grlibdevice.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file containing the definition of a baseclass
//             for all grlib tlm models. It implements the the device
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

#ifndef GRLIB_DEVICE_H
#define GRLIB_DEVICE_H

#include <stdint.h>

/// @addtogroup utils
/// @{

/// This class is a base class for grlib models. It implements the device plug and play informations.
/// Together with the GrlibPnP class it implements the plug and play feature of the grlib.
/// @see GrlibPnP
class CGrlibDevice {
    public:
        /// Device type
        enum grlib_t {
            APBIO = 1, AHBMEM = 2, AHBIO = 3
        };

        /// All device informations are needed while constructing a device.
        /// The register content is formed here.
        CGrlibDevice(uint8_t vendorid, uint16_t deviceid, uint8_t version,
                     uint8_t irq, uint32_t bar0, uint32_t bar1 = 0,
                     uint32_t bar2 = 0, uint32_t bar3 = 0);

        /// Empty destructor
        virtual ~CGrlibDevice();

        /// Returns the device register file.
        virtual const uint32_t *GetDevicePointer();

    private:
        /// Impementation of the device register file.
        uint32_t m_register[8];
};

/// This function returns a grlib bank address register.
/// It is needed to set the plug and play informations in each device model.
///
/// @return The bank address register content.
/// @see GrlibDevice
/// @see GrlibPnP
uint32_t GrlibBAR(CGrlibDevice::grlib_t type, uint16_t mask, bool cacheable,
                  bool prefetchable, uint16_t address);

/// @}

#endif
