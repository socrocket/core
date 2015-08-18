// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup platform
/// @{
/// @file leon3processor.h
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
#ifndef PLATFORM_BASE_LEON3PROCESSOR_H_
#define PLATFORM_BASE_LEON3PROCESSOR_H_
#include "core/platforms/base/ambabase.h"
#include "core/common/clkdevice.h"
#include "core/common/systemc.h"

#include "gaisler/leon3/mmucache/mmu_cache.h"
#include "gaisler/leon3/intunit.h"
#include "gaisler/extern/LEON3/simulatorSources/leon3.funcat.h"

class Leon3Processor : public sc_core::sc_module, public CLKDevice {
  public:
    Leon3Processor(
      sc_core::sc_module_name name = "",  ///< SystemC module name
      bool mmu_cache_icen = true,                   ///< instruction cache enable
      uint32_t mmu_cache_irepl = 1,                 ///< instruction cache replacement strategy
      uint32_t mmu_cache_isets = 4,                 ///< number of instruction cache sets
      uint32_t mmu_cache_ilinesize = 8,             ///< instruction cache line size (in bytes)
      uint32_t mmu_cache_isetsize = 8,              ///< size of an instruction cache set (in kbytes)
      uint32_t mmu_cache_isetlock = true,           ///< enable instruction cache locking
      uint32_t mmu_cache_dcen = true,               ///< data cache enable
      uint32_t mmu_cache_drepl = 1,                 ///< data cache replacement strategy
      uint32_t mmu_cache_dsets = 2,                 ///< number of data cache sets
      uint32_t mmu_cache_dlinesize = 4,             ///< data cache line size (in bytes)
      uint32_t mmu_cache_dsetsize = 8,              ///< size of a data cache set (in kbytes)
      bool mmu_cache_dsetlock = true,               ///< enable data cache locking
      bool mmu_cache_dsnoop = true,                 ///< enable data cache snooping
      bool mmu_cache_ilram = false,                 ///< enable instruction scratch pad
      uint32_t mmu_cache_ilramsize = 0x000,         ///< size of the instruction scratch pad (in kbytes)
      uint32_t mmu_cache_ilramstart = 0x000,        ///< start address of the instruction scratch pad
      uint32_t mmu_cache_dlram = false,             ///< enable data scratch pad
      uint32_t mmu_cache_dlramsize = 0x000,         ///< size of the data scratch pad (in kbytes)
      uint32_t mmu_cache_dlramstart = 0x000,        ///< start address of the data scratch pad
      uint32_t mmu_cache_cached = 0,                ///< fixed cacheability mask
      bool mmu_cache_mmu_en = true,                 ///< mmu enable
      uint32_t mmu_cache_itlb_num = 8,              ///< number of instruction TLBs
      uint32_t mmu_cache_dtlb_num = 8,              ///< number of data TLBs
      uint32_t mmu_cache_tlb_type = 0,              ///< split or shared instruction and data TLBs
      uint32_t mmu_cache_tlb_rep = 1,               ///< TLB replacement strategy
      uint32_t mmu_cache_mmupgsz = 0,               ///< MMU page size
      uint32_t mmu_cache_hindex = 0,                ///< ID of the bus master
      bool pow_mon = false,              ///< Enable power monitoring in AHBCtrl and APBCtrl
      amba::amba_layer_ids ambaLayer = amba::amba_LT) : 
      sc_core::sc_module(nm),
    mmu_cache(
      "mmu_cache",
      mmu_cache_icen,
      mmu_cache_irepl,
      mmu_cache_isets,
      mmu_cache_ilinesize,
      mmu_cache_isetsize,
      mmu_cache_isetlock,
      mmu_cache_dcen,
      mmu_cache_drepl,
      mmu_cache_dsets,
      mmu_cache_dlinesize,
      mmu_cache_dsetsize,
      mmu_cache_dsetlock,
      mmu_cache_dsnoop,
      mmu_cache_ilram,
      mmu_cache_ilramsize,
      mmu_cache_ilramstart,
      mmu_cache_dlram,
      mmu_cache_dlramsize,
      mmu_cache_dlramstart,
      mmu_cache_cached,
      mmu_cache_mmu_en,
      mmu_cache_itlb_num,
      mmu_cache_dtlb_num,
      mmu_cache_tlb_type,
      mmu_cache_tlb_rep,
      mmu_cache_mmupgsz,
      mmu_cache_hindex,
      pow_mon,
      ambaLayer) {
/*
      if(ambaLayer == amba::AT) {
        // LEON3 AT Processor
        // ==================
        leon3at = new leon3_funcat_trap::Processor_leon3_funcat("leon3", sc_core::sc_time(10, SC_NS), pow_mon);
        leon3at->ENTRY_POINT   = 0x0;
        leon3at->MPROC_ID      = mmu_cache_hindex << 28;

        // Connect cpu to mmu-cache
        leon3at->instrMem.initSocket(mmu_cache.icio);
        leon3at->dataMem.initSocket(mmu_cache.dcio);

        // History logging
        //std::string history = p_proc_history;
        //if(!history.empty()) {
        //  leon3->enableHistory(history + boost::lexical_cast<std::string>(i));
        //}

        //connect(irqmp.irq_req, leon3->IRQ_port.irq_signal, i);
        //connect(leon3->irqAck.initSignal, irqmp.irq_ack, i);
        //connect(leon3->irqAck.run, irqmp.cpu_rst, i);
        //connect(leon3->irqAck.status, irqmp.cpu_stat, i);

        // GDBStubs
        //if(p_gdb_en && p_gdb_proc == i) {
        if(p_gdb_en) {
          GDBStub<uint32_t> *gdbStub = new GDBStub<uint32_t>(*(leon3->abiIf));
          leon3->toolManager.addTool(*gdbStub);
          gdbStub->initialize(p_gdb_port + i);
          //leon3->instrMem.setDebugger(gdbStub);
          //leon3->dataMem.setDebugger(gdbStub);
        }
        // OS Emulator
        // ===========
        // is activating the leon traps to map basic io functions to the host system
        // set_brk, open, read, ...
        if(!((std::string)p_system_osemu).empty()) {
          v::warn << "OSEmu" << "content " << p_system_osemu << v::endl;

          if(boost::filesystem::exists(boost::filesystem::path((std::string)p_system_osemu))) {
            v::warn << "OSEmu" << "Enabled" << v::endl;
            OSEmulator<unsigned int> *osEmu = new OSEmulator<unsigned int>(*(leon3->abiIf));
            osEmu->initSysCalls(p_system_osemu);
            std::vector<std::string> options;
            options.push_back(p_system_osemu);
            if(vm.count("argument")) {
              std::vector<std::string> argvec = vm["argument"].as<std::vector<std::string> >();
              for(std::vector<std::string>::iterator iter = argvec.begin(); iter != argvec.end(); iter++) {
                options.push_back(*iter);
              }
            }
            osEmu->set_program_args(options);
            //OSEmulator<unsigned int>::set_program_args(options);
            leon3->toolManager.addTool(*osEmu);
          } else {
            v::warn << "main" << "File " << p_system_osemu << " not found!" << v::endl;
            exit(1);
          }
        }
      } else {
        // LEON3 LT Processor
        // ==================
        v::info << "main" << "Instantiating LT Processor" << i << v::endl;
        leon3_funclt_trap::Processor_leon3_funclt *leon3 = new leon3_funclt_trap::Processor_leon3_funclt(sc_core::sc_gen_unique_name("leon3", false), sc_core::sc_time(p_system_clock, SC_NS), p_report_power);
        leon3->ENTRY_POINT   = 0x0;
        leon3->MPROC_ID      = (p_mmu_cache_index + i) << 28;

        // Connect cpu to mmu-cache
        leon3->instrMem.initSocket(mmu_cache_inst->icio);
        leon3->dataMem.initSocket(mmu_cache_inst->dcio);

        // History logging
        std::string history = p_proc_history;
        if(!history.empty()) {
          leon3->enableHistory(history + boost::lexical_cast<std::string>(i));
        }

        connect(irqmp.irq_req, leon3->IRQ_port.irq_signal, i);
        connect(leon3->irqAck.initSignal, irqmp.irq_ack, i);
        connect(leon3->irqAck.run, irqmp.cpu_rst, i);
        connect(leon3->irqAck.status, irqmp.cpu_stat, i);

        // GDBStubs
        //if(p_gdb_en && p_gdb_proc == i) {
        if(p_gdb_en) {
          GDBStub<uint32_t> *gdbStub = new GDBStub<uint32_t>(*(leon3->abiIf));
          leon3->toolManager.addTool(*gdbStub);
          gdbStub->initialize(p_gdb_port + i);
          //leon3->instrMem.setDebugger(gdbStub);
          //leon3->dataMem.setDebugger(gdbStub);
        }
        // OS Emulator
        // ===========
        // is activating the leon traps to map basic io functions to the host system
        // set_brk, open, read, ...
        if(!((std::string)p_system_osemu).empty()) {
          v::warn << "OSEmu" << "content " << p_system_osemu << v::endl;
          if(boost::filesystem::exists(boost::filesystem::path((std::string)p_system_osemu))) {
            v::warn << "OSEmu" << "Enabled" << v::endl;
            OSEmulator< unsigned int> *osEmu = new OSEmulator<unsigned int>(*(leon3->abiIf));
            osEmu->initSysCalls(p_system_osemu);
            std::vector<std::string> options;
            options.push_back(p_system_osemu);
            if(vm.count("argument")) {
              std::vector<std::string> argvec = vm["argument"].as<std::vector<std::string> >();
              for(std::vector<std::string>::iterator iter = argvec.begin(); iter != argvec.end(); iter++) {
                options.push_back(*iter);
              }
            }
            //OSEmulator<unsigned int>::set_program_args(options);
            osEmu->set_program_args(options);
            leon3->toolManager.addTool(*osEmu);
          } else {
            v::warn << "main" << "File " << p_system_osemu << " not found!" << v::endl;
            exit(1);
          }
        }
      }
*/
    }
    ~Leon3mpPlatform() {
    }

    mmu_cache mmu_cache;
    leon3_funclt_trap::Processor_leon3_funclt *leon3lt;
    leon3_funcat_trap::Processor_leon3_funcat *leon3at;

    void clkcng() {
    }

    void dorst() {
    }

};
#endif  // PLATFORM_BASE_LEON3PROCESSOR_H_
///@}

