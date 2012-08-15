//*********************************************************************
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
//*********************************************************************
// Title:      mapmemory.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implementation of the generic memory model to be used 
//             with the SoCRocket MCTRL. Can be configured as ROM, 
//             IO, SRAM or SDRAM. Underlying memory is implemented 
//             as a flexible vmap.
//             Recommended for simulation of large, sparsely
//             populated memories.
//
// Modified on $Date: 2011-05-09 20:31:53 +0200 (Mon, 09 May 2011) $
//          at $Revision: 416 $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Dennis Bode
// Reviewed:
//*********************************************************************

#include "mapmemory.h"

#include <tlm.h>

using namespace sc_core;
using namespace tlm;
using namespace std;

// Constructor implementation
MapMemory::MapMemory(sc_core::sc_module_name name, MEMDevice::device_type type, uint32_t banks, uint32_t bsize, uint32_t bits, uint32_t cols, bool powmon) :
  sc_module(name),
  MEMDevice(type, banks, bsize, bits, cols), 
  bus("bus"), 
  m_pow_mon(powmon), 
  m_performance_counters("performance_counters"),
  m_reads("bytes_read", 0ull, m_performance_counters), 
  m_writes("bytes_writen", 0ull, m_performance_counters),
  sta_power_norm("power." + get_type_name() + ".sta_power_norm", 1269.53125, true), // Normalized static power input 
  int_power_norm("power." + get_type_name() + ".int_power_norm", 1.61011e-12, true), // Normalized internal power input (act. independ)
  dyn_read_energy_norm("power." + get_type_name() + ".dyn_read_energy_norm", 7.57408e-13, true), // Normalized read energy input
  dyn_write_energy_norm("power.mapmemory.dyn_write_energy_norm", 7.57408e-13, true), // Normalized write energy input
  power("power"),
  sta_power("sta_power", 0.0, power),  // Static power output
  int_power("int_power", 0.0, power),  // Internal power output
  swi_power("swi_power", 0.0, power),  // Switching power output
  power_frame_starting_time("power_frame_starting_time", SC_ZERO_TIME, power),
  dyn_read_energy("dyn_read_energy", 0.0, power), // Energy per read access
  dyn_write_energy("dyn_write_energy", 0.0, power), // Energy per write access
  dyn_reads("dyn_reads", 0ull, power), // Read access counter for power computation
  dyn_writes("dyn_writes", 0ull, power) // Write access counter for power computation
  
 {
   // TLM 2.0 socket configuration
   gs::socket::config<tlm::tlm_base_protocol_types> bus_cfg;
   bus_cfg.use_mandatory_phase(BEGIN_REQ);
   bus_cfg.use_mandatory_phase(END_REQ);
   //mem_cfg.treat_unknown_as_ignorable();
   bus.set_config(bus_cfg);

   // Obtain pointer to GreenControl API
   m_api = gs::cnf::GCnf_Api::getApiInstance(this);
    
   // Register TLM 2.0 transport functions 
   bus.register_b_transport(this, &MapMemory::b_transport);
   bus.register_transport_dbg(this, &MapMemory::transport_dbg);

   // Register power callback functions
   if (m_pow_mon) {
    
     GC_REGISTER_TYPED_PARAM_CALLBACK(&sta_power, gs::cnf::pre_read, MapMemory, sta_power_cb);
     GC_REGISTER_TYPED_PARAM_CALLBACK(&int_power, gs::cnf::pre_read, MapMemory, int_power_cb);
     GC_REGISTER_TYPED_PARAM_CALLBACK(&swi_power, gs::cnf::pre_read, MapMemory, swi_power_cb);
    
     // Set norm power - depending on type
     if (get_type_name() == "sram") {
      
       m_api->setInitValue("power.sram.sta_power_norm", "1269.53125");
       m_api->setInitValue("power.sram.int_power_norm", "0.000161011");
       m_api->setInitValue("power.sram.dyn_read_energy_norm", "7.57408e-13");
       m_api->setInitValue("power.sram.dyn_write_energy_norm", "7.57408e-13");

     } else if (get_type_name() == "rom") {

       m_api->setInitValue("power.rom.sta_power_norm", "1269.53125");
       m_api->setInitValue("power.rom.int_power_norm", "0.000161011");
       m_api->setInitValue("power.rom.dyn_read_energy_norm", "7.57408e-13");
       m_api->setInitValue("power.rom.dyn_write_energy_norm", "7.57408e-13");

     } else if (get_type_name() == "io") {

       m_api->setInitValue("power.io.sta_power_norm", "1269.53125");
       m_api->setInitValue("power.io.int_power_norm", "0.000161011");
       m_api->setInitValue("power.io.dyn_read_energy_norm", "7.57408e-13");
       m_api->setInitValue("power.io.dyn_write_energy_norm", "7.57408e-13");

     } else {

       m_api->setInitValue("power.sdram.sta_power_norm", "2539.0625");
       m_api->setInitValue("power.sdram.int_power_norm", "0.000322022");
       m_api->setInitValue("power.sdram.dyn_read_energy_norm", "15e-13");
       m_api->setInitValue("power.sdram.dyn_write_energy_norm", "15e-13");

     }
   }

   // Module configuration report
   v::info << this->name() << " ******************************************************************************* " << v::endl;
   v::info << this->name() << " * Created MapMemory with following parameters: " << v::endl;
   v::info << this->name() << " * ------------------------------------------------ " << v::endl;
   v::info << this->name() << " * device_type (ROM, IO, SRAM, SDRAM): " << get_type_name() << v::endl;
   v::info << this->name() << " * banks: " << banks << v::endl;
   v::info << this->name() << " * bsize (bytes): " << hex << bsize << v::endl;
   v::info << this->name() << " * bit width: " << bits << v::endl;
   v::info << this->name() << " * cols (SD only): " << cols << v::endl;
   v::info << this->name() << " * pow_mon: " << powmon << v::endl;
   v::info << this->name() << " ******************************************************************************* " << v::endl;
   
}

MapMemory::~MapMemory() {

  GC_UNREGISTER_CALLBACKS();

}

// Automatically called at start of simulation
void MapMemory::start_of_simulation() {

  // Intitialize power model
  if (m_pow_mon) {

    power_model();

  }
}

// Print execution statistic at end of simulation
void MapMemory::end_of_simulation() {
     
    v::report << name() << " ********************************************" << v::endl;
    v::report << name() << " * "<< get_type_name() <<" Memory Statistic:" << v::endl;
    v::report << name() << " * -----------------------------------------" << v::endl;
    v::report << name() << " * Bytes read:    " << m_reads << v::endl;
    v::report << name() << " * Bytes written: " << m_writes << v::endl;
    v::report << name() << " ******************************************** " << v::endl;
}

// Calculate power/energy values from normalized input data
void MapMemory::power_model() {

  // Static power calculation (pW)
  sta_power = sta_power_norm * (get_bsize() << 3);

  // Cell internal power (uW)
  int_power = int_power_norm * (get_bsize() << 3) * 1/(clock_cycle.to_seconds()*1.0e+6);

  // Energy per read access (uJ)
  dyn_read_energy =  dyn_read_energy_norm * 32 * (get_bsize() << 3);

  // Energy per write access (uJ)
  dyn_write_energy = dyn_write_energy_norm * 32 * (get_bsize() << 3);

}

// Static power callback
void MapMemory::sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Static power of MapMemory is constant !!

}

// Internal power callback
void MapMemory::int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Internal power of MapMemory is constant !!

}

// Switching power callback
void MapMemory::swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  swi_power = ((dyn_read_energy * dyn_reads) + (dyn_write_energy * dyn_writes)) / (sc_time_stamp() - power_frame_starting_time).to_seconds();

}

// TLM 2.0 blocking transport function
void MapMemory::b_transport(tlm::tlm_generic_payload& gp, sc_time& delay) {

  // Extract erase extension
  ext_erase *ers;
  gp.get_extension(ers);

  if(ers) {
  
    // Check erase extension first:
    // The start address is encoded in the TLM address field.
    uint32_t start = gp.get_address();
    // The end address is encoded in the TLM data field as a uint32_t.
    uint32_t end = *reinterpret_cast<uint32_t *>(gp.get_data_ptr());

    if (end < start) {

      v::error << name() << "Error in erasing memory!" << v::endl;

      gp.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);

    } else {

      // Erase designated memory region
      erase(start, end);

      gp.set_response_status(tlm::TLM_OK_RESPONSE);
      
      // Count write operations for power calculation
      dyn_writes += (end-start) >> 2;

      v::debug << name() << "Erase memory from " << v::uint32 << start << " to " << v::uint32 << end << "." << v::endl;
    }
  } else {

    // Read or write transaction
    tlm::tlm_command cmd = gp.get_command();
    uint32_t addr        = gp.get_address();
    uint32_t len         = gp.get_data_length();
    unsigned char* ptr   = gp.get_data_ptr();
      
    if(cmd == tlm::TLM_READ_COMMAND) {
      
      for(uint32_t i = 0; i < len; i++) {
        ptr[i] = read(addr + i);
      }
        
      v::debug << name() << "Read memory at " << v::uint32 << addr << " with length " << len << "." << v::endl;
      gp.set_response_status(tlm::TLM_OK_RESPONSE);
        
      // Count read operations for power calculation
      dyn_reads += (len >> 2) + 1;
        
    } else if (cmd == tlm::TLM_WRITE_COMMAND) {

      for(uint32_t i = 0; i < len; i++) {
        write(addr + i, ptr[i]);
      }

      v::debug << name() << "Write memory at " << v::uint32 << addr << " with length " << len << "." << v::endl;
      gp.set_response_status(tlm::TLM_OK_RESPONSE);

      // Count write operations for power calculation
      dyn_writes += (len >> 2) + 1;
        
    } else {

      v::warn << name() << "Command not valid / or TLM_IGNORE" << v::endl;

    }
  }
}

// TLM 2.0 debug transport function
unsigned int MapMemory::transport_dbg(tlm::tlm_generic_payload& gp) {

  tlm::tlm_command cmd = gp.get_command();
  uint32_t addr        = gp.get_address();
  uint32_t len         = gp.get_data_length();
  unsigned char* ptr   = gp.get_data_ptr();

  switch(cmd) {
  case tlm::TLM_READ_COMMAND:
    for(uint32_t i=0; i<len; i++) {
      ptr[i] = read(addr + i);
    }
    v::debug << name() << "Debug read memory at " << v::uint32 << addr << " with length " << len << "." << v::endl;
    gp.set_response_status(tlm::TLM_OK_RESPONSE);
    return len;
            
  case tlm::TLM_WRITE_COMMAND:
    for(uint32_t i=0; i<len; i++) {
      write(addr + i, ptr[i]);
    }
    v::debug << name() << "Debug write memory at " << v::uint32 << addr << " with length " << len << "." << v::endl;
    gp.set_response_status(tlm::TLM_OK_RESPONSE);
    return len;
  default:
    return 0;
  }
}

// Write byte to memory
void MapMemory::write(const uint32_t addr, const uint8_t byte) {
    memory[addr] = byte;
    m_writes++;
    v::debug << name() << v::uint32 << addr << ": "<< v::uint8 << (uint32_t)byte << v::endl;
}

// Read byte from memory
uint8_t MapMemory::read(const uint32_t addr) {
    uint8_t byte = memory[addr];
    m_reads++;
    v::debug << name() << v::uint32 << addr << ": "<< v::uint8 << (uint32_t)byte << v::endl;
    return byte;
}

// Erase memory
void MapMemory::erase(uint32_t start, uint32_t end) {
  v::debug << name() << "Erase memory region from: " << v::uint32 << start 
           << " to: " << v::uint32 << end << v::endl;

  // Find or insert start address
  map_mem::iterator start_iter = memory.find(start);
  if(start_iter==memory.end()) {
    memory.insert(std::make_pair(start, 0));
    start_iter = memory.find(start);
  }
    
  // Find or insert end address
  map_mem::iterator end_iter = memory.find(end);
  if(end_iter==memory.end()) {
    memory.insert(std::make_pair(end, 0));
    end_iter = memory.find(end);
  }

  // Erase section
  memory.erase(start_iter, end_iter);

}

