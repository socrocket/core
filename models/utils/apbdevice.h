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
// Title:      ahbdevice.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file containing the definition of a baseclass
//             for all ahb tlm models. It implements the the device
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

#ifndef APB_DEVICE_H
#define APB_DEVICE_H

#include <stdint.h>
#include <amba.h>

/// @addtogroup utils
/// @{

/// This class is a base class for grlib models. It implements the device plug and play informations.
/// Together with the APBBridge class it implements the plug and play feature of the grlib.
/// @see APBCtrl
class APBDevice : public amba_slave_base {
    public:
        /// Device type
        /// See grlib manual for more information
        /// Section 14.2.2
        enum device_type {
            NONE = 0,   /// Bar is not existing
            APBIO = 1,  /// Bar is APB region
            AHBMEM = 2, /// Bar is memory region
            AHBIO = 3   /// Bar is IO region
        };

        /// All device informations are needed while constructing a device.
        /// The register content is formed here.
        APBDevice(uint32_t bus_id, uint8_t vendorid, uint16_t deviceid, uint8_t version,
                  uint8_t irq, APBDevice::device_type type, uint16_t mask, 
                  bool cacheable, bool prefetchable, uint16_t address);

        APBDevice();
        /// Empty destructor
        virtual ~APBDevice();

        /// Returns the device register file.
        /// A set of 8 registers as specified by the grlib manual.
        /// See section: 14.2.2 (Page 79)
        virtual const uint32_t *get_device_info() const;

        /// Returns the device type.
        /// Should be APBIO ;-)
        virtual const device_type get_type() const;
        /// Returns the Bus specific most significant 12bit of the base address
        /// Shifted to the lowest bits in the word.
        virtual const uint32_t get_base() const;

        /// Returns the Bus specific mask of the most significant 12bit of the address
        /// Shifted to the lowest bits in the word.
        virtual const uint32_t get_mask() const;
        
        /// Returns the Bus specific base address of the device.
        /// @see get_bar_addr
        /// @return The device base address.
        virtual sc_dt::uint64 get_base_addr();
        virtual const uint32_t get_base_addr_() const;

        /// Returns the size of the hole device as seen from the bus.
        /// @see get_bar_size
        /// @return The device size.
        virtual sc_dt::uint64 get_size();
        virtual const uint32_t get_size_() const;

	/// Returns the bus id of the module (pindex)
	const uint32_t get_busid() const;

        /// Prints the device info of the device.
        virtual void print_device_info(char *name) const;
    private:
        /// Impementation of the device register file.
        uint32_t m_register[2];

	/// The slave bus id of the device (pindex)
	uint32_t m_busid;
};

#endif
