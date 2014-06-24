#ifndef MEMORYPOWER_H
#define MEMORYPOWER_H


#include <greencontrol/config.h>
#include <greensocket/target/single_socket.h>
#include <tlm.h>
#include <systemc.h>

#include "verbose.h"
#include "memdevice.h"
#include "clkdevice.h"
#include "basememory.h"

//template<class BASE = sc_core::sc_module>
//class MemoryPower : public BASE {
class MemoryPower : public sc_core::sc_module, public CLKDevice, public MEMDevice, public BaseMemory {
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
    gs::cnf::callback_return_type swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);
    
    /// 32-bit-reads counter callback
    gs::cnf::callback_return_type dyn_reads_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

    /// 32-bit-writes counter callback
    gs::cnf::callback_return_type dyn_writes_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

    /// Power monitoring
    bool m_pow_mon;
   
    /// GreenControl API Pointer
    gs::cnf::cnf_api *m_api;
   
    /// Performance Counter Array
    gs::gs_param_array m_performance_counters;
   
    /// Performance counter to store transaction byte reads
    //gs::gs_param<unsigned long long> m_reads;
   
    /// Performance counter to store the transaction byte writes
    //gs::gs_param<unsigned long long> m_writes;
   
    /// *****************************************************
    /// Power Modeling Parameters
   
    /// Parameter array for power data output
    gs::gs_param_array power;
   
    /// Normalized static power input
    gs::gs_param<double> sta_power_norm;
   
    /// Normalized internal power input (activation independent)
    gs::gs_param<double> int_power_norm;
   
    /// Normalized read access energy
    gs::gs_param<double> dyn_read_energy_norm;
   
    /// Normalized write access energy
    gs::gs_param<double> dyn_write_energy_norm;
   
    /// Static power of module
    gs::gs_param<double> sta_power;
   
    /// Internal power of module
    gs::gs_param<double> int_power;
   
    /// Switching power of module
    gs::gs_param<double> swi_power;
   
    /// Power frame starting time
    gs::gs_param<sc_core::sc_time> power_frame_starting_time;
   
    /// Dynamic energy per read access
    gs::gs_param<double> dyn_read_energy;
   
    /// Dynamic energy per write access
    gs::gs_param<double> dyn_write_energy;
   
    /// Number of reads from memory (read & reset by monitor)
    gs::gs_param<unsigned long long> dyn_reads;
   
    /// Number of writes to memory (read & reset by monitor)
    gs::gs_param<unsigned long long> dyn_writes;
    

  private:
    
};

#endif

