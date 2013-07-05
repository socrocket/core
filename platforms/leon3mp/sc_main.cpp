// *********************************************************************
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
// *********************************************************************
// Title:      sc_main.cpp
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
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
// *********************************************************************
#include <greencontrol/config.h>

#include <greencontrol/config_api_lua_file_parser.h>
#include "json_parser.h"
#include "paramprinter.h"

#include <execLoader.hpp>
#include <osEmulator.hpp>

#include "mmu_cache.h"
#include "ahbin.h"
#include "arraymemory.h"
#include "mapmemory.h"
#include "apbctrl.h"
#include "ahbmem.h"
#include "mctrl.h"
#include "defines.h"
#include "gptimer.h"
#include "apbuart.h"
#include "tcpio.h"
#include "nullio.h"
#include "irqmp.h"
#include "ahbctrl.h"
#include "AHB2Socwire.h"
#include "ahbprof.h"

#include <iostream>
#include <vector>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <amba.h>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include "verbose.h"
#include "powermonitor.h"

#include <GDBStub.hpp>
#include <systemc.h>
#include <tlm.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>

#include "leon3.funclt.h"
#include "leon3.funcat.h"

using namespace std;
using namespace sc_core;
using namespace socw;

namespace trap {
  extern int exitValue;
};

boost::filesystem::path find_top_path(char *start) {
    boost::filesystem::path path = boost::filesystem::absolute(boost::filesystem::path(start).parent_path());
    boost::filesystem::path waf("waf");
    while(!boost::filesystem::exists(path/waf)&&!path.empty()) {
        path = path.parent_path();
    }
    return path;
}

int sc_main(int argc, char** argv) {
    boost::program_options::options_description desc("Options");
    desc.add_options()
      ("help", "Shows this message.")
      ("jsonconfig,j", boost::program_options::value<std::string>(), "The main configuration file. Usual config.json.")
      ("option,o", boost::program_options::value<std::vector<std::string> >(), "Additional configuration options.")
      ("argument,a", boost::program_options::value<std::vector<std::string> >(), "Arguments to the software running inside the simulation.")
      ("listoptions,l", "Show a list of all avaliable options")
      ("listoptionsfiltered,f", boost::program_options::value<std::string>(), "Show a list of avaliable options containing a keyword")
      ("listgsconfig,c", "Show a list of all avaliable gs_config options")
      ("listgsconfigfiltered,g", boost::program_options::value<std::string>(), "Show a list of avaliable options containing a keyword")
      ("saveoptions,s", boost::program_options::value<std::string>(), "Save options to json file. Default: saved_config.json");

    boost::program_options::positional_options_description p;
    p.add("argument", -1);

    boost::program_options::variables_map vm;
    try {
      boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
      boost::program_options::notify(vm);
    } catch(boost::program_options::unknown_option o) {
      std::cout << "Comand line argument '" << o.get_option_name() << "' unknown. Please use -a,[argument] to add it to the software arguments or correct it." << std::endl;
      exit(1);
    }

    if(vm.count("help")) {
        std::cout << std::endl << "SoCRocket -- LEON3 Multi-Processor Platform" << std::endl;
        std::cout << std::endl << "Usage: " << argv[0] << " [options]" << std::endl;
        std::cout << std::endl << desc << std::endl;
        return 0;
    }

    clock_t cstart, cend;
    std::string prom_app;

    bool paramlist = false, paramlistfiltered = false, configlist = false, configlistfiltered = false, saveoptions = false;

    gs::ctr::GC_Core       core;
    gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
    gs::cnf::ConfigPlugin configPlugin(&cnfdatabase);

    json_parser* jsonreader = new json_parser();

    if(vm.count("jsonconfig")) {
        setenv("JSONCONFIG", vm["jsonconfig"].as<std::string>().c_str(), true);
    }

    // Find *.json
    // - First search on the comandline
    // - Then search environment variable
    // - Then search in current dir
    // - and finaly in the application directory
    // - searches in the source directory
    // Print an error if it is not found!
    boost::filesystem::path topdir = find_top_path(argv[0]);
    boost::filesystem::path appdir = (boost::filesystem::path(argv[0]).parent_path());
    boost::filesystem::path srcdir = (topdir / boost::filesystem::path("build") / boost::filesystem::path(__FILE__).parent_path());
    boost::filesystem::path json("leon3mp.singlecore.json");
    char *json_env = std::getenv("JSONCONFIG");
    /*if(vm.count("luaconfig")) {
        jsonlua = boost::filesystem::path(vm["luaconfig"].as<std::string>());
        if(!boost::filesystem::exists(jsonlua)) {
            v::error << "main" << "The Lua configuration provided by command line does not exist: " << jsonlua << v::endl;
            exit(1);
        }
    } else*/
    if(json_env) {
        json = boost::filesystem::path(json_env);
    } else if(boost::filesystem::exists(json)) {
        json = json;
    } else if(boost::filesystem::exists(appdir / json)) {
        json = appdir / json;
    } else if(boost::filesystem::exists(srcdir / json)) {
        json = srcdir / json;
    }
    if(boost::filesystem::exists(boost::filesystem::path(json))) {
        v::info << "main" << "Open Configuration " << json << v::endl;
        jsonreader->config(json.c_str());
    } else {
        v::warn << "main" << "No *.json found. Please put it in the current work directory, application directory or put the path to the file in the JSONCONFIG environment variable" << v::endl;
    }

    gs::cnf::cnf_api *mApi = gs::cnf::GCnf_Api::getApiInstance(NULL);
    if(vm.count("option")) {
        std::vector<std::string> vec = vm["option"].as< std::vector<std::string> >();
        for(std::vector<std::string>::iterator iter = vec.begin(); iter!=vec.end(); iter++) {
           std::string parname;
           std::string parvalue;

           // *** Check right format (parname=value)
           // of no space
           if(iter->find_first_of("=") == std::string::npos) {
               v::warn << "main" << "Option value in command line option has no '='. Type '--help' for help. " << *iter;
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
           mApi->setInitValue(parname, parvalue);
        }
    }


    if(vm.count("listoptions")) {
       paramlist = true;
    }

    if(vm.count("listgsconfig")) {
       configlist = true;
    }

    if(vm.count("saveoptions")) {
       saveoptions = true;
    }

		std::string optionssearchkey = "";
		if(vm.count("listoptionsfiltered")) {
        optionssearchkey = vm["listoptionsfiltered"].as<std::string>();
				paramlistfiltered = true;
    }

		std::string configssearchkey = "";
		if(vm.count("listgsconfigfiltered")) {
        configssearchkey = vm["listgsconfigfiltered"].as<std::string>();
				configlistfiltered= true;
    }

    // Build GreenControl Configuration Namespace
    // ==========================================
    gs::gs_param_array p_conf("conf");
    gs::gs_param_array p_system("system", p_conf);

    // Decide whether LT or AT
    gs::gs_param<bool> p_system_at("at", false, p_system);
    gs::gs_param<unsigned int> p_system_ncpu("ncpu", 1, p_system);
    gs::gs_param<unsigned int> p_system_clock("clk", 10, p_system);
    gs::gs_param<std::string> p_system_osemu("osemu", "", p_system);
    gs::gs_param<std::string> p_system_log("log", "", p_system);

    gs::gs_param_array p_report("report", p_conf);
    gs::gs_param<bool> p_report_timing("timing", true, p_report);
    gs::gs_param<bool> p_report_power("power", true, p_report);

    if(!((std::string)p_system_log).empty()) {
        v::logApplication(((std::string)p_system_log).c_str());
    }

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
    gs::gs_param<unsigned int> p_irqmp_addr("addr", 0x1F0, p_irqmp);
    gs::gs_param<unsigned int> p_irqmp_mask("mask", 0xFFF, p_irqmp);
    gs::gs_param<unsigned int> p_irqmp_index("index", 2, p_irqmp);
    gs::gs_param<unsigned int> p_irqmp_eirq("eirq", 4, p_irqmp);

    Irqmp irqmp("irqmp",
                p_irqmp_addr,  // paddr
                p_irqmp_mask,  // pmask
                p_system_ncpu,  // ncpu
                p_irqmp_eirq,  // eirq
                p_irqmp_index, // pindex
                p_report_power // power monitoring
    );

    // Connect to APB and clock
    apbctrl.apb(irqmp.apb_slv);
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
    gs::gs_param<unsigned int> p_mctrl_prom_bsize("bsize", 256, p_mctrl_prom);
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
    MapMemory rom( "rom",
    //ArrayMemory rom( "rom",
                     MEMDevice::ROM,
                     p_mctrl_prom_banks,
                     p_mctrl_prom_bsize * 1024 * 1024,
                     p_mctrl_prom_width,
                     0,
                     p_report_power
    );

    // Connect to memory controller and clock
    mctrl.mem(rom.bus);
    rom.set_clk(p_system_clock, SC_NS);

    // ELF loader from leon (Trap-Gen)
    gs::gs_param<std::string> p_mctrl_prom_elf("elf", "", p_mctrl_prom);
    if(!((std::string)p_mctrl_prom_elf).empty()) {
      if(boost::filesystem::exists(boost::filesystem::path((std::string)p_mctrl_prom_elf))) {
        uint8_t *execData;
        v::info << "rom" << "Loading Prom with " << p_mctrl_prom_elf << v::endl;
        ExecLoader prom_loader(p_mctrl_prom_elf);
        execData = prom_loader.getProgData();

        for(unsigned int i = 0; i < prom_loader.getProgDim(); i++) {
          rom.write(prom_loader.getDataStart() + i - ((((unsigned int)p_mctrl_prom_addr)&((unsigned int)p_mctrl_prom_mask))<<20), execData[i]);
        }
      } else {
        v::warn << "rom" << "File " << p_mctrl_prom_elf << " does not exist!" << v::endl;
        exit(1);
      }
    }

    // IO memory instantiation
    MapMemory io( "io",
    //ArrayMemory io( "io",
                    MEMDevice::IO,
                    p_mctrl_prom_banks,
                    p_mctrl_prom_bsize * 1024 * 1024,
                    p_mctrl_prom_width,
                    0,
                    p_report_power

    );

    // Connect to memory controller and clock
    mctrl.mem(io.bus);
    io.set_clk(p_system_clock, SC_NS);

    // ELF loader from leon (Trap-Gen)
    gs::gs_param<std::string> p_mctrl_io_elf("elf", "", p_mctrl_io);

    if(!((std::string)p_mctrl_io_elf).empty()) {
      if(boost::filesystem::exists(boost::filesystem::path((std::string)p_mctrl_io_elf))) {
        uint8_t *execData;
        v::info << "io" << "Loading IO with " << p_mctrl_io_elf << v::endl;
        ExecLoader loader(p_mctrl_io_elf);
        execData = loader.getProgData();

        for(unsigned int i = 0; i < loader.getProgDim(); i++) {
          io.write(loader.getDataStart() + i - ((((unsigned int)p_mctrl_io_addr)&((unsigned int)p_mctrl_io_mask))<<20), execData[i]);
        }
      } else {
        v::warn << "io" << "File " << p_mctrl_io_elf << " does not exist!" << v::endl;
        exit(1);
      }
    }

    // SRAM instantiation
    MapMemory sram( "sram",
    //ArrayMemory sram( "sram",
                      MEMDevice::SRAM,
                      p_mctrl_ram_sram_banks,
                      p_mctrl_ram_sram_bsize * 1024 * 1024,
                      p_mctrl_ram_sram_width,
                      0,
                      p_report_power

    );

    // Connect to memory controller and clock
    mctrl.mem(sram.bus);
    sram.set_clk(p_system_clock, SC_NS);

    // ELF loader from leon (Trap-Gen)
    gs::gs_param<std::string> p_mctrl_ram_sram_elf("elf", "", p_mctrl_ram_sram);

    if(!((std::string)p_mctrl_ram_sram_elf).empty()) {
      if(boost::filesystem::exists(boost::filesystem::path((std::string)p_mctrl_ram_sram_elf))) {
        uint8_t *execData;
        v::info << "sram" << "Loading SRam with " << p_mctrl_ram_sram_elf << v::endl;
        ExecLoader loader(p_mctrl_ram_sram_elf);
        execData = loader.getProgData();

        for(unsigned int i = 0; i < loader.getProgDim(); i++) {
          sram.write(loader.getDataStart() + i - ((((unsigned int)p_mctrl_ram_addr)&((unsigned int)p_mctrl_ram_mask))<<20), execData[i]);
        }
      } else {
        v::warn << "sram" << "File " << p_mctrl_ram_sram_elf << " does not exist!" << v::endl;
        exit(1);
      }
    }

    // SDRAM instantiation
    ArrayMemory sdram( "sdram",
                       MEMDevice::SDRAM,
                       p_mctrl_ram_sdram_banks,
                       p_mctrl_ram_sdram_bsize * 1024 * 1024,
                       p_mctrl_ram_sdram_width,
                       p_mctrl_ram_sdram_cols,
                       p_report_power
    );

    // Connect to memory controller and clock
    mctrl.mem(sdram.bus);
    sdram.set_clk(p_system_clock, SC_NS);

    // ELF loader from leon (Trap-Gen)
    gs::gs_param<std::string> p_mctrl_ram_sdram_elf("elf", "", p_mctrl_ram_sdram);

    if(!((std::string)p_mctrl_ram_sdram_elf).empty()) {
      if(boost::filesystem::exists(boost::filesystem::path((std::string)p_mctrl_ram_sdram_elf))) {
        uint8_t *execData;
        v::info << "sdram" << "Loading SDRam with " << p_mctrl_ram_sdram_elf << v::endl;
        ExecLoader loader(p_mctrl_ram_sdram_elf);
        execData = loader.getProgData();

        for(unsigned int i = 0; i < loader.getProgDim(); i++) {
          sdram.write(loader.getDataStart() + i - ((((unsigned int)p_mctrl_ram_addr)&((unsigned int)p_mctrl_ram_mask))<<20), execData[i]);
        }
      } else {
        v::warn << "sdram" << "File " << p_mctrl_ram_sdram_elf << " does not exist!" << v::endl;
        exit(1);
      }
    }


    //leon3.ENTRY_POINT   = 0;
    //leon3.PROGRAM_LIMIT = 0;
    //leon3.PROGRAM_START = 0;

    // AHBSlave - AHBMem
    // =================
    gs::gs_param_array p_ahbmem("ahbmem", p_conf);
    gs::gs_param<bool> p_ahbmem_en("en", true, p_ahbmem);
    gs::gs_param<unsigned int> p_ahbmem_addr("addr", 0xA00, p_ahbmem);
    gs::gs_param<unsigned int> p_ahbmem_mask("mask", 0xFFF, p_ahbmem);
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

      // ELF loader from leon (Trap-Gen)
      if(!((std::string)p_ahbmem_elf).empty()) {
        if(boost::filesystem::exists(boost::filesystem::path((std::string)p_ahbmem_elf))) {
          uint8_t *execData;
          v::info << "ahbmem" << "Loading AHBMem with " << p_ahbmem_elf << v::endl;
          ExecLoader prom_loader(p_ahbmem_elf);
          execData = prom_loader.getProgData();

          for(unsigned int i = 0; i < prom_loader.getProgDim(); i++) {
            ahbmem->writeByteDBG(prom_loader.getDataStart() + i - ((((unsigned int)p_ahbmem_addr)&((unsigned int)p_ahbmem_mask))<<20), execData[i]);
          }
        } else {
          v::warn << "ahbmem" << "File " << p_ahbmem_elf << " does not exist!" << v::endl;
          exit(1);
        }
      }
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
      signalkit::connect(irqmp.irq_in, ahbin->irq, p_ahbin_irq);
    }

    // CREATE LEON3 Processor
    // ===================================================
    // Always enabled.
    // Needed for basic platform.
    gs::gs_param_array p_mmu_cache("mmu_cache", p_conf);
    gs::gs_param_array p_mmu_cache_ic("ic", p_mmu_cache);
    gs::gs_param<bool> p_mmu_cache_ic_en("en", true, p_mmu_cache_ic);
    gs::gs_param<int> p_mmu_cache_ic_repl("repl", 3, p_mmu_cache_ic);
    gs::gs_param<int> p_mmu_cache_ic_sets("sets", 4, p_mmu_cache_ic);
    gs::gs_param<int> p_mmu_cache_ic_linesize("linesize", 4, p_mmu_cache_ic);
    gs::gs_param<int> p_mmu_cache_ic_setsize("setsize", 16, p_mmu_cache_ic);
    gs::gs_param<bool> p_mmu_cache_ic_setlock("setlock", 1, p_mmu_cache_ic);
    gs::gs_param_array p_mmu_cache_dc("dc", p_mmu_cache);
    gs::gs_param<bool> p_mmu_cache_dc_en("en", true, p_mmu_cache_dc);
    gs::gs_param<int> p_mmu_cache_dc_repl("repl", 3, p_mmu_cache_dc);
    gs::gs_param<int> p_mmu_cache_dc_sets("sets", 2, p_mmu_cache_dc);
    gs::gs_param<int> p_mmu_cache_dc_linesize("linesize", 4, p_mmu_cache_dc);
    gs::gs_param<int> p_mmu_cache_dc_setsize("setsize", 1, p_mmu_cache_dc);
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
    gs::gs_param<bool> p_mmu_cache_mmu_en("en", false, p_mmu_cache_mmu);
    gs::gs_param<unsigned int> p_mmu_cache_mmu_itlb_num("itlb_num", 8, p_mmu_cache_mmu);
    gs::gs_param<unsigned int> p_mmu_cache_mmu_dtlb_num("dtlb_num", 8, p_mmu_cache_mmu);
    gs::gs_param<unsigned int> p_mmu_cache_mmu_tlb_type("tlb_type", 0u, p_mmu_cache_mmu);
    gs::gs_param<unsigned int> p_mmu_cache_mmu_tlb_rep("tlb_rep", 1, p_mmu_cache_mmu);
    gs::gs_param<unsigned int> p_mmu_cache_mmu_mmupgsz("mmupgsz", 0u, p_mmu_cache_mmu);

    gs::gs_param<std::string> p_proc_history("history", "", p_system);

    gs::gs_param_array p_gdb("gdb", p_conf);
    gs::gs_param<bool> p_gdb_en("en", false, p_gdb);
    gs::gs_param<int> p_gdb_port("port", 1500, p_gdb);
    gs::gs_param<int> p_gdb_proc("proc", 0, p_gdb);
    for(int i=0; i< p_system_ncpu; i++) {
      // AHBMaster - MMU_CACHE
      // =====================
      // Always enabled.
      // Needed for basic platform.
      mmu_cache *mmu_cache_inst = new mmu_cache(
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
              sc_core::sc_gen_unique_name("mmu_cache", false), // name of sysc module
              p_mmu_cache_index + i,     // Id of the AHB master
              p_report_power,            // Power Monitor,
              ambaLayer                  // TLM abstraction layer
      );

      // Connecting AHB Master
      mmu_cache_inst->ahb(ahbctrl.ahbIN);

      // Set clock
      mmu_cache_inst->set_clk(p_system_clock, SC_NS);
      connect(mmu_cache_inst->snoop, ahbctrl.snoop);

      // For each Abstraction is another model needed
      if(p_system_at) {
        // LEON3 AT Processor
        // ==================
        v::info << "main" << "Instantiating AT Processor" << i << v::endl;
        leon3_funcat_trap::Processor_leon3_funcat *leon3 = new leon3_funcat_trap::Processor_leon3_funcat(sc_core::sc_gen_unique_name("leon3", false), sc_core::sc_time(p_system_clock, SC_NS), p_report_power);
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
            osEmu->set_program_args(options);
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
            osEmu->set_program_args(options);
            leon3->toolManager.addTool(*osEmu);
          } else {
            v::warn << "main" << "File " << p_system_osemu << " not found!" << v::endl;
            exit(1);
          }
        }
      }
    }

    // APBSlave - GPTimer
    // ==================
/*********
    gs::gs_param_array p_gptimer("gptimer", p_conf);
    gs::gs_param<bool> p_gptimer_en("en", true, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_addr("addr", 0x0F0, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_mask("mask", 0xFFF, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_index("index", 3, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_irq("irq", 8, p_gptimer);
    gs::gs_param<bool> p_gptimer_sepirq("sepirq", true, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_ntimers("ntimers", 7, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_sbits("sbit", 16, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_nbits("nbits", 32, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_wdog("wdog", 0u, p_gptimer);

    if(p_gptimer_en) {
      GPTimer *gptimer = new GPTimer("gptimer",
        p_gptimer_ntimers,// ntimers
        p_gptimer_index,  // index
        p_gptimer_addr,   // paddr
        p_gptimer_mask,   // pmask
        p_gptimer_irq,    // pirq
        p_gptimer_sepirq, // sepirq
        p_gptimer_sbits,  // sbits
        p_gptimer_nbits,  // nbits
        p_gptimer_wdog,   // wdog
        p_report_power    // powmon
      );
***************/
    gs::gs_param_array p_gptimer("gptimer", p_conf);
    gs::gs_param<bool> p_gptimer_en("en", true, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_addr("addr", 0x0F0, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_mask("mask", 0xFFF, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_index("index", 3, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_irq("irq", 8, p_gptimer);
    gs::gs_param<bool> p_gptimer_sepirq("sepirq", true, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_ntimers("ntimers", 7, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_sbits("sbit", 16, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_nbits("nbits", 32, p_gptimer);
    gs::gs_param<unsigned int> p_gptimer_wdog("wdog", 0u, p_gptimer);

    if(p_gptimer_en) {
      GPTimer *gptimer = new GPTimer("gptimer",
        7,  // ntimers
        3,  // index
        0x0F0,   // paddr
        0xFFF,   // pmask
        8,    // pirq
        true, // sepirq
        16,  // sbits
        32,  // nbits
        0u,   // wdog
        p_report_power  // powmon
      );

      // Connect to apb and clock
      apbctrl.apb(gptimer->bus);
      gptimer->set_clk(p_system_clock,SC_NS);

      // Connecting Interrupts
      for(int i=0; i < 8; i++) {
        signalkit::connect(irqmp.irq_in, gptimer->irq, p_gptimer_irq + i);
      }

    }

    // APBSlave - APBUart
    // ==================
    std::string n_uart = "conf.uart";
    gs::gs_param_array *p_uart = new gs::gs_param_array(n_uart);
    int i = 0;
    while(mApi->getParamList(n_uart + "." + boost::lexical_cast<std::string>(i), false).size()!=0) {
      std::string n_inst = n_uart + "." + boost::lexical_cast<std::string>(i);
      io_if *io = NULL;
      int port = 2000, type = 0, index = 1, addr = 0x001, mask = 0xFFF, irq = 2;
      mApi->getValue(std::string(n_inst + ".type"), type);
      mApi->getValue(std::string(n_inst + ".index"), index);
      mApi->getValue(std::string(n_inst + ".addr"), addr);
      mApi->getValue(std::string(n_inst + ".mask"), mask);
      mApi->getValue(std::string(n_inst + ".irq"), irq);

      switch(type) {
        case 1:
          mApi->getValue(std::string(n_inst + ".port"), port);
          io = new TcpIo(port);
          break;
        default:
          io = new NullIO();
          break;
      }

      APBUART *apbuart = new APBUART(sc_core::sc_gen_unique_name("apbuart", false), io,
        index,           // index
        addr,            // paddr
        mask,            // pmask
        irq,             // pirq
        p_report_power   // powmon
      );

      // Connecting APB Slave
      apbctrl.apb(apbuart->bus);
      // Connecting Interrupts
      signalkit::connect(irqmp.irq_in, apbuart->irq, irq);
      // Set clock
      apbuart->set_clk(p_system_clock,SC_NS);
      // ******************************************

      i++;
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


    // * Param Listing **************************
		paramprinter printer;
    if(paramlist) {
    	printer.printParams();
    	exit(0);
    }

    if(configlist){
      printer.printConfigs();
    	exit(0);
    }

    if(paramlistfiltered ){
      printer.printParams(optionssearchkey);
    	exit(0);
    }

    if(configlistfiltered ){
      printer.printConfigs(configssearchkey);
    	exit(0);
    }

    // ******************************************

    signalkit::signal_out<bool, Irqmp> irqmp_rst;
    connect(irqmp_rst, irqmp.rst);
    irqmp_rst.write(0);
    irqmp_rst.write(1);

    // * Power Monitor **************************
    if(p_report_power) {
        powermonitor *pow_mon =  new powermonitor("pow_mon");
    }
    try {
        cstart = clock();
        sc_core::sc_start();
        cend = clock();
    } catch(std::runtime_error &error) {
        v::error << "main" << "Execution is stoped caused by a runtime_error. Maybe you forgot to select an executable?" << v::endl;
        v::error << "main" << error.what();
    }

    if(p_report_timing) {
        v::info << "Summary" << "Start: " << dec << cstart << v::endl;
        v::info << "Summary" << "End:   " << dec << cend << v::endl;
        v::info << "Summary" << "Delta: " << dec << setprecision(0) << ((double)(cend - cstart) / (double)CLOCKS_PER_SEC * 1000) << "ms" << v::endl;
    }
    return trap::exitValue;
}
