// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file leon3_leon3_mmu_cache.cpp
/// Implementation of LEON2/3 cache-subsystem consisting of instruction cache,
/// data cache, i/d localrams and memory management unit. The leon3_leon3_mmu_cache class
/// provides two TLM slave sockets for connecting the cpu and an AHB master
/// interface for connecting the processor bus.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include <boost/filesystem.hpp>
#include "core/models/mmu_cache/lib/leon3_mmu_cache.h"
#include "core/common/report.h"
#include "core/common/vendian.h"

//SC_HAS_PROCESS(leon3_mmu_cache<>);
/// Constructor
leon3_mmu_cache::leon3_mmu_cache(
      ModuleName name,
      bool icen, 
      uint32_t irepl, 
      uint32_t isets,
      uint32_t ilinesize, 
      uint32_t isetsize,
      uint32_t isetlock,
      uint32_t dcen,
      uint32_t drepl,
      uint32_t dsets,
      uint32_t dlinesize,
      uint32_t dsetsize,
      bool dsetlock,
      bool dsnoop,
      bool ilram,
      uint32_t ilramsize,
      uint32_t ilramstart,
      uint32_t dlram,
      uint32_t dlramsize,
      uint32_t dlramstart,
      uint32_t cached,
      bool mmu_en,
      uint32_t itlb_num,
      uint32_t dtlb_num,
      uint32_t tlb_type,
      uint32_t tlb_rep,
      uint32_t mmupgsz,
      uint32_t hindex,
      bool pow_mon,
     AbstractionLayer abstractionLayer) :
  mmu_cache_base(
      name,
      icen, 
      irepl, 
      isets,
      ilinesize, 
      isetsize,
      isetlock,
      dcen,
      drepl,
      dsets,
      dlinesize,
      dsetsize,
      dsetlock,
      dsnoop,
      ilram,
      ilramsize,
      ilramstart,
      dlram,
      dlramsize,
      dlramstart,
      cached,
      mmu_en,
      itlb_num,
      dtlb_num,
      tlb_type,
      tlb_rep,
      mmupgsz,
      hindex,
      pow_mon,
      abstractionLayer),
  cpu("leon3", this, sc_core::sc_time(10, sc_core::SC_NS), pow_mon),
  debugger(NULL),
  osEmu(NULL),
  g_gdb("gdb", 0, m_generics),
  g_history("history", "", m_generics),
  g_osemu("osemu", "", m_generics),
  g_args("args", m_generics) {
    // TODO(rmeyer): This looks a lot like gs_configs!!!
    cpu.ENTRY_POINT   = 0x0;
    cpu.MPROC_ID      = (hindex) << 28;
    
    GC_REGISTER_TYPED_PARAM_CALLBACK(&g_gdb, gs::cnf::post_write, leon3_mmu_cache, g_gdb_callback);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&g_history, gs::cnf::post_write, leon3_mmu_cache, g_history_callback);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&g_osemu, gs::cnf::post_write, leon3_mmu_cache, g_osemu_callback);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&g_args, gs::cnf::post_write, leon3_mmu_cache, g_args_callback);
}

leon3_mmu_cache::~leon3_mmu_cache() {

  GC_UNREGISTER_CALLBACKS();

}

void leon3_mmu_cache::clkcng() {
  mmu_cache_base::clkcng();
  cpu.latency = clock_cycle;
}

gs::cnf::callback_return_type leon3_mmu_cache::g_gdb_callback(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {
  int port = 0;
  changed_param.getValue(port);
  if(port) {
    debugger = new GDBStub<uint32_t>(*(cpu.abiIf));
    cpu.toolManager.addTool(*debugger);
    debugger->initialize(port);
  } else {
    //delete debugger;
    debugger = NULL;
  }
  return GC_RETURN_OK;
}

gs::cnf::callback_return_type leon3_mmu_cache::g_history_callback(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {
  std::string history;
  changed_param.getValue(history);
  if(!history.empty()) {
    cpu.enableHistory(history);
  }
  return GC_RETURN_OK;
}

gs::cnf::callback_return_type leon3_mmu_cache::g_osemu_callback(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {
  std::string osemu;
  changed_param.getValue(osemu);
  if(!osemu.empty()) {
    if(boost::filesystem::exists(boost::filesystem::path(osemu))) {
      osEmu = new OSEmulator<unsigned int>(*(cpu.abiIf));
      osEmu->initSysCalls(osemu);
    }
    cpu.toolManager.addTool(*osEmu);
  } else {
    v::warn << name() << "File " << osemu << " not found!" << v::endl;
    exit(1);
  }
  return GC_RETURN_OK;
}

gs::cnf::callback_return_type leon3_mmu_cache::g_args_callback(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {
  std::vector<std::string> options;
  options.push_back((std::string)g_osemu);
  for(uint32_t i = 0; i < g_args.size(); i++) {
      options.push_back(g_args[i]);
  }
  osEmu->set_program_args(options);
  return GC_RETURN_OK;
}
// Read instruction
unsigned int leon3_mmu_cache::read_instr(const unsigned int & address, const unsigned int flush) throw() {

    unsigned int datum = 0;
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    unsigned int debug = 0;
    exec_instr(
        address,
        reinterpret_cast<uint8_t *>(&datum),
        &debug,
        flush,
        delay,
        false);

    //Now lets keep track of time
    this->cpu.quantKeeper.set(delay);
    if(this->cpu.quantKeeper.need_sync()){
//std::cout << "Quantum (external) sync" << std::endl;
      this->cpu.quantKeeper.sync();
    }
    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    swapEndianess(datum);
    v::debug << name() << "Read word:0x" << hex << v::setw(8) << v::setfill('0')
             << datum << ", from:0x" << hex << v::setw(8) << v::setfill('0')
             << address << endl;
    return datum;
}

// Read dword
sc_dt::uint64 leon3_mmu_cache::read_dword(
    const uint32_t &address,
    const uint32_t asi,
    const uint32_t flush,
    const uint32_t lock) throw() {

    sc_dt::uint64 datum = 0;
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    uint32_t debug = 0;
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;

    exec_data(
        tlm::TLM_READ_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        asi,
        &debug,
        flush,
        lock,
        delay,
        false,
        response);

    //Now lets keep track of time
    this->cpu.quantKeeper.set(delay);
    if(this->cpu.quantKeeper.need_sync()){
        this->cpu.quantKeeper.sync();
    }

    #ifdef LITTLE_ENDIAN_BO
    uint32_t datum1 = (uint32_t)(datum);
    swapEndianess(datum1);
    uint32_t datum2 = (uint32_t)(datum >> 32);
    swapEndianess(datum2);
    datum = datum1 | (((sc_dt::uint64)datum2) << 32);
    #endif

    return datum;
}

// Read data word 
uint32_t leon3_mmu_cache::read_word(
    const uint32_t &address,
    const uint32_t asi,
    const uint32_t flush,
    const uint32_t lock) throw() {

    uint32_t datum = 0;
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    uint32_t debug = 0;
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;

    exec_data(
        tlm::TLM_READ_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        asi,
        &debug,
        flush,
        lock,
        delay,
        false,
        response);

    //Now lets keep track of time
    this->cpu.quantKeeper.set(delay);
    if(this->cpu.quantKeeper.need_sync()){
      this->cpu.quantKeeper.sync();
    }
    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    #ifdef LITTLE_ENDIAN_BO
    swapEndianess(datum);
    #endif
    v::debug << name() << "Read word:0x" << hex << v::setw(8) << v::setfill('0')
             << datum << ", from:0x" << hex << v::setw(8) << v::setfill('0')
             << address << endl;

    return datum;
}

// read half word
uint16_t leon3_mmu_cache::read_half(
    const uint32_t &address,
    const uint32_t asi,
    const uint32_t flush,
    const uint32_t lock) throw() {

    uint16_t datum = 0;
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    uint32_t debug = 0;
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;
    
    exec_data(
        tlm::TLM_READ_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        asi,
        &debug,
        flush,
        lock,
        delay,
        false,
        response);

    //Now lets keep track of time
    this->cpu.quantKeeper.set(delay);
    if(this->cpu.quantKeeper.need_sync()){
        this->cpu.quantKeeper.sync();
    }

    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    swapEndianess(datum);
    return datum;
}

// read byte
uint8_t leon3_mmu_cache::read_byte(
    const uint32_t &address,
    const uint32_t asi,
    const uint32_t flush,
    const uint32_t lock) throw() {

    uint8_t datum = 0;
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    uint32_t debug = 0;
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;
    
    exec_data(
        tlm::TLM_READ_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        asi,
        &debug,
        flush,
        lock,
        delay,
        false,
        response);

    // Now lets keep track of time
    this->cpu.quantKeeper.set(delay);
    if(this->cpu.quantKeeper.need_sync()){
        this->cpu.quantKeeper.sync();
    }

    return datum;
}

// Write dword
void leon3_mmu_cache::write_dword(
    const uint32_t & address, 
    sc_dt::uint64 datum,
    const uint32_t asi,
    const uint32_t flush,
    const uint32_t lock) throw(){

    uint32_t datum1 = (uint32_t)(datum);
    swapEndianess(datum1);
    uint32_t datum2 = (uint32_t)(datum >> 32);
    swapEndianess(datum2);
    datum = datum1 | (((sc_dt::uint64)datum2) << 32);
    if(this->debugger != NULL){
        this->debugger->notifyAddress(address, sizeof(datum));
    }

    sc_time delay = this->cpu.quantKeeper.get_local_time();
    uint32_t debug = 0;
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;
    
    exec_data(
        tlm::TLM_WRITE_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        asi,
        &debug,
        flush,
        lock,
        delay,
        false,
        response);

    //Now lets keep track of time
    this->cpu.quantKeeper.set(delay);
    if(this->cpu.quantKeeper.need_sync()){
        this->cpu.quantKeeper.sync();
    }
}

void leon3_mmu_cache::write_word(
  const unsigned int &address, 
  unsigned int datum,
  const unsigned int asi,
  const unsigned int flush,
  const unsigned int lock) throw() {

    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    swapEndianess(datum);
    if(this->debugger != NULL){
        v::debug << name() << "Debugger" << endl;
        this->debugger->notifyAddress(address, sizeof(datum));
    }
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    unsigned int debug = 0;
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;
    
    exec_data(
        tlm::TLM_WRITE_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        asi,
        &debug,
        flush,
        lock,
        delay,
        false,
        response);
    
    v::debug << name() << "Wrote word:0x" << hex << v::setw(8) << v::setfill('0')
             << datum << ", at:0x" << hex << v::setw(8) << v::setfill('0')
             << address << endl;

    //Now lets keep track of time
    this->cpu.quantKeeper.set(delay);
    if(this->cpu.quantKeeper.need_sync()){
      this->cpu.quantKeeper.sync();
    }
}

// write half word
void leon3_mmu_cache::write_half(
    const uint32_t &address, 
    uint16_t datum,
    uint32_t asi,
    uint32_t flush,
    uint32_t lock) throw() {

    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    swapEndianess(datum);
    if(this->debugger != NULL){
        this->debugger->notifyAddress(address, sizeof(datum));
    }

    sc_time delay = this->cpu.quantKeeper.get_local_time();
    uint32_t debug = 0;
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;
    
    exec_data(
        tlm::TLM_WRITE_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        asi,
        &debug,
        flush,
        lock,
        delay,
        false,
        response);
    
    // Now lets keep track of time
    this->cpu.quantKeeper.set(delay);
    if(this->cpu.quantKeeper.need_sync()){
        this->cpu.quantKeeper.sync();
    }
}

// write byte
void leon3_mmu_cache::write_byte(
    const uint32_t &address, 
    uint8_t datum,
    uint32_t asi,
    uint32_t flush,
    uint32_t lock) throw() {

    if(this->debugger != NULL){
        this->debugger->notifyAddress(address, sizeof(datum));
    }
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    uint32_t debug = 0;
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;
    
    exec_data(
        tlm::TLM_WRITE_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        asi,
        &debug,
        flush,
        lock,
        delay,
        false,
        response);

    //Now lets keep track of time
    this->cpu.quantKeeper.set(delay);
    if(this->cpu.quantKeeper.need_sync()){
        this->cpu.quantKeeper.sync();
    }
}

sc_dt::uint64 leon3_mmu_cache::read_dword_dbg(const uint32_t &address) throw() {

    sc_dt::uint64 datum = 0;
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    uint32_t debug = 0;
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;

    exec_data(
        tlm::TLM_READ_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        8,
        &debug,
        0,
        0,
        delay,
        true,
        response);

    uint32_t datum1 = (uint32_t)(datum);
    swapEndianess(datum1);
    uint32_t datum2 = (uint32_t)(datum >> 32);
    swapEndianess(datum2);
    datum = datum1 | (((sc_dt::uint64)datum2) << 32);
    
    return datum;
}

uint32_t leon3_mmu_cache::read_word_dbg(const uint32_t &address) throw(){
    uint32_t debug = 0;
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    uint32_t datum = 0;
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;

    exec_data(
        tlm::TLM_READ_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        8,
        &debug,
        0,
        0,
        delay,
        true,
        response);

    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    swapEndianess(datum);

    return datum;
}

uint16_t leon3_mmu_cache::read_half_dbg(const uint32_t &address) throw() {
    uint32_t debug = 0;
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    uint16_t datum = 0;
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;

    exec_data(
        tlm::TLM_READ_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        8,
        &debug,
        0,
        0,
        delay,
        true,
        response);

    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    swapEndianess(datum);
    return datum;
}

uint8_t leon3_mmu_cache::read_byte_dbg(const uint32_t &address) throw(){
    uint32_t debug = 0;
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    uint8_t datum = 0;
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;

    exec_data(
        tlm::TLM_READ_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        8,
        &debug,
        0,
        0,
        delay,
        true,
        response);

    return datum;
}

void leon3_mmu_cache::write_dword_dbg(const uint32_t &address, sc_dt::uint64 datum) throw() {
    uint32_t datum1 = static_cast<uint32_t>(datum);
    swapEndianess(datum1);
    uint32_t datum2 = static_cast<uint32_t>(datum >> 32);
    swapEndianess(datum2);
    datum = datum1 | (((sc_dt::uint64)datum2) << 32);

    sc_time delay = this->cpu.quantKeeper.get_local_time();
    uint32_t debug = 0;
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;
    
    exec_data(
        tlm::TLM_WRITE_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        8,
        &debug,
        0,
        0,
        delay,
        true,
        response);
}

void leon3_mmu_cache::write_word_dbg(const uint32_t &address, uint32_t datum) throw() {
    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    swapEndianess(datum);

    uint32_t debug = 0;
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;
    
    exec_data(
        tlm::TLM_WRITE_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        8,
        &debug,
        0,
        0,
        delay,
        true,
        response);
}

void leon3_mmu_cache::write_half_dbg(const uint32_t &address, uint16_t datum) throw() {
    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    swapEndianess(datum);
    uint32_t debug = 0;
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;
    
    exec_data(
        tlm::TLM_WRITE_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        8,
        &debug,
        0,
        0,
        delay,
        true,
        response);
}

void leon3_mmu_cache::write_byte_dbg(const uint32_t &address, uint8_t datum) throw() {
    uint32_t debug = 0;
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    tlm::tlm_response_status response = tlm::TLM_INCOMPLETE_RESPONSE;
    
    exec_data(
        tlm::TLM_WRITE_COMMAND,
        address,
        reinterpret_cast<uint8_t *>(&datum),
        sizeof(datum),
        8,
        &debug,
        0,
        0,
        delay,
        true,
        response);
}

void leon3_mmu_cache::lock() {

}

void leon3_mmu_cache::unlock() {

}

void leon3_mmu_cache::trigger_exception(unsigned int exception) {
  // somehow trigger the exception in the CPU
  v::info << name() << "Going to trigger exception " << exception << v::endl;
  cpu.triggerException(exception);
  v::info << name() << "Returned from trigger exception " << v::endl;
  
}


/// @}
