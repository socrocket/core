// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file ahbdevice.h
///
///
/// @date 2013-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author
///

#ifndef MODELS_UTILS_AHBDEVICE_H_
#define MODELS_UTILS_AHBDEVICE_H_

#include <amba.h>
#include <stdint.h>
#include <tlm.h>
#include "common/base.h"
#include "common/amba.h"
#include "common/gs_config.h"

class AHBDeviceBase : public amba_slave_base {
  public:
    /// Empty destructor
    virtual ~AHBDeviceBase() {}

    /// Returns the device id.
    virtual const uint16_t get_device_id() const throw() = 0;

    /// Returns the vendor id.
    virtual const uint8_t get_vendor_id() const throw() = 0;

    /// Returns the device register file.
    /// A set of 8 registers as specified by the grlib manual.
    /// See section: 4.2.3 (Page 50)
    virtual const uint32_t *get_device_info() const throw() = 0;

    /// Returns the Bus specific base address of the device.
    /// Legacy for AMBAKit
    /// Please use get_bar_address instead. It will work with gaps between slave areas.
    /// @see get_bar_addr
    /// @return The device base address.
    virtual const uint32_t get_base_addr_() const throw() = 0;
    virtual sc_dt::uint64 get_base_addr() throw() = 0;

    /// Returns the size of the hole device as seen from the bus.
    /// Legacy for AMBAKit
    /// Please use get_bar_size instead. It will work with gaps between the slave areas.
    /// @see get_bar_size
    /// @return The device size.
    virtual const uint32_t get_size_() const throw() = 0;
    virtual sc_dt::uint64 get_size() throw() = 0;

    /// Returns the type of the bar.
    /// @param bar The selected bar
    /// @see AMBADeviceType
    virtual const AMBADeviceType get_bar_type(uint32_t bar) const throw() = 0;

    /// Returns the Bus specific most significant 12bit of the bar base address
    /// Shifted to the lowest bits in the word.
    virtual const uint32_t get_bar_base(uint32_t bar) const throw() = 0;

    /// Returns the Bus specific mask of the most significant 12bit of the bar address
    /// Shifted to the lowest bits in the word.
    virtual const uint32_t get_bar_mask(uint32_t bar) const throw() = 0;

    /// Returns the Bus specific base address of the device.
    /// Returns the address of one bar in byte offset as seen from the bus.
    /// @param bar The selected bar
    virtual const uint32_t get_bar_addr(uint32_t bar) const throw() = 0;

    /// Returns the size of one bar in bytes as seen from the bus.
    /// @param bar The selected bar
    virtual const uint32_t get_bar_size(uint32_t bar) const throw() = 0;

    /// Returns the BAR relative address
    /// @param bar The selected bar
    virtual const uint32_t get_bar_relative_addr(uint32_t bar, uint32_t addr) const throw() = 0;

    /// Returns the bus id of the module (hindex)
    virtual const uint32_t get_busid() const throw() = 0;

    /// Prints the device info of the device.
    virtual void print_device_info(char *name) const = 0;

    /// Collect common transport statistics.
    virtual void transport_statistics(tlm::tlm_generic_payload &gp) throw() = 0;  // NOLINT(runtime/references)

    /// Print common transport statistics.
    virtual void print_transport_statistics(const char *name) const throw() = 0;
};

/// This class is a base class for grlib models. It implements the device plug and play informations.
/// Together with the AHBCtrl class it implements the plug and play feature of the grlib.
/// @see AHBCtrl
template<class BASE = DefaultBase>
class AHBDevice : public BaseModule<BASE>, public AHBDeviceBase {
  public:
    /// All device informations are needed while constructing a device.
    /// The register content is formed here.
    AHBDevice(
        ModuleName mn,
        uint32_t busid,
        uint8_t vendorid,
        uint16_t deviceid,
        uint8_t version,
        uint8_t irq,
        uint32_t bar0,
        uint32_t bar1 = 0,
        uint32_t bar2 = 0,
        uint32_t bar3 = 0,
        uint32_t register_count = 0);

    /// All device informations are needed while constructing a device.
    /// The register content is formed here.
    /// Before ending the constructor of the subclass you have to call init!
    AHBDevice(
        ModuleName mn,
        uint32_t register_count = 0);

    /// All device informations missing by the second constructor.
    /// The register content is formed here.
    void init(
        uint32_t busid,
        uint8_t vendorid,
        uint16_t deviceid,
        uint8_t version,
        uint8_t irq,
        uint32_t bar0,
        uint32_t bar1 = 0,
        uint32_t bar2 = 0,
        uint32_t bar3 = 0);

    /// Empty destructor
    virtual ~AHBDevice();

    /// Returns the device id.
    virtual const uint16_t get_device_id() const throw();

    /// Returns the vendor id.
    virtual const uint8_t get_vendor_id() const throw();

    /// Returns the device register file.
    /// A set of 8 registers as specified by the grlib manual.
    /// See section: 4.2.3 (Page 50)
    virtual const uint32_t*get_device_info() const throw();

    /// Returns the Bus specific base address of the device.
    /// Legacy for AMBAKit
    /// Please use get_bar_address instead. It will work with gaps between slave areas.
    /// @see get_bar_addr
    /// @return The device base address.
    virtual const uint32_t get_base_addr_() const throw();
    virtual sc_dt::uint64 get_base_addr() throw();

    /// Returns the size of the hole device as seen from the bus.
    /// Legacy for AMBAKit
    /// Please use get_bar_size instead. It will work with gaps between the slave areas.
    /// @see get_bar_size
    /// @return The device size.
    virtual const uint32_t get_size_() const throw();
    virtual sc_dt::uint64 get_size() throw();

    /// Returns the type of the bar.
    /// @param bar The selected bar
    /// @see AMBADeviceType
    virtual const AMBADeviceType get_bar_type(uint32_t bar) const throw();

    /// Returns the Bus specific most significant 12bit of the bar base address
    /// Shifted to the lowest bits in the word.
    virtual const uint32_t get_bar_base(uint32_t bar) const throw();

    /// Returns the Bus specific mask of the most significant 12bit of the bar address
    /// Shifted to the lowest bits in the word.
    virtual const uint32_t get_bar_mask(uint32_t bar) const throw();

    /// Returns the Bus specific base address of the device.
    /// Returns the address of one bar in byte offset as seen from the bus.
    /// @param bar The selected bar
    virtual const uint32_t get_bar_addr(uint32_t bar) const throw();

    /// Returns the size of one bar in bytes as seen from the bus.
    /// @param bar The selected bar
    virtual const uint32_t get_bar_size(uint32_t bar) const throw();

    /// Returns the BAR relative address
    /// @param bar The selected bar
    virtual const uint32_t get_bar_relative_addr(uint32_t bar, uint32_t addr) const throw();

    /// Returns whether a bar is prefetchable
    /// @param bar The selected bar
    inline const bool get_bar_prefetchable(uint32_t bar) const throw() {
      // Look if bit is set in the coresponding bar register
      return m_register[4 + bar] & (1 << 17);
    }

    /// Returns whether a bar is cachable
    /// @param bar The selected bar
    inline const bool get_bar_cachable(uint32_t bar) const throw() {
      // Look if bit is set in the coresponding bar register
      return m_register[4 + bar] & (1 << 16);
    }

    /// Returns the bus id of the module (hindex)
    virtual const uint32_t get_busid() const throw();

    /// Prints the device info of the device.
    virtual void print_device_info(char *name) const;

    /// Collect common transport statistics.
    virtual void transport_statistics(tlm::tlm_generic_payload &gp) throw();  // NOLINT(runtime/references)

    /// Print common transport statistics.
    virtual void print_transport_statistics(const char *name) const throw();

  private:
    /// Impementation of the device register file.
    uint32_t m_register[8];

    /// The master of slave bus id of the device
    uint32_t m_busid;
};

#include "ahbdevice.tpp"

#endif  // MODELS_UTILS_AHBDEVICE_H_
/// @}
