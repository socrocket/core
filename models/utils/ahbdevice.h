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
#include "common/gs_config.h"
#include <stdint.h>
#include <tlm.h>

/// This class is a base class for grlib models. It implements the device plug and play informations.
/// Together with the AHBCtrl class it implements the plug and play feature of the grlib.
/// @see AHBCtrl
class AHBDevice : public amba_slave_base {
  public:
    /// Device type
    /// See grlib manual for more information
    /// Section 4.2.3
    enum device_type {
      NONE = 0,         ///< Bar is not existing
      APBIO = 1,        ///< Bar is an APB Device
      AHBMEM = 2,       ///< Bar is absolute addressed to 0 at AHB Bus
      AHBIO = 3         ///< Bar is relative to AHBIO region
    };

    /// All device informations are needed while constructing a device.
    /// The register content is formed here.
    AHBDevice(uint32_t bus_id, uint8_t vendorid, uint16_t deviceid, uint8_t version,
    uint8_t irq, uint32_t bar0, uint32_t bar1 = 0,
    uint32_t bar2 = 0, uint32_t bar3 = 0);

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
    /// @see device_type
    virtual const device_type get_bar_type(uint32_t bar) const throw();

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
    const uint32_t get_busid() const throw();

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

    /// GreenControl API container
    // gs::cnf::cnf_api *m_api;

    /// Open a namespace for performance counting in the greencontrol realm
    // gs::gs_param_array m_performance_counters;

    /// Stores the number of Bytes read from the device
    // gs::gs_config<uint64_t> m_reads;

    /// Stores the number of Bytes written from the device
    // gs::gs_config<uint64_t> m_writes;
};

/// This function returns a grlib bank address register.
/// It is needed to set the plug and play informations in each device model.
///
/// @return The bank address register content.
/// @see AHBDevice
/// @see AHBCtrl
uint32_t BAR(AHBDevice::device_type type, uint16_t mask, bool cacheable,
  bool prefetchable, uint16_t address) throw();

#endif  // MODELS_UTILS_AHBDEVICE_H_
/// @}
