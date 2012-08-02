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
// Title:      arraymemory.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implementation of the generic memory model to be used 
//             with the SoCRocket MCTRL. Can be configured as ROM, 
//             IO, SRAM or SDRAM. Underlying memory is implemented 
//             as a flat array.
//             Recommended for fast simulation of small memories.
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

#include "arraymemory.h"
#include "power_monitor.h"
#include <tlm.h>

using namespace sc_core;
using namespace tlm;
using namespace std;

ArrayMemory::ArrayMemory(sc_core::sc_module_name name, MEMDevice::device_type type, uint32_t banks, uint32_t bsize, uint32_t bits, uint32_t cols, bool powmon) : 
  MEMDevice(type, banks, bsize, bits, cols), 
  bus("bus"), 
  g_powmon(powmon), 
  m_performance_counters("performance_counters"),
  m_reads("bytes_read", 0ull, m_performance_counters), 
  m_writes("bytes_writen", 0ull, m_performance_counters),
  sta_power_norm("power." + get_type_name() + ".sta_power_norm", 0.0, true), // Normalized static power input
  dyn_power_norm("power." + get_type_name() + ".dyn_power_norm", 0.0, true), // Normalized dyn power input (act. independent)
  dyn_read_energy_norm("power." + get_type_name() + ".dyn_read_energy_norm", 0.0, true), // Normalized read energy input
  dyn_write_energy_norm("power." + get_type_name() + ".dyn_write_energy_norm", 0.0, true), // Normalized write energy input
  power("power"),
  sta_power("sta_power", 0.0, power),  // Static power output
  dyn_power("dyn_power", 0.0, power),  // Dynamic power output
  dyn_read_energy("dyn_read_energy", 0.0, power), // Energy per read access
  dyn_write_energy("dyn_write_energy", 0.0, power), // Energy per write access
  dyn_reads("dyn_reads", 0ull, power), // Read access counter for power computation
  dyn_writes("dyn_writes", 0ull, power) // Write access counter for power computation  

{
    // register transport functions to sockets
    gs::socket::config<tlm::tlm_base_protocol_types> bus_cfg;
    bus_cfg.use_mandatory_phase(BEGIN_REQ);
    bus_cfg.use_mandatory_phase(END_REQ);
    //mem_cfg.treat_unknown_as_ignorable();
    bus.set_config(bus_cfg);

    // gs_param class identifier
    m_api = gs::cnf::GCnf_Api::getApiInstance(this);

    bus.register_b_transport(this, &ArrayMemory::b_transport);
    bus.register_transport_dbg(this, &ArrayMemory::transport_dbg);

    // Module configuration report
    v::info << this->name() << " ******************************************************************************* " << v::endl;
    v::info << this->name() << " * Created ArrayMemory with following parameters: " << v::endl;
    v::info << this->name() << " * ------------------------------------------------ " << v::endl;
    v::info << this->name() << " * device_type (ROM, IO, SRAM, SDRAM): " << get_type_name() << v::endl;
    v::info << this->name() << " * banks: " << banks << v::endl;
    v::info << this->name() << " * bsize (bytes): " << hex << bsize << v::endl;
    v::info << this->name() << " * bit width: " << bits << v::endl;
    v::info << this->name() << " * cols (SD only): " << cols << v::endl;
    v::info << this->name() << " * pow_mon: " << powmon << v::endl;
    v::info << this->name() << " ******************************************************************************* " << v::endl;
    
    // Calculate array size
    size_t mem_size = bsize;
    mem_size *= (banks<5)?banks:8;
    memory = new uint8_t[mem_size+1];
    erase(0, mem_size);
}

ArrayMemory::~ArrayMemory() {
    delete[] memory;
}

// Print execution statistic at end of simulation
void ArrayMemory::end_of_simulation() {
     
    v::report << name() << " ********************************************" << v::endl;
    v::report << name() << " * "<< get_type_name() << " Memory Statistic:" << v::endl;
    v::report << name() << " * -----------------------------------------" << v::endl;
    v::report << name() << " * Bytes read:    " << m_reads << v::endl;
    v::report << name() << " * Bytes written: " << m_writes << v::endl;
    v::report << name() << " ******************************************** " << v::endl;
}

void ArrayMemory::b_transport(tlm::tlm_generic_payload& gp, sc_time& delay) {
    ext_erase *ers;
    gp.get_extension(ers);
    if(ers) {
        // check erase extension first
        // The start address is encoded in the TLM address field.
        uint32_t start = gp.get_address();
        // The end address is encoded in the TLM data field as a uint32_t.
        uint32_t end = *reinterpret_cast<uint32_t *>(gp.get_data_ptr());
        erase(start, end);
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
        
        // calculating delay
        v::debug << name() << "Erase memory from " << v::uint32 << start << " to " << v::uint32 << end << "." << v::endl;
    } else {
        // Read or write transaction
        tlm::tlm_command cmd = gp.get_command();
        uint32_t addr        = gp.get_address();
        uint32_t len         = gp.get_data_length();
        uint8_t *ptr         = gp.get_data_ptr();
        if(cmd == tlm::TLM_READ_COMMAND) {
            for(uint32_t i = 0; i < len; i++) {
                ptr[i] = read(addr + i);
            }
            v::debug << name() << "Read memory at " << v::uint32 << addr << " with length " << len << "." << v::endl;
            gp.set_response_status(tlm::TLM_OK_RESPONSE);
            // calculating delay
        } else if(cmd == tlm::TLM_WRITE_COMMAND) {
            for(uint32_t i = 0; i < len; i++) {
                write(addr + i, ptr[i]);
            }
            v::debug << name() << "Write memory at " << v::uint32 << addr << " with length " << len << "." << v::endl;
            gp.set_response_status(tlm::TLM_OK_RESPONSE);
            // calculating delay
        } else {
            v::warn << name() << "Memory received TLM_IGNORE command" << v::endl;
        }
    }
}

unsigned int ArrayMemory::transport_dbg(tlm::tlm_generic_payload& gp) {
    tlm::tlm_command cmd = gp.get_command();
    uint32_t addr        = gp.get_address();
    uint32_t len         = gp.get_data_length();
    uint8_t *ptr         = gp.get_data_ptr();

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

void ArrayMemory::write(const uint32_t addr, const uint8_t byte) {
    memory[addr] = byte;
    m_writes++;
    v::debug << name() << v::uint32 << addr << ": "<< v::uint8 << (uint32_t)byte << v::endl;
}

uint8_t ArrayMemory::read(const uint32_t addr) {
    uint8_t byte = memory[addr];
    m_reads++;
    v::debug << name() << v::uint32 << addr << ": "<< v::uint8 << (uint32_t)byte << v::endl;
    return byte;
}

//erase memory
void ArrayMemory::erase(uint32_t start, uint32_t end) {
    v::debug << name() << "eraising memory from " << v::uint32 << start 
                       << " to " << v::uint32 << end << v::endl;

    for(size_t i = start; i < end + 1; i++) {
        memory[i] = 0;
    }
}

