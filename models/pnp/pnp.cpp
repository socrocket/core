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
//*******************************************************************
// Title       grlibpnp.cpp
//
// ScssId:
// 
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Contains the implementation of the grlib
//             ahbmaster plug and play functionality. Due to the use
//             of the AMBAKit the plug and play functionality is
//             implemented in its own model
//
// Method:
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*******************************************************************

#include "pnp.h"

CPnP::CPnP(sc_core::sc_module_name nm, uint16_t cfgaddr, uint16_t cfgmask) :
        ahb_slave("ahb_slave", amba::amba_AHB, amba::amba_LT, false), 
        mMasterCount(0), mSlaveCount(0), 
        mem_size(0xFFF), baseAddr((cfgaddr & cfgmask) << 20) {

    ahb_slave.register_b_transport(this, &CPnP::BTransport);
}

sc_dt::uint64 CPnP::get_size() {
    return mem_size;
}

sc_dt::uint64 CPnP::get_base_addr() {
    return baseAddr;
}


uint32_t CPnP::GetRegister(uint32_t nr // The number of the register to get
) const {
    bool slave = (nr >> 9) & 0x1;
    uint8_t dev = (nr >> 3) & 0x3F;
    uint8_t reg = (nr) & 0x7;

    if (slave) {
        if (dev < mSlaveCount) {
            return mSlaves[dev][reg];
        }
    } else {
        if (dev < mMasterCount) {
            return mMasters[dev][reg];
        }
    }
    return 0;
}

CPnP::~CPnP() {
}

void CPnP::BTransport(tlm::tlm_generic_payload& gp, // Generic Payload of the current Transaction
                      sc_core::sc_time &t // Time to delay master
) {
    uint32_t *data = reinterpret_cast<unsigned int *> (gp.get_data_ptr());
    if (gp.is_write()) {
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    if (gp.is_read()) {
        uint32_t address = (gp.get_address() & 0xFFF) >> 2;
        uint32_t length = gp.get_data_length() >> 2;
        for (uint32_t i = 0; i < length; i++) {
            data[i] = GetRegister(address + i);
        }
    }
    gp.set_response_status(tlm::TLM_OK_RESPONSE);
}

void CPnP::RegisterMaster(CGrlibDevice *dev // Device to register
) {
    mMasters[mMasterCount++] = dev->GetDevicePointer();
}

void CPnP::RegisterSlave(CGrlibDevice *dev // Device to register
) {
    mSlaves[mSlaveCount++] = dev->GetDevicePointer();
}
