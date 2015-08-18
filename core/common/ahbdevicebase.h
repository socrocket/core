// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file ahbdevicebase.h
///
///
/// @date 2013-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef MODELS_UTILS_AHBDEVICEBASE_H_
#define MODELS_UTILS_AHBDEVICEBASE_H_

#include "core/common/amba.h"
#include <stdint.h>
#include <tlm.h>
#include "core/common/base.h"

class AHBDeviceBase {
  public:
    /// Empty destructor
    virtual ~AHBDeviceBase() {}

    /// Returns the device id.
    virtual const uint16_t get_ahb_device_id() const throw() = 0;

    /// Returns the vendor id.
    virtual const uint8_t get_ahb_vendor_id() const throw() = 0;

    /// Returns the device register file.
    /// A set of 8 registers as specified by the grlib manual.
    /// See section: 4.2.3 (Page 50)
    virtual const uint32_t *get_ahb_device_info() throw() = 0;

    /// Returns the Bus specific base address of the device.
    /// Legacy for AMBAKit
    /// Please use get_bar_address instead. It will work with gaps between slave areas.
    /// @see get_bar_addr
    /// @return The device base address.
    virtual const uint32_t get_ahb_base_addr_() const throw() = 0;
#ifndef SWIG
    virtual sc_dt::uint64 get_ahb_base_addr() throw() = 0;
#endif

    /// Returns the size of the hole device as seen from the bus.
    /// Legacy for AMBAKit
    /// Please use get_bar_size instead. It will work with gaps between the slave areas.
    /// @see get_bar_size
    /// @return The device size.
    virtual const uint32_t get_ahb_size_() const throw() = 0;
#ifndef SWIG
    virtual sc_dt::uint64 get_ahb_size() throw() = 0;
#endif

    /// Returns the type of the bar.
    /// @param bar The selected bar
    /// @see AMBADeviceType
    virtual const AMBADeviceType get_ahb_bar_type(uint32_t bar) const throw() = 0;

    /// Returns the Bus specific most significant 12bit of the bar base address
    /// Shifted to the lowest bits in the word.
    virtual const uint32_t get_ahb_bar_base(uint32_t bar) const throw() = 0;

    /// Returns the Bus specific mask of the most significant 12bit of the bar address
    /// Shifted to the lowest bits in the word.
    virtual const uint32_t get_ahb_bar_mask(uint32_t bar) const throw() = 0;

    /// Returns the Bus specific base address of the device.
    /// Returns the address of one bar in byte offset as seen from the bus.
    /// @param bar The selected bar
    virtual const uint32_t get_ahb_bar_addr(uint32_t bar) const throw() = 0;

    /// Returns the size of one bar in bytes as seen from the bus.
    /// @param bar The selected bar
    virtual const uint32_t get_ahb_bar_size(uint32_t bar) const throw() = 0;

    /// Returns the BAR relative address
    /// @param bar The selected bar
    virtual const uint32_t get_ahb_bar_relative_addr(uint32_t bar, uint32_t addr) const throw() = 0;

    /// Returns the bus id of the module (hindex)
    virtual const uint32_t get_ahb_hindex() const throw() = 0;

    /// Prints the device info of the device.
    virtual void print_ahb_device_info(char *name) const = 0;

    /// Collect common transport statistics.
    virtual void transport_statistics(tlm::tlm_generic_payload &gp) throw() = 0;  // NOLINT(runtime/references)
};


#endif  // MODELS_UTILS_AHBDEVICEBASE_H
/// @}
