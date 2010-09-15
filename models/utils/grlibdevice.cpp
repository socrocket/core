/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       grlibdevice.cpp                                         */
/*             contains the implementation of a baseclass              */
/*             for all grlib tlm models. It implements the the device  */
/*             information register needed for the plug and play       */
/*             interface.                                              */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#include "grlibdevice.h"

CGrlibDevice::CGrlibDevice(uint8_t vendorid, uint16_t deviceid, uint8_t version, uint8_t irq, 
                           uint32_t bar0, uint32_t bar1, uint32_t bar2, uint32_t bar3) {
    m_register[0] = (irq & 0x1F) | ((version & 0x1F) << 5) | 
                    ((deviceid & 0xFFF) << 12) | (vendorid << 24);
    m_register[1] = m_register[2] = m_register[3] = 0;
    m_register[4] = bar0;
    m_register[5] = bar1;
    m_register[6] = bar2;
    m_register[7] = bar3;
}

CGrlibDevice::~CGrlibDevice() {}

const uint32_t *CGrlibDevice::GetDevicePointer() {
    return m_register;
}
    
uint32_t GrlibBAR(CGrlibDevice::grlib_t type, uint16_t mask, bool cacheable, bool prefetchable, uint16_t address) {
  return static_cast<uint8_t>(type) | 
         (mask << 4) | 
         (cacheable << 16) | 
         (prefetchable << 17) | 
         (address << 20);
}

