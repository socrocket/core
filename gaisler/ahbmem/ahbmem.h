// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup ahbmem AHBMemory
/// @{
/// @file ahbmem.h
/// Provide a test bench memory class with AHB slave interface.
///
/// @date 2010-2015
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

#include "core/common/sr_param.h"


#include <map>

#include "core/common/ahbslave.h"
#include "core/common/clkdevice.h"
#include "gaisler/memory/basememory.h"
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
      bool pow_mon = false
    ) __attribute__ ((deprecated));

    AHBMem(
      const ModuleName nm,
      AbstractionLayer ambaLayer,
      uint16_t haddr_ = 0,
      uint16_t hmask_ = 0,
      uint32_t slave_id = 0,
      bool cacheable = 1,
      uint32_t wait_states = 0,
      bool pow_mon = false
    );

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

    bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data);
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

    /// Creates the memory backend
    void end_of_elaboration();

    /// Generates execution statistic at end of simulation
    void end_of_simulation();

    const char* get_name() const {
      return this->name();
    }

  private:
    /// Parent array for generics
    gs::cnf::gs_param_array g_conf;

    /// 12 bit MSB address (constructor parameters)
    sr_param<uint32_t> g_haddr;

    /// 12 bit MSB mask (constructor parameters)
    sr_param<uint32_t> g_hmask;

    /// AHB Slave Index (constructor parameters)
    //sr_param<uint32_t> g_hindex;

    /// Device cacheable or not
    sr_param<bool> g_cacheable;

    /// Number of wait states to be inserted for each transfer
    sr_param<uint32_t> g_wait_states;

    /// Power monitoring on/off
    sr_param<bool> g_pow_mon;

    /// Stores the type of memory used
    sr_param<std::string> g_storage_type;

  public:
    /// Power Modeling Parameters

    /// Normalized static power input
    sr_param<double> sta_power_norm;

    /// Normalized internal power input (activation independent)
    sr_param<double> int_power_norm;

    /// Normalized read access energy
    sr_param<double> dyn_read_energy_norm;

    /// Normalized write access energy
    sr_param<double> dyn_write_energy_norm;

    /// Parameter array for power data output
    gs::gs_param_array power;

    /// Static power of module
    sr_param<double> sta_power;

    /// Dynamic power of module (activation independent)
    sr_param<double> int_power;

    /// Switching power of module
    sr_param<double> swi_power;

    /// Power frame starting time
    sr_param<sc_core::sc_time> power_frame_starting_time;

    /// Dynamic energy per read access
    sr_param<double> dyn_read_energy;

    /// Dynamic energy per write access
    sr_param<double> dyn_write_energy;

    /// Number of reads from memory (read & reset by monitor)
    sr_param<uint64_t> dyn_reads;  // NOLINT(runtime/int)

    /// Number of writes to memory (read & reset by monitor)
    sr_param<uint64_t> dyn_writes;  // NOLINT(runtime/int)
};

#endif  // MODELS_AHBMEM_AHBMEM_H_
/// @}
