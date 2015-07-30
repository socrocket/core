// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file memdevice.h
/// header file containing the definition of a baseclass for all memory tlm
/// models. It implements the the device information register needed for the
/// plug and play interface.
///
/// @date 2010-2015
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
#include "core/common/sr_param.h"
#include "core/common/base.h"

/// @brief This class is a base class for memory models. It implements the device plug and play informations.
/// 
/// @details The class MEMDevice is the base class of all memories to be connected to the MCTRL. The library 
/// provides a Generic Memory, which implements the given interface. The included functions are required to 
/// determine the features of the attached component for correct access and delay calculation.
///
/// @see mctrl
class MEMDevice : public BaseModule<DefaultBase> {
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
    MEMDevice(sc_module_name name, device_type type, uint32_t banks = 0, uint32_t bsize = 0, uint32_t bits = 32, uint32_t cols = 0);

    /// Empty destructor
    virtual ~MEMDevice();

    /// Initialize MEMDevice generics
    virtual void init_mem_generics();

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

    /// Returns the size of the whole memory
    virtual const uint32_t get_size() const;

  private:
    sr_param<uint32_t> g_type;
    sr_param<uint32_t> g_banks;
    sr_param<uint32_t> g_bsize;
    sr_param<uint32_t> g_bits;
    sr_param<uint32_t> g_cols;
};

#endif  // MODELS_UTILS_MEMDEVICE_H_
/// @}
