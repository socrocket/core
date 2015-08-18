// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common Common
/// @{
/// @file amba.h
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
#ifndef COMMON_AMBA_H_
#define COMMON_AMBA_H_

#include <amba.h>
#include "core/common/systemc.h"
#include "core/common/base.h"
#include "core/common/sr_report.h"

typedef amba::amba_layer_ids AbstractionLayer;

/// Device type
/// See grlib manual for more information
/// Section 4.2.3
enum AMBADeviceType {
  NONE = 0,         ///< Bar is not existing
  APBIO = 1,        ///< Bar is an APB Device
  AHBMEM = 2,       ///< Bar is absolute addressed to 0 at AHB Bus
  AHBIO = 3         ///< Bar is relative to AHBIO region
};

class BAR {
  public:
    BAR(
        AMBADeviceType type = NONE,
        uint16_t mask = 0x000,
        bool cacheable = false,
        bool prefetchable = false,
        uint16_t address = 0x000) throw() :
      type(type),
      mask(mask),
      cacheable(cacheable),
      prefetchable(prefetchable),
      address(address) {}

    /// This function returns a grlib bank address register.
    /// It is needed to set the plug and play informations in each device model.
    ///
    /// @return The bank address register content.
    /// @see AHBDevice
    /// @see AHBCtrl
    uint32_t toRegister() throw() {
      return static_cast<uint8_t>(type) | (mask << 4) | (cacheable << 16)
             | (prefetchable << 17) | (address << 20);
    }

    AMBADeviceType type;
    uint16_t mask;
    bool cacheable;
    bool prefetchable;
    uint16_t address;
};

#endif  // COMMON_AMBA_H_
/// @}
