// *****************************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is 
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
// *****************************************************************************
// Title:      ahbmem.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Provide a test bench memory class with AHB slave interface.
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
// *****************************************************************************

#ifndef AHBMEM_H
#define AHBMEM_H

#include <tlm.h>
#include <amba.h>
#include <map>

#include "ahbslave.h"
#include "clkdevice.h"
#include "msclogger.h"

#include <greencontrol/config.h>

#if defined(MTI_SYSTEMC)
#include "peq_with_get.h"
#else
#include "tlm_utils/peq_with_get.h"
#endif

class AHBMem : public AHBSlave<>, public CLKDevice {

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
  AHBMem(const sc_core::sc_module_name nm, 
         uint16_t haddr_, 
         uint16_t hmask_ = 0, 
         amba::amba_layer_ids ambaLayer = amba::amba_LT, 
         uint32_t slave_id = 0,
         bool cacheable = 1,
         uint32_t wait_states = 0,
         bool pow_mon = false);

  /// Destructor
  ~AHBMem();

  /// Reset callback
  void dorst();

  /// @brief Delete memory content
  void clear_mem() {
    mem.clear();
  }

  /// @brief Method to write a byte into the memory
  /// @param addr Write address
  /// @param byte Write data
  void writeByteDBG(const uint32_t addr, const uint8_t byte);

  uint32_t exec_func(tlm::tlm_generic_payload &gp, sc_time &delay, bool debug = false);

  sc_core::sc_time get_clock();

  /// Called by scheduler at start of simulation
  void start_of_simulation();

  /// Calculate power/energy values from normalized input data
  void power_model();

  /// Static power callback
  void sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Dynamic/Internal power callback
  void int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Dynamic/Switching power callback
  void swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);

  /// Generates execution statistic at end of simulation
  void end_of_simulation();

 private:

  /// The actual memory
  std::map<uint32_t, uint8_t> mem;
        
  /// AHB slave base address and size
  const uint32_t ahbBaseAddress;
  // size is saved in bytes
  const uint32_t ahbSize;
  
  /// 12 bit MSB address and mask (constructor parameters)
  const uint32_t mhaddr;
  const uint32_t mhmask;
  
  /// Device cacheable or not
  const bool mcacheable;

  /// Number of wait states to be inserted for each transfer
  const uint32_t mwait_states;

  /// Power monitoring on/off
  const bool m_pow_mon;

 public:

  /// *****************************************************
  /// Power Modeling Parameters

  /// Normalized static power input
  gs::gs_param<double> sta_power_norm;

  /// Normalized internal power input (activation independent)
  gs::gs_param<double> int_power_norm;

  /// Normalized read access energy
  gs::gs_param<double> dyn_read_energy_norm;

  /// Normalized write access energy
  gs::gs_param<double> dyn_write_energy_norm;

  /// Parameter array for power data output
  gs::gs_param_array power;

  /// Static power of module
  gs::gs_param<double> sta_power;

  /// Dynamic power of module (activation independent)
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

};

#endif
