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
// Title:      mctrl.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    implementation of the mctrl module
//             is included by mctrl.h template header file
//
// Method:
//
// Modified on $Date: 2011-08-04 16:52:17 +0200 (Thu, 04 Aug 2011) $
//          at $Revision: 481 $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Dennis Bode
// Reviewed:
//*********************************************************************

#include "mctrl.h"
#include <tlm.h>
#include <algorithm>

using namespace sc_core;
using namespace tlm;
using namespace std;

//constructor
Mctrl::Mctrl(sc_module_name name, int _romasel, int _sdrasel,
             int _romaddr, int _rommask, int _ioaddr, int _iomask,
             int _ramaddr, int _rammask, int _paddr, int _pmask, int _wprot,
             int _srbanks, int _ram8, int _ram16, int _sepbus, int _sdbits,
             int _mobile, int _sden, unsigned int hindex, unsigned int pindex, 
	     bool powermon, amba::amba_layer_ids ambaLayer) :
            AHBSlave<gs::reg::gr_device>( name,
                                          hindex,
                                          0x04, // ven: ESA
                                          0x0F, // dev: MCTRL
                                          0, 
                                          0, // VER, IRQ
                                          ambaLayer,
                                          BAR(AHBDevice::AHBMEM, _rommask, true, true, _romaddr),
                                          BAR(AHBDevice::AHBMEM, _iomask, false, false, _ioaddr),
                                          BAR(AHBDevice::AHBMEM, _rammask, true, true, _ramaddr),
                                          0),
            APBDevice( pindex,
                    0x04, // ven: ESA
                    0x0F, // dev: MCTRL
                    0, 0, // VER, IRQ
                    APBDevice::APBIO, _pmask, 0, 0, _paddr),
            apb("apb", r, //name and register container of the greenreg_socket
                    APBDevice::get_base_addr_(), //apb base address
                    APBDevice::get_size_(),      //apb address space size
                    ::amba::amba_APB,            //bus type
                    ::amba::amba_LT,             //abstraction level
                    false //socket is not used for arbitration
            ),
            mem("mem", gs::socket::GS_TXN_ONLY),
            busy(false), 
            m_total_transactions("total_transactions", 0ull, m_performance_counters), 
            m_right_transactions("successful_transactions", 0ull, m_performance_counters), 
            m_power_down_time("total_power_down", sc_core::SC_ZERO_TIME, m_performance_counters), 
            m_power_down_start("last_power_down", sc_core::SC_ZERO_TIME, m_performance_counters),
            m_deep_power_down_time("total_deep_power_down", sc_core::SC_ZERO_TIME, m_performance_counters), 
            m_deep_power_down_start("last_deep_power_down", sc_core::SC_ZERO_TIME, m_performance_counters),
            m_self_refresh_time("total_self_refresh", sc_core::SC_ZERO_TIME, m_performance_counters), 
            m_self_refresh_start("last_self_refresh", sc_core::SC_ZERO_TIME, m_performance_counters),
            g_romasel(_romasel), g_sdrasel(_sdrasel), g_romaddr(_romaddr), g_rommask(_rommask), 
            g_ioaddr(_ioaddr), g_iomask(_iomask), g_ramaddr(_ramaddr), 
            g_rammask(_rammask), g_paddr(_paddr), g_pmask(_pmask), g_wprot(_wprot),
            g_srbanks(_srbanks), g_ram8(_ram8), g_ram16(_ram16), g_sepbus(_sepbus),
            g_sdbits(_sdbits), g_mobile(_mobile), g_sden(_sden), m_pow_mon(powermon),
            sta_power_norm("power.mctrl.sta_power_norm", 1.7e+8, true), // Normalized static power of controller
            int_power_norm("power.mctrl.int_power_norm", 1.874e-8, true), // Normalized internal power of controller
            dyn_read_energy_norm("power.mctrl.dyn_read_energy_norm", 1.175e-8, true), // Normalized read energy
            dyn_write_energy_norm("power.mctrl.dyn_write_energy_norm", 1.175e-8, true), // Normalized write energy
            power("power"),
            sta_power("sta_power", 0.0, power), // Static power
            int_power("int_power", 0.0, power), // Internal power
            swi_power("swi_power", 0.0, power), // Switching power
            power_frame_starting_time("power_frame_starting_time", SC_ZERO_TIME, power),
            dyn_read_energy("dyn_read_energy", 0.0, power), // Energy for read access
            dyn_write_energy("dyn_write_energy", 0.0, power), // Energy for write access
            dyn_reads("dyn_reads", 0ull, power), // Number of read accesses
            dyn_writes("dyn_writes", 0ull, power) // Number of write accesses

 {

    // Display APB slave information 
    v::info << this->name() << "APB slave @" << v::uint32 << apb.get_base_addr() 
            << " size: " << v::uint32 << apb.get_size() << " byte" << v::endl;
    v::info << this->name() << "(" << hex << _paddr << ":" << hex << _pmask << ")" << hex << ::APBDevice::get_device_info()[1] << v::endl;
 
    //check consistency of address space generics
    //rom space in MByte: 4GB - masked area (rommask)
    //rom space in Byte: 2^(romasel + 1)
    //same for ram and sdrasel
    if((4096 - _rommask) != (1 << (_romasel - 19))) {
        v::error << this->name() << "Inconsistent address space parameters. " 
                 << "Check romasel and (rom-addr/-mask parameter." << v::endl;
    }
    if((4096 - _rammask) != (1 << (_sdrasel - 19))) {
        v::error << this->name() << "Inconsistent address space parameters. " 
                 << "Check sdrasel vs. ram-addr/-mask parameter." << v::endl;
    }
    if(_romaddr < _ioaddr  && (_romaddr + 4096 - _rommask) > _ioaddr) {
        v::error << this->name() << "Inconsistent address space parameters. "
                 << "Check rom/io address and mask for overlaps." << v::endl;
    }
    
    if(_romaddr < _ramaddr  && (_romaddr + 4096 - _rommask) > _ramaddr) {
        v::error << this->name() << "Inconsistent address space parameters. "
                 << "Check rom/ram address and mask for overlaps." << v::endl;
    }
    
    if(_ioaddr  < _romaddr && (_ioaddr  + 4096 - _iomask)  > _romaddr) {
        v::error << this->name() << "Inconsistent address space parameters. "
                 << "Check io/rom address and mask for overlaps." << v::endl;
    }
    
    if(_ioaddr  < _ramaddr && (_ioaddr  + 4096 - _iomask)  > _ramaddr) {
        v::error << this->name() << "Inconsistent address space parameters. "
                 << "Check io/ram address and mask for overlaps." << v::endl;
    }
    
    if(_ramaddr < _romaddr && (_ramaddr + 4096 - _rammask) > _romaddr) {
        v::error << this->name() << "Inconsistent address space parameters. "
                 << "Check *ram/rom address and mask for overlaps." << v::endl;
    }
    
    if(_ramaddr < _ioaddr  && (_ramaddr + 4096 - _rammask) > _ioaddr) {
        v::error << this->name() << "Inconsistent address space parameters. "
                 << "Check ram/io address and mask for overlaps." << v::endl;
    }

    // Create register | name + description
    r.create_register( "MCFG1", "Memory Configuration Register 1", 0x00, // offset
                       gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | // configuration
                       gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                       MCFG1_DEFAULT, // init value(to be calculated from the generics for all 4 registers)
                       // write mask             
                       // FIXME: check consistency of ram8, 
                       //        ram 16 generics vs. default and 
                       //        mask of PROM WIDTH field
                       MCFG1_WRITE_MASK | 1 << 9 | 1 << 8,
                       32, // reg width (maximum 32 bit)
                       0x00 // lock mask: Not implementet, has to be zero.
                       );
    r.create_register( "MCFG2", "Memory Configuration Register 2", 0x04,
                       gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | 
                       gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                       MCFG2_DEFAULT,  
                       //FIXME: check consistency of ram8, 
                       //       ram 16 generics vs. default and 
                       //       mask of RAM WIDTH field
                       MCFG2_WRITE_MASK | 1 << 5 | 1 << 4,
                       32, 0x00);
    r.create_register( "MCFG3", "Memory Configuration Register 3", 0x08,
                       gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | 
                       gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                       MCFG3_DEFAULT, MCFG3_WRITE_MASK, 32, 0x00);
    r.create_register( "MCFG4", "Power-Saving Configuration Register", 0x0C,
                       gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | 
                       gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                       MCFG4_DEFAULT, MCFG4_WRITE_MASK, 32, 0x00);

    gs::socket::config<tlm::tlm_base_protocol_types> mem_cfg;
    mem_cfg.use_mandatory_phase(BEGIN_REQ);
    mem_cfg.use_mandatory_phase(END_REQ);
    //mem_cfg.treat_unknown_as_ignorable();
    mem.set_config(mem_cfg);

    // Module configuration report
    v::info << this->name() << " ******************************************************************************* " << v::endl;
    v::info << this->name() << " * Created MCTRL with following parameters: " << v::endl;
    v::info << this->name() << " * ---------------------------------------- " << v::endl;
    v::info << this->name() << " * romasel: " << _romasel << v::endl;
    v::info << this->name() << " * sdrasel: " << _sdrasel << v::endl;
    v::info << this->name() << " * romaddr/rommask: " << hex << _romaddr << "/" << _rommask << v::endl;
    v::info << this->name() << " * ioaddr/iomask: " << hex << _ioaddr << "/" << _iomask << v::endl;
    v::info << this->name() << " * ramaddr/rammask: " << hex << _ramaddr << "/" << _rammask << v::endl;
    v::info << this->name() << " * paddr/pmask: " << hex << _paddr << "/" << _pmask << v::endl;
    v::info << this->name() << " * wprot: " << _wprot << v::endl;
    v::info << this->name() << " * srbanks: " << _srbanks << v::endl;
    v::info << this->name() << " * ram8: " << _ram8 << v::endl;
    v::info << this->name() << " * ram16: " << _ram16 << v::endl;
    v::info << this->name() << " * sepbus: " << _sepbus << v::endl;
    v::info << this->name() << " * sdbits: " << _sdbits << v::endl;
    v::info << this->name() << " * mobile: " << _mobile << v::endl;
    v::info << this->name() << " * sden: " << _sden << v::endl;
    v::info << this->name() << " * hindex: " << hindex << v::endl;
    v::info << this->name() << " * pindex: " << pindex << v::endl;
    v::info << this->name() << " * pow_mon: " << powermon << v::endl;
    v::info << this->name() << " * abstractionLayer (LT = 8 / AT = 4): " << ambaLayer << v::endl;
    v::info << this->name() << " ******************************************************************************* " << v::endl;

    
}

//destructor unregisters callbacks
Mctrl::~Mctrl() {
    GC_UNREGISTER_CALLBACKS();
}

//register GreenReg callback after elaboration
void Mctrl::end_of_elaboration() {
    // create bit accessors for green registers
    // callbacks can then be registers on the defined bits only instead 
    // of the entire register
    // arguments: br.create(name, start bit, end bit)
    r[MCFG2].br.create("lmr", 26, 26);      // tcas needs LMR command
    r[MCFG4].br.create("emr", 0, 6);        // DS, TCSR, PASR need EMR command
    r[MCFG2].br.create("launch", 19, 20);   // SDRAM command field
    r[MCFG4].br.create("pmode", 16, 18);    // SDRAM power saving mode field
    r[MCFG2].br.create("si", 13, 13);       // SRAM disable, address space calculation
    r[MCFG2].br.create("se", 14, 14);       // SDRAM enable, address space calculation
    r[MCFG2].br.create("sr_bk", 9, 12);     // SRAM bank size
    r[MCFG2].br.create("sdr_bk", 23, 25);   // SDRAM bank size
    r[MCFG2].br.create("sdr_trfc", 27, 29); // SDRAM refresh cycle

    //register callbacks
    //The following callbacks affect SDRAM only and are therefore not required if SDRAM is disabled
    GR_FUNCTION(Mctrl, mcfg1_write);
    GR_SENSITIVE(r[MCFG1].add_rule(gs::reg::POST_WRITE, "mcfg1_write", gs::reg::NOTIFY));
    GR_FUNCTION(Mctrl, mcfg2_write);
    GR_SENSITIVE(r[MCFG2].add_rule(gs::reg::POST_WRITE, "mcfg2_write", gs::reg::NOTIFY));
    if (g_sden) {
        GR_FUNCTION(Mctrl, launch_sdram_command);
        GR_SENSITIVE(r[MCFG2].br["launch"].add_rule(gs::reg::POST_WRITE, "sdrcmd", gs::reg::NOTIFY));

        GR_FUNCTION(Mctrl, switch_power_mode);
        GR_SENSITIVE(r[MCFG4].br["pmode"].add_rule(gs::reg::POST_WRITE, "erase_sdr", gs::reg::NOTIFY));
    }

    //initialize mctrl according to generics
    dorst();
}

// Calculate power/energy values from normalized input data
void Mctrl::power_model() {

  // Static power calculation (pW)
  sta_power = sta_power_norm;

  // Cell internal power (uW)
  int_power = int_power_norm * 1/(clock_cycle.to_seconds()*1.0e+6);

  // Energy per read access (uJ)
  dyn_read_energy = dyn_read_energy_norm;

  // Energy per write access (uJ)
  dyn_write_energy = dyn_write_energy_norm;

}

// Static power callback
void Mctrl::sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nominal operation mode only

}

// Internal power callback
void Mctrl::int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Internal power of Mctrl is constant !!

}

// Switching power callback
void Mctrl::swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  swi_power = ((dyn_read_energy * dyn_reads) + (dyn_write_energy * dyn_writes)) / (sc_time_stamp() - power_frame_starting_time).to_seconds();

}

// Automatically called at the beginning of the simulation
void Mctrl::start_of_simulation() {
    uint32_t slaves = mem.size();
    c_rom.id   = 100;
    c_io.id    = 100;
    c_sram.id  = 100;
    c_sdram.id = 100;
    c_null.id  = 100;

    for(uint32_t i = 0; i < slaves; i++) {
        uint32_t a = 0;
        gs::socket::bindability_base<tlm::tlm_base_protocol_types> *other_socket = mem.get_other_side(i, a);
        sc_core::sc_object *obj = other_socket->get_parent();
        MEMDevice * device = dynamic_cast<MEMDevice *>(obj);
        if(device) {
            MEMDevice::type type = device->get_type();
            MEMPort port(i, device);
            switch(type) {
                case MEMDevice::ROM: {
                    if(c_rom.id > 10) {
                        c_rom = port;
                        uint32_t bits = (device->get_bits() >> 3) & 3;
                        r[MCFG1] = r[MCFG1] | (bits << 8);
                    } else {
                        v::error << name() << "More than one ROM is connected to the Controller!" << v::endl;
                    }
                } break;
                case MEMDevice::IO: {
                    if(c_io.id > 10) {
                        c_io = port;
                        uint32_t bits = (device->get_bits() >> 3) & 3;
                        r[MCFG1] = r[MCFG1] | (bits << 27);
                    } else {
                        v::error << name() << "More than one IO area is connected to the Controller!" << v::endl;
                    }
                } break;
                case MEMDevice::SRAM: {
                    if(c_sram.id > 10) {
                        c_sram = port;
                        // set ram width and ram bank size
                        r[MCFG2] = (r[MCFG2].get() & ~0x00001EC0) | 
                                   (((int)(log2(device->get_bsize())-13) & 0xF) << 9) | 
                                   (((int)(log2(device->get_bits())-3) & 0x3) << 4);
                    } else {
                        v::error << name() << "More than one SRAM area is connected to the Controller!" << v::endl;
                    }
                } break;
                case MEMDevice::SDRAM: {
                    if(c_sdram.id > 10) {
                        c_sdram = port;
                        r[MCFG2] = (r[MCFG2].get() & ~0x003E0000) | 
                                     (((int)(log2(device->get_bsize())-22) & 0x7) << 23) | 
                                     (((int)(log2(device->get_cols())-8) & 0x3) << 21);
                    } else {
                        v::error << name() << "More than one SDRAM area is connected to the Controller!" << v::endl;
                    }
                } break;
            }
        } else {
              v::warn << name() << "There is a device connected on the mem bus which is not inherite by MEMDevice named " << obj->name() << v::endl;
        }
    }
    
    // Initialize power model
    if (m_pow_mon) {

      power_model();

    }
}

// Print execution statistic at end of simulation
void Mctrl::end_of_simulation() {
 
  sc_time nominal_time;
 
  switch_power_mode(); 
  v::report << name() << " ********************************************" << v::endl;
  v::report << name() << " * Mctrl Statistic:" << v::endl;
  v::report << name() << " * ----------------" << v::endl;
  v::report << name() << " * Successful Transactions: " << m_right_transactions << v::endl;
  v::report << name() << " * Total Transactions: " << m_total_transactions << v::endl;
  v::report << name() << " *  " << v::endl;
  // Calculate time in nominal operation mode
  nominal_time = sc_time_stamp() - (m_self_refresh_time.getValue() + m_power_down_time.getValue() + m_deep_power_down_time.getValue());

  v::report << name() << " * Total time in nominal operation: " << nominal_time  << " (" << (nominal_time*100/sc_time_stamp()) << "%)" << v::endl;
  v::report << name() << " * Total time in self-refresh mode: " << m_self_refresh_time << " (" << (m_self_refresh_time.getValue()*100/sc_time_stamp()) << "%)" << v::endl;
  v::report << name() << " * Total time in power down mode: " << m_power_down_time << " (" << (m_power_down_time.getValue()*100/sc_time_stamp()) << "%)" << v::endl;
  v::report << name() << " * Total time in deep power down mode: " << m_deep_power_down_time << " (" <<(m_deep_power_down_time.getValue()*100/sc_time_stamp()) << "%)" << v::endl;
  v::report << name() << " *  " << v::endl;
  v::report << name() << " * AHB Slave interface reports: " << v::endl;
  print_transport_statistics(name());
  v::report << name() << " ******************************************** " << v::endl;
}

Mctrl::MEMPort::MEMPort(uint32_t _id, MEMDevice *_dev) : id(_id), dev(_dev), addr(0), length(0) {}
Mctrl::MEMPort::MEMPort() : id(100), dev(NULL), addr(0), length(0) {}

//function to initialize and reset memory address space constants
void Mctrl::dorst() {
    //reset callback delay
    callback_delay = SC_ZERO_TIME;

    r[MCFG1] = 0x000000FF;
    r[MCFG2] = 0x1F100000;
    r[MCFG3] = 0x00000000;
    r[MCFG4] = 0x00F00000 | ((1 && g_mobile) << 31);
    
    //set default values of mobile SDRAM
    if(g_sden) {
        uint32_t mcfg;
        switch(g_mobile) {
            //case 0 is default value (set by initialization)
            case 1:
                //enable mobile SDRAM support
                mcfg = static_cast<uint32_t> (r[MCFG2] | MCFG2_MS);
                r[MCFG2].set(mcfg);
                break;
            case 2:
                //enable mobile SDRAM support
                mcfg = static_cast<uint32_t> (r[MCFG2] | MCFG2_MS);
                r[MCFG2].set(mcfg);
                //enable mobile SDRAM
                mcfg = static_cast<uint32_t> (r[MCFG4] | MCFG4_ME);
                r[MCFG4].set(mcfg);
                break;
                // Case 3 would be the same as 2 here, 
                // the difference being that 3 disables std SDRAM,
                //i.e. mobile cannot be disabled. 
                //This will be implemented wherever someone tries to
                //disable mobile SDRAM.
            default:;
        }
    }

    // --- set register values according to generics
    uint32_t set;
    if(g_sden) {
        set = r[MCFG2] | MCFG2_SDRF | MCFG2_SE;
        if (g_sepbus) {
            set |= g_sdbits << 18;
            r[MCFG2] = set;
        }
    }
    if(c_rom.id != 100 && c_rom.dev != NULL) {
        set = (c_rom.dev->get_bits() >> 3) & 3;
        r[MCFG1] = r[MCFG1] | (set << 8);
    }
    if(c_io.id != 100 && c_io.dev != NULL) {
        set = (c_io.dev->get_bits() >> 3) & 3;
        r[MCFG1] = r[MCFG1] | (set << 27);
    }
    if(c_sram.id != 100 && c_sram.dev != NULL) {
        r[MCFG2] = (r[MCFG2].get() & ~0x00001EC0) | 
                   (((int)(log2(c_sram.dev->get_bsize())-13) & 0xF) << 9) | 
                   (((int)(log2(c_sram.dev->get_bits())-3) & 0x3) << 4);
    }
    if(c_sdram.id != 100 && c_sram.dev != NULL) {
        r[MCFG2] = (r[MCFG2].get() & ~0x003E0000) | 
                   (((int)(log2(c_sdram.dev->get_bsize())-22) & 0x7) << 23) | 
                   (((int)(log2(c_sdram.dev->get_cols())-8) & 0x3) << 21);
    }
}

// Interface to functional part of the model
uint32_t Mctrl::exec_func(tlm_generic_payload &gp, sc_time &delay, bool debug) {

    uint32_t word_delay = 0;
    uint32_t trans_delay = 0;
    uint32_t addr   = gp.get_address();
    uint32_t length = gp.get_data_length();
    uint32_t width  = 4;
    uint32_t mem_width = 4;
    unsigned char *orig_data = gp.get_data_ptr();
    unsigned char *data = orig_data;
    bool rmw = (r[MCFG2].get() >> 6) & 1;
    MEMPort  port   = get_port(addr);
    sc_time mem_delay;
    
    m_total_transactions++;
    
    v::debug << name() << "Try to access memory at " << v::uint32 << addr << " of length " << length << "." << " pmode: " << (uint32_t)m_pmode << v::endl;

    // Log event count for power monitoring
    if (m_pow_mon) {

      if (gp.get_command() == tlm::TLM_READ_COMMAND) {

        dyn_reads += (length >> 2) + 1;
        
      } else {

        dyn_writes += (length >> 2) + 1;

      }
    }

    if(port.id!=100) {
        tlm_generic_payload memgp;
        memgp.set_command(gp.get_command());
        memgp.set_address(port.addr);
        if(port.addr+length<=port.length) {
            // The AHBCtrl will allways have a burst size of 4
            // No need for checking burst size extension.
            // If you want use the component in another system with the need to check
            // Check for the code in an older revision around r560
            width = 4;
            v::debug << name() << "MCFG1: " << v::uint32 << r[MCFG1].get() << ", MCFG2: " << v::uint32 << r[MCFG2].get() << v::endl;
            // Get defined mem bit width from registers.
            switch(port.dev->get_type()) {
                case MEMDevice::ROM:
                    mem_width = (r[MCFG1].get() >> 8) & 0x3;
                    break;
                case MEMDevice::IO:
                    mem_width = (r[MCFG1].get() >> 27) & 0x3;
                    break;
                case MEMDevice::SRAM:
                case MEMDevice::SDRAM:
                    mem_width = (r[MCFG2].get() >> 4) & 0x3;
                    break;
            }
            
            // Set mem_width in byte from bitmask
            switch(mem_width) {
                default: mem_width = 4; break;
                case 1:  mem_width = 2; break;
                case 0:  mem_width = 1; break;
            }
            
            // Calculate delay: The static delay for the hole transaction and the per word delay:
            switch(port.dev->get_type()) {
                case MEMDevice::ROM:
                    rmw = false;
                    if(gp.is_write()) {
                        if(!r[MCFG1].bit_get(11)) {
                            v::error << name() << "Invalid memory access: Writing to PROM is disabled." << v::endl;
                            gp.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
                            return 0;
                        }
                        trans_delay = 0;
                        word_delay = 3 + ((r[MCFG1].get()>>4) & 0xF);
                    } else {
                        trans_delay = 1;
                        word_delay = (2 + ((r[MCFG1].get()>>0) & 0xF));
                        
                        // The RTL Model reads every mem_word as an 32bit word from the memory.
                        // So we need to ensure the same behaviour here we multiply the read times to fit 32bit each.
                        // GRIP 59.5
                        uint32_t tmp = length;
                        // Multiply with the number of memory accesses per length and mem_width.
                        switch(mem_width) {
                            case 1:  switch(tmp) {
                                         case 1: tmp = 4; break;
                                         case 2: tmp = 2; break;
                                         default: tmp = 1; break;
                                     } break;
                            case 2: switch(tmp) {
                                        case 1:
                                        case 2: tmp = 1; break;
                                        default: tmp = 2; break;
                                    } break;
                            default: tmp =1; break;
                        }
                        word_delay *= tmp;
                    }
                    break;
                case MEMDevice::IO:
                    if(!r[MCFG1].bit_get(19)) {
                        v::error << name() << "Invalid memory access: Access to IO is disabled." << v::endl;
                        gp.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
                        return 0;
                    }
                    if(gp.is_write()) {
                        word_delay = ( 3 + ((r[MCFG1].get()>>20) & 0xF));
                    } else {
                        word_delay = ( 5 + ((r[MCFG1].get()>>20) & 0xF));
                    }
                    break;
                case MEMDevice::SRAM:
                    if(gp.is_write()) {
                        trans_delay = 0;
                        word_delay = 3 + ((r[MCFG2].get()>>2) & 0x3);
                        if(rmw && (mem_width>length)) {
                            trans_delay += 0;
                            word_delay += 2 + ((r[MCFG2].get()>>0) & 0x3);
                        }
                    } else {
                        trans_delay = 0;
                        word_delay = 4 + ((r[MCFG2].get()>>0) & 0x3);
                    }
                    break;
                case MEMDevice::SDRAM:
                    // I assume Tcas and Trcd are always equal.
                    // That would mean the delay for a transaction is something like:
                    // Trcd + (words/col_width)*Tcas for read
                    // Trcd + (words/col_width)*Twr for write
                    // And it is by default read modify write, due to the fact that we have to load a column.
                    rmw = true;
                    if(gp.is_write()) {
                        trans_delay = (r[MCFG2].bit_get(26)?2:1);
                        word_delay = 0;//((r[MCFG2].bit_get(26)?3:2));
                    } else {
                        // RCD DELAY
                        trans_delay = 2 + (r[MCFG2].bit_get(30)?3:2);
                        // CAS DELAY
                        word_delay = 3 + (r[MCFG2].bit_get(26)?3:2);
                        //word_delay = 0; //((r[MCFG2].get()>>0) & 0xF);
                    }
                    if(g_mobile) {
                      switch(m_pmode) {
                          default: break;
                          case 1: trans_delay += 1; break; // Power-Down Mode Delay
                          case 2: trans_delay += 1; 
                              v::warn << name() << "The Controller is in Auto-Self-Refresh Mode. Transaction might not be wanted!" <<v::endl;
                              break;  // Auto-Self Refresh
                          case 5: { // Deep power down! No transaction possible:
                              v::error << name() << "The Controller is in Deep-Power-Down Mode. No transactions possible." <<v::endl;
                              gp.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
                              return 0;
                          }
                      } 
                    }
                    break;
            }
            
            v::debug << name() << "Length: " << std::dec << length << ", mem_width: " << std::dec << mem_width << ", width: " << width << v::endl;
            v::debug << name() << "RMW Enabled: " << rmw << v::endl;
            if(gp.is_write()) {
                // RMW in case of subword access
                if(rmw&&length<mem_width) {
                    length = (length & ~(mem_width-1)) + mem_width;
                    v::debug << name() << "RMW Fetch: " << v::uint32 << (uint32_t)(port.addr&~(mem_width-1)) << ", length: " << std::dec << length << ", pos: " << v::uint32 << (uint32_t)(port.addr&(mem_width-1)) << v::endl;
                    // RMW enabled!
                    data = new unsigned char[length];
                    memgp.set_command(TLM_READ_COMMAND);
                    memgp.set_address(port.addr&~(mem_width-1));
                    memgp.set_data_length(length);
                    memgp.set_streaming_width(mem_width);
                    memgp.set_byte_enable_ptr(gp.get_byte_enable_ptr());
                    memgp.set_data_ptr(data);
                    mem[port.id]->b_transport(memgp, mem_delay);
                    memcpy(&data[port.addr&(mem_width-1)], gp.get_data_ptr(), gp.get_data_length());
                    port.addr = port.addr&~(mem_width-1);
                } else if(length<mem_width) {
                    // Error in case of subword access
                    v::error << name() << "Invalid memory access: Transaction width is not compatible with memory width (Transaction-Width: "
                             << width << ", Memory-Width: " << mem_width << ", Data-Length: " << length << ". Please change width or enable Read-Modify-Write Transactions." 
                             <<v::endl;
                    gp.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
                    return 0;
                }
            }
            
            memgp.set_command(gp.get_command());
            memgp.set_address(port.addr);
            memgp.set_data_length(length);
            memgp.set_streaming_width(mem_width);
            memgp.set_byte_enable_ptr(gp.get_byte_enable_ptr());
            memgp.set_data_ptr(data);
            mem[port.id]->b_transport(memgp, mem_delay);
            //transport_statistics(gp);
            gp.set_response_status(memgp.get_response_status());
            m_right_transactions++;

            // Bus Ready used? 
            // If IO Bus Ready take the delay from the memmory.
              // Or if the RAM Bus Ready is set.
            if((port.dev->get_type() == MEMDevice::IO && (r[MCFG1].get() & MCFG1_IBRDY)) ||
               (port.dev->get_type() == MEMDevice::SRAM && r[MCFG2].get() & MCFG2_RBRDY)) {
                delay += mem_delay;
            } else {
                delay += (trans_delay + (((length-1)/mem_width)+1) * word_delay) * clock_cycle;
            }
            if(data!=orig_data) {
              delete[] data;
            }
            return length;
        } else {
            v::warn << name() << "Transaction is including a memory type border." << v::endl;
        }
    } else {
    //no memory device at given address
        v::error << name() << "Invalid memory access: No device at address "
                 << v::uint32 << addr << "." << v::endl;
        gp.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
        return 0;
    }
    
    return length;
}

//--------------CALLBACK--FUNCTIONS--------------//

//write into SDRAM_CMD field of MCFG2
// In preperation for at maybe but the precarge times should not even matter in at!
void Mctrl::launch_sdram_command() {
    if (!g_sden) {
        return;
    }
    uint8_t cmd = (((r[MCFG2] & MCFG2_SDRAM_CMD) >> 19) & 0x000000FF);
    switch(cmd) {
        // LMR / EMR
        case 3:
            break;
        // Auto-Refresh: Forces a refresh, which needs idle state!
        // How can that be guaranteed?
        // Refresh asap, i.e. right after termination of active command? 
        // Always send a precharge before Refresh?
        // Whatever, for LT it's as simple as waiting for exactly 
        // one refresh cycle:
        // --> The previous transaction will always have finished 
        // before the Sim Kernel takes note of this callback.
        case 2:
            callback_delay += clock_cycle *(3 + (MCFG2_SDRAM_TRFC_DEFAULT >> 30));
            break;

        // Precharge: Terminate current burst transaction 
        // (no effect in LT) --> wait for tRP
        case 1:
            callback_delay += clock_cycle * (2 + (MCFG2_TRP_DEFAULT >> 29));
            break;
        default:
            break;
            // Idle ??
    }
    //clear command bits
    uint32_t set = static_cast<uint32_t> (r[MCFG2] & ~MCFG2_SDRAM_CMD);
    r[MCFG2].set(set);
}

// Use data or address field to report the banks to delete bitwise? Maybe?
//change of PMODE
void Mctrl::switch_power_mode() {
  //prepare transaction, including erase extension
    sc_time delay;
    uint32_t data;
    ext_erase* erase = new ext_erase;
    tlm_generic_payload gp;
    gp.set_command(TLM_WRITE_COMMAND);
    gp.set_streaming_width(4);
    gp.set_data_length(4);
    gp.set_data_ptr((unsigned char*)&data);
    gp.set_extension(erase);
    switch(m_pmode) {
        case 1: 
            m_power_down_time = m_power_down_time.getValue() + (sc_time_stamp() - m_power_down_start.getValue());
            break;
        case 2: 
            m_self_refresh_time = m_self_refresh_time.getValue() + (sc_time_stamp() - m_self_refresh_start.getValue());
            break;
        case 5: 
            m_deep_power_down_time = m_deep_power_down_time.getValue() + (sc_time_stamp() - m_deep_power_down_start.getValue());
            break;
        default:
            break;
    }
    v::debug << name() << "set pmode: " << (uint32_t)((r[MCFG4] >> 16) & 0x7) << v::endl;
    switch((r[MCFG4] >> 16) & 0x7) {
        default:
        // None
        case 0: {
            m_pmode = 0;
            v::debug << name() << "Power Mode: None" << v::endl;
            //PM::send_idle(this, "idle", sc_time_stamp(), true);
        }   break;
        
        // Dont do anything in PowerDown mode!
        case 1: { 
            v::debug << name() << "Power Mode: Power Down" << v::endl;
            m_pmode = 1;
            //PM::send_idle(this, "idle_powerdown", sc_time_stamp(), true);
            m_power_down_start = sc_time_stamp();
        }   break;
        
        //partial array self refresh: partially erase SDRAM
        case 2: {
            uint8_t pasr = r[MCFG4] & MCFG4_PASR;
            m_self_refresh_start = sc_time_stamp();
            if(pasr) {
                // Delete
                v::debug << name() << "Power Mode: Partial-Self Refresh" << v::endl;
                uint32_t start = 0;
                uint32_t dbanks = c_sdram.dev->get_banks();
                uint32_t dbsize = c_sdram.dev->get_bsize();
                uint32_t dsize  = dbanks * dbsize;
                switch(pasr) {
                    case 1:
                        start = dsize / 2;
                        //PM::send_idle(this, "idle_partpowerdown2", sc_time_stamp(), true);
                        break;
                    case 2:
                        start = dsize / 4;
                        //PM::send_idle(this, "idle_partpowerdown4", sc_time_stamp(), true);
                        break;
                    case 5:
                        start = dsize / 8;
                        //PM::send_idle(this, "idle_partpowerdown8", sc_time_stamp(), true);
                        break;
                    case 6:
                        start = dsize / 16;
                        //PM::send_idle(this, "idle_partpowerdown16", sc_time_stamp(), true);
                        break;
                }
                gp.set_address(start);
                data = dsize;
                mem[c_sdram.id]->b_transport(gp,delay);
            } else {
                v::debug << name() << "Power Mode: Self Refresh" << v::endl;
                //PM::send_idle(this, "idle_selfrefresh", sc_time_stamp(), true);
            }
        }   break;
        //deep power down: erase entire SDRAM
        case 5: {
            v::debug << name() << "Power Mode: Deep-Power Down" << v::endl;
            //PM::send_idle(this, "idle_deeppowerdown", sc_time_stamp(), true);
            m_pmode = 5;
            uint32_t dbanks = c_sdram.dev->get_banks();
            uint32_t dbsize = c_sdram.dev->get_bsize();
            uint32_t dsize  = dbanks * dbsize;
            // erase all
            gp.set_address(0);
            data = dsize;
            mem[c_sdram.id]->b_transport(gp,delay);
            m_deep_power_down_start = sc_time_stamp();
        }   break;
    }
}

void Mctrl::mcfg1_write() {
    uint32_t mcfg = r[MCFG1].get();
    if((((mcfg>>8)&0x3)==0)&&!g_ram8) {
      mcfg &= ~MCFG1_PROM_WIDTH;
      mcfg |= (2 << 8);
      v::warn << name() << "PROM access with 8bit width configured in MCFG1, but RAM8 Generic is 0 so 32bit access assumed." << v::endl;
    }
    if((((mcfg>>8)&0x3)==1)&&!g_ram16) {
      mcfg &= ~MCFG1_PROM_WIDTH;
      mcfg |= (2 << 8);
      v::warn << name() << "PROM access with 16bit width configured in MCFG1, but RAM16 Generic is 0 so 32bit access assumed." << v::endl;
    }
    v::debug << name() << "Old MCFG1: " << v::uint32 << r[MCFG1].get() 
                       << " new MCFG1: " << v::uint32 << mcfg 
                       << " ram8,16: " << g_ram8 << "," << g_ram16 << v::endl;
    r[MCFG1].set(mcfg);
}

void Mctrl::mcfg2_write() {
    uint32_t mcfg = r[MCFG2].get();
    if(!g_sden) {
      mcfg &= ~MCFG2_SE; 
    }
    if((((mcfg>>4)&0x3)==0)&&!g_ram8) {
      mcfg &= ~MCFG2_RAM_WIDTH;
      mcfg |= (2 << 4);
      v::warn << name() << "RAM access with 8bit width configured in MCFG2, but RAM8 Generic is 0 so 32bit access assumed." << v::endl;
    }
    if((((mcfg>>4)&0x3)==1)&&!g_ram16) {
      mcfg &= ~MCFG2_RAM_WIDTH;
      mcfg |= (2 << 4);
      v::warn << name() << "RAM access with 16bit width configured in MCFG2, but RAM16 Generic is 0 so 32bit access assumed." << v::endl;
    }
    v::debug << name() << "Old MCFG2: " << v::uint32 << r[MCFG2].get() 
                       << " new MCFG2: " << v::uint32 << mcfg 
                       << " ram8,16: " << g_ram8 << "," << g_ram16 << v::endl;
    r[MCFG2].set(mcfg);
}

Mctrl::MEMPort Mctrl::get_port(uint32_t addr) {
    MEMPort result;
    // MEMPort::id of 100 means it is not in use
    // Memorytype not connected
    if(c_rom.id!=100 && (get_bar_addr(0) <= addr) && (addr < get_bar_addr(0) + get_bar_size(0))) {
        // ROM Bar Area
        result = c_rom;
        result.addr = addr - get_bar_addr(0);
        result.length = get_bar_size(0);
        return result;
    } else if(c_io.id!=100 && (get_bar_addr(1) <= addr) && (addr < get_bar_addr(1) + get_bar_size(1))) {
        // IO Bar Area
        result = c_io;
        result.addr = addr - get_bar_addr(1);
        result.length = get_bar_size(1);
        return result;
    } else if((get_bar_addr(2) <= addr) && (addr < get_bar_addr(2) + get_bar_size(2))) {
        // RAM Bar Area
        if(r[MCFG2] & MCFG2_SE) {
            // SDRAM Enabled
            if(c_sdram.id != 100 && (r[MCFG2] & MCFG2_SI)) {
                // And SRAM Disabled
                uint32_t banks = c_sdram.dev->get_banks();
                uint32_t bsize = c_sdram.dev->get_bsize();
                uint32_t size = banks * bsize;
                if(addr < get_bar_addr(2) + size) {
                    result = c_sdram;
                    result.addr = addr - get_bar_addr(2);
                    result.length = size;
                    return result;
                } else {
                    return c_null;
                }
            } else if(c_sram.id != 100 && c_sdram.id != 100) {
                // And SRAM Enabled
                uint32_t sbanks = min(c_sram.dev->get_banks(), 4u);
                uint32_t sbsize = c_sram.dev->get_bsize();
                uint32_t ssize  = sbanks * sbsize;
                uint32_t dbanks = min(c_sdram.dev->get_banks(), 2u);
                uint32_t dbsize = c_sdram.dev->get_bsize();
                uint32_t dsize  = dbanks * dbsize;
                if(addr < get_bar_addr(2) + ssize) {
                    result = c_sram;
                    result.addr = addr - get_bar_addr(2);
                    result.length = ssize;
                    return result;
                } else if(c_sdram.id != 100 && (addr < get_bar_addr(2) + ssize + dsize)) {
                    result = c_sdram;
                    result.addr = addr - get_bar_addr(2) - ssize;
                    result.length = dsize;
                    return result;
                } else {
                    return c_null;
                }
            } else {
              return c_null;
            }
        } else if(c_sram.id != 100) {
            // SDRAM Disabled, just SRAM
            uint32_t banks = c_sram.dev->get_banks();
            uint32_t bsize = c_sram.dev->get_bsize();
            uint32_t size = 0;
            if(banks < 5) {
                size = banks * bsize;
            } else {
                size = 8 * bsize;
            }
            if(addr < get_bar_addr(2) + size) {
                // addres has to be shortend!
                result = c_sram;
                result.addr = addr - get_bar_addr(2);
                result.length = size;
                return result;
            } else {
                return c_null;
            }
        } else {
            return c_null;
        }
    }
    return c_null;
}

sc_core::sc_time Mctrl::get_clock() {

  return clock_cycle;

}
