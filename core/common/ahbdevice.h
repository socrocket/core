// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file ahbdevice.h
///
///
/// @date 2013-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
/// @author Thomas Schuster
///

#ifndef MODELS_UTILS_AHBDEVICE_H_
#define MODELS_UTILS_AHBDEVICE_H_

#include "core/common/ahbdevicebase.h"
#include "core/common/sr_param.h"

/// @brief This class is a base class for grlib models. It implements the device plug and play informations.
/// Together with the AHBCtrl class it implements the plug and play feature of the grlib.
///
/// All simulation models that are supposed to be connected to the TLM AHBCTRL must be derived from 
/// the class AHBDevice. Usually, this is indirectly done by inheriting from the AHBMaster or AHBSlave 
/// classes. The Aeroflex Gaisler AHBCTRL implements a Plug & Play mechanism, which 
/// relies on configuration information that is collected from the attached masters and slaves. 
/// AHBDevice models the respective configuration data records. The structure of these records is 
/// described in [GRLIB IP Core User’s Manual](http://gaisler.com/products/grlib/grip.pdf). At 
/// start_of_simulation the TLM AHBCTRL iterates through all connected modules 
/// to retrieve AHB bar & mask and build up its internal routing table.
///
/// @see AHBCtrl
///
template <class BASE = BaseModule<DefaultBase> >
class AHBDevice : public BASE, public AHBDeviceBase {
  public:
    /// All device informations are needed while constructing a device.
    /// The register content is formed here.
    AHBDevice(
        ModuleName mn,
        uint32_t hindex,
        uint8_t vendorid,
        uint16_t deviceid,
        uint8_t version,
        uint8_t irq,
        BAR bar0,
        BAR bar1 = BAR(),
        BAR bar2 = BAR(),
        BAR bar3 = BAR());

    /// All device informations are needed while constructing a device.
    /// The register content is formed here.
    /// Before ending the constructor of the subclass you have to call init!
    AHBDevice(ModuleName mn);

    /// All device informations missing by the second constructor.
    /// The register content is formed here.
    void init(
        uint32_t hindex,
        uint8_t vendorid,
        uint16_t deviceid,
        uint8_t version,
        uint8_t irq,
        BAR bar0,
        BAR bar1 = BAR(),
        BAR bar2 = BAR(),
        BAR bar3 = BAR());


    void init_ahb_generics();

    /// Empty destructor
    virtual ~AHBDevice();

    /// Returns the device id.
    virtual const uint16_t get_ahb_device_id() const throw();

    /// Returns the vendor id.
    virtual const uint8_t get_ahb_vendor_id() const throw();

    /// Returns the device register file.
    /// A set of 8 registers as specified by the grlib manual.
    /// See section: 4.2.3 (Page 50)
    virtual const uint32_t*get_ahb_device_info() throw();

    /// Returns the Bus specific base address of the device.
    /// Legacy for AMBAKit
    /// Please use get_bar_address instead. It will work with gaps between slave areas.
    /// @see get_bar_addr
    /// @return The device base address.
    virtual const uint32_t get_ahb_base_addr_() const throw();
    virtual sc_dt::uint64 get_ahb_base_addr() throw();

    /// Returns the size of the hole device as seen from the bus.
    /// Legacy for AMBAKit
    /// Please use get_bar_size instead. It will work with gaps between the slave areas.
    /// @see get_bar_size
    /// @return The device size.
    virtual const uint32_t get_ahb_size_() const throw();
    virtual sc_dt::uint64 get_ahb_size() throw();

    /// Returns the type of the bar.
    /// @param bar The selected bar
    /// @see AMBADeviceType
    virtual const AMBADeviceType get_ahb_bar_type(uint32_t bar) const throw();

    /// Returns the Bus specific most significant 12bit of the bar base address
    /// Shifted to the lowest bits in the word.
    virtual const uint32_t get_ahb_bar_base(uint32_t bar) const throw();

    /// Returns the Bus specific mask of the most significant 12bit of the bar address
    /// Shifted to the lowest bits in the word.
    virtual const uint32_t get_ahb_bar_mask(uint32_t bar) const throw();

    /// Returns the Bus specific base address of the device.
    /// Returns the address of one bar in byte offset as seen from the bus.
    /// @param bar The selected bar
    virtual const uint32_t get_ahb_bar_addr(uint32_t bar) const throw();

    /// Returns the size of one bar in bytes as seen from the bus.
    /// @param bar The selected bar
    virtual const uint32_t get_ahb_bar_size(uint32_t bar) const throw();

    /// Returns the BAR relative address
    /// @param bar The selected bar
    virtual const uint32_t get_ahb_bar_relative_addr(uint32_t bar, uint32_t addr) const throw();

    /// Returns whether a bar is prefetchable
    /// @param bar The selected bar
    inline const bool get_ahb_bar_prefetchable(uint32_t bar) const throw() {
      // Look if bit is set in the coresponding bar register
      return m_register[4 + bar] & (1 << 17);
    }

    /// Returns whether a bar is cachable
    /// @param bar The selected bar
    inline const bool get_ahb_bar_cachable(uint32_t bar) const throw() {
      // Look if bit is set in the coresponding bar register
      return m_register[4 + bar] & (1 << 16);
    }

    /// Returns the bus id of the module (hindex)
    virtual const uint32_t get_ahb_hindex() const throw();

    /// Prints the device info of the device.
    virtual void print_ahb_device_info(char *name) const;

    /// Collect common transport statistics.
    virtual void transport_statistics(tlm::tlm_generic_payload &gp) throw();  // NOLINT(runtime/references)

  protected:
    /// Impementation of the device register file.
    uint32_t m_register[8];

    ParameterArray g_bars;
    ParameterArray g_bar0;
    ParameterArray g_bar1;
    ParameterArray g_bar2;
    ParameterArray g_bar3;
    sr_param<uint32_t> g_hindex;
    sr_param<uint8_t> g_hvendorid;
    sr_param<uint16_t> g_hdeviceid;
    sr_param<uint8_t> g_hversion;
    sr_param<uint8_t> g_hirq;
    sr_param<uint32_t> g_bar0haddr;
    sr_param<uint32_t> g_bar0hmask;
    sr_param<uint32_t> g_bar0htype;
    sr_param<bool> g_bar0hcacheable;
    sr_param<bool> g_bar0hprefetchable;
    sr_param<uint32_t> g_bar1haddr;
    sr_param<uint32_t> g_bar1hmask;
    sr_param<uint32_t> g_bar1htype;
    sr_param<bool> g_bar1hcacheable;
    sr_param<bool> g_bar1hprefetchable;
    sr_param<uint32_t> g_bar2haddr;
    sr_param<uint32_t> g_bar2hmask;
    sr_param<uint32_t> g_bar2htype;
    sr_param<bool> g_bar2hcacheable;
    sr_param<bool> g_bar2hprefetchable;
    sr_param<uint32_t> g_bar3haddr;
    sr_param<uint32_t> g_bar3hmask;
    sr_param<uint32_t> g_bar3htype;
    sr_param<bool> g_bar3hcacheable;
    sr_param<bool> g_bar3hprefetchable;
};

#include "core/common/ahbdevice.tpp"

#endif  // MODELS_UTILS_AHBDEVICE_H_
/// @}
