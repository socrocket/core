// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup platform
/// @{
/// @file sc_main.cpp
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///
#ifdef HAVE_USI
#include "pysc/usi.h"
#endif

#include "core/common/sr_param.h"
#include "core/common/systemc.h"
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include "core/common/amba.h"
#include "core/common/trapgen/debugger/GDBStub.hpp"
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

#include "core/common/verbose.h"
#include "gaisler/leon3/leon3.h"
#include "gaisler/ahbin/ahbin.h"
#include "gaisler/memory/memory.h"
#include "gaisler/apbctrl/apbctrl.h"
#include "gaisler/ahbmem/ahbmem.h"
#include "gaisler/mctrl/mctrl.h"
#include "gaisler/leon3/mmucache/defines.h"
#include "gaisler/gptimer/gptimer.h"
#include "gaisler/apbuart/apbuart.h"
#include "gaisler/apbuart/tcpio.h"
#include "gaisler/apbuart/reportio.h"
#include "gaisler/irqmp/irqmp.h"
#include "gaisler/ahbctrl/ahbctrl.h"
#include "gaisler/ahbprof/ahbprof.h"
#include <boost/filesystem.hpp>

#ifdef HAVE_SOCWIRE
#include "models/socwire/AHB2Socwire.h"
#endif
#ifdef HAVE_GRETH
#include "models/greth/greth/greth.h"
#include "vphy/tapdev.h"
#include "vphy/loopback.h"
#endif
#ifdef HAVE_AHBDISPLAY
#include "models/ahbdisplay/ahbdisplay.h"
#endif
#ifdef HAVE_AHBCAMERA
#include "models/ahbcamera/ahbcamera.h"
#endif
#ifdef HAVE_AHBSHUFFLER
#include "models/ahbshuffler/ahbshuffler.h"
#endif
#ifdef HAVE_AHBGPGPU
#include "ahbgpgpu/models/nyuzi/nyuzi.h"
#endif /* #ifdef HAVE_AHBGPGPU */

//#include "vphy/trafgen.h"

using namespace std;
using namespace sc_core;
#ifdef HAVE_SOCWIRE
using namespace socw;
#endif

extern int exitValue;

void stopSimFunction(int sig) {
  v::warn << "main" << "Simulation interrupted by user" << std::endl;
  sc_core::sc_stop();
  wait(SC_ZERO_TIME);
}

string greth_script_path;
void grethVPHYHook(char* dev_name)
{
  stringstream command;
  command << greth_script_path.c_str() << " " << dev_name << " &"; // Do not wait
  v::info << "GREth Post Script Path: " << command.str() << v::endl;
  string command_str = command.str();
  const char* command_c = command_str.c_str();

  int res = std::system(command_c);
  if(res<0)
  {
    v::error << "GREth Post Script Path: " << command << v::endl;
  }
  v::info << "GREth Post Script PID: " << res << v::endl;

  return;
}

class irqmp_rst_stimuli : sc_core::sc_module {
  public:
    sr_signal::signal_out<bool, Irqmp> irqmp_rst;
    irqmp_rst_stimuli(sc_core::sc_module_name mn) :
        sc_core::sc_module(mn),
        irqmp_rst("rst") {

    }
    void start_of_simulation() {
      irqmp_rst.write(0);
      irqmp_rst.write(1);
    }
};

int sc_main(int argc, char** argv) {
    clock_t cstart, cend;
    std::string prom_app;
    sr_report_handler::handler = sr_report_handler::default_handler;

    gs::ctr::GC_Core       core;
    gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
    gs::cnf::ConfigPlugin configPlugin(&cnfdatabase);

    SR_INCLUDE_MODULE(ArrayStorage);
    SR_INCLUDE_MODULE(MapStorage);
    SR_INCLUDE_MODULE(ReportIO);
    SR_INCLUDE_MODULE(TcpIO);


#ifdef HAVE_USI
    // Initialize Python
    USI_HAS_MODULE(systemc);
    USI_HAS_MODULE(sr_registry);
    USI_HAS_MODULE(delegate);
    USI_HAS_MODULE(intrinsics);
    USI_HAS_MODULE(greensocket);
    USI_HAS_MODULE(scireg);
    USI_HAS_MODULE(amba);
    USI_HAS_MODULE(sr_report);
    USI_HAS_MODULE(sr_signal);
    USI_HAS_MODULE(cci);
    USI_HAS_MODULE(mtrace);
    usi_init(argc, argv);
    //sr_report_handler::handler = sr_report_handler::default_handler; <<-- Uncoment for C++ handler


    // Core APIs will be loaded by usi_init:
    // usi, usi.systemc, usi.api.delegate, usi.api.report
    usi_load("usi.api.greensocket");
    usi_load("sr_register.scireg");
    usi_load("sr_signal.sr_signal");
    usi_load("usi.api.amba");

    usi_load("usi.log");
    usi_load("usi.tools.args");
    usi_load("usi.cci");
    //usi_load("tools.python.power");
    usi_load("usi.shell");
    usi_load("usi.tools.execute");
    usi_load("usi.tools.elf");

    usi_start_of_initialization();
#endif  // HAVE_USI

    // Build GreenControl Configuration Namespace
    // ==========================================
    gs::gs_param_array p_conf("conf");
    gs::gs_param_array p_system("system", p_conf);

    // Decide whether LT or AT
    gs::gs_param<bool> p_system_at("at", false, p_system);
    gs::gs_param<unsigned int> p_system_ncpu("ncpu", 1, p_system);
    gs::gs_param<unsigned int> p_system_clock("clk", 10.0, p_system);
    gs::gs_param<std::string> p_system_osemu("osemu", "", p_system);
    gs::gs_param<std::string> p_system_log("log", "", p_system);

    gs::gs_param_array p_report("report", p_conf);
    gs::gs_param<bool> p_report_timing("timing", true, p_report);
    gs::gs_param<bool> p_report_power("power", true, p_report);
/*
    if(!((std::string)p_system_log).empty()) {
        v::logApplication((char *)((std::string)p_system_log).c_str());
    }
*/
    amba::amba_layer_ids ambaLayer;
    if(p_system_at) {
        ambaLayer = amba::amba_AT;
    } else {
        ambaLayer = amba::amba_LT;
    }

    // *** CREATE MODULES

    // AHBCtrl
    // =======
    // Always enabled.
    // Needed for basic platform
    gs::gs_param_array p_ahbctrl("ahbctrl", p_conf);
    gs::gs_param<unsigned int> p_ahbctrl_ioaddr("ioaddr", 0xFFF, p_ahbctrl);
    gs::gs_param<unsigned int> p_ahbctrl_iomask("iomask", 0xFFF, p_ahbctrl);
    gs::gs_param<unsigned int> p_ahbctrl_cfgaddr("cfgaddr", 0xFF0, p_ahbctrl);
    gs::gs_param<unsigned int> p_ahbctrl_cfgmask("cfgmask", 0xFF0, p_ahbctrl);
    gs::gs_param<bool> p_ahbctrl_rrobin("rrobin", false, p_ahbctrl);
    gs::gs_param<unsigned int> p_ahbctrl_defmast("defmast", 0u, p_ahbctrl);
    gs::gs_param<bool> p_ahbctrl_ioen("ioen", true, p_ahbctrl);
    gs::gs_param<bool> p_ahbctrl_fixbrst("fixbrst", false, p_ahbctrl);
    gs::gs_param<bool> p_ahbctrl_split("split", false, p_ahbctrl);
    gs::gs_param<bool> p_ahbctrl_fpnpen("fpnpen", true, p_ahbctrl);
    gs::gs_param<bool> p_ahbctrl_mcheck("mcheck", true, p_ahbctrl);

    AHBCtrl ahbctrl("ahbctrl",
		    p_ahbctrl_ioaddr,                // The MSB address of the I/O area
		    p_ahbctrl_iomask,                // The I/O area address mask
		    p_ahbctrl_cfgaddr,               // The MSB address of the configuration area
		    p_ahbctrl_cfgmask,               // The address mask for the configuration area
		    p_ahbctrl_rrobin,                // 1 - round robin, 0 - fixed priority arbitration (only AT)
		    p_ahbctrl_split,                 // Enable support for AHB SPLIT response (only AT)
		    p_ahbctrl_defmast,               // Default AHB master
		    p_ahbctrl_ioen,                  // AHB I/O area enable
		    p_ahbctrl_fixbrst,               // Enable support for fixed-length bursts (disabled)
		    p_ahbctrl_fpnpen,                // Enable full decoding of PnP configuration records
		    p_ahbctrl_mcheck,                // Check if there are any intersections between core memory regions
        p_report_power,                  // Enable/disable power monitoring
		    ambaLayer
    );

    // Set clock
    ahbctrl.set_clk(p_system_clock, SC_NS);

    // AHBSlave - APBCtrl
    // ==================

    gs::gs_param_array p_apbctrl("apbctrl", p_conf);
    gs::gs_param<unsigned int> p_apbctrl_haddr("haddr", 0x800, p_apbctrl);
    gs::gs_param<unsigned int> p_apbctrl_hmask("hmask", 0xFFF, p_apbctrl);
    gs::gs_param<unsigned int> p_apbctrl_index("hindex", 2u, p_apbctrl);
    gs::gs_param<bool> p_apbctrl_check("mcheck", true, p_apbctrl);

    APBCtrl apbctrl("apbctrl",
        p_apbctrl_haddr,    // The 12 bit MSB address of the AHB area.
        p_apbctrl_hmask,    // The 12 bit AHB area address mask
        p_apbctrl_check,    // Check for intersections in the memory map
        p_apbctrl_index,    // AHB bus index
        p_report_power,     // Power Monitoring on/off
		    ambaLayer           // TLM abstraction layer
    );

    // Connect to AHB and clock
    ahbctrl.ahbOUT(apbctrl.ahb);
    apbctrl.set_clk(p_system_clock, SC_NS);

    // APBSlave - IRQMP
    // ================
    // Needed for basic platform.
    // Always enabled

    gs::gs_param_array p_irqmp("irqmp", p_conf);
    gs::gs_param<unsigned int> p_irqmp_addr("addr", /*0x1F0*/0x2, p_irqmp); // SoCRocket default: 0x1F0, try to mimic TSIM therefore 0x2 -- psiegl
    gs::gs_param<unsigned int> p_irqmp_mask("mask", 0xFFF, p_irqmp);
    gs::gs_param<unsigned int> p_irqmp_index("index", 2, p_irqmp);
    gs::gs_param<unsigned int> p_irqmp_eirq("eirq", 0u, p_irqmp);

    Irqmp irqmp("irqmp",
                p_irqmp_addr,  // paddr
                p_irqmp_mask,  // pmask
                p_system_ncpu,  // ncpu
                p_irqmp_eirq,  // eirq
                p_irqmp_index, // pindex
                p_report_power // power monitoring
    );

    // Connect to APB and clock
    apbctrl.apb(irqmp.apb);
    irqmp.set_clk(p_system_clock,SC_NS);

    // AHBSlave - MCtrl, ArrayMemory
    // =============================
    gs::gs_param_array p_mctrl("mctrl", p_conf);
    gs::gs_param_array p_mctrl_apb("apb", p_mctrl);
    gs::gs_param_array p_mctrl_prom("prom", p_mctrl);
    gs::gs_param_array p_mctrl_io("io", p_mctrl);
    gs::gs_param_array p_mctrl_ram("ram", p_mctrl);
    gs::gs_param_array p_mctrl_ram_sram("sram", p_mctrl_ram);
    gs::gs_param_array p_mctrl_ram_sdram("sdram", p_mctrl_ram);
    gs::gs_param<unsigned int> p_mctrl_apb_addr("addr", 0x000u, p_mctrl_apb);
    gs::gs_param<unsigned int> p_mctrl_apb_mask("mask", 0xFFF, p_mctrl_apb);
    gs::gs_param<unsigned int> p_mctrl_apb_index("index", 0u, p_mctrl_apb);
    gs::gs_param<unsigned int> p_mctrl_prom_addr("addr", 0x000u, p_mctrl_prom);
    gs::gs_param<unsigned int> p_mctrl_prom_mask("mask", 0xE00, p_mctrl_prom);
    gs::gs_param<unsigned int> p_mctrl_prom_asel("asel", 28, p_mctrl_prom);
    gs::gs_param<unsigned int> p_mctrl_prom_banks("banks", 2, p_mctrl_prom);
    gs::gs_param<unsigned int> p_mctrl_prom_bsize("bsize", 2, p_mctrl_prom);
    gs::gs_param<unsigned int> p_mctrl_prom_width("width", 32, p_mctrl_prom);
    gs::gs_param<unsigned int> p_mctrl_io_addr("addr", 0x200, p_mctrl_io);
    gs::gs_param<unsigned int> p_mctrl_io_mask("mask", 0xE00, p_mctrl_io);
    gs::gs_param<unsigned int> p_mctrl_io_banks("banks", 1, p_mctrl_io);
    gs::gs_param<unsigned int> p_mctrl_io_bsize("bsize", 512, p_mctrl_io);
    gs::gs_param<unsigned int> p_mctrl_io_width("width", 32, p_mctrl_io);
    gs::gs_param<unsigned int> p_mctrl_ram_addr("addr", 0x400, p_mctrl_ram);
    gs::gs_param<unsigned int> p_mctrl_ram_mask("mask", 0xC00, p_mctrl_ram);
    gs::gs_param<bool> p_mctrl_ram_wprot("wprot", false, p_mctrl_ram);
    gs::gs_param<unsigned int> p_mctrl_ram_asel("asel", 29, p_mctrl_ram);
    gs::gs_param<unsigned int> p_mctrl_ram_sram_banks("banks", 4, p_mctrl_ram_sram);
    gs::gs_param<unsigned int> p_mctrl_ram_sram_bsize("bsize", 128, p_mctrl_ram_sram);
    gs::gs_param<unsigned int> p_mctrl_ram_sram_width("width", 32, p_mctrl_ram_sram);
    gs::gs_param<unsigned int> p_mctrl_ram_sdram_banks("banks", 2, p_mctrl_ram_sdram);
    gs::gs_param<unsigned int> p_mctrl_ram_sdram_bsize("bsize", 256, p_mctrl_ram_sdram);
    gs::gs_param<unsigned int> p_mctrl_ram_sdram_width("width", 32, p_mctrl_ram_sdram);
    gs::gs_param<unsigned int> p_mctrl_ram_sdram_cols("cols", 16, p_mctrl_ram_sdram);
    gs::gs_param<unsigned int> p_mctrl_index("index", 0u, p_mctrl);
    gs::gs_param<bool> p_mctrl_ram8("ram8", true, p_mctrl);
    gs::gs_param<bool> p_mctrl_ram16("ram16", true, p_mctrl);
    gs::gs_param<bool> p_mctrl_sden("sden", true, p_mctrl);
    gs::gs_param<bool> p_mctrl_sepbus("sepbus", false, p_mctrl);
    gs::gs_param<unsigned int> p_mctrl_sdbits("sdbits", 32, p_mctrl);
    gs::gs_param<unsigned int> p_mctrl_mobile("mobile", 0u, p_mctrl);
    Mctrl mctrl( "mctrl",
        p_mctrl_prom_asel,
        p_mctrl_ram_asel,
        p_mctrl_prom_addr,
        p_mctrl_prom_mask,
        p_mctrl_io_addr,
        p_mctrl_io_mask,
        p_mctrl_ram_addr,
        p_mctrl_ram_mask,
        p_mctrl_apb_addr,
        p_mctrl_apb_mask,
        p_mctrl_ram_wprot,
        p_mctrl_ram_sram_banks,
        p_mctrl_ram8,
        p_mctrl_ram16,
        p_mctrl_sepbus,
        p_mctrl_sdbits,
        p_mctrl_mobile,
        p_mctrl_sden,
        p_mctrl_index,
        p_mctrl_apb_index,
        p_report_power,
        ambaLayer
    );

    // Connecting AHB Slave
    ahbctrl.ahbOUT(mctrl.ahb);
    // Connecting APB Slave
    apbctrl.apb(mctrl.apb);
    // Set clock
    mctrl.set_clk(p_system_clock, SC_NS);

    // CREATE MEMORIES
    // ===============

    // ROM instantiation
    Memory rom( "rom",
                     MEMDevice::ROM,
                     p_mctrl_prom_banks,
                     p_mctrl_prom_bsize * 1024,
                     p_mctrl_prom_width,
                     0,
                     "ArrayStorage",
                     p_report_power
    );

    // Connect to memory controller and clock
    mctrl.mem(rom.bus);
    rom.set_clk(p_system_clock, SC_NS);

    // IO memory instantiation
    Memory io( "io",
               MEMDevice::IO,
               p_mctrl_prom_banks,
               p_mctrl_prom_bsize * 1024 * 1024,
               p_mctrl_prom_width,
               0,
               "MapStorage",
               p_report_power
    );

    // Connect to memory controller and clock
    mctrl.mem(io.bus);
    io.set_clk(p_system_clock, SC_NS);

    // ELF loader from leon (Trap-Gen)
    gs::gs_param<std::string> p_mctrl_io_elf("elf", "", p_mctrl_io);

    // SRAM instantiation
    Memory sram( "sram",
                 MEMDevice::SRAM,
                 p_mctrl_ram_sram_banks,
                 p_mctrl_ram_sram_bsize * 1024 * 1024,
                 p_mctrl_ram_sram_width,
                 0,
                 "MapStorage",
                 p_report_power
    );

    // Connect to memory controller and clock
    mctrl.mem(sram.bus);
    sram.set_clk(p_system_clock, SC_NS);

    // ELF loader from leon (Trap-Gen)
    gs::gs_param<std::string> p_mctrl_ram_sram_elf("elf", "", p_mctrl_ram_sram);

    // SDRAM instantiation
    Memory sdram( "sdram",
                       MEMDevice::SDRAM,
                       p_mctrl_ram_sdram_banks,
                       p_mctrl_ram_sdram_bsize * 1024 * 1024,
                       p_mctrl_ram_sdram_width,
                       p_mctrl_ram_sdram_cols,
                       "ArrayStorage",
                       p_report_power
    );

    // Connect to memory controller and clock
    mctrl.mem(sdram.bus);
    sdram.set_clk(p_system_clock, SC_NS);

    // ELF loader from leon (Trap-Gen)
    gs::gs_param<std::string> p_mctrl_ram_sdram_elf("elf", "", p_mctrl_ram_sdram);


    //leon3.ENTRY_POINT   = 0;
    //leon3.PROGRAM_LIMIT = 0;
    //leon3.PROGRAM_START = 0;

    // AHBSlave - AHBMem
    // =================
    gs::gs_param_array p_ahbmem("ahbmem", p_conf);
    gs::gs_param<bool> p_ahbmem_en("en", true, p_ahbmem);
    gs::gs_param<unsigned int> p_ahbmem_addr("addr", 0xA00, p_ahbmem);
    gs::gs_param<unsigned int> p_ahbmem_mask("mask", 0xFF0, p_ahbmem);
    gs::gs_param<unsigned int> p_ahbmem_index("index", 1, p_ahbmem);
    gs::gs_param<bool> p_ahbmem_cacheable("cacheable", 1, p_ahbmem);
    gs::gs_param<unsigned int> p_ahbmem_waitstates("waitstates", 0u, p_ahbmem);
    gs::gs_param<std::string> p_ahbmem_elf("elf", "", p_ahbmem);

    if(p_ahbmem_en) {

      AHBMem *ahbmem = new AHBMem("ahbmem",
                                  p_ahbmem_addr,
                                  p_ahbmem_mask,
                                  ambaLayer,
                                  p_ahbmem_index,
                                  p_ahbmem_cacheable,
                                  p_ahbmem_waitstates,
                                  p_report_power

      );

      // Connect to ahbctrl and clock
      ahbctrl.ahbOUT(ahbmem->ahb);
      ahbmem->set_clk(p_system_clock, SC_NS);
    }


    // AHBMaster - ahbin (input_device)
    // ================================
    gs::gs_param_array p_ahbin("ahbin", p_conf);
    gs::gs_param<bool> p_ahbin_en("en", false, p_ahbin);
    gs::gs_param<unsigned int> p_ahbin_index("index", 1, p_ahbin);
    gs::gs_param<unsigned int> p_ahbin_irq("irq", 5, p_ahbin);
    gs::gs_param<unsigned int> p_ahbin_framesize("framesize", 128, p_ahbin);
    gs::gs_param<unsigned int> p_ahbin_frameaddr("frameaddr", 0xA00, p_ahbin);
    gs::gs_param<unsigned int> p_ahbin_interval("interval", 1, p_ahbin);
    if(p_ahbin_en) {
        AHBIn *ahbin = new AHBIn("ahbin",
          p_ahbin_index,
          p_ahbin_irq,
          p_ahbin_framesize,
          p_ahbin_frameaddr,
          sc_core::sc_time(p_ahbin_interval, SC_MS),
          p_report_power,
          ambaLayer
      );

      // Connect sensor to bus
      ahbin->ahb(ahbctrl.ahbIN);
      ahbin->set_clk(p_system_clock, SC_NS);

      // Connect interrupt out
      sr_signal::connect(irqmp.irq_in, ahbin->irq, p_ahbin_irq);
    }

    // CREATE LEON3 Processor
    // ===================================================
    // Always enabled.
    // Needed for basic platform.
    gs::gs_param_array p_mmu_cache("mmu_cache", p_conf);
    gs::gs_param_array p_mmu_cache_ic("ic", p_mmu_cache);
    gs::gs_param<bool> p_mmu_cache_ic_en("en", true, p_mmu_cache_ic);
    gs::gs_param<int> p_mmu_cache_ic_repl("repl", 0, p_mmu_cache_ic);
    gs::gs_param<int> p_mmu_cache_ic_sets("sets", 1, p_mmu_cache_ic);
    gs::gs_param<int> p_mmu_cache_ic_linesize("linesize", 8, p_mmu_cache_ic);
    gs::gs_param<int> p_mmu_cache_ic_setsize("setsize", 4, p_mmu_cache_ic);
    gs::gs_param<bool> p_mmu_cache_ic_setlock("setlock", 1, p_mmu_cache_ic);
    gs::gs_param_array p_mmu_cache_dc("dc", p_mmu_cache);
    gs::gs_param<bool> p_mmu_cache_dc_en("en", true, p_mmu_cache_dc);
    gs::gs_param<int> p_mmu_cache_dc_repl("repl", 0, p_mmu_cache_dc);
    gs::gs_param<int> p_mmu_cache_dc_sets("sets", 1, p_mmu_cache_dc);
    gs::gs_param<int> p_mmu_cache_dc_linesize("linesize", 4, p_mmu_cache_dc);
    gs::gs_param<int> p_mmu_cache_dc_setsize("setsize", 4, p_mmu_cache_dc);
    gs::gs_param<bool> p_mmu_cache_dc_setlock("setlock", 1, p_mmu_cache_dc);
    gs::gs_param<bool> p_mmu_cache_dc_snoop("snoop", 1, p_mmu_cache_dc);
    gs::gs_param_array p_mmu_cache_ilram("ilram", p_mmu_cache);
    gs::gs_param<bool> p_mmu_cache_ilram_en("en", false, p_mmu_cache_ilram);
    gs::gs_param<unsigned int> p_mmu_cache_ilram_size("size", 0u, p_mmu_cache_ilram);
    gs::gs_param<unsigned int> p_mmu_cache_ilram_start("start", 0u, p_mmu_cache_ilram);
    gs::gs_param_array p_mmu_cache_dlram("dlram", p_mmu_cache);
    gs::gs_param<bool> p_mmu_cache_dlram_en("en", false, p_mmu_cache_dlram);
    gs::gs_param<unsigned int> p_mmu_cache_dlram_size("size", 0u, p_mmu_cache_dlram);
    gs::gs_param<unsigned int> p_mmu_cache_dlram_start("start", 0u, p_mmu_cache_dlram);
    gs::gs_param<unsigned int> p_mmu_cache_cached("cached", 0u, p_mmu_cache);
    gs::gs_param<unsigned int> p_mmu_cache_index("index", 0u, p_mmu_cache);
    gs::gs_param_array p_mmu_cache_mmu("mmu", p_mmu_cache);
    gs::gs_param<bool> p_mmu_cache_mmu_en("en", true, p_mmu_cache_mmu);
    gs::gs_param<unsigned int> p_mmu_cache_mmu_itlb_num("itlb_num", 8, p_mmu_cache_mmu);
    gs::gs_param<unsigned int> p_mmu_cache_mmu_dtlb_num("dtlb_num", 8, p_mmu_cache_mmu);
    gs::gs_param<unsigned int> p_mmu_cache_mmu_tlb_type("tlb_type", 1u, p_mmu_cache_mmu);
    gs::gs_param<unsigned int> p_mmu_cache_mmu_tlb_rep("tlb_rep", 1, p_mmu_cache_mmu);
    gs::gs_param<unsigned int> p_mmu_cache_mmu_mmupgsz("mmupgsz", 0u, p_mmu_cache_mmu);

    gs::gs_param<std::string> p_proc_history("history", "", p_system);

    gs::gs_param_array p_gdb("gdb", p_conf);
    gs::gs_param<bool> p_gdb_en("en", false, p_gdb);
    gs::gs_param<int> p_gdb_port("port", 1500, p_gdb);
    gs::gs_param<int> p_gdb_proc("proc", 0, p_gdb);
    Leon3 *first_leon = NULL;
    for(uint32_t i=0; i< p_system_ncpu; i++) {
      // AHBMaster - MMU_CACHE
      // =====================
      // Always enabled.
      // Needed for basic platform.
      Leon3 *leon3 = new Leon3(
              sc_core::sc_gen_unique_name("leon3", false), // name of sysc module
              p_mmu_cache_ic_en,         //  int icen = 1 (icache enabled)
              p_mmu_cache_ic_repl,       //  int irepl = 0 (icache LRU replacement)
              p_mmu_cache_ic_sets,       //  int isets = 4 (4 instruction cache sets)
              p_mmu_cache_ic_linesize,   //  int ilinesize = 4 (4 words per icache line)
              p_mmu_cache_ic_setsize,    //  int isetsize = 16 (16kB per icache set)
              p_mmu_cache_ic_setlock,    //  int isetlock = 1 (icache locking enabled)
              p_mmu_cache_dc_en,         //  int dcen = 1 (dcache enabled)
              p_mmu_cache_dc_repl,       //  int drepl = 2 (dcache random replacement)
              p_mmu_cache_dc_sets,       //  int dsets = 2 (2 data cache sets)
              p_mmu_cache_dc_linesize,   //  int dlinesize = 4 (4 word per dcache line)
              p_mmu_cache_dc_setsize,    //  int dsetsize = 1 (1kB per dcache set)
              p_mmu_cache_dc_setlock,    //  int dsetlock = 1 (dcache locking enabled)
              p_mmu_cache_dc_snoop,      //  int dsnoop = 1 (dcache snooping enabled)
              p_mmu_cache_ilram_en,      //  int ilram = 0 (instr. localram disable)
              p_mmu_cache_ilram_size,    //  int ilramsize = 0 (1kB ilram size)
              p_mmu_cache_ilram_start,   //  int ilramstart = 8e (0x8e000000 default ilram start address)
              p_mmu_cache_dlram_en,      //  int dlram = 0 (data localram disable)
              p_mmu_cache_dlram_size,    //  int dlramsize = 0 (1kB dlram size)
              p_mmu_cache_dlram_start,   //  int dlramstart = 8f (0x8f000000 default dlram start address)
              p_mmu_cache_cached,        //  int cached = 0xffff (fixed cacheability mask)
              p_mmu_cache_mmu_en,        //  int mmu_en = 0 (mmu not present)
              p_mmu_cache_mmu_itlb_num,  //  int itlb_num = 8 (8 itlbs - not present)
              p_mmu_cache_mmu_dtlb_num,  //  int dtlb_num = 8 (8 dtlbs - not present)
              p_mmu_cache_mmu_tlb_type,  //  int tlb_type = 0 (split tlb mode - not present)
              p_mmu_cache_mmu_tlb_rep,   //  int tlb_rep = 1 (random replacement)
              p_mmu_cache_mmu_mmupgsz,   //  int mmupgsz = 0 (4kB mmu page size)>
              p_mmu_cache_index + i,     // Id of the AHB master
              p_report_power,            // Power Monitor,
              ambaLayer                  // TLM abstraction layer
      );
      if(!first_leon) {
        first_leon = leon3;
      }

      // Connecting AHB Master
      leon3->ahb(ahbctrl.ahbIN);
      // Set clock
      leon3->set_clk(p_system_clock, SC_NS);
      connect(leon3->snoop, ahbctrl.snoop);

      // History logging
      std::string history = p_proc_history;
      if(!history.empty()) {
        leon3->g_history = history;
      }

      connect(irqmp.irq_req, leon3->cpu.IRQ_port.irq_signal, i);
      connect(leon3->cpu.irqAck.initSignal, irqmp.irq_ack, i);
      connect(leon3->cpu.irqAck.run, irqmp.cpu_rst, i);
      connect(leon3->cpu.irqAck.status, irqmp.cpu_stat, i);

      // GDBStubs
      if(p_gdb_en) {
        leon3->g_gdb = p_gdb_port;
      }
      // OS Emulator
      // ===========
      // is activating the leon traps to map basic io functions to the host system
      // set_brk, open, read, ...
      if(!((std::string)p_system_osemu).empty()) {
        leon3->g_osemu = p_system_osemu;
      }
    }

    // APBSlave - GPTimer
    // ==================
    gs::gs_param_array p_gptimer("gptimer", p_conf);
    gs::gs_param<bool> p_gptimer_en("en", true, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_ntimers("ntimers", 2, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_pindex("pindex", 3, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_paddr("paddr", 0x3, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_pmask("pmask", 0xfff, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_pirq("pirq", 8, p_gptimer);
    gs::gs_param<bool> p_gptimer_sepirq("sepirq", true, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_sbits("sbits", 16, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_nbits("nbits", 32, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_wdog("wdog", 1u, p_gptimer);

    if(p_gptimer_en) {
      GPTimer *gptimer = new GPTimer("gptimer",
        p_gptimer_ntimers,  // ntimers
        p_gptimer_pindex,   // index
        p_gptimer_paddr,    // paddr
        p_gptimer_pmask,    // pmask
        p_gptimer_pirq,     // pirq
        p_gptimer_sepirq,   // sepirq
        p_gptimer_sbits,    // sbits
        p_gptimer_nbits,    // nbits
        p_gptimer_wdog,     // wdog
        p_report_power      // powmon
      );

      // Connect to apb and clock
      apbctrl.apb(gptimer->apb);
      gptimer->set_clk(p_system_clock,SC_NS);

      // Connecting Interrupts
      for(int i=0; i < 8; i++) {
        sr_signal::connect(irqmp.irq_in, gptimer->irq, p_gptimer_pirq + i);
      }

    }

    // APBSlave - APBUart
    // ==================
    gs::gs_param_array p_apbuart("apbuart0", p_conf);
    gs::gs_param<bool> p_apbuart_en("en", false, p_apbuart);
    gs::gs_param<unsigned int> p_apbuart_index("index", 1, p_apbuart);
    gs::gs_param<unsigned int> p_apbuart_addr("addr", 0x001, p_apbuart);
    gs::gs_param<unsigned int> p_apbuart_mask("mask", 0xFFF, p_apbuart);
    gs::gs_param<unsigned int> p_apbuart_irq("irq", 2u, p_apbuart);
    gs::gs_param<unsigned int> p_apbuart_type("type", 1, p_apbuart);
    gs::gs_param<unsigned int> p_apbuart_port("port", 2000, p_apbuart);
    std::string uart_backend;
    //int port = (unsigned int)p_apbuart_port;
    //io_if *uart_io = NULL;
    if(p_apbuart_en) {
      switch(p_apbuart_type) {
        case 1:
          uart_backend = "TcpIO";
          break;
        default:
          uart_backend = "ReportIO";
          break;
      }

      APBUART *apbuart = new APBUART(sc_core::sc_gen_unique_name("apbuart0", true),
        uart_backend,
        p_apbuart_index,           // index
        p_apbuart_addr,            // paddr
        p_apbuart_mask,            // pmask
        p_apbuart_irq,             // pirq
        p_report_power   // powmon
      );

      // Connecting APB Slave
      apbctrl.apb(apbuart->apb);
      // Connecting Interrupts
      sr_signal::connect(irqmp.irq_in, apbuart->irq, p_apbuart_irq);
      // Set clock
      apbuart->set_clk(p_system_clock,SC_NS);
      // ******************************************
    }

    // APBSlave - APBUart
    // ==================
    gs::gs_param_array p_apbuart1("apbuart1", p_conf);
    gs::gs_param<bool> p_apbuart1_en("en", true, p_apbuart1);
    gs::gs_param<unsigned int> p_apbuart1_index("index", 9, p_apbuart1);
    gs::gs_param<unsigned int> p_apbuart1_addr("addr", 0x009, p_apbuart1);
    gs::gs_param<unsigned int> p_apbuart1_mask("mask", 0xFFF, p_apbuart1);
    gs::gs_param<unsigned int> p_apbuart1_irq("irq", 3, p_apbuart1); // 4???
    gs::gs_param<unsigned int> p_apbuart1_type("type", 0u, p_apbuart1);
    gs::gs_param<unsigned int> p_apbuart1_port("port", 3000, p_apbuart1);
    if(p_apbuart1_en) {
      switch(p_apbuart1_type) {
        case 1:
          uart_backend = "TcpIO";
          break;
        default:
          uart_backend = "ReportIO";
          break;
      }

      APBUART *apbuart1 = new APBUART(sc_core::sc_gen_unique_name("apbuart1", true),
        uart_backend,
        p_apbuart1_index,           // index
        p_apbuart1_addr,            // paddr
        p_apbuart1_mask,            // pmask
        p_apbuart1_irq,             // pirq
        p_report_power   // powmon
      );

      // Connecting APB Slave
      apbctrl.apb(apbuart1->apb);
      // Connecting Interrupts
      sr_signal::connect(irqmp.irq_in, apbuart1->irq, p_apbuart1_irq);
      // Set clock
      apbuart1->set_clk(p_system_clock,SC_NS);
      // ******************************************
    }

    // AHBSlave - AHBProf
    // ==================
    gs::gs_param_array p_ahbprof("ahbprof", p_conf);
    gs::gs_param<bool> p_ahbprof_en("en", true, p_ahbprof);
    gs::gs_param<unsigned int> p_ahbprof_addr("addr", 0x900, p_ahbprof);
    gs::gs_param<unsigned int> p_ahbprof_mask("mask", 0xFFF, p_ahbprof);
    gs::gs_param<unsigned int> p_ahbprof_index("index", 6, p_ahbprof);
    if(p_ahbprof_en) {
      AHBProf *ahbprof = new AHBProf("ahbprof",
        p_ahbprof_index,  // index
        p_ahbprof_addr,   // paddr
        p_ahbprof_mask,   // pmask
        ambaLayer
      );

      // Connecting APB Slave
      ahbctrl.ahbOUT(ahbprof->ahb);
      ahbprof->set_clk(p_system_clock,SC_NS);
    }
#ifdef HAVE_SOCWIRE
    // CREATE AHB2Socwire bridge
    // =========================
    gs::gs_param_array p_socwire("socwire", p_conf);
    gs::gs_param<bool> p_socwire_en("en", false, p_socwire);
    gs::gs_param_array p_socwire_apb("apb", p_socwire);
    gs::gs_param<unsigned int> p_socwire_apb_addr("addr", 0x010, p_socwire_apb);
    gs::gs_param<unsigned int> p_socwire_apb_mask("mask", 0xFFF, p_socwire_apb);
    gs::gs_param<unsigned int> p_socwire_apb_index("index", 3, p_socwire_apb);
    gs::gs_param<unsigned int> p_socwire_apb_irq("irq", 10, p_socwire_apb);
    gs::gs_param_array p_socwire_ahb("ahb", p_socwire);
    gs::gs_param<unsigned int> p_socwire_ahb_index("index", 1, p_socwire_ahb);
    if(p_socwire_en) {
      AHB2Socwire *ahb2socwire = new AHB2Socwire("ahb2socwire",
        p_socwire_apb_addr,  // paddr
        p_socwire_apb_mask,  // pmask
        p_socwire_apb_index, // pindex
        p_socwire_apb_irq,   // pirq
        p_socwire_ahb_index, // hindex
        ambaLayer            // abstraction
      );

      // Connecting AHB Master
      ahb2socwire->ahb(ahbctrl.ahbIN);

      // Connecting APB Slave
      apbctrl.apb(ahb2socwire->apb);

      // Connecting Interrupts
      connect(irqmp.irq_in, ahb2socwire->irq, p_socwire_apb_irq);

      // Connect socwire ports as loopback
      ahb2socwire->socwire.master_socket(ahb2socwire->socwire.slave_socket);
    }
#endif
#ifdef HAVE_GRETH
  // ===========================================================
    // GREth Media Access Controller with EDCL support (AHBMaster)
    // ===========================================================
  GREthConfiguration *greth_config = new GREthConfiguration();
  // Load default configuration
    gs::gs_param_array p_greth("greth", p_conf);
    gs::gs_param<bool> p_greth_en("en", false, p_greth);
    gs::gs_param<uint8_t> p_greth_hindex("hindex",greth_config->hindex, p_greth);
    gs::gs_param<uint8_t> p_greth_pindex("pindex",greth_config->pindex, p_greth);
    gs::gs_param<uint16_t> p_greth_paddr("paddr",greth_config->paddr, p_greth);
    gs::gs_param<uint16_t> p_greth_pmask("pmask",greth_config->pmask, p_greth);
    gs::gs_param<uint32_t> p_greth_pirq("pirq",greth_config->pirq, p_greth);
    gs::gs_param<uint32_t> p_greth_memtech("memtech",greth_config->memtech, p_greth);
    gs::gs_param<uint16_t> p_greth_ifg_gap("ifg_gap",greth_config->ifg_gap, p_greth);
    gs::gs_param<uint16_t> p_greth_attempt_limit("attempt_limit",greth_config->attempt_limit, p_greth);
    gs::gs_param<uint16_t> p_greth_backoff_limit("backoff_limit",greth_config->backoff_limit, p_greth);
    gs::gs_param<uint16_t> p_greth_slot_time("slot_time",greth_config->slot_time, p_greth);
    gs::gs_param<uint16_t> p_greth_mdcscaler("mdcscaler",greth_config->mdcscaler, p_greth);
    gs::gs_param<bool> p_greth_enable_mdio("enable_mdio",greth_config->enable_mdio, p_greth);
    gs::gs_param<uint8_t> p_greth_fifosize("fifosize",greth_config->fifosize, p_greth);
    gs::gs_param<uint8_t> p_greth_nsync("nsync",greth_config->nsync, p_greth);
    gs::gs_param<uint8_t> p_greth_edcl("edcl",greth_config->edcl, p_greth);
    gs::gs_param<uint8_t> p_greth_edclbufsz("edclbufsz",greth_config->edclbufsz, p_greth);
    gs::gs_param<uint32_t> p_greth_macaddrh("macaddrh",greth_config->macaddrh, p_greth);
    gs::gs_param<uint32_t> p_greth_macaddrl("macaddrl",greth_config->macaddrl, p_greth);
    gs::gs_param<uint16_t> p_greth_ipaddrh("ipaddrh",greth_config->ipaddrh, p_greth);
    gs::gs_param<uint16_t> p_greth_ipaddrl("ipaddrl",greth_config->ipaddrl, p_greth);
    gs::gs_param<uint8_t> p_greth_phyrstadr("phyrstadr",greth_config->phyrstadr, p_greth);
    gs::gs_param<bool> p_greth_rmii("rmii",greth_config->rmii, p_greth);
    gs::gs_param<bool> p_greth_oepol("oepol",greth_config->oepol, p_greth);
    gs::gs_param<bool> p_greth_mdint_pol("mdint_pol",greth_config->mdint_pol, p_greth);
    gs::gs_param<bool> p_greth_enable_mdint("enable_mdint",greth_config->enable_mdint, p_greth);
    gs::gs_param<bool> p_greth_multicast("multicast",greth_config->multicast, p_greth);
    gs::gs_param<uint8_t> p_greth_ramdebug("ramdebug",greth_config->ramdebug, p_greth);
    gs::gs_param<uint8_t> p_greth_ehindex("ehindex",greth_config->ehindex, p_greth);
    gs::gs_param<bool> p_greth_edclsepahb("edclsepahb",greth_config->edclsepahb, p_greth);
    gs::gs_param<uint8_t> p_greth_mdiohold("mdiohold",greth_config->mdiohold, p_greth);
    gs::gs_param<uint16_t> p_greth_maxsize("maxsize",greth_config->maxsize, p_greth);
    gs::gs_param<int> p_greth_vphy_ctrl("vphy_ctrl",1 , p_greth);

    // Set custom configuration adaptions
    greth_config->hindex = p_greth_hindex;
    greth_config->pindex = p_greth_pindex;
    greth_config->paddr = p_greth_paddr;
    greth_config->pmask = p_greth_pmask;
    greth_config->pirq = p_greth_pirq;
    greth_config->memtech = p_greth_memtech;
    greth_config->ifg_gap = p_greth_ifg_gap;
    greth_config->attempt_limit = p_greth_attempt_limit;
    greth_config->backoff_limit = p_greth_backoff_limit;
    greth_config->slot_time = p_greth_slot_time;
    greth_config->mdcscaler = p_greth_mdcscaler;
    greth_config->enable_mdio = p_greth_enable_mdio;
    greth_config->fifosize = p_greth_fifosize;
    greth_config->nsync = p_greth_nsync;
    greth_config->edcl = p_greth_edcl;
    greth_config->edclbufsz = p_greth_edclbufsz;
    greth_config->macaddrh = p_greth_macaddrh;
    greth_config->macaddrl = p_greth_macaddrl;
    greth_config->ipaddrh = p_greth_ipaddrh;
    greth_config->ipaddrl = p_greth_ipaddrl;
    greth_config->phyrstadr = p_greth_phyrstadr;
    greth_config->rmii = p_greth_rmii;
    greth_config->oepol = p_greth_oepol;
    greth_config->mdint_pol = p_greth_mdint_pol;
    greth_config->enable_mdint = p_greth_enable_mdint;
    greth_config->multicast = p_greth_multicast;
    greth_config->ramdebug = p_greth_ramdebug;
    greth_config->ehindex = p_greth_ehindex;
    greth_config->edclsepahb = p_greth_edclsepahb;
    greth_config->mdiohold = p_greth_mdiohold;
    greth_config->maxsize = p_greth_maxsize;
    // Optional params for tap offset and post script
    /*if(vm.count("greth")) {
        std::vector<std::string> vec = vm["greth"].as< std::vector<std::string> >();
        for(std::vector<std::string>::iterator iter = vec.begin(); iter!=vec.end(); iter++) {
           std::string parname;
           std::string parvalue;

           // *** Check right format (parname=value)
           if(iter->find_first_of("=") == std::string::npos) {
               v::warn << "main" << "Option value in command line greth has no '='. Type '--help' for help. " << *iter;
           }
           // if not space before equal sign
           if(iter->find_first_of(" ") < iter->find_first_of("=")) {
               v::warn << "main" << "Option value in command line option may not contain a space before '='. " << *iter;
           }

           // Parse parameter name
           parname = iter->substr(0,iter->find_first_of("="));
           // Parse parameter value
           parvalue = iter->substr(iter->find_first_of("=")+1);

           // Set parameter
           if(parname.compare("tap.offset") == 0)
           {
               cout << "GRETH OTIONS:" << parname << " - " << parvalue << endl;
               TAP::deviceno = atoi(parvalue.c_str());
           }
           if(parname.compare("post.script") == 0)
           {
               cout << "GRETH OTIONS:" << parname << " - " << parvalue << endl;
               greth_script_path = parvalue.c_str();
           }
        }
    }*/

    GREth *greth;
    if(p_greth_en)  {
        MII *vphyLink;
        switch(p_greth_vphy_ctrl) {
        case 0:
          vphyLink = new Loopback();
          break;
        case 1:
        vphyLink = new Tap();
        break;
        //case 2:
        //vphyLink = new TrafGen();
        //break;
        default:
        v::info << "GREth.init" << "No VPHY defined -> choosing TAP as default." << v::endl;
        vphyLink = new Tap();
        break;
        }

      // Initialize the GREth with a virtual PHY
      // ==========================
      greth = new GREth("GREth",
            greth_config,   // CONFIG
            vphyLink,     // VPHY Controller
            ambaLayer,      // LT or AT (AT not supported at time)
            grethVPHYHook);   // Callback function, to init the TAP


      greth->set_clk(40, SC_NS);

      // Connect TLM buses
      // ==========================
      greth->ahb(ahbctrl.ahbIN);
      apbctrl.apb(greth->apb);

      // Connect IRQ
      // ==========================
      connect(irqmp.irq_in, greth->irq);
    }
  // GREth done. ==========================
#endif  // HAVE_GRETH
    // ******************************************

#ifdef HAVE_AHBDISPLAY
    int frameWidth = 960;
    int frameHeight = 720;

    // AHBDisplay - AHBMaster
    // ==================
    gs::gs_param_array p_ahbdisplay("ahbdisplay", p_conf);
    gs::gs_param<bool> p_ahbdisplay_en("en", false, p_ahbdisplay);
    gs::gs_param<unsigned int> p_ahbdisplay_hindex("hindex", 2, p_ahbdisplay);
    gs::gs_param<unsigned int> p_ahbdisplay_pindex("pindex", 4, p_ahbdisplay);
    gs::gs_param<unsigned int> p_ahbdisplay_paddr("paddr", 0x500, p_ahbdisplay);
    gs::gs_param<unsigned int> p_ahbdisplay_pmask("pmask", 0xFFF, p_ahbdisplay);
    if(p_ahbdisplay_en) {
      AHBDisplay *ahbdisplay = new AHBDisplay("ahbdisplay",
        p_ahbdisplay_hindex,  // ahb index
        p_ahbdisplay_pindex,  // apb index
        p_ahbdisplay_paddr,  // apb address
        p_ahbdisplay_pmask,  // apb mask
        frameWidth, frameHeight
      );

      // Connecting APB Slave
      ahbdisplay->ahb(ahbctrl.ahbIN);
      apbctrl.apb(ahbdisplay->apb);
      ahbdisplay->set_clk(p_system_clock,SC_NS);
    }
#endif
#ifdef HAVE_AHBCAMERA
    // AHBCamera - AHBMaster
    // ==================
    gs::gs_param_array p_ahbcamera("ahbcamera", p_conf);
    gs::gs_param<bool> p_ahbcamera_en("en", false, p_ahbcamera);
    gs::gs_param<unsigned int> p_ahbcamera_hindex("hindex", 3, p_ahbcamera);
    gs::gs_param<unsigned int> p_ahbcamera_pindex("pindex", 5, p_ahbcamera);
    gs::gs_param<unsigned int> p_ahbcamera_paddr("paddr", 0x501, p_ahbcamera);
    gs::gs_param<unsigned int> p_ahbcamera_pmask("pmask", 0xFFF, p_ahbcamera);
    gs::gs_param<std::string> p_ahbcamera_video("video", "bigbuckbunny_small.m2v", p_ahbcamera);
    if(p_ahbcamera_en) {
      AHBCamera *ahbcamera = new AHBCamera("ahbcamera",
        p_ahbcamera_hindex,  // ahb index
        p_ahbcamera_pindex,  // apb index
        p_ahbcamera_paddr,   // apb addr
        p_ahbcamera_pmask,   // apb make
        frameWidth, frameHeight,
        ((std::string)p_ahbcamera_video).c_str(),
        ambaLayer
      );

      // Connecting APB Slave
      ahbcamera->ahb(ahbctrl.ahbIN);
      apbctrl.apb(ahbcamera->apb);
      ahbcamera->set_clk(p_system_clock,SC_NS);
    }
#endif
#ifdef HAVE_AHBSHUFFLER
    // AHBShuffler - AHBMaster
    // ==================
    int inX = 0;
    int inY = 0;
    int outX = 320;
    int outY = 0;
    int videoWidth = 320;
    uint8_t p_ahbshuffler_matrix[16] = {
      1,0,2,12,
      4,5,14,8,
      3,9,10,11,
      7,13,6,15,
    };

    gs::gs_param_array p_ahbshuffler("ahbshuffler", p_conf);
    gs::gs_param<bool> p_ahbshuffler_en("en", false, p_ahbshuffler);
    gs::gs_param<unsigned int> p_ahbshuffler_hindex("hindex", 4, p_ahbshuffler);
    gs::gs_param<unsigned int> p_ahbshuffler_pindex("pindex", 6, p_ahbshuffler);
    gs::gs_param<unsigned int> p_ahbshuffler_paddr("paddr", 0x502, p_ahbshuffler);
    gs::gs_param<unsigned int> p_ahbshuffler_pmask("pmask", 0xFFF, p_ahbshuffler);
    if(p_ahbshuffler_en) {
      AHBShuffler *ahbshuffler = new AHBShuffler("ahbshuffler",
        p_ahbshuffler_hindex,  // ahb index
        p_ahbshuffler_pindex,  // apb index
        p_ahbshuffler_paddr,   // apb addr
        p_ahbshuffler_pmask,   // apb make
        p_ahbshuffler_matrix,
        inX, inY,
        outX, outY,
        videoWidth,
        frameWidth, frameHeight,
        ambaLayer
      );

      // Connecting APB Slave
      ahbshuffler->ahb(ahbctrl.ahbIN);
      apbctrl.apb(ahbshuffler->apb);
      ahbshuffler->set_clk(p_system_clock,SC_NS);
    }
#endif // HAVE_REPO_MEDIA

#ifdef HAVE_AHBGPGPU
    gs::gs_param_array p_nyuzi("nyuzi", p_conf );
    gs::gs_param<bool> p_nyuzi_en("en", false, p_nyuzi );
    gs::gs_param<unsigned int> p_nyuzi_pindex("pindex", 15, p_nyuzi );
    gs::gs_param<unsigned int> p_nyuzi_hindex("hindex", 10, p_nyuzi );
    gs::gs_param<unsigned int> p_nyuzi_irq("irq", 7, p_nyuzi );
    gs::gs_param<uint16_t> p_nyuzi_paddr("paddr", 0x7, p_nyuzi ); // fixme -> requires APB to address, descriptor-based?
    gs::gs_param<uint16_t> p_nyuzi_pmask("pmask", 0xFFF, p_nyuzi );
    if( p_nyuzi_en ) {
      Nyuzi * nyuzi = new Nyuzi("nyuzi",
        p_nyuzi_hindex,
        p_nyuzi_pindex,
        p_nyuzi_paddr,
        p_nyuzi_pmask,
        ambaLayer,
        p_nyuzi_irq
      );

      // Connecting AHB Master
      nyuzi->ahb(ahbctrl.ahbIN);

      // Connecting APB Slave
      apbctrl.apb(nyuzi->apb);
      // Set clock
      //nyuzi->set_clk(p_system_clock, SC_NS);
      //connect(nyuzi->snoop, nyuzi.snoop);
    }
#endif /* #ifdef HAVE_AHBGPGPU */
    irqmp_rst_stimuli stimuli("platform_stimuli");
    connect(stimuli.irqmp_rst, irqmp.rst);
#ifndef HAVE_USI
    (void) signal(SIGINT, stopSimFunction);
    (void) signal(SIGTERM, stopSimFunction);
#endif
    cstart = cend = clock();
    cstart = clock();
//    mtrace();
#ifdef HAVE_USI
    usi_end_of_initialization();
    usi_start();
#else
    sc_core::sc_start();
#endif
//    muntrace();
    cend = clock();

    v::report << "Summary" << "Start: " << dec << cstart << v::endl;
    v::report << "Summary" << "End:   " << dec << cend << v::endl;
    v::report << "Summary" << "Delta: " << dec << setprecision(4) << ((double)(cend - cstart) / (double)CLOCKS_PER_SEC * 1000) << "ms" << v::endl;
    return first_leon->cpu.getInterface().getExitValue();
}
/// @}
