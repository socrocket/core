// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup memory Memory
/// @{
/// @file memortpower.h
/// Memory power estimation functions functions. Based on information provided
/// by the statistical module of the BaseMemory implementation
///
/// @date 2014-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Jan Wagner, Thomas Schuster
///

#ifndef MODELS_MEMORY_MEMORYPOWER_H_
#define MODELS_MEMORY_MEMORYPOWER_H_

#include "core/common/sr_param.h"
#include <greensocket/target/single_socket.h>
#include "core/common/base.h"
#include "core/common/systemc.h"
#include <tlm.h>

#include "gaisler/memory/basememory.h"
#include "core/common/clkdevice.h"
#include "core/common/memdevice.h"
#include "core/common/verbose.h"

// template<class BASE = DefaultBase>
class MemoryPower : public MEMDevice, public CLKDevice, public BaseMemory {
  public:
    GC_HAS_CALLBACKS();

    MemoryPower(
    sc_module_name name,
    MEMDevice::device_type type = MEMDevice::SRAM,
    uint32_t banks = 4,
    uint32_t bsize = 128,
    uint32_t bits = 32,
    uint32_t cols = 16,
    bool pow_mon = false);

    ~MemoryPower();

    /// Calculate power/energy values from normalized input data
    void power_model();

    /// Dynamic/Switching power callback
    gs::cnf::callback_return_type swi_power_cb(gs::gs_param_base &changed_param, gs::cnf::callback_type reason);

    /// 32-bit-reads counter callback
    gs::cnf::callback_return_type dyn_reads_cb(gs::gs_param_base &changed_param, gs::cnf::callback_type reason);

    /// 32-bit-writes counter callback
    gs::cnf::callback_return_type dyn_writes_cb(gs::gs_param_base &changed_param, gs::cnf::callback_type reason);
    
    /// 32-bit-reads counter callback, write
    gs::cnf::callback_return_type dyn_reads_write_cb(gs::gs_param_base &changed_param, gs::cnf::callback_type reason);

    /// 32-bit-writes counter callback, write
    gs::cnf::callback_return_type dyn_writes_write_cb(gs::gs_param_base &changed_param, gs::cnf::callback_type reason);

    /// Power monitoring
    bool m_pow_mon;

    /// GreenControl API Pointer
    gs::cnf::cnf_api *m_api;

    /// Performance Counter Array
    gs::gs_param_array m_performance_counters;

    /// Performance counter to store transaction byte reads
    // sr_param<uint64_t> m_reads;

    /// Performance counter to store the transaction byte writes
    // sr_param<uint64_t> m_writes;

    /// *****************************************************
    /// Power Modeling Parameters

    /// Parameter array for power data output
    gs::gs_param_array power;

    /// Normalized static power input
    sr_param<double> sta_power_norm;

    /// Normalized internal power input (activation independent)
    sr_param<double> int_power_norm;

    /// Normalized read access energy
    sr_param<double> dyn_read_energy_norm;

    /// Normalized write access energy
    sr_param<double> dyn_write_energy_norm;

    /// Static power of module
    sr_param<double> sta_power;

    /// Internal power of module
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
    sr_param<uint64_t> dyn_reads;

    /// Number of writes to memory (read & reset by monitor)
    sr_param<uint64_t> dyn_writes;

  private:
    /// Offset for dyn_reads
    unsigned long long dyn_reads_offset;

    /// Offset for dyn_writes
    unsigned long long dyn_writes_offset;
};

#endif  // MODELS_MEMORY_MEMORYPOWER_H_

