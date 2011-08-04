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
#include "generic_memory.h"
#include "apbctrl.h"
#include "mctrl.h"
#include "defines.h"
#include "gptimer.h"
#include "irqmp.h"
#include "ahbctrl.h"
#include "AHB2Socwire.h"

#include <iostream>
#include <amba.h>
#include "verbose.h"

#include "config.h"
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

#define LOCAL_CLOCK 10
#define CACHE_MASTER_ID 0

int sc_main(int argc, char** argv) {

    char *sram_app, *prom_app;
    if(argc == 3) {
       prom_app = argv[1];
       sram_app = argv[2];
    } else {
       v::error << "Please use: '" << argv[0] << " [prom.elf] [sram.elf]" << "' to define an application." << endl;
       return -1;
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
		    amba::amba_LT
    );
    // Set clock
    ahbctrl.clk(LOCAL_CLOCK,SC_NS);
    
    // CREATE LEON3 Processor
    // ===================================================
    // Always enabled.
    // Needed for basic platform.
    leon3_funclt_trap::Processor_leon3_funclt leon3("leon3", sc_core::sc_time(1, SC_US));


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
            CACHE_MASTER_ID,                // - id of the AHB master
            false,                          // Power Monitor
	          amba::amba_LT                   // LT abstraction
    );
    
    // Connecting AHB Master
    mmu_cache.ahb_master(ahbctrl.ahbIN);
    // Connecting Testbench

    // Connect cpu to mmu-cache
    leon3.instrMem.initSocket(mmu_cache.icio);
    leon3.dataMem.initSocket(mmu_cache.dcio);
   
    // Set clock
    mmu_cache.clk(LOCAL_CLOCK,SC_NS);
    
    // CREATE AHB APB BRIDGE
    // =====================
    APBCtrl apbctrl("apbctrl", 
		    conf_apbctrl_ioaddr,                // The MSB address of the AHB area. Sets the 12 MSBs of the AHB address
		    conf_apbctrl_iomask,                // The 12bit AHB area address mask
		    //conf_apbctrl_cfgaddr,             // The MSB address of the AHB area. Sets the 12 MSBs of the AHB address
		    //conf_apbctrl_cfgmask,             // The 12bit AHB area address mask
		    conf_apbctrl_check,                 // Check if there are any intersections between APB slave memory regions 
		    amba::amba_LT
    );
    // Connecting AHB Slave
    ahbctrl.ahbOUT(apbctrl.ahb);
    // Set clock
    apbctrl.clk(LOCAL_CLOCK,SC_NS);

    // CREATE MEMORY CONTROLLER
    // ========================
    const int romasel = conf_memctrl_prom_asel;
    const int sdrasel = conf_memctrl_ram_asel;
    const int romaddr = conf_memctrl_prom_addr;
    const int rommask = conf_memctrl_prom_mask;
    const int ioaddr  = conf_memctrl_io_addr;
    const int iomask  = conf_memctrl_io_mask;
    const int ramaddr = conf_memctrl_ram_addr;
    const int rammask = conf_memctrl_ram_mask;
    const int paddr   = conf_memctrl_apb_addr;
    const int pmask   = conf_memctrl_apb_mask;
    const int wprot   = conf_memctrl_ram_wprot;
    const int srbanks = 4;
    const int ram8    = conf_memctrl_ram8;
    const int ram16   = conf_memctrl_ram16;
    const int sepbus  = conf_memctrl_sepbus;
    const int sdbits  = conf_memctrl_sdbits;
    const int mobile  = conf_memctrl_mobile;
    const int sden    = conf_memctrl_sden;

    //instantiate mctrl, generic memory, and testbench
    Mctrl mctrl("mctrl_inst0", romasel, sdrasel, romaddr, rommask, ioaddr,
            iomask, ramaddr, rammask, paddr, pmask, wprot, srbanks, ram8,
            ram16, sepbus, sdbits, mobile, sden
    );

    // CREATE MEMORIES
    // ===============
    Generic_memory<uint8_t>  generic_memory_rom("generic_memory_rom");
    Generic_memory<uint32_t> generic_memory_io("generic_memory_io");
    Generic_memory<uint8_t>  generic_memory_sram("generic_memory_sram");
    Generic_memory<uint32_t> generic_memory_sdram("generic_memory_sdram");
    // Connecting AHB Slave
    ahbctrl.ahbOUT(mctrl.ahb);
    // Connecting APB Slave
    apbctrl.apb(mctrl.apb);
    // connect memories to memory controller
    mctrl.mctrl_rom(generic_memory_rom.slave_socket);
    mctrl.mctrl_io(generic_memory_io.slave_socket);
    mctrl.mctrl_sram(generic_memory_sram.slave_socket);
    mctrl.mctrl_sdram(generic_memory_sdram.slave_socket);
    // Set clock
    mctrl.clk(LOCAL_CLOCK,SC_NS);
    // * ELF Loader ****************************
    // ELF loader from leon (Trap-Gen)
    // Loads the application into the memmory.
    // Initialize memory
    ExecLoader prom_loader(prom_app); 
    unsigned char* execData = prom_loader.getProgData();
    for(unsigned int i = 0; i < prom_loader.getProgDim(); i++) {
       generic_memory_rom.writeByteDBG(prom_loader.getDataStart() + i, execData[i]);
    }
    leon3.ENTRY_POINT   = prom_loader.getProgStart();
    //leon3.PROGRAM_LIMIT = prom_loader.getProgDim() + prom_loader.getDataStart();
    //leon3.PROGRAM_START = prom_loader.getDataStart();
    
    ExecLoader sram_loader(sram_app); 
    execData = sram_loader.getProgData();
    for(unsigned int i = 0; i < sram_loader.getProgDim(); i++) {
       generic_memory_sram.writeByteDBG(sram_loader.getDataStart() + i, execData[i]);
    }
    //leon3.ENTRY_POINT   = sram_loader.getProgStart();
    leon3.PROGRAM_LIMIT = sram_loader.getProgDim() + sram_loader.getDataStart();
    leon3.PROGRAM_START = sram_loader.getDataStart();
    assert((sram_loader.getProgDim() + sram_loader.getDataStart()) < 0x1fffffff);
    // ******************************************
    // * GPTimer ********************************
#if gptimer != 0
    // CREATE GPTimer
    // ==============
    CGPTimer u_gptimer("u_gptimer",
        conf_gptimer_ntimers,  // ntimers
        conf_gptimer_addr,  // gpaddr
        conf_gptimer_mask,  // gpmask
        conf_gptimer_pirq,  // gpirq
        conf_gptimer_sepirq,  // gsepirq
        conf_gptimer_sbits,  // gsbits
        conf_gptimer_nbits, // gnbits
        conf_gptimer_wdog // wdog
    );
    // Connecting APB Slave
    apbctrl.apb(u_gptimer.bus);
    // Connecting Interrupts
    //u_irqmp.irq_in(u_gptimer.irq, 0);
    // Set clock
    u_gptimer.clk(LOCAL_CLOCK,SC_NS);
#endif
    // ******************************************
    // * IRQMP **********************************
    // CREATE IRQ controller
    // =====================
    // Needed for basic platform.
    // Always enabled
    CIrqmp u_irqmp("u_irqmp",
        conf_irqmp_addr,  // paddr
        conf_irqmp_mask,  // pmask
        conf_irqmp_ncpu,  // ncpu
        conf_irqmp_eirq   // eirq
    );
    // Connecting APB Slave
    apbctrl.apb(u_irqmp.apb_slv);
    // Set clock
    u_irqmp.clk(LOCAL_CLOCK,SC_NS);
    // ******************************************
    // * SoCWire ********************************
#if socwire != 0
    // CREATE AHB2Socwire bridge
    // =========================
    CAHB2Socwire u_ahb2socwire("u_ahb2socwire",
                               conf_socwire_apb_addr,  // paddr
                               conf_socwire_apb_mask,  // pmask
                               conf_socwire_apb_irq,  // pirq
                               amba::amba_LT,
                               conf_socwire_socw_datawidth,  // data width
                               conf_socwire_socw_speed,  // sw speed
                               conf_socwire_socw_after64,  // short timeout
                               conf_socwire_socw_after128, // long timeout
                               conf_socwire_socw_disconnect_detection); // disconnect timeout
    // Connecting AHB Master
    u_ahb2socwire.ahb(ahbctrl.ahbIN);
    // Connecting APB Slave
    apbctrl.apb(u_ahb2socwire.apb);
    // Connecting Interrupts
    //u_irqmp.irq_in(u_ahb2socwire.irq, 1);
    // Connect socwire ports as loopback
    u_ahb2socwire.master_socket(u_ahb2socwire.slave_socket);
#endif
    // ******************************************
    
    // * OS Emulator ****************************
    // OS Emulator is activating the leon traps to map basic io functions to the host system
    // set_brk, open, read, ...
    OSEmulator< unsigned int> osEmu(*(leon3.abiIf));
    osEmu.initSysCalls(sram_app);
    std::vector<std::string> options;
    options.push_back(sram_app);
    OSEmulatorBase::set_program_args(options);

    leon3.toolManager.addTool(osEmu);
    leon3.enableHistory("cmdHistLTSys"); 
    // ******************************************
    
    // start simulation
    sc_core::sc_start();


    //cout << "************* Test case summary **************" << endl;
    //cout << " Error count: " << dec << tb.get_error_count() << endl;
    //cout << " Pass  count: " << dec << tb.get_pass_count() << endl;
    //if(!tb.get_error_count()) {
    //   cout << " Test case passed :)" << endl;
    //} else {
    //   cout << " Test case failed :(" << endl;
    //}
    //cout << "**********************************************" << endl;

    return 0; //tb.get_error_count();

}

