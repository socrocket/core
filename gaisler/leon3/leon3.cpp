// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup leon3_Leon3
/// @{
/// @file leon3_Leon3.cpp
/// Implementation of LEON2/3 cache-subsystem consisting of instruction cache,
/// data cache, i/d localrams and memory management unit. The leon3_Leon3 class
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
#include "gaisler/leon3/leon3.h"
#include "core/common/sr_report.h"
#include "core/common/vendian.h"

SR_HAS_MODULE(Leon3);

/// Constructor
Leon3::Leon3(
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
  cpu("cpu", this, sc_core::sc_time(10, sc_core::SC_NS), pow_mon),
  debugger(NULL),
  m_intrinsics("intrinsics", *(cpu.abiIf)),
  g_gdb("gdb", 0, m_generics),
  g_icen("icen", icen, m_generics),
  g_irepl("irepl", irepl, m_generics),
  g_isets("isets", isets, m_generics),
  g_ilinesize("ilinesize", ilinesize, m_generics),
  g_isetsize("isetsize", isetsize, m_generics),
  g_isetlock("isetlock", isetlock, m_generics),
  g_dcen("dcen", dcen, m_generics),
  g_drepl("drepl", drepl, m_generics),
  g_dsets("dsets", dsets, m_generics),
  g_dlinesize("dlinesize", dlinesize, m_generics),
  g_dsetsize("dsetsize", dsetsize, m_generics),
  g_dsetlock("dsetlock", dsetlock, m_generics),
  g_dsnoop("dsnoop", dsnoop, m_generics),
  g_ilram("ilram", ilram, m_generics),
  g_ilramsize("ilramsize", ilramsize, m_generics),
  g_ilramstart("ilramstart", ilramstart, m_generics),
  g_dlram("dlram", dlram, m_generics),
  g_dlramsize("dlramsize", dlramsize, m_generics),
  g_dlramstart("dlramstart", dlramstart, m_generics),
  g_cached("cached", cached, m_generics),
  g_mmu_en("mmu_en", mmu_en, m_generics),
  g_itlb_num("itlb_num", itlb_num, m_generics),
  g_dtlb_num("dtlb_num", dtlb_num, m_generics),
  g_tlb_type("tlb_type", tlb_type, m_generics),
  g_tlb_rep("tlb_rep", tlb_rep, m_generics),
  g_mmupgsz("mmupgsz", mmupgsz, m_generics),
  //g_hindex("hindex", hindex, m_generics),
  g_args("args", m_generics),
  g_stdout_filename("stdout_filename", "", m_generics) {
    // TODO(rmeyer): This looks a lot like gs_configs!!!

    GC_REGISTER_TYPED_PARAM_CALLBACK(&g_gdb, gs::cnf::post_write, Leon3, g_gdb_callback);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&g_args, gs::cnf::post_write, Leon3, g_args_callback);
    Leon3::init_generics();
    cpu.toolManager.addTool(m_intrinsics);
}

Leon3::~Leon3() {

  GC_UNREGISTER_CALLBACKS();

}
void Leon3::init_generics(){
    g_icen.add_properties()
    ("vhdl_name","icen");
    g_irepl.add_properties()
    ("vhdl_name","irepl");
    g_isets.add_properties()
    ("vhdl_name","isets");
    g_ilinesize.add_properties()
    ("vhdl_name","ilinesize");
    g_isetsize.add_properties()
    ("vhdl_name","isetsize");
    g_isetlock.add_properties()
    ("vhdl_name","isetlock");
    g_dcen.add_properties()
    ("vhdl_name","dcen");
    g_drepl.add_properties()
    ("vhdl_name","drepl");
    g_dsets.add_properties()
    ("vhdl_name","dsets");
    g_dlinesize.add_properties()
    ("vhdl_name","dlinesize");
    g_dsetsize.add_properties()
    ("vhdl_name","dsetsize");
    g_dsetlock.add_properties()
    ("vhdl_name","dsetlock");
    g_dsnoop.add_properties()
    ("vhdl_name","dsnoop");
    g_ilram.add_properties()
    ("vhdl_name","ilram");
    g_ilramsize.add_properties()
    ("vhdl_name","ilramsize");
    g_ilramstart.add_properties()
    ("vhdl_name","ilramstart");
    g_dlram.add_properties()
    ("vhdl_name","dlram");
    g_dlramsize.add_properties()
    ("vhdl_name","dlramsize");
    g_dlramstart.add_properties()
    ("vhdl_name","dlramstart");
    g_cached.add_properties()
    ("vhdl_name","cached");
    g_mmu_en.add_properties()
    ("vhdl_name","mmuen");
    g_itlb_num.add_properties()
    ("vhdl_name","itlbnum");
    g_dtlb_num.add_properties()
    ("vhdl_name","dtlbnum");
    g_tlb_type.add_properties()
    ("vhdl_name","tlb_type");
    g_tlb_rep.add_properties()
    ("vhdl_name","tlb_rep");
    g_mmupgsz.add_properties()
    ("vhdl_name","mmupgsz");
    g_hindex.add_properties()
    ("vhdl_name","hindex");
}

void Leon3::start_of_simulation() {
  cpu.ENTRY_POINT   = 0x0;
  cpu.MPROC_ID      = (g_hindex) << 28;
  g_args_callback(g_args, gs::cnf::no_callback);
}

void Leon3::clkcng() {
  mmu_cache_base::clkcng();
  cpu.latency = clock_cycle;
}

gs::cnf::callback_return_type Leon3::g_gdb_callback(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {
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

gs::cnf::callback_return_type Leon3::g_args_callback(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {
  std::vector<std::string> options;
  for(uint32_t i = 0; i < g_args.size(); i++) {
      options.push_back(g_args[i]);
  }
  m_intrinsics.set_program_args(options);
  return GC_RETURN_OK;
}
// Read instruction
unsigned int Leon3::read_instr(const unsigned int & address, const unsigned int asi, const unsigned int flush) throw() {

    unsigned int datum = 0;
    sc_time delay = this->cpu.quantKeeper.get_local_time();
    unsigned int debug = 0;
    exec_instr(
        address,
        reinterpret_cast<uint8_t *>(&datum),
        asi,
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
sc_dt::uint64 Leon3::read_dword(
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
uint32_t Leon3::read_word(
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
uint16_t Leon3::read_half(
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
uint8_t Leon3::read_byte(
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
void Leon3::write_dword(
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

void Leon3::write_word(
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
void Leon3::write_half(
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
void Leon3::write_byte(
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

sc_dt::uint64 Leon3::read_dword_dbg(const uint32_t &address) throw() {

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

uint32_t Leon3::read_word_dbg(const uint32_t &address) throw(){
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

uint16_t Leon3::read_half_dbg(const uint32_t &address) throw() {
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

uint8_t Leon3::read_byte_dbg(const uint32_t &address) throw(){
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

void Leon3::write_dword_dbg(const uint32_t &address, sc_dt::uint64 datum) throw() {
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

void Leon3::write_word_dbg(const uint32_t &address, uint32_t datum) throw() {
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

void Leon3::write_half_dbg(const uint32_t &address, uint16_t datum) throw() {
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

void Leon3::write_byte_dbg(const uint32_t &address, uint8_t datum) throw() {
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

void Leon3::lock() {

}

void Leon3::unlock() {

}

void Leon3::trigger_exception(unsigned int exception) {
  // somehow trigger the exception in the CPU
  v::info << name() << "Going to trigger exception " << exception << v::endl;
  cpu.triggerException(exception);
  v::info << name() << "Returned from trigger exception " << v::endl;

}


/// @}
