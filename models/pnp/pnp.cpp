//*******************************************************************
// Project:    HW-SW SystemC Co-Simulation SoC Validation Platform
//
// File:       grlibpnp.cpp
//             Contains the implementation of the grlib
//             ahbmaster plug and play functionality. Due to the use
//             of the AMBAKit the plug and play functionality is
//             implemented in its own model
// 
// Modified on $Date$
//          at $Revision$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
//*******************************************************************

#include "pnp.h"

CPnP::CPnP(
    sc_core::sc_module_name nm, 
    uint16_t ioaddr, 
    uint16_t iomask, 
    uint16_t cfgaddr, 
    uint16_t cfgmask
): 
    ahb_slave(
        "ahb_slave", 
        amba::amba_AHB, 
        amba::amba_LT, 
        false), 
    mMasterCount(0), 
    mSlaveCount(0)
{
   
    ahb_slave.register_b_transport(this, &CPnP::BTransport);
}

uint32_t CPnP::GetRegister(
    uint32_t nr                  // The number of the register to get
) const
{
    bool      slave   = (nr >> 9) & 0x1;
    uint8_t   dev     = (nr >> 3) & 0x3F;
    uint8_t   reg     = (nr)      & 0x7;
  
    if(slave)
    {
        if(dev < mSlaveCount)
        {
            return mSlaves[dev][reg];
        }
    } 
    else 
    {
        if(dev < mMasterCount) 
        {
            return mMasters[dev][reg];
        }
    }
    return 0;
}

CPnP::~CPnP()
{
}

void CPnP::BTransport(
    tlm::tlm_generic_payload& gp, // Generic Payload of the current Transaction
    sc_core::sc_time &t           // Time to delay master
) 
{
    uint32_t *data    = reinterpret_cast<unsigned int *>(gp.get_data_ptr());
    if(gp.is_write())
    {
        gp.set_response_status(tlm::TLM_OK_RESPONSE);  
    }
    
    if(gp.is_read())
    {
        uint32_t address = (gp.get_address() & 0xFFF) >> 2;
        uint32_t length = gp.get_data_length() >> 2;
        for (uint32_t i=0; i<length; i++)
        {
            data[i] = GetRegister(address + i);
        }
    }
    gp.set_response_status(tlm::TLM_OK_RESPONSE); 
}

void CPnP::RegisterMaster(
    CGrlibDevice *dev            // Device to register
) 
{
    mMasters[mMasterCount++] = dev->GetDevicePointer();
}
    
void CPnP::RegisterSlave(
    CGrlibDevice *dev           // Device to register
) 
{
    mSlaves[mSlaveCount++] = dev->GetDevicePointer();
} 
