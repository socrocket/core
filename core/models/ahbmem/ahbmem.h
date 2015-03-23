// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup ahbmem AHBMemory
/// @{
/// @file ahbmem.h
/// Provide a test bench memory class with AHB slave interface.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef MODELS_AHBMEM_AHBMEM_H_
#define MODELS_AHBMEM_AHBMEM_H_

#include "core/common/amba.h"
#include <tlm.h>

#if defined(MTI_SYSTEMC)
#include <peq_with_get.h>
#else
#include <tlm_utils/peq_with_get.h>
#endif

#include "core/common/gs_config.h"


#include <map>

#include "core/common/ahbslave.h"
#include "core/common/clkdevice.h"
#include "core/models/memory/basememory.h"
#include "core/common/msclogger.h"

class AHBMem : public AHBSlave<>, public CLKDevice, public BaseMemory{
  public:
    GC_HAS_CALLBACKS();
    SC_HAS_PROCESS(AHBMem);

    /// Constructor
    /// @brief Constructor for the test bench memory class
    /// @param haddr AHB address of the AHB slave socket (12 bit)
    /// @param hmask AHB address mask (12 bit)
    /// @param ambaLayer Abstraction layer used (AT/LT)
    /// @param infile File name of a text file to initialize the memory from
    /// @param addr Start address for memory initilization
    AHBMem(
      const ModuleName nm,
      uint16_t haddr_ = 0,
      uint16_t hmask_ = 0,
      AbstractionLayer ambaLayer = amba::amba_LT,
      uint32_t slave_id = 0,
      bool cacheable = 1,
      uint32_t wait_states = 0,
      bool pow_mon = false);

    /// Destructor
    ~AHBMem();

    /// Initialize generics
    void init_generics();

    /// Reset callback
    void dorst();

    /// @brief Delete memory content
    void clear_mem() {
      erase(0, get_ahb_size()-1);
    }

    /// @brief Method to write a byte into the memory
    /// @param addr Write address
    /// @param byte Write data
    void writeByteDBG(const uint32_t addr, const uint8_t byte);

    // AHBSlave behavior callback
    uint32_t exec_func(
        tlm::tlm_generic_payload &gp,  // NOLINT(runtime/references)
        sc_time &delay,                // NOLINT(runtime/references)
        bool debug = false);

    // Implement AHBSlave virtual function
    sc_core::sc_time get_clock();

    /// Called by scheduler at start of simulation
    void start_of_simulation();

    /// Calculate power/energy values from normalized input data
    void power_model();

    /// Static power callback
    gs::cnf::callback_return_type sta_power_cb(
        gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
        gs::cnf::callback_type reason);

    /// Dynamic/Internal power callback
    gs::cnf::callback_return_type int_power_cb(
        gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
        gs::cnf::callback_type reason);

    /// Dynamic/Switching power callback
    gs::cnf::callback_return_type swi_power_cb(
        gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
        gs::cnf::callback_type reason);

    /// Generates execution statistic at end of simulation
    void end_of_simulation();

  private:
    /// AHB slave base address and size
    const uint32_t ahbBaseAddress;
    // size is saved in bytes
    const uint32_t ahbSize;

    /// Parent array for generics
    gs::cnf::gs_param_array g_conf;

    /// 12 bit MSB address (constructor parameters)
    gs::cnf::gs_config<uint32_t> g_haddr;

    /// 12 bit MSB mask (constructor parameters)
    gs::cnf::gs_config<uint32_t> g_hmask;

    /// AHB Slave Index (constructor parameters)
    gs::cnf::gs_config<uint32_t> g_hindex;

    /// Device cacheable or not
    gs::cnf::gs_config<bool> g_cacheable;

    /// Number of wait states to be inserted for each transfer
    gs::cnf::gs_config<uint32_t> g_wait_states;

    /// Power monitoring on/off
    gs::cnf::gs_config<bool> g_pow_mon;

  public:
    /// Power Modeling Parameters

    /// Normalized static power input
    gs::gs_config<double> sta_power_norm;

    /// Normalized internal power input (activation independent)
    gs::gs_config<double> int_power_norm;

    /// Normalized read access energy
    gs::gs_config<double> dyn_read_energy_norm;

    /// Normalized write access energy
    gs::gs_config<double> dyn_write_energy_norm;

    /// Parameter array for power data output
    gs::gs_param_array power;

    /// Static power of module
    gs::gs_config<double> sta_power;

    /// Dynamic power of module (activation independent)
    gs::gs_config<double> int_power;

    /// Switching power of module
    gs::gs_config<double> swi_power;

    /// Power frame starting time
    gs::gs_config<sc_core::sc_time> power_frame_starting_time;

    /// Dynamic energy per read access
    gs::gs_config<double> dyn_read_energy;

    /// Dynamic energy per write access
    gs::gs_config<double> dyn_write_energy;

    /// Number of reads from memory (read & reset by monitor)
    gs::gs_config<uint64_t> dyn_reads;  // NOLINT(runtime/int)

    /// Number of writes to memory (read & reset by monitor)
    gs::gs_config<uint64_t> dyn_writes;  // NOLINT(runtime/int)
};

#endif  // MODELS_AHBMEM_AHBMEM_H_
/// @}
