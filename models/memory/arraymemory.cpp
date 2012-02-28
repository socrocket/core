/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       generic_memory.tpp                                      */
/*             implementation of the generic_memory module             */
/*                                                                     */
/* Modified on $Date: 2011-08-04 16:52:17 +0200 (Thu, 04 Aug 2011) $   */
/*          at $Revision: 481 $                                        */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

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
  m_reads("bytes_read", 0llu, m_performance_counters), 
  m_writes("bytes_writen", 0llu, m_performance_counters) {
    // register transport functions to sockets
    gs::socket::config<tlm::tlm_base_protocol_types> bus_cfg;
    bus_cfg.use_mandatory_phase(BEGIN_REQ);
    bus_cfg.use_mandatory_phase(END_REQ);
    //mem_cfg.treat_unknown_as_ignorable();
    bus.set_config(bus_cfg);

    // gs_param class identifier
    m_api = gs::cnf::GCnf_Api::getApiInstance(this);

    char *type_name;
    switch(type) {
      case MEMDevice::IO:
          type_name = "io";
          break;
      case MEMDevice::SRAM:
          type_name = "sram";
          break;
      case MEMDevice::SDRAM:
          type_name = "sdram";
          break;
      default:
        type_name = "rom";
    }
    PM::registerIP(this, type_name, powmon);
    PM::send_idle(this, "idle", sc_time_stamp(), true);
    
    bus.register_b_transport(this, &ArrayMemory::b_transport);
    bus.register_transport_dbg(this, &ArrayMemory::transport_dbg);

    // Module configuration report
    v::info << this->name() << " ******************************************************************************* " << v::endl;
    v::info << this->name() << " * Created ArrayMemory with following parameters: " << v::endl;
    v::info << this->name() << " * ------------------------------------------------ " << v::endl;
    v::info << this->name() << " * device_type (0-ROM, 1-IO, 2-SRAM, 3-SDRAM): " << type << v::endl;
    v::info << this->name() << " * banks: " << banks << v::endl;
    v::info << this->name() << " * bsize (bytes): " << hex << bsize << v::endl;
    v::info << this->name() << " * bit width: " << bits << v::endl;
    v::info << this->name() << " * cols (SD only): " << cols << v::endl;
    v::info << this->name() << " * pow_mon: " << powmon << v::endl;
    v::info << this->name() << " ******************************************************************************* " << v::endl;
    
    // Calculate array size
    size_t mem_size = bsize;
    mem_size *= (banks<5)?banks:8;
    memory = new uint8_t[mem_size];
    erase(0, mem_size);
}

ArrayMemory::~ArrayMemory() {}

// Print execution statistic at end of simulation
void ArrayMemory::end_of_simulation() {
    char *type_name;
    switch(get_type()) {
      case MEMDevice::IO:
          type_name = "IO";
          break;
      case MEMDevice::SRAM:
          type_name = "SRAM";
          break;
      case MEMDevice::SDRAM:
          type_name = "SDRAM";
          break;
      default:
        type_name = "ROM";
    }
    
    v::report << name() << " ********************************************" << v::endl;
    v::report << name() << " * "<< type_name <<" Memory Statistic:" << v::endl;
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

