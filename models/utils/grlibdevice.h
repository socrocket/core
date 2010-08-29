/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       grlibdevice.h                                           */
/*             header file containing the definition of a baseclass    */
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

#ifndef GRLIB_DEVICE_H
#define GRLIB_DEVICE_H

#include <stdint.h>

/// @addtogroup utils
/// @{

/// Device type
enum GrlibType {
  APBIO = 1,
  AHBMEM = 2,
  AHBIO = 3
};

/// This function returns a grlib bank address register.
/// It is needed to set the plug and play informations in each device model.
///
/// @return The bank address register content.
/// @see GrlibDevice
/// @see GrlibPnP
inline uint32_t GrlibBAR(GrlibType type, uint16_t mask, bool cacheable, bool prefetchable, uint16_t address) {
  return static_cast<uint8_t>(type) | 
         (mask << 4) | 
         (cacheable << 16) | 
         (prefetchable << 17) | 
         (address << 20);
}

/// This class is a base class for grlib models. It implements the device plug and play informations.
/// Together with the GrlibPnP class it implements the plug and play feature of the grlib.
/// @see GrlibPnP
class GrlibDevice {
  public:
    /// All device informations are needed while constructing a device.
    /// The register content is formed here.
    GrlibDevice(uint8_t vendorid, uint16_t deviceid, uint8_t version, uint8_t irq, 
                uint32_t bar0, uint32_t bar1 = 0, uint32_t bar2 = 0, uint32_t bar3 = 0) {
      m_register[0] = (irq & 0x1F) | ((version & 0x1F) << 5) | 
                      ((deviceid & 0xFFF) << 12) | (vendorid << 24);
      m_register[1] = m_register[2] = m_register[3] = 0;
      m_register[4] = bar0;
      m_register[5] = bar1;
      m_register[6] = bar2;
      m_register[7] = bar3;
    }

    /// Empty destructor
    virtual ~GrlibDevice() {}
    
    /// Returns the device register file.
    virtual const uint32_t *get_device_pointer() {
      return m_register;
    }
    
  private:
    /// Impementation of the device register file.
    uint32_t m_register[8];
};

/// @}

#endif
