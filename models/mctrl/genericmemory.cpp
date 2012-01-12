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

#include "genericmemory.h"
#include "power_monitor.h"
#include <tlm.h>

using namespace sc_core;
using namespace tlm;
using namespace std;

GenericMemory::GenericMemory(sc_core::sc_module_name name, MEMDevice::device_type type, uint32_t banks, uint32_t bsize, uint32_t bits, uint32_t cols, bool powmon) : MEMDevice(type, banks, bsize, bits, cols), bus("bus"), g_powmon(powmon) {
    // register transport functions to sockets
    gs::socket::config<tlm::tlm_base_protocol_types> bus_cfg;
    bus_cfg.use_mandatory_phase(BEGIN_REQ);
    bus_cfg.use_mandatory_phase(END_REQ);
    //mem_cfg.treat_unknown_as_ignorable();
    bus.set_config(bus_cfg);

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
    
    bus.register_b_transport(this, &GenericMemory::b_transport);
    bus.register_transport_dbg(this, &GenericMemory::transport_dbg);
}

GenericMemory::~GenericMemory() {}

void GenericMemory::b_transport(tlm::tlm_generic_payload& gp, sc_time& delay) {
    ext_erase::ext_erase* ers;
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
        unsigned char* ptr   = gp.get_data_ptr();
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

unsigned int GenericMemory::transport_dbg(tlm::tlm_generic_payload& gp) {
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

void GenericMemory::write(const uint32_t addr, const uint8_t byte) {
    memory[addr] = byte;
    v::debug << name() << v::uint32 << addr << ": "<< v::uint8 << (uint32_t)byte << v::endl;
}

uint8_t GenericMemory::read(const uint32_t addr) {
    uint8_t byte = memory[addr];
    v::debug << name() << v::uint32 << addr << ": "<< v::uint8 << (uint32_t)byte << v::endl;
    return byte;
}

//erase memory
void GenericMemory::erase(uint32_t start, uint32_t end) {
    v::debug << name() << "eraising memory from " << v::uint32 << start 
                       << " to " << v::uint32 << end << v::endl;

    // Find or insert start address
    type::iterator start_iter = memory.find(start);
    if(start_iter==memory.end()) {
        memory.insert(std::make_pair(start, 0));
        start_iter = memory.find(start);
    }
    
    // Find or insert end address
    type::iterator end_iter = memory.find(end);
    if(end_iter==memory.end()) {
        memory.insert(std::make_pair(end, 0));
        end_iter = memory.find(end);
    }

    // erase section
    memory.erase(start_iter, end_iter);
}

