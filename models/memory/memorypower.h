// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup memory Memory
/// @{
/// @file memortpower.h
/// Memory power estimation functions functions. Based on information provided
/// by the statistical module of the BaseMemory implementation
///
/// @date 2014-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Jan Wagner, Thomas Schuster
///

#ifndef MODELS_MEMORY_MEMORYPOWER_H_
#define MODELS_MEMORY_MEMORYPOWER_H_

#include "core/common/gs_config.h"
#include <greensocket/target/single_socket.h>
#include "core/common/base.h"
#include "core/common/systemc.h"
#include <tlm.h>

#include "core/models/memory/basememory.h"
#include "core/common/clkdevice.h"
#include "core/common/memdevice.h"
#include "core/common/verbose.h"

// template<class BASE = DefaultBase>
// class MemoryPower : public BASE {
class MemoryPower : public DefaultBase, public CLKDevice, public MEMDevice, public BaseMemory {
  public:
    GC_HAS_CALLBACKS();

    MemoryPower(
    sc_module_name name,
    MEMDevice::device_type type,
    uint32_t banks,
    uint32_t bsize,
    uint32_t bits,
    uint32_t cols,
    BaseMemory::implementation_type implementation = BaseMemory::ARRAY,
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
    // gs::gs_config<uint64_t> m_reads;

    /// Performance counter to store the transaction byte writes
    // gs::gs_config<uint64_t> m_writes;

    /// *****************************************************
    /// Power Modeling Parameters

    /// Parameter array for power data output
    gs::gs_param_array power;

    /// Normalized static power input
    gs::gs_config<double> sta_power_norm;

    /// Normalized internal power input (activation independent)
    gs::gs_config<double> int_power_norm;

    /// Normalized read access energy
    gs::gs_config<double> dyn_read_energy_norm;

    /// Normalized write access energy
    gs::gs_config<double> dyn_write_energy_norm;

    /// Static power of module
    gs::gs_config<double> sta_power;

    /// Internal power of module
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
    gs::gs_config<uint64_t> dyn_reads;

    /// Number of writes to memory (read & reset by monitor)
    gs::gs_config<uint64_t> dyn_writes;

  private:
    /// Offset for dyn_reads
    unsigned long long dyn_reads_offset;

    /// Offset for dyn_writes
    unsigned long long dyn_writes_offset;
};

#endif  // MODELS_MEMORY_MEMORYPOWER_H_

