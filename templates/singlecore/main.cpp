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
// Title:      main.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Top level file (sc_main) for system test.
//             THIS FILE IS AUTOMATIC GENERATED.
//             CHANGES MIGHT GET LOST!!
//
// Method:
//
// Modified on $Date: 2011-02-24 10:53:41 +0100 (Thu, 24 Feb 2011) $
//          at $Revision: 385 $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#include <systemc.h>
#include <tlm.h>

#include "processor.hpp"
#include <execLoader.hpp>
#include <osEmulator.hpp>
#include "mmu_cache.h"
#include "genericmemory.h"
#include "apbctrl.h"
#include "mctrl.h"
#include "defines.h"
#include "gptimer.h"
#include "irqmp.h"
#include "ahbctrl.h"
#include "AHB2Socwire.h"

#include <iostream>
#include <vector>
#include <sys/time.h>
#include <time.h>
#include <amba.h>
#include <cstring>
#include "verbose.h"
#include "power_monitor.h"

#include "config.h"
#include <GDBStub.hpp>
#include "ahbmem.h"
using namespace std;
using namespace sc_core;

#if conf_mmu_cache_icen == false
#  define conf_mmu_cache_icen_repl 0
#  define conf_mmu_cache_icen_sets 0
#  define conf_mmu_cache_icen_linesize 0
#  define conf_mmu_cache_icen_setsize 0
#  define conf_mmu_cache_icen_setlock 0
#endif

#if conf_mmu_cache_dcen == false
#  define conf_mmu_cache_dcen_repl 0
#  define conf_mmu_cache_dcen_sets 0
#  define conf_mmu_cache_dcen_linesize 0
#  define conf_mmu_cache_dcen_setsize 0
#  define conf_mmu_cache_dcen_setlock 0
#  define conf_mmu_cache_dcen_snoop 1
#endif

#if conf_mmu_cache_ilram == false
#  define conf_mmu_cache_ilram_size 0
#  define conf_mmu_cache_ilram_start 0
#endif

#if conf_mmu_cache_dlram == false
#  define conf_mmu_cache_dlram_size 0
#  define conf_mmu_cache_dlram_start 0
#endif

#if conf_mmu_cache_mmu_en == false
#  define conf_mmu_cache_mmu_en_itlb_num 8
#  define conf_mmu_cache_mmu_en_dtlb_num 8
#  define conf_mmu_cache_mmu_en_tlb_type 0
#  define conf_mmu_cache_mmu_en_tlb_rep 1
#  define conf_mmu_cache_mmu_en_mmupgsz 0
#endif

#ifndef conf_paramlist
#define conf_paramlist false
#endif

#define LOCAL_CLOCK 10
#define CACHE_MASTER_ID 0

namespace trap {
  extern int exitValue;
};

int sc_main(int argc, char** argv) {

    clock_t cstart, cend;
    char *sram_app, *prom_app;
    int app_argc = 0;
    bool gdb_en = false;
    bool paramlist = false;

    for(int i = 1; i < argc; i++) {
        if(std::strcmp("listparams", argv[i])==0) {
            paramlist = true;
            break;
        }
    }
    
    // Sort out arguments
    if(argc >= 3) {
       prom_app = argv[1];
       sram_app = argv[2];
       app_argc += 3;
       v::logApplication(sram_app);
    } else {
       v::error << "Please use: '" << argv[0] << " prom.elf sram.elf [gdb] [...]" << "' to define an application." << endl;
       return -1;
    }
    
    // Whant to connect to GDB
    gdb_en = ((argc>3) && (std::strcmp(argv[3],"gdb")==0));
    if(gdb_en) {
        app_argc++;
    }

    // Decide weather LT or AT
    amba::amba_layer_ids ambaLayer;
    if(conf_sys_lt_at) {
        ambaLayer = amba::amba_LT;
    } else {
        ambaLayer = amba::amba_AT;
    }
    
    // *** CREATE MODULES

    // CREATE AHBCTRL unit
    // ===================
    // Always enabled.
    // Needed for basic platform
    AHBCtrl ahbctrl("ahbctrl",
		    conf_ahbctrl_ioaddr,                // The MSB address of the I/O area
		    conf_ahbctrl_iomask,                // The I/O area address mask
		    conf_ahbctrl_cfgaddr,               // The MSB address of the configuration area
		    conf_ahbctrl_cfgmask,               // The address mask for the configuration area
		    conf_ahbctrl_rrobin,                // 1 - round robin, 0 - fixed priority arbitration (only AT)
		    conf_ahbctrl_split,                 // Enable support for AHB SPLIT response (only AT)
		    conf_ahbctrl_defmast,               // Default AHB master
		    conf_ahbctrl_ioen,                  // AHB I/O area enable
		    conf_ahbctrl_fixbrst,               // Enable support for fixed-length bursts (disabled)
		    conf_ahbctrl_fpnpen,                // Enable full decoding of PnP configuration records
		    conf_ahbctrl_mcheck,                // Check if there are any intersections between core memory regions
        true, // Powermon
		    ambaLayer
    );
    // Set clock
    ahbctrl.set_clk(LOCAL_CLOCK,SC_NS);
    
    // CREATE LEON3 Processor
    // ===================================================
    // Always enabled.
    // Needed for basic platform.

    // For each Abstraction is another model needed
    // Unfortuantely they use the same includes so we have to build them new per abstraction
#if conf_sys_lt_at
    leon3_funclt_trap::Processor_leon3_funclt leon3("leon3", sc_core::sc_time(1, SC_US));
#else
    leon3_funcat_trap::Processor_leon3_funcat leon3("leon3", sc_core::sc_time(1, SC_US));
#endif

    // CREATE MMU_CACHE
    // ================
    // Always enabled.
    // Needed for basic platform.
    mmu_cache mmu_cache(
            conf_mmu_cache_icen,            //  int icen = 1 (icache enabled)
            conf_mmu_cache_icen_repl,       //  int irepl = 0 (icache LRU replacement)
            conf_mmu_cache_icen_sets,       //  int isets = 4 (4 instruction cache sets)
            conf_mmu_cache_icen_linesize,   //  int ilinesize = 4 (4 words per icache line)
            conf_mmu_cache_icen_setsize,    //  int isetsize = 16 (16kB per icache set)
            conf_mmu_cache_icen_setlock,    //  int isetlock = 1 (icache locking enabled)
            conf_mmu_cache_dcen,            //  int dcen = 1 (dcache enabled)
            conf_mmu_cache_dcen_repl,       //  int drepl = 2 (dcache random replacement)
            conf_mmu_cache_dcen_sets,       //  int dsets = 2 (2 data cache sets)
            conf_mmu_cache_dcen_linesize,   //  int dlinesize = 4 (4 word per dcache line)
            conf_mmu_cache_dcen_setsize,    //  int dsetsize = 1 (1kB per dcache set)
            conf_mmu_cache_dcen_setlock,    //  int dsetlock = 1 (dcache locking enabled)
            conf_mmu_cache_dcen_snoop,      //  int dsnoop = 1 (dcache snooping enabled)
            conf_mmu_cache_ilram,           //  int ilram = 0 (instr. localram disable)
            conf_mmu_cache_ilram_size,      //  int ilramsize = 0 (1kB ilram size)
            conf_mmu_cache_ilram_start,     //  int ilramstart = 8e (0x8e000000 default ilram start address)
            conf_mmu_cache_dlram,           //  int dlram = 0 (data localram disable)
            conf_mmu_cache_dlram_size,      //  int dlramsize = 0 (1kB dlram size)
            conf_mmu_cache_dlram_start,     //  int dlramstart = 8f (0x8f000000 default dlram start address)
            conf_mmu_cache_cached?0xFFFF:0, //  int cached = 0xffff (fixed cacheability mask)
            conf_mmu_cache_mmu_en,          //  int mmu_en = 0 (mmu not present)
            conf_mmu_cache_mmu_en_itlb_num, //  int itlb_num = 8 (8 itlbs - not present)
            conf_mmu_cache_mmu_en_dtlb_num, //  int dtlb_num = 8 (8 dtlbs - not present)
            conf_mmu_cache_mmu_en_tlb_type, //  int tlb_type = 0 (split tlb mode - not present)
            conf_mmu_cache_mmu_en_tlb_rep,  //  int tlb_rep = 1 (random replacement)
            conf_mmu_cache_mmu_en_mmupgsz,  //  int mmupgsz = 0 (4kB mmu page size)>
            "mmu_cache",                    // name of sysc module
		        //conf_mmu_cache_addr,          // The MSB address of the AHB area. Sets the 12 MSBs of the AHB address
		        //conf_mmu_cache_mask,          // The 12bit AHB area address mask
            //conf_mmu_cache_dsu,           // Enable debug support unit interface
            conf_mmu_cache_index,           // Id of the AHB master
            false,                          // Power Monitor,
	          ambaLayer                       // LT abstraction
    );
    
    // Connecting AHB Master
    mmu_cache.ahb(ahbctrl.ahbIN);
    // Connecting Testbench

    // Connect cpu to mmu-cache
    leon3.instrMem.initSocket(mmu_cache.icio);
    leon3.dataMem.initSocket(mmu_cache.dcio);
   
    // Set clock
    mmu_cache.set_clk(LOCAL_CLOCK,SC_NS);
    
    // CREATE AHB APB BRIDGE
    // =====================
    APBCtrl apbctrl("apbctrl", 
		    conf_apbctrl_ioaddr,                // The MSB address of the AHB area. Sets the 12 MSBs of the AHB address
		    conf_apbctrl_iomask,                // The 12bit AHB area address mask
		    conf_apbctrl_check,                 // Check if there are any intersections between APB slave memory regions 
        conf_apbctrl_index,
		    ambaLayer
    );
    // Connecting AHB Slave
    ahbctrl.ahbOUT(apbctrl.ahb);
    // Set clock
    apbctrl.set_clk(LOCAL_CLOCK,SC_NS);

    // CREATE MEMORY CONTROLLER
    // ========================
    Mctrl mctrl(
        "mctrl", 
        conf_memctrl_prom_asel, 
        conf_memctrl_ram_asel, 
        conf_memctrl_prom_addr, 
        conf_memctrl_prom_mask, 
        conf_memctrl_io_addr,
        conf_memctrl_io_mask, 
        conf_memctrl_ram_addr, 
        conf_memctrl_ram_mask, 
        conf_memctrl_apb_addr, 
        conf_memctrl_apb_mask, 
        conf_memctrl_ram_wprot, 
        conf_memctrl_ram_s_banks,
        conf_memctrl_ram8,
        conf_memctrl_ram16, 
        conf_memctrl_sepbus, 
        conf_memctrl_sdbits, 
        conf_memctrl_mobile, 
        conf_memctrl_sden, 
        conf_memctrl_ahb_index, 
        conf_memctrl_apb_index, 
        false // <-- powermon
    );
    
    // Connecting AHB Slave
    ahbctrl.ahbOUT(mctrl.ahb);
    // Connecting APB Slave
    apbctrl.apb(mctrl.apb);
    // Set clock
    mctrl.set_clk(LOCAL_CLOCK,SC_NS);

    // CREATE MEMORIES
    // ===============                                                  
    GenericMemory rom(
        "rom", 
        MEMDevice::ROM, 
        conf_memctrl_prom_banks, 
        conf_memctrl_prom_bsize * 1024 * 1024, 
        conf_memctrl_prom_width, 
        0
    );
    GenericMemory io(
        "io", 
        MEMDevice::IO, 
        conf_memctrl_prom_banks, 
        conf_memctrl_prom_bsize * 1024 * 1024, 
        conf_memctrl_prom_width, 
        0
    );
    GenericMemory sram(
        "sram", 
        MEMDevice::SRAM, 
        conf_memctrl_ram_s_banks, 
        conf_memctrl_ram_s_bsize * 1024 * 1024, 
        conf_memctrl_ram_s_width, 
        0
    );
    GenericMemory sdram(
        "sdram", 
        MEMDevice::SDRAM, 
        conf_memctrl_ram_sd_banks, 
        conf_memctrl_ram_sd_bsize * 1024 * 1024, 
        conf_memctrl_ram_sd_width, 
        conf_memctrl_ram_sd_cols
    );
    
    // connect memories to memory controller
    mctrl.mem(rom.bus);
    mctrl.mem(io.bus);
    mctrl.mem(sram.bus);
    mctrl.mem(sdram.bus);
    
    //AHBMemem ahb_mem("AHBMEM", 0x0, 0x800);
    //ahbctrl.ahbOUT(ahb_mem.ahb);
    
    // * ELF Loader ****************************
    // ELF loader from leon (Trap-Gen)
    // Loads the application into the memmory.
    // Initialize memory
    uint8_t *execData;
    ExecLoader sdram_loader(sram_app); 
    execData = sdram_loader.getProgData();
    for(unsigned int i = 0; i < sdram_loader.getProgDim(); i++) {
       sdram.write(sdram_loader.getDataStart() + i - ((conf_memctrl_ram_addr&conf_memctrl_ram_mask)<<20), execData[i]);
    }
    
    //leon3.ENTRY_POINT   = sdram_loader.getProgStart();
    leon3.PROGRAM_LIMIT = sdram_loader.getProgDim() + sdram_loader.getDataStart();
    leon3.PROGRAM_START = sdram_loader.getDataStart();
    ExecLoader prom_loader(prom_app); 
    execData = prom_loader.getProgData();
    
    for(unsigned int i = 0; i < prom_loader.getProgDim(); i++) {
       rom.write(prom_loader.getDataStart() + i - ((conf_memctrl_prom_addr&conf_memctrl_prom_mask)<<20), execData[i]);
       //ahb_mem.writeByteDBG(prom_loader.getDataStart() + i, execData[i]);
       //v::debug << "sc_main" << "Write to PROM: Addr: " << v::uint32 << prom_loader.getDataStart() + i << " Data: " << v::uint8 << (uint32_t)execData[i] << v::endl;
    }
    leon3.ENTRY_POINT   = prom_loader.getProgStart();
    //leon3.PROGRAM_LIMIT = prom_loader.getProgDim() + prom_loader.getDataStart();
    //leon3.PROGRAM_START = prom_loader.getDataStart();
    //leon3.ENTRY_POINT   = 0;
    //leon3.PROGRAM_LIMIT = 0;
    //leon3.PROGRAM_START = 0;
    
    //assert((sram_loader.getProgDim() + sram_loader.getDataStart()) < 0x1fffffff);
    // ******************************************
    
#if conf_ahbmem != 0
    AHBMem ahbmem("ahbmem",
        conf_ahbmem_addr,
        conf_ahbmem_mask,
        ambaLayer,
        conf_ahbmem_index
    );
    ahbctrl.ahbOUT(ahbmem.ahb);
#endif
    
    // * IRQMP **********************************
    // CREATE IRQ controller
    // =====================
    // Needed for basic platform.
    // Always enabled
    Irqmp irqmp("irqmp",
        conf_irqmp_addr,  // paddr
        conf_irqmp_mask,  // pmask
        conf_irqmp_ncpu,  // ncpu
        conf_irqmp_eirq,  // eirq
        conf_irqmp_index
    );
    // Connecting APB Slave
    apbctrl.apb(irqmp.apb_slv);
    // Set clock
    irqmp.set_clk(LOCAL_CLOCK,SC_NS);
    connect(irqmp.irq_req, leon3.IRQ_port.irq_signal, 0);
    connect(leon3.irqAck.initSignal, irqmp.irq_ack, 0);
    // ******************************************
    
    // * GPTimer ********************************
#if conf_gptimer != 0
    // CREATE GPTimer
    // ==============
    GPTimer gptimer("gptimer",
        conf_gptimer_ntimers,// ntimers
        conf_gptimer_index,  // index
        conf_gptimer_addr,   // paddr
        conf_gptimer_mask,   // pmask
        conf_gptimer_pirq,   // pirq
        conf_gptimer_sepirq, // sepirq
        conf_gptimer_sbits,  // sbits
        conf_gptimer_nbits,  // nbits
        conf_gptimer_wdog,   // wdog
        false                // powmon
    );
    // Connecting APB Slave
    apbctrl.apb(gptimer.bus);
    // Connecting Interrupts
    for(int i=0; i < 8; i++) {
      signalkit::connect(irqmp.irq_in, gptimer.irq, conf_gptimer_pirq + i);
    }
    // Set clock
    gptimer.set_clk(LOCAL_CLOCK,SC_NS);
#endif
    // ******************************************
    
    // * SoCWire ********************************
#if disconf_socwire != 0
    // CREATE AHB2Socwire bridge
    // =========================
    AHB2Socwire ahb2socwire("ahb2socwire",
        conf_socwire_apb_addr,                   // paddr
        conf_socwire_apb_mask,                   // pmask
        conf_socwire_apb_index,                  // pindex
        conf_socwire_apb_irq,                    // pirq
        conf_socwire_socw_index,                 // hindex
        amba::amba_LT                            // abstraction
    );
    
    // Connecting AHB Master
    ahb2socwire.ahb(ahbctrl.ahbIN);
    
    // Connecting APB Slave
    apbctrl.apb(ahb2socwire.apb);
    
    // Connecting Interrupts
    connect(irqmp.irqi_in, ahb2socwire.irq, conf_socwire_apb_irq);
    
    // Connect socwire ports as loopback
    ahb2socwire.master_socket(ahb2socwire.slave_socket);
#endif
    // ******************************************
    
    // * OS Emulator ****************************
    // OS Emulator is activating the leon traps to map basic io functions to the host system
    // set_brk, open, read, ...
    OSEmulator< unsigned int> osEmu(*(leon3.abiIf));
    osEmu.initSysCalls(sram_app);
    std::vector<std::string> options;
    options.push_back(sram_app);
    for(int i = app_argc; i < argc; i++) {
        options.push_back(argv[i]);
    }
    OSEmulatorBase::set_program_args(options);

    leon3.toolManager.addTool(osEmu);
    //leon3.enableHistory("cmdHistLTSys"); 
    // ******************************************
   
    // * GDBStubs *******************************
    if(gdb_en) {
        GDBStub<uint32_t> *gdbStub = new GDBStub<uint32_t>(*(leon3.abiIf));
        leon3.toolManager.addTool(*gdbStub);
        gdbStub->initialize(); 
        leon3.instrMem.setDebugger(gdbStub);
        leon3.dataMem.setDebugger(gdbStub);
    }
    // ******************************************

    // * Param Listing **************************
    v::info << "main" << "Param" << paramlist << v::endl;
    if(paramlist) {
        gs::cnf::cnf_api *CFG = gs::cnf::GCnf_Api::getApiInstance(NULL);
        v::info << "main" << "System Values:" << v::endl;
        std::vector<std::string> plist = CFG->getParamList();
        for(uint32_t i = 0; i < plist.size(); i++) {
            v::info << "main" << plist[i] << v::endl;
        }
    }
    // ******************************************

    // start simulation
    cstart = clock();
    sc_core::sc_start();
    cend = clock();
    // call power analyzer
    //if(conf_sys_power) {
    //    PM::analyze("./models/","main-power.dat","singlecore2.eslday1.stats");
    //}

    if(conf_sys_timing) {
        v::info << "Summary" << "Start: " << dec << cstart << v::endl;
        v::info << "Summary" << "End:   " << dec << cend << v::endl;
        v::info << "Summary" << "Delta: " << dec << setprecision(0) << ((double)(cend - cstart) / (double)CLOCKS_PER_SEC * 1000) << "ms" << v::endl;
    }
    return trap::exitValue;

}
