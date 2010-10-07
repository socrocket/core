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
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// ScssId:
//
// Title:      grlibpnp.h
//
// Purpose:    header file containing the definition of the grlib
//             ahbmaster plug and play functionality. Due to the use
//             of the AMBAKit the plug and play functionality is
//             implemented in its own model
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

#ifndef GRLIB_PLUG_AND_PLAY_H
#define GRLIB_PLUG_AND_PLAY_H

#include "grlibdevice.h"
#include "amba.h"

/// @addtogroup pnp Grlib PnP
/// @{

/// This model contains the plug and play functionality of the GRLIB AHB Master.
/// It provides the registers to the AHB Bus. And gathers its content from the instantiated model in the simulation environment.
///
/// See the GRLIB user’s manual for more information.
/// These reegister are combined into an array which is connected to the AHB controller unit.
/// The plug&play information is mapped on a read-only address area, defined by the cfgaddr and cfgmask
/// VHDL generics, in combination with the ioaddr and iomask VHDL generics. By default, the
/// area is mapped on address 0xFFFFF000 - 0xFFFFFFFF. The master information is placed on the first
/// 2 kbyte of the block (0xFFFFF000 - 0xFFFFF800), while the slave information is placed on the second
/// 2 kbyte block. Each unit occupies 32 bytes, which means that the area has place for 64 masters
/// and 64 slaves. The address of the plug&play information for a certain unit is defined by its bus index.
/// The address for masters is thus 0xFFFFF000 + n*32, and 0xFFFFF800 + n*32 for slaves.
///
/// In oder to use this implementation you have to know that ther is no mechanism to unregister devices
/// , once a device is registered it connected. Furthermore there is no logic to handle the deletion of
/// a device.
class CPnP : public sc_core::sc_module {
    public:

        /// AHB Slave Socket
        amba::amba_slave_socket<32> ahb_slave;

        SC_HAS_PROCESS( CPnP);

        /// Constructor
        ///
        /// @param ioaddr   The MSB address of the I/O area. Sets the 12 most significant bits in the
        ///                 32-bit AHB address (i.e. 31 downto 20).
        ///                 Allowed values are between 0 and 16#FFF#.
        ///                 Default value is 16#FFF#.
        /// @param iomask   The I/O area address mask. Sets the size of the I/O area and the start
        ///                 address together with ioaddr. Allowed values are between 0 and 16#FFF#.
        ///                 Default value is 16#FFF#.
        /// @param cfgaddr  The MSB address of the configuration area. Sets 12 bits in the 32-bit AHB
        ///                 address (i.e. 19 downto 8). Allowed are values from 0 to 16#FFF#.
        ///                 Default value is 16#FF0#.
        /// @param cfgmask  The address mask of the configuration area. Sets the size of the configuration
        ///                 area and the start address together with cfgaddr.
        ///                 If set to 0, the configuration
        ///                 will be disabled. Values between 0 and 16#FFF# are allowed, default is 16#FF0#.
        CPnP(sc_core::sc_module_name nm, uint16_t ioaddr = 4095,
             uint16_t iomask = 4095, uint16_t cfgaddr = 4080, uint16_t cfgmask =
                     4080);

        /// Empty destructor
        ~CPnP();

        /// Blocking Transport function for the AHB Slave Socket.
        /// Incomming requests will be processed here. Write requests are ignored.
        /// Read requests will seperated in register access and processt in
        /// get_registers(uint32_t nr).
        ///
        /// @param gp The payload object form the socket.
        /// @param t  A time object for delaying.
        void BTransport(tlm::tlm_generic_payload& gp, sc_core::sc_time &t);

        /// Return the content of a specific register.
        /// The register nr is seperated into slave or maste area,
        /// device number and registernumber to access the register.
        uint32_t GetRegister(uint32_t nr) const;

        /// This function is used to register a master device.
        /// In oder to know all mambers on the bus they need to be registerd.
        ///
        /// @param dev The master device which will be registerd.
        void RegisterMaster(CGrlibDevice *dev);

        /// This function is used to register a slave device.
        /// In oder to know all mambers on the bus they need to be registerd.
        ///
        /// @param dev The slave device which will be registerd.
        void RegisterSlave(CGrlibDevice *dev);

    private:
        /// An array of master device registers files.
        const uint32_t *mMasters[64];

        /// An array of slave device registers files.
        const uint32_t *mSlaves[64];

        /// The number of registerd masters.
        uint8_t mMasterCount;

        /// The number of registerd slaves.
        uint8_t mSlaveCount;
};

/// @}

#endif
