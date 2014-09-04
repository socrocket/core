// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file memdevice.h
/// header file containing the definition of a baseclass for all memory tlm
/// models. It implements the the device information register needed for the
/// plug and play interface.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef MODELS_UTILS_MEMDEVICE_H_
#define MODELS_UTILS_MEMDEVICE_H_

#include <stdint.h>
#include <string>

/// This class is a base class for memory models. It implements the device plug and play informations.
/// @see mctrl
class MEMDevice {
  public:
    /// Device type
    enum device_type {
      ROM   = 0,
      IO    = 1,
      SRAM  = 2,
      SDRAM = 3
    };
    typedef device_type type;

    /// All device informations are needed while constructing a device.
    /// The register content is formed here.
    MEMDevice(device_type type, uint32_t banks = 0, uint32_t bsize = 0, uint32_t bits = 32, uint32_t cols = 0);

    MEMDevice();
    /// Empty destructor
    virtual ~MEMDevice();

    /// Returns the memory configuration.
    virtual const char*get_device_info() const;

    /// Returns the device type.
    virtual const device_type get_type() const;

    /// Return the device type name
    virtual const std::string get_type_name() const;

    /// Returns the number of banks of the memory (sram or sdram bank if needed)
    virtual const uint32_t get_banks() const;

    /// Returns the bank size of the memory bank.
    /// All memory banks of a type should have the same size.
    virtual const uint32_t get_bsize() const;

    /// Returns the bit width of a memory word.
    /// 8, 16, 32 or 64 bits are allowed.
    virtual const uint32_t get_bits() const;

    /// Returns the column size of the memory (sdram)
    virtual const uint32_t get_cols() const;

  private:
    device_type m_type;
    uint32_t m_banks;
    uint32_t m_bsize;
    uint32_t m_bits;
    uint32_t m_cols;

    // Memory type name (rom, io, sram, sdram)
    std::string m_type_name;
};

#endif  // MODELS_UTILS_MEMDEVICE_H_
/// @}
