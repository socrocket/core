// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup irqmp
/// @{
/// @file irqmp.cpp
/// Implementation of multi-processor interrupt controller (IRQMP).
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#include <string>
#include <utility>

#include "gaisler/irqmp/irqmp.h"
#include "core/common/verbose.h"

SR_HAS_MODULE(Irqmp);

/// Constructor
Irqmp::Irqmp(ModuleName name,
  int paddr,
  int pmask,
  int ncpu,
  int eirq,
  unsigned int pindex,
  bool powmon) :
  APBSlave(name, pindex, 0x01, 0x00D, 2, /* VER: SoCRocket default: 3, try to Mimic TSIM therefore 2 -- psiegl */
            0, APBIO, pmask, false, false, paddr),
  cpu_rst("cpu_rst"), 
  cpu_stat("cpu_stat"),
  irq_req("cpu_req"),
  irq_ack(&Irqmp::acknowledged_irq, "irq_ack"),
  irq_in(&Irqmp::incomming_irq, "irq_in"),
  g_ncpu("ncpu", ncpu, m_generics), 
  g_eirq("eirq", eirq, m_generics),
  m_irq_counter("irq_line_activity", 32, m_counters),
  m_cpu_counter("cpu_line_activity", ncpu, m_counters),
  m_pow_mon(powmon),
  sta_power_norm("sta_power_norm", 3.07e+8, m_power),           // Normalized static power of controller
  int_power_norm("int_power_norm", 3.26e-10, m_power),           // Normalized internal power of controller
  sta_power("sta_power", 0.0, m_power),           // Static power of controller
  int_power("int_power", 0.0, m_power) {          // Dynamic power of controller

  forcereg = new uint32_t[g_ncpu];
  Irqmp::init_generics(); 
  // Display APB slave information
  srInfo()
    ("paddr", paddr)
    ("pmask", pmask)
    ("pindex", pindex)
    ("APB slave");

  assert(ncpu < 17 && "the IRQMP can only handle up to 16 CPUs");

  init_registers();

  SC_THREAD(launch_irq);

  // Initialize performance counter by zerooing
  // Keep in mind counter will not be reseted in reset
  for (int i = 0; i < 32; i++) {
    m_irq_counter[i] = 0;
  }

  if (m_pow_mon) {
    GC_REGISTER_TYPED_PARAM_CALLBACK(&sta_power, gs::cnf::pre_read, Irqmp, sta_power_cb);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&int_power, gs::cnf::pre_read, Irqmp, int_power_cb);
  }

  srInfo()
    ("paddr", paddr)
    ("pmask", pmask)
    ("pindex", pindex)
    ("ncpu", ncpu)
    ("eirq", eirq)
    ("pow_mon", powmon)
    ("Created an Irqmp with this parameters");
}

Irqmp::~Irqmp() {
  delete[] forcereg;
}

void Irqmp::init_generics() {
  g_ncpu.add_properties()
    ("name"," Number of CPUs in the System")
    ("vhdl_name","ncpu")
    ("Needet to determ the number of receiver lines.");
    
    g_eirq.add_properties()
    ("name"," Extended Interrupt Number")
    ("vhdl_name","eirq")
    ("Behind this interrupt are all extended interrupt cascaded.");
}

void Irqmp::init_registers() {
  // create register | name + description
  r.create_register("level", "Interrupt Level Register",
    0x00,                   // offset
    0x00000000,             // init value
    Irqmp::IR_LEVEL_IL);    // write mask

  r.create_register("pending", "Interrupt Pending Register", 
    0x04,
    0x00000000,
    IR_PENDING_EIP | IR_PENDING_IP)
  .callback(SR_POST_WRITE, this, &Irqmp::pending_write);

  // Following register is part of the manual, but will never be used.
  // 1) A system with 0 cpus will never be implemented
  // 2) If there were 0 cpus, no cpu would need an IR force register
  // 3) The IR force register for the first CPU ('CPU 0') will always be located at address 0x80
  r.create_register("force", "Interrupt Force Register", 
    0x08,
    0x00000000, 
    IR_FORCE_IF)
  .callback(SR_POST_WRITE, this, &Irqmp::pending_write);

  r.create_register("clear", "Interrupt Clear Register",
    0x0C, 0x00000000, IR_CLEAR_IC)
  .callback(SR_POST_WRITE, this, &Irqmp::clear_write);

  r.create_register("mpstat", "Multiprocessor Status Register",
    0x10, 0x00000000, MP_STAT_WMASK)
  .callback(SR_PRE_READ, this, &Irqmp::mpstat_read)
  .callback(SR_POST_WRITE, this, &Irqmp::mpstat_write);

  r.create_register("broadcast", "Interrupt broadcast Register",
    0x14, 0x00000000, BROADCAST_BM);

  // 3 IRQ(A)MP Registers pinned to 0 to indecate that we are a single IRQMP
  r.create_register("asymctrl", "Asymetric Multicore Control Register",
    0x20, 0x00000000, 0x00000000);
  r.create_register("irqctrlsel0", "Interrupt Controller Select Register for Processors 0 - 7",
    0x24, 0x00000000, 0x00000000);
  r.create_register("irqctrlsel1", "Interrupt Controller Select Register for Processors 8 - 15",
    0x28, 0x00000000, 0x00000000);

  for (int i = 0; i < g_ncpu; ++i) {
    r.create_register(gen_unique_name("mask", false),
      "Interrupt Mask Register", 
      0x40 + 0x4 * i,
      0xFFFFFFFE,
      PROC_MASK_EIM | Irqmp::PROC_MASK_IM)
    // send interrupts to processors after write to pending / force regs
    .callback(SR_POST_WRITE, this, &Irqmp::pending_write);

    r.create_register(gen_unique_name("force", false),
      "Interrupt Force Register",
      0x80 + 0x4 * i,
      0x00000000, 
      PROC_IR_FORCE_IFC | PROC_IR_FORCE_IF)
    .callback(SR_POST_WRITE, this, &Irqmp::force_write);

    r.create_register(gen_unique_name("eir_id", false),
      "Extended Interrupt Identification Register",
      0xC0 + 0x4 * i,
      0x00000000,
      PROC_EXTIR_ID_EID);

    // Reset corresponding performance counter
    m_cpu_counter[i] = 0;
    forcereg[i] = 0;
  }
}

// Automatically called at start of simulation
void Irqmp::start_of_simulation() {
  // Initialize power model
  if (m_pow_mon) {
    power_model();
  }
}

void Irqmp::end_of_elaboration() {
}

// Print execution statistic at end of simulation
void Irqmp::end_of_simulation() {
  v::report << name() << " ********************************************" << v::endl;
  v::report << name() << " * IRQMP statistic:" << v::endl;
  v::report << name() << " * ================" << v::endl;
  uint64_t sum = 0;
  for (int i = 1; i < 32; i++) {
    v::report << name() << " * + IRQ Line " << std::dec << i << ":    " << m_irq_counter[i] << v::endl;
    sum += m_irq_counter[i];
  }
  v::report << name() << " * -------------------------------------- " << v::endl;
  v::report << name() << " * = Sum      :    " << sum << v::endl;
  v::report << name() << " * " << v::endl;
  sum = 0;
  for (int i = 0; i < g_ncpu; i++) {
    v::report << name() << " * + CPU Line " << std::dec << i << ":    " << m_cpu_counter[i] << v::endl;
    sum += m_cpu_counter[i];
  }
  v::report << name() << " * -------------------------------------- " << v::endl;
  v::report << name() << " * = Sum      :    " << sum << v::endl;
  v::report << name() << " ******************************************** " << v::endl;
}

void Irqmp::power_model() {
  // Static power calculation (pW)
  sta_power = sta_power_norm;

  // Cell internal power (uW)
  int_power = int_power_norm * 1 / (clock_cycle.to_seconds());
}

// Static power callback
gs::cnf::callback_return_type Irqmp::sta_power_cb(
    gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
    gs::cnf::callback_type reason) {
  // Nothing to do !!
  // Static power of Irqmp is constant !!
  return GC_RETURN_OK;
}

// Internal power callback
gs::cnf::callback_return_type Irqmp::int_power_cb(
    gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
    gs::cnf::callback_type reason) {
  // Nothing to do !!
  // Internal power of Irqmp is constant !!
  return GC_RETURN_OK;
}

// Reset registers to default values
// Process sensitive to reset signal
void Irqmp::dorst() {
  // initialize registers with values defined above
  srInfo()("Do Reset");
  r[IR_LEVEL]   = static_cast<uint32_t>(LEVEL_DEFAULT);
  r[IR_PENDING] = static_cast<uint32_t>(PENDING_DEFAULT);
  if (g_ncpu == 0) {
    r[IR_FORCE] = static_cast<uint32_t>(FORCE_DEFAULT);
  }
  r[IR_CLEAR] = static_cast<uint32_t>(CLEAR_DEFAULT);
  // mp status register contains g_ncpu and g_eirq at bits 31..28 and 19..16 respectively
  r[MP_STAT] = 0xFFFE | (g_ncpu << 28) | (g_eirq << 16);
  r[BROADCAST] = static_cast<uint32_t>(BROADCAST_DEFAULT);
  for (int cpu = 0; cpu < g_ncpu; cpu++) {
    r[PROC_IR_MASK(cpu)]  = static_cast<uint32_t>(MASK_DEFAULT);
    r[PROC_IR_FORCE(cpu)] = static_cast<uint32_t>(PROC_FORCE_DEFAULT);
    r[PROC_EXTIR_ID(cpu)] = static_cast<uint32_t>(EXTIR_ID_DEFAULT);
    forcereg[cpu] = 0;
  }
  cpu_rst.write(1, true);
}

//  - watch interrupt bus signals (apbi.pirq)
//  - write incoming interrupts into pending or force registers
//
// process sensitive to apbi.pirq
void Irqmp::incomming_irq(const std::pair<uint32_t, bool> &irq, const sc_time &time) {
  if (!irq.second) {
    // Return if the value turned to false.
    // Interrupts will not be unset this way.
    // So we cann simply ignore a false value.
    return;
  }
  for(int32_t line = 0; line<32; line++) {
    if((1 << line) & irq.first) {
      // Performance counter increase
      m_irq_counter[line] = m_irq_counter[line] + 1;
      v::debug << name() << "Interrupt line " << line << " triggered" << v::endl;

      // If the incomming interrupt is not listed in the broadcast register
      // it goes in the pending register
      if (!r[BROADCAST].bit(line+1)) {
        r[IR_PENDING].bit(line, true);
      }

      // If it is not listed n the broadcast register and not an extended interrupt it goes into the force registers.
      // EIRs cannot be forced
      if (r[BROADCAST].bit(line+1) && (line < 16)) {
        // set force registers for broadcasted interrupts
        for (int32_t cpu = 0; cpu < g_ncpu; cpu++) {
          r[PROC_IR_FORCE(cpu)].bit(line, true);
          forcereg[cpu] |= (true << line);
        }
      }
    }
  }
  // Pending and force regs are set now.
  // To call an explicit launch_irq signal is set here
  e_signal.notify(2 * clock_cycle);
}

// launch irq:
//  - combine pending, force, and mask register
//  - prioritize pending interrupts
//  - send highest priority IR to processor via irqo port
//
// callback registered on IR pending register,
//                        IR force registers
void Irqmp::launch_irq() {
  int16_t high;
  uint32_t masked, pending, all;
  bool eirq_en;
  int cpu = 0;
  while (1) {
    wait(e_signal);
    for (cpu = g_ncpu - 1; cpu > -1; cpu--) {
      // Pending register for this CPU line.
      pending = (r[IR_PENDING] | r[IR_FORCE]) & r[PROC_IR_MASK(cpu)];
      v::debug << name() << "For CPU " << cpu << " pending: " << v::uint32 << r[IR_PENDING].read() << ", force: " <<
      v::uint32 << r[IR_FORCE].read() << ", proc_ir_mask: " << r[PROC_IR_MASK(cpu)].read() << v::endl;

      // All relevant interrupts for this CPU line to determ pending extended interrupts
      masked  = pending | (r[PROC_IR_FORCE(cpu)] & IR_FORCE_IF);
      // if any pending extended interrupts
      if (g_eirq != 0) {
        // Set the pending pit in the pending register.
        eirq_en = masked & IR_PENDING_EIP;
        r[IR_PENDING].bit(g_eirq, eirq_en);
      } else {
        eirq_en = 0;
      }
      // Recalculate relevant interrupts
      all = pending | (eirq_en << g_eirq) | (r[PROC_IR_FORCE(cpu)] & IR_FORCE_IF);
      v::debug << name() << "For CPU " << cpu << " pending: " << v::uint32 << pending << ", all " << v::uint32 << all <<
      v::endl;

      // Find the highes not extended interrupt on level 1
      masked = (all & r[IR_LEVEL]) & IR_PENDING_IP;
      for (high = 15; high > 0; high--) {
        if (masked & (1 << high)) {
          break;
        }
      }

      // If no IR on level 1 found check level 0.
      if (high == 0) {
        // Find the highes not extended interrupt on level 0
        masked = (all & ~r[IR_LEVEL]) & IR_PENDING_IP;
        for (high = 15; high > 0; high--) {
          if (masked & (1 << high)) {
            break;
          }
        }
      }
      // If an interrupt is selected send it out to the CPU.
      if (high != 0) {
        v::debug << name() << "For CPU " << cpu << " send IRQ: " << high << v::endl;
        std::pair<uint32_t, bool> value(0xF & high, true);
        if (value != irq_req.read(cpu)) {
          v::debug << name() << "For CPU " << cpu << " really sent IRQ: " << high << v::endl;
          irq_req.write(1 << cpu, value);

          m_cpu_counter[cpu]++;
        }
      } else if (irq_req.read(cpu).first != 0) {
        irq_req.write(1 << cpu, std::pair<uint32_t, bool>(0, false));
      }
    }
  }
}

//
// clear acknowledged IRQs                                           (three processes)
//  - interrupts can be cleared
//    - by software writing to the IR_Clear register                 (process 1: clear_write)
//    - by software writing to the upper half of IR_Force registers  (process 2: Clear_forced_ir)
//    - by processor sending irqi.intack signal and irqi.irl data    (process 3: clear_acknowledged_irq)
//  - remove IRQ from pending / force registers
//  - in case of eirq, release eirq ID in eirq ID register
//
// callback registered on interrupt clear register
void Irqmp::clear_write() {
  // pending reg only: forced IRs are cleared in the next function
  bool extirq = false;
  for (int cpu = 0; cpu < g_ncpu; cpu++) {
    if ((g_eirq != 0) && (r[PROC_EXTIR_ID(cpu)] != 0)) {
      r[IR_PENDING] = r[IR_PENDING] & ~(1 << r[PROC_EXTIR_ID(cpu)]);
      r[PROC_EXTIR_ID(cpu)] = 0;
      extirq = true;
    }
  }
  for (int i = 31; i > 15; --i) {
    if ((1 << i) & r[IR_CLEAR]) {
      extirq = true;
    }
  }
  if (extirq) {
    irq_req.write(~0, std::pair<uint32_t, bool>(g_eirq, false));
  }
  for (int i = 15; i > 0; --i) {
    if ((1 << i) & r[IR_CLEAR]) {
      irq_req.write(~0, std::pair<uint32_t, bool>(i, false));
    }
  }

  uint32_t cleared_vector = r[IR_PENDING] & ~r[IR_CLEAR];
  r[IR_PENDING] = cleared_vector;
  cleared_vector = r[IR_FORCE] & ~r[IR_CLEAR];
  r[IR_FORCE] = cleared_vector;
  r[IR_CLEAR]   = 0;
  e_signal.notify(2 * clock_cycle);
}

// callback registered on interrupt force registers
void Irqmp::force_write() {
  for (int cpu = 0; cpu < g_ncpu; cpu++) {
    uint32_t reg = r[PROC_IR_FORCE(cpu)];
    for (int i = 15; i > 0; --i) {
      // Set irqs to zero for all cleard once
      if ((1 << i) & (reg >> 16)) {
        irq_req.write(~0, std::pair<uint32_t, bool>(i, false));
      }
    }

    v::debug << name() << "Force " << cpu << "  write " << v::uint32 << forcereg[cpu] << " old " << v::endl;
    forcereg[cpu] |= reg;

    // write mask clears IFC bits:
    // IF && !IFC && write_mask
    forcereg[cpu] &= (~(forcereg[cpu] >> 16) & PROC_IR_FORCE_IF);

    r[PROC_IR_FORCE(cpu)] = forcereg[cpu];
    v::debug << name() << "Force " << cpu << "  write " << v::uint32 << forcereg[cpu] << v::endl;
    if ((g_eirq != 0) && (r[PROC_EXTIR_ID(cpu)] != 0)) {
      r[IR_PENDING] = r[IR_PENDING] & ~(1 << r[PROC_EXTIR_ID(cpu)]);
    }
  }
  e_signal.notify(2 * clock_cycle);
}

// process sensitive to ack_irq
void Irqmp::acknowledged_irq(const uint32_t &irq, const uint32_t &cpu, const sc_core::sc_time &time) {
  bool f = false;
  v::debug << name() << "Acknowledgeing IRQ " << irq << " from CPU " << cpu << v::endl;
  if ((g_eirq != 0) && (r[PROC_EXTIR_ID(cpu)] != 0)) {
    r[IR_PENDING] = r[IR_PENDING] & ~(1 << r[PROC_EXTIR_ID(cpu)]);
  }

  // clear interrupt from pending and force register
  // if(r[BROADCAST].bit_get(irq)) {
  r[PROC_IR_FORCE(cpu)].bit(irq, f);
  forcereg[cpu] &= ~(1 << irq) & 0xFFFE;
  // }

  irq_req.write(~0, std::pair<uint32_t, bool>(irq, false));
  r[IR_PENDING].bit(irq, f);
  r[IR_FORCE].bit(irq, f);
  r[PROC_EXTIR_ID(cpu)] = 0;
  e_signal.notify(2 * clock_cycle);
}

// reset cpus after write to cpu status register
// callback registered on mp status register
void Irqmp::mpstat_write() {
  uint32_t stat = r[MP_STAT] & 0xFFFF;
  srDebug()("mpstat", stat)("written mpstat");
  for (int i = 0; i < g_ncpu; i++) {
    if ((stat & (1 << i)) && !cpu_stat.read(i)) {
      cpu_rst.write(1 << i, true);
      srDebug()("cpu", i)("Enable CPU");
    }
  }
}

void Irqmp::mpstat_read() {
  uint32_t reg = (g_ncpu << 28) | (g_eirq << 16);
  for (int i = 0; i < g_ncpu; i++) {
    reg |= ((!cpu_stat.read(i)) << i);
    srDebug()("cpu", i)("CPU enabled");
  }
  r[MP_STAT] = reg;
  srDebug()("mpstat", reg)("new mpstat");
}

void Irqmp::pending_write() {
  v::info << name() << "Pending write" << v::endl;
  e_signal.notify(1 * clock_cycle);
}

/// @}
