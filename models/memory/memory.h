// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup memory
/// @{
/// @file mapmemory.h
/// Class definition of the generic memory model to be used with the SoCRocket
/// MCTRL. Can be configured as ROM, IO, SRAM or SDRAM. Underlying memory is
/// implemented as a flexible vmap. Recommended for simulation of large,
/// sparsely populated memories.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Dennis Bode
///

#ifndef MEMORY_H
#define MEMORY_H

//#include "vmap.h"
//#include "memdevice.h"
#include "clkdevice.h"
//#include "basememory.h"
#include "memorypower.h"

#include <greencontrol/config.h>
#include <greensocket/target/single_socket.h>
#include "ext_erase.h"
#include <tlm.h>
#include <systemc.h>

#include "verbose.h"

/// @brief This class models a generic memory. Depending on the configuration
/// it can be used as ROM, IO, SRAM or SDRAM, in conjunction with the SoCRocket MCTRL.
class Memory : public MemoryPower {

 public:

  GC_HAS_CALLBACKS();

  /// Slave socket -  for communication with Mctrl
  gs::socket::target_socket<32> bus;

  /// Creates a new Instance of Memory
  ///
  /// @param name The SystemC name of the component to be created
  /// @param type The type of memory to be modeled (0-ROM, 1-IO, 2-SRAM, 3-SDRAM)
  /// @param banks Number of parallel banks
  /// @param bsize Size of one memory bank in bytes (all banks always considered to have equal size)
  /// @param bits Bit width of memory
  /// @param cols Number of SDRAM cols.
  Memory(sc_module_name name,
            MEMDevice::device_type type,
            uint32_t banks,
            uint32_t bsize,
            uint32_t bits,
            uint32_t cols,
            BaseMemory::implementation_type implementation = BaseMemory::ARRAY,
            bool pow_mon = false);

  /// Destructor
  ~Memory();

  /// SystemC start of simulation callback
  void start_of_simulation();

  /// read counter callback
  gs::cnf::callback_return_type m_reads_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// write counter callback
  gs::cnf::callback_return_type m_writes_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// SystemC end of simulation
  void end_of_simulation();

  /// TLM 2.0 blocking transport function
  void b_transport(tlm::tlm_generic_payload& gp, sc_time& delay);

  /// TLM 2.0 debug transport function
  unsigned int transport_dbg(tlm::tlm_generic_payload& gp); 
  
  gs::gs_param<unsigned long long> m_writes;
  gs::gs_param<unsigned long long> m_reads;

};

#endif
/// @}
