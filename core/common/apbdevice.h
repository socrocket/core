// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file apbdevice.h
/// header file containing the definition of a baseclass for all ahb tlm models.
/// It implements the the device information register needed for the plug and
/// play interface.
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef MODELS_UTILS_APBDEVICE_H_
#define MODELS_UTILS_APBDEVICE_H_

#include "core/common/apbdevicebase.h"

/// @brief This class is a base class for grlib models. It implements the device plug and play informations.
/// Together with the APBBridge class it implements the plug and play feature of the grlib.
/// 
/// @details All simulation models that are supposed to be connected to the TLM APBCTRL must be derived from 
/// class APBDevice. Similar to the concept of AHBDevice, the child inherits Plug & Play configuration records 
/// representing its device type and address. At start_of_simulation the APBCTRL iterates through the 
/// connected slaves collecting all APB bar and mask settings for building up its routing table.
///
/// Modules, like the MCTRL, which posses an AHB as well as an APB interface must be derived from AHBDevice and APBDevice.
///
/// @see APBCtrl
template<class BASE = BaseModule<DefaultBase> >
class APBDevice : public BASE , public APBDeviceBase {
  public:
    /// All device informations are needed while constructing a device.
    /// The register content is formed here.
    APBDevice(ModuleName mn, uint32_t bus_id, uint8_t vendorid, uint16_t deviceid, uint8_t version,
      uint8_t irq, AMBADeviceType type, uint16_t mask,
      bool cacheable, bool prefetchable, uint16_t address);

    APBDevice(ModuleName mn);

    void init_apb(uint32_t pindex, uint8_t vendorid, uint16_t deviceid, uint8_t version,
      uint8_t irq, AMBADeviceType type, uint16_t mask,
      bool cacheable, bool prefetchable, uint16_t address);

    /// Initialize the APBDevice generics
    void init_apb_generics();

    /// Empty destructor
    virtual ~APBDevice();

    /// Returns the device id.
    virtual uint16_t get_apb_device_id() const;

    /// Returns the vendor id.
    virtual uint8_t get_apb_vendor_id() const;

    /// Returns the device register file.
    /// A set of 8 registers as specified by the grlib manual.
    /// See section: 14.2.2 (Page 79)
    virtual const uint32_t *get_apb_device_info();

    /// Returns the device type.
    /// Should be APBIO ;-)
    virtual const AMBADeviceType get_apb_type() const;

    /// Returns the Bus specific most significant 12bit of the base address
    /// Shifted to the lowest bits in the word.
    virtual uint32_t get_apb_base() const;

    /// Returns the Bus specific mask of the most significant 12bit of the address
    /// Shifted to the lowest bits in the word.
    virtual uint32_t get_apb_mask() const;

    /// Returns the Bus specific base address of the device.
    /// @see get_bar_addr
    /// @return The device base address.
    virtual sc_dt::uint64 get_apb_base_addr();
    virtual uint32_t get_apb_base_addr_() const;

    /// Returns the size of the hole device as seen from the bus.
    /// @see get_bar_size
    /// @return The device size.
    virtual sc_dt::uint64 get_apb_size();
    virtual uint32_t get_apb_size_() const;

    virtual uint32_t get_apb_relative_addr(uint32_t addr) const;

    /// Returns the bus id of the module (pindex)
    virtual uint32_t get_apb_pindex() const;

    /// Prints the device info of the device.
    virtual void print_apb_device_info(char *name) const;

  protected:
    /// Impementation of the device register file.
    uint32_t m_register[2];

    sr_param<uint32_t> g_pindex;
    sr_param<uint8_t> g_pvendorid;
    sr_param<uint16_t> g_pdeviceid;
    sr_param<uint8_t> g_pversion;
    sr_param<uint8_t> g_pirq;
    sr_param<uint32_t> g_paddr;
    sr_param<uint32_t> g_pmask;
    sr_param<uint32_t> g_ptype;
    sr_param<bool> g_pcacheable;
    sr_param<bool> g_pprefetchable;
};

#include "core/common/apbdevice.tpp"

#endif  // MODELS_UTILS_APBDEVICE_H_
/// @}
