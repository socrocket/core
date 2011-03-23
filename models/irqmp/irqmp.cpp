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
// Title:      irqmp.tpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    implementation of irqmp module
//             is included by irqmp.h template header file
//
// Method:
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Dennis Bode
// Reviewed:
//*********************************************************************

/*  2-DO
 - take care of pindex / pconfig signals (apbo: PnP)
 - take care of pwd and fpen signals (irqi)
 - take care of rst, run, and rstvec signals (irqo)
 */

/// @addtogroup irqmp
/// @{

#define COUT_TIMING

#include "irqmp.h"
#include "verbose.h"
//#include "irqmpregisters.h"
#include <string>

/// Constructor
CIrqmp::CIrqmp(sc_core::sc_module_name name, int _paddr, int _pmask, int _ncpu, int _eirq) :
            gr_device(name, //sc_module name
                    gs::reg::ALIGNED_ADDRESS, //address mode (options: aligned / indexed)
                    0xFF, //dword size (of register file)
                    NULL //parent module
            ),
            apb_slv(
                    "APB_SLAVE", //name
                    r, //register container
                    (_paddr & _pmask) << 8, // start address
                    (((~_pmask & 0xfff) + 1) << 8), // register space length
                    ::amba::amba_APB, // bus type
                    ::amba::amba_LT, // communication type / abstraction level
                    false // not used
            ), 
            rst(&CIrqmp::reset_registers, "RESET"), 
            cpu_rst("CPU_RESET"), irq_req("CPU_REQUEST"), 
            irq_ack(&CIrqmp::acknowledged_irq,"IRQ_ACKNOWLEDGE"), 
            irq_in(&CIrqmp::register_irq, "IRQ_INPUT"), 
            ncpu(_ncpu), eirq(_eirq) {

            forcereg = new uint32_t[ncpu];
   // Display APB slave information
   v::info << name << "APB slave @0x" << hex << v::setw(8)
           << v::setfill('0') << apb_slv.get_base_addr() << " size: 0x" << hex
           << v::setw(8) << v::setfill('0') << apb_slv.get_size() << " byte"
           << endl;

    // create register | name + description
    r.create_register("level", "Interrupt Level Register",
            // offset
            0x00,
            
            // config
            gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER
                    | gs::reg::FULL_WIDTH,
            
            // init value
            0x00000000,
            
            // write mask
            CIrqmp::IR_LEVEL_IL,
            
            // reg width (maximum 32 bit)
            32,
            
            // lock mask: Not implementet, has to be zero.
            0x00);
    
    r.create_register("pending", "Interrupt Pending Register", 0x04,
            gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | 
            gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH, 0x00000000, 
            IR_PENDING_EIP | IR_PENDING_IP, 32, 0x00);
    //Following register is part of the manual, but will never be used.
    // 1) A system with 0 cpus will never be implemented
    // 2) If there were 0 cpus, no cpu would need an IR force register
    // 3) The IR force register for the first CPU ('CPU 0') will always be located at address 0x80
    if (ncpu == 0) {
        r.create_register("force", "Interrupt Force Register", 0x08,
                gs::reg::STANDARD_REG | gs::reg::SINGLE_IO
                        | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                0x00000000, IR_FORCE_IF, 32, 0x00);
    }
    r.create_register("clear", "Interrupt Clear Register", 0x0C,
            gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER
                    | gs::reg::FULL_WIDTH, 0x00000000, IR_CLEAR_IC, 32,
            0x00);
    r.create_register("mpstat", "Multiprocessor Status Register", 0x10,
            gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER
                    | gs::reg::FULL_WIDTH, 0x00000000, MP_STAT_NCPU
                    | MP_STAT_EIRQ | MP_STAT_STAT(), 32, 0x00);
    r.create_register("broadcast", "Interrupt broadcast Register", 0x14,
            gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER
                    | gs::reg::FULL_WIDTH, 0x00000000, BROADCAST_BM, 32,
            0x00);

    for (int i = 0; i < ncpu; ++i) {
        r.create_register(gen_unique_name("mask", false),
                "Interrupt Mask Register", 0x40 + 0x4 * i,
                gs::reg::STANDARD_REG | gs::reg::SINGLE_IO
                        | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                0xFFFFFFFE, PROC_MASK_EIM | CIrqmp::PROC_MASK_IM, 32, 0x00);
        r.create_register(gen_unique_name("force", false),
                "Interrupt Force Register", 0x80 + 0x4 * i,
                gs::reg::STANDARD_REG | gs::reg::SINGLE_IO
                        | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                0x00000000, PROC_IR_FORCE_IFC | PROC_IR_FORCE_IF, 32,
                0x00);
        r.create_register(gen_unique_name("eir_id", false),
                "Extended Interrupt Identification Register", 0xC0 + 0x4 * i,
                gs::reg::STANDARD_REG | gs::reg::SINGLE_IO
                        | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                0x00000000, PROC_EXTIR_ID_EID, 32, 0x00);
    }

    SC_THREAD(launch_irq); 
}

/// Destructor
CIrqmp::~CIrqmp() {
    GC_UNREGISTER_CALLBACKS();
}

/// Hook up callback functions to registers
void CIrqmp::end_of_elaboration() {

    // send interrupts to processors after write to pending / force regs
    GR_FUNCTION(CIrqmp, pending_write); // args: module name, callback function name
    GR_SENSITIVE(r[IR_PENDING].add_rule(gs::reg::POST_WRITE,"pending_write", gs::reg::NOTIFY));

    // unset pending bits of cleared interrupts
    GR_FUNCTION(CIrqmp, clear_write);
    GR_SENSITIVE(r[IR_CLEAR].add_rule(gs::reg::POST_WRITE, "clear_write", gs::reg::NOTIFY));

    // unset pending bits of cleared interrupts
    for (int i_cpu = 0; i_cpu < ncpu; i_cpu++) {
        GR_FUNCTION(CIrqmp, force_write);
        GR_SENSITIVE(r[PROC_IR_FORCE(i_cpu)].add_rule(
                gs::reg::POST_WRITE, gen_unique_name("force_write", false), gs::reg::NOTIFY));
    }

    // manage cpu run / reset signals after write into MP status reg
    GR_FUNCTION(CIrqmp, mpstat_write);
    GR_SENSITIVE(r[MP_STAT].add_rule(gs::reg::POST_WRITE, "mpstat_write", gs::reg::NOTIFY));
}

//P R O C E S S   I M P L E M E N T A T I O N

/// Reset registers to default values
/// Process sensitive to reset signal
void CIrqmp::reset_registers(const bool &value, const sc_core::sc_time &time) {
    if (!value) {
        //mp status register contains ncpu and eirq at bits 31..28 and 19..16 respectively
        uint32_t stat_ncpu = ncpu << 28;
        uint32_t stat_eirq = eirq << 16;

        //initialize registers with values defined above
        r[IR_LEVEL]   = static_cast<uint32_t>(LEVEL_DEFAULT);
        r[IR_PENDING] = static_cast<uint32_t>(PENDING_DEFAULT);
        if (ncpu == 0) {
            r[IR_FORCE] = static_cast<uint32_t>(FORCE_DEFAULT);
        }
        r[IR_CLEAR] = static_cast<uint32_t>(CLEAR_DEFAULT);
        r[MP_STAT] = MP_STAT_DEFAULT | stat_ncpu | stat_eirq;
        r[BROADCAST] = static_cast<uint32_t>(BROADCAST_DEFAULT);
        for(int cpu = 0; cpu < ncpu; cpu++) {
            r[PROC_IR_MASK(cpu)]  = static_cast<uint32_t>(MASK_DEFAULT);
            r[PROC_IR_FORCE(cpu)] = static_cast<uint32_t>(PROC_FORCE_DEFAULT);
            r[PROC_EXTIR_ID(cpu)] = static_cast<uint32_t>(EXTIR_ID_DEFAULT);
            forcereg[cpu] = 0;
        }
    }
}

///
/// register irq
///  - watch interrupt bus signals (apbi.pirq)
///  - write incoming interrupts into pending or force registers
///
/// process sensitive to apbi.pirq
void CIrqmp::register_irq(const bool &value, const uint32_t &irq,
                          const sc_core::sc_time &time) {
    if(!value) {
        return;
    }
    //set pending register
    bool t = true;
    if(!r[BROADCAST].bit_get(irq)) {
        r[IR_PENDING].bit_set(irq, t);
    }
    
    // EIRs cannot be forced
    if(r[BROADCAST].bit_get(irq) && (irq < 16)) {
        //set force registers for broadcasted interrupts
        for(int32_t cpu = 0; cpu < ncpu; cpu++) { 
            r[PROC_IR_FORCE(cpu)].bit_set(irq, t);
            forcereg[cpu] |= (t << irq);
        }
    }
    // Pending and force regs are set now. To call an explicit launch_irq signal is set here
    signal.notify(2 * CLOCK_PERIOD, SC_NS);
}

///
/// launch irq:
///  - combine pending, force, and mask register
///  - prioritize pending interrupts
///  - send highest priority IR to processor via irqo port
///
/// callback registered on IR pending register,
///                        IR force registers
void CIrqmp::launch_irq() {
    int16_t high;          // highest priority interrupt (to be launched)
    uint32_t masked, pending, all;
    bool eirq_en;
    while(1) {
        wait(signal);
        for(int cpu = 0; cpu < ncpu; cpu++) {
            // process call might be triggered by acknowledge of forced IR by writing to IFC bits of IF register
            // masked = (pending || force) && mask
            pending = r[IR_PENDING] & r[PROC_IR_MASK(cpu)];
            masked  = pending | (r[PROC_IR_FORCE(cpu)] & IR_FORCE_IF);
            // any pending extended interrupts?
            if(eirq != 0) {
                eirq_en = masked & IR_PENDING_EIP;
                r[IR_PENDING].bit_set(eirq, eirq_en);
            } else {
                eirq_en = 0;
            }
            all = pending | (eirq_en << eirq) | (r[PROC_IR_FORCE(cpu)] & IR_FORCE_IF);
            
            // prioritize interrupts
            // (pending or force) and mask and level
            masked = (all & r[IR_LEVEL]) & IR_PENDING_IP;
            for(high = 15; high > 0; high--) {
                if(masked & (1 << high)) {
                    break;
                }
            }

            // If no IR on level 1, check level 0.
            if(high == 0) {
                // (pending or force) and mask and not level
                masked = (all & ~r[IR_LEVEL]) & IR_PENDING_IP;
                for(high = 15; high > 0; high--) {
                    if(masked & (1 << high)) {
                        break;
                    }
                }
            }
            if(high!=0) {
                irq_req.write(1 << cpu, 0xF & high);
            }
        }
    }
}

///
/// clear acknowledged IRQs                                           (three processes)
///  - interrupts can be cleared
///    - by software writing to the IR_Clear register                 (process 1: clear_write)
///    - by software writing to the upper half of IR_Force registers  (process 2: Clear_forced_ir)
///    - by processor sending irqi.intack signal and irqi.irl data    (process 3: clear_acknowledged_irq)
///  - remove IRQ from pending / force registers
///  - in case of eirq, release eirq ID in eirq ID register
///
/// callback registered on interrupt clear register
void CIrqmp::clear_write() {
    // pending reg only: forced IRs are cleared in the next function
    for(int cpu = 0; cpu < ncpu; cpu++) {
        if(eirq != 0) {
            r[IR_PENDING] = r[IR_PENDING] & ~(1 << r[PROC_EXTIR_ID(cpu)]);
            r[PROC_EXTIR_ID(cpu)] = 0;
        }
    }
    uint32_t cleared_vector = r[IR_PENDING] & ~r[IR_CLEAR];
    r[IR_PENDING] = cleared_vector;
    r[IR_CLEAR]   = 0;
    signal.notify(2 * CLOCK_PERIOD, SC_NS);
}

/// callback registered on interrupt force registers
void CIrqmp::force_write() {
    for(int cpu = 0; cpu < ncpu; cpu++) {
        forcereg[cpu] |= r[PROC_IR_FORCE(cpu)];
        //write mask clears IFC bits:
        //IF && !IFC && write_mask
        forcereg[cpu] &= (~(forcereg[cpu] >> 16) & PROC_IR_FORCE_IF);
        r[PROC_IR_FORCE(cpu)] = forcereg[cpu];
    }
    signal.notify(2 * CLOCK_PERIOD, SC_NS);
}

/// process sensitive to ack_irq
void CIrqmp::acknowledged_irq(const uint32_t &irq, const uint32_t &cpu, const sc_core::sc_time &time) {
    bool f = false;
    //extended IR handling: Identify highest pending EIR
    if(eirq != 0 && irq == (uint32_t)eirq && !(eirq&r[BROADCAST])) {
        r[IR_PENDING] = r[IR_PENDING] & ~(1 << r[PROC_EXTIR_ID(cpu)]);
        r[PROC_IR_FORCE(cpu)] = r[PROC_IR_FORCE(cpu)] & ~(1 << r[PROC_EXTIR_ID(cpu)]);
        forcereg[cpu] &= ~(1 << r[PROC_EXTIR_ID(cpu)]) & 0xFFFE;
    } else {
        //clear interrupt from pending and force register
        r[IR_PENDING].bit_set(irq, f);
        if(r[BROADCAST].bit_get(irq)) {
            r[PROC_IR_FORCE(cpu)].bit_set(irq, f);
            forcereg[cpu] &= ~(1 << irq) & 0xFFFE;
        }
    }
    r[PROC_EXTIR_ID(cpu)] = 0;
    signal.notify(2 * CLOCK_PERIOD, SC_NS);
}

/// reset cpus after write to cpu status register
/// callback registered on mp status register
void CIrqmp::mpstat_write() {
    cpu_rst.write(0xFFFFFFFF, true);
}

void CIrqmp::pending_write() {
    signal.notify(2 * CLOCK_PERIOD, SC_NS);
}


/// @}
