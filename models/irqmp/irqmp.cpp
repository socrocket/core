/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       irqmp.tpp                                               */
/*             implementation of irqmp module                          */
/*             is included by irqmp.h template header file             */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

/*  2-DO
- take care of pindex / pconfig signals (apbo: PnP)
- take care of pwd and fpen signals (irqi)
- take care of rst, run, and rstvec signals (irqo)
*/

#ifndef IRQMP_TPP
#define IRQMP_TPP

/// @addtogroup irqmp
/// @{

#define COUT_TIMING

#include "irqmp.h"
#include "irqmpregisters.h"
#include <string>

#define LEVEL                    (0x00)
#define PENDING                  (0x04)
#define FORCE                    (0x08)
#define CLEAR                    (0x0C)
#define MP_STAT                  (0x10)
#define BROADCAST                (0x14)
#define PROC_IR_MASK(CPU_INDEX)  (0x40 + 0x4 * CPU_INDEX)
#define PROC_IR_FORCE(CPU_INDEX) (0x80 + 0x4 * CPU_INDEX)
#define PROC_EXTIR_ID(CPU_INDEX) (0xC0 + 0x4 * CPU_INDEX)

/// Constructor
CIrqmp::CIrqmp(sc_core::sc_module_name name, int _paddr, int _pmask, int _ncpu, int _eirq)
  :
  gr_device(
            name,                      //sc_module name
            gs::reg::ALIGNED_ADDRESS,  //address mode (options: aligned / indexed)
            48 + 4*_ncpu,               //dword size (of register file)
            NULL                       //parent module
           ),
  bus( //greenreg_socket
      "bus",            //name
      r,                //register container
      0x0,              // start address
      0xFFFFFFFF,       // register space length
      ::amba::amba_APB, // bus type
      ::amba::amba_LT,  // communication type / abstraction level
      false             // not used
     ),
  rst(&CIrqmp::reset_registers, "RESET"),
  cpu_rst("CPU_RESET"),
  irq_req("CPU_REQUEST"),
  irq_ack(&CIrqmp::clear_acknowledged_irq, "IRQ_ACKNOWLEDGE"),
  irq_in(&CIrqmp::register_irq, "IRQ_INPUT"),
  ncpu(_ncpu), eirq(_eirq)
//  pindex("PINDEX"),
//  conf_defaults((sepirq << 8) | ((pirq & 0xF) << 3) | (ntimers & 0x7)),
//  clockcycle(10.0, sc_core::SC_NS)
                                     {

  // create register | name + description
  r.create_register( "level", "Interrupt Level Register",
                   // offset
                      0x00,
                   // config
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                   // init value
                      0x00000000,
                   // write mask
                      IRQMP_IR_LEVEL_IL,
                   // reg width (maximum 32 bit)
                      32,
                   // lock mask: Not implementet, has to be zero.
                      0x00
                   );
  r.create_register( "pending", "Interrupt Pending Register",
                      0x04,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      0x00000000,
                      IRQMP_IR_PENDING_EIP | IRQMP_IR_PENDING_IP,
                      32,
                      0x00
                   );
  //Following register is part of the manual, but will never be used.
  // 1) A system with 0 cpus will never be implemented
  // 2) If there were 0 cpus, no cpu would need an IR force register
  // 3) The IR force register for the first CPU ('CPU 0') will always be located at address 0x80
  if (ncpu == 0) {
    r.create_register( "force", "Interrupt Force Register", 
                        0x08,
                        gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                        0x00000000, 
                        IRQMP_IR_FORCE_IF,
                        32, 
                        0x00
                     );
  }
  r.create_register( "clear", "Interrupt Clear Register", 
                      0x0C,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      0x00000000, 
                      IRQMP_IR_CLEAR_IC,
                      32,
                      0x00
                   );
  r.create_register( "mpstat", "Multiprocessor Status Register", 
                      0x10,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      0x00000000, 
                      IRQMP_MP_STAT_NCPU | IRQMP_MP_STAT_EIRQ | IRQMP_MP_STAT_STAT,
                      32,
                      0x00
                   );
  r.create_register( "broadcast", "Interrupt broadcast Register", 
                      0x14,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      0x00000000, 
                      IRQMP_BROADCAST_BM,
                      32,
                      0x00
                   );

  for(int i=0; i<ncpu; ++i) {
    r.create_register( gen_unique_name("mask", false), "Interrupt Mask Register", 
                       0x40 + 0x4 * i,
                       gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                       0xFFFFFFFF, 
                       IRQMP_PROC_MASK_EIM | IRQMP_PROC_MASK_IM, 
                       32, 
                       0x00
                     );
    r.create_register( gen_unique_name("force", false), "Interrupt Force Register", 
                       0x80 + 0x4 * i,
                       gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                       0x00000000, 
                       IRQMP_PROC_IR_FORCE_IFC | IRQMP_IR_FORCE_IF,
                       32, 
                       0x00
                     );
    r.create_register( gen_unique_name("eir_id", false), "Extended Interrupt Identification Register", 
                       0xC0 + 0x4 * i,
                       gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                       0x00000000, 
                       IRQMP_PROC_EXTIR_ID_EID, 
                       32, 
                       0x00
                     );
  }
}

/// Destructor
CIrqmp::~CIrqmp() {
  GC_UNREGISTER_CALLBACKS();
}


/// Hook up callback functions to registers
void CIrqmp::end_of_elaboration() {   

  // send interrupts to processors after write to pending / force regs
  GR_FUNCTION(CIrqmp, launch_irq);                         // args: module name, callback function name
  GR_SENSITIVE(r[PENDING].add_rule( gs::reg::POST_WRITE,  // function to be called after register write
                                    "launch_irq",         // function name
                                    gs::reg::NOTIFY));    // notification on every register access
  for (int i_cpu=0; i_cpu<ncpu; i_cpu++) {
    GR_FUNCTION(CIrqmp, launch_irq);
    GR_SENSITIVE(r[PROC_IR_FORCE(i_cpu)].add_rule( gs::reg::POST_WRITE, 
                                     gen_unique_name("launch_irq", false), 
                                     gs::reg::NOTIFY));
  }

  // unset pending bits of cleared interrupts
  GR_FUNCTION(CIrqmp, clear_write);
  GR_SENSITIVE(r[CLEAR].add_rule( gs::reg::POST_WRITE,
                                  "clear_write",
                                  gs::reg::NOTIFY));

  // unset pending bits of cleared interrupts
  GR_FUNCTION(CIrqmp, clear_forced_ir);
  for (int i_cpu=0; i_cpu<ncpu; i_cpu++) {
    GR_SENSITIVE(r[PROC_IR_FORCE(i_cpu)].add_rule( gs::reg::POST_WRITE,
                                                   gen_unique_name("clear_forced_ir", false),
                                                   gs::reg::NOTIFY));
  }

  // manage cpu run / reset signals after write into MP status reg
  GR_FUNCTION(CIrqmp, mpstat_write);
  GR_SENSITIVE(r[MP_STAT].add_rule( gs::reg::POST_WRITE, 
                                   "mpstat_write", 
                                   gs::reg::NOTIFY));

  // read registers
  GR_FUNCTION(CIrqmp, register_read);
  GR_SENSITIVE(r[LEVEL].add_rule( gs::reg::PRE_READ, 
                                   "level_read", 
                                   gs::reg::NOTIFY));
  GR_SENSITIVE(r[PENDING].add_rule( gs::reg::PRE_READ, 
                                   "pending_read", 
                                   gs::reg::NOTIFY));
  GR_SENSITIVE(r[CLEAR].add_rule( gs::reg::PRE_READ, 
                                   "clear_read", 
                                   gs::reg::NOTIFY));
  GR_SENSITIVE(r[MP_STAT].add_rule( gs::reg::PRE_READ, 
                                   "mp_stat_read", 
                                   gs::reg::NOTIFY));
  GR_SENSITIVE(r[BROADCAST].add_rule( gs::reg::PRE_READ, 
                                   "broadcast_read", 
                                   gs::reg::NOTIFY));

  for (int i_cpu=0; i_cpu<ncpu;i_cpu++) {
    GR_FUNCTION(CIrqmp, register_read);
    GR_SENSITIVE(r[PROC_IR_MASK(i_cpu)].add_rule( gs::reg::POST_WRITE, 
                                     gen_unique_name("mask_read", false), 
                                     gs::reg::NOTIFY));
    GR_SENSITIVE(r[PROC_IR_FORCE(i_cpu)].add_rule( gs::reg::POST_WRITE, 
                                     gen_unique_name("force_read", false), 
                                     gs::reg::NOTIFY));
    GR_SENSITIVE(r[PROC_EXTIR_ID(i_cpu)].add_rule( gs::reg::POST_WRITE, 
                                     gen_unique_name("extir_id_read", false), 
                                     gs::reg::NOTIFY));
  }
}


//P R O C E S S   I M P L E M E N T A T I O N

/// Reset registers to default values
/// Process sensitive to reset signal
void CIrqmp::reset_registers(const bool &value, signalkit::signal_in_if<bool> *signal, signalkit::signal_out_if<bool> *sender, const sc_core::sc_time &time) {
#if 0
#ifdef COUT_TIMING
    cout << endl << "reset_registers called by trigger at:" << sc_time_stamp();
#endif
    next_trigger(CLOCK_PERIOD, SC_NS);
#endif
  if(!value) {
#ifdef COUT_TIMING
    cout << endl << "reset_registers called after delay at:" << sc_time_stamp();
#endif

    //mp status register contains ncpu and eirq at bits 31..28 and 19..16 respectively
    unsigned int stat_ncpu = ncpu << 28;
    unsigned int stat_eirq = eirq << 16;

    //set function below demands unsigned int. Inline typecast does not do its job.
    unsigned int set_level =     static_cast<unsigned int>(IRQMP_LEVEL_DEFAULT);
    unsigned int set_pending =   static_cast<unsigned int>(IRQMP_PENDING_DEFAULT);
    unsigned int set_force =     static_cast<unsigned int>(IRQMP_FORCE_DEFAULT);
    unsigned int set_clear =     static_cast<unsigned int>(IRQMP_CLEAR_DEFAULT);
                                                        //CAUTION: bool value?
    unsigned int set_mp_stat =   static_cast<unsigned int>(IRQMP_MP_STAT_DEFAULT or stat_ncpu or stat_eirq);
    unsigned int set_broadcast = static_cast<unsigned int>(IRQMP_BROADCAST_DEFAULT);
    unsigned int set_mask =      static_cast<unsigned int>(IRQMP_MASK_DEFAULT);
    unsigned int set_pforce =    static_cast<unsigned int>(IRQMP_PROC_FORCE_DEFAULT);
    unsigned int set_extir_id =  static_cast<unsigned int>(IRQMP_EXTIR_ID_DEFAULT);

    //initialize registers with values defined above
    r[LEVEL].set(set_level);
    r[PENDING].set(set_pending);
    if (ncpu == 0) {
      r[FORCE].set(set_force);
    }
    r[CLEAR].set(set_clear);
    r[MP_STAT].set(set_mp_stat);
    r[BROADCAST].set(set_broadcast);
    for (int i_cpu=0; i_cpu<ncpu; i_cpu++) {
      r[PROC_IR_MASK(i_cpu)].set(set_mask);
      r[PROC_IR_FORCE(i_cpu)].set(set_pforce);
      r[PROC_EXTIR_ID(i_cpu)].set(set_extir_id);
    }
  }
}


///
/// register irq
///  - watch interrupt bus signals (apbi.pirq)
///  - write incoming interrupts into pending or force registers
///
/// process sensitive to apbi.pirq
void CIrqmp::register_irq(const uint32_t &value, const unsigned int &channel, signalkit::signal_in_if<uint32_t> *signal, signalkit::signal_out_if<uint32_t> *sender, const sc_core::sc_time &time) {

  //static bool delay = true;

#if 0
  //either model the timing
  if (delay) {
#ifdef COUT_TIMING
    cout << endl << "register_irq called by trigger at:" << sc_time_stamp();
#endif
    delay = false;
    next_trigger(2*CLOCK_PERIOD, SC_NS);
  }
  //or model the functionality
  else {
#endif
//    delay += 2*CLOCK_PERIOD;
#ifdef COUT_TIMING
    cout << endl << "register_irq called after delay at:" << sc_time_stamp();
#endif
    //set pending register
    unsigned int irq = value;
    unsigned int set = static_cast<unsigned int>( irq & !r[BROADCAST].get() );
    r[PENDING].set( set );

    //set force registers for broadcasted interrupts
    for (int i_cpu = 0; i_cpu<ncpu; i_cpu++) {                                             // EIRs cannot be forced
      unsigned int set = static_cast<unsigned int>( irq & r[BROADCAST].get() & IRQMP_PROC_IR_FORCE_IF );
      r[PROC_IR_FORCE(i_cpu)].set( set );
    }
#if 0
    delay = true;
  }
#endif
  /* FIXME:
   *  Pending and force regs are set now. Is an explicit call of the launch_irq function required here?
   *  To be checken in simulation.
   */

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
  short int high_ir;        // highest priority interrupt (to be launched)
  sc_uint<32> masked_ir;    // vector of pending, masked interrupts
  //l3_irq_in_type temp;      // temp var required for write access to single signals inside the struct

  for (int i_cpu=0; i_cpu<ncpu; i_cpu++) {
    // process call might be triggered by acknowledge of forced IR by writing to IFC bits of IF register
    unsigned int cleared_force_reg =  r[PROC_IR_FORCE(i_cpu)].get()        and not   // keep previous values
                                     (r[PROC_IR_FORCE(i_cpu)].get() >> 16) and       // clear IRs as indicated by IFC
                                      IRQMP_PROC_IR_FORCE_IF;                        // clear IFC
    r[PROC_IR_FORCE(i_cpu)].set(cleared_force_reg);

    // masked = (pending || force) && mask
    masked_ir = (r[PENDING].get() | r[PROC_IR_FORCE(i_cpu)].get() ) & r[PROC_IR_MASK(i_cpu)].get();

    // any pending extended interrupts?
    if (eirq != 0) {
      sc_uint<32> eirq_pending = masked_ir & IRQMP_IR_PENDING_EIP;
      bool eirq_pending_bool = eirq_pending.or_reduce();
      r[PENDING].bit_set(eirq, eirq_pending_bool);
    }

    // prioritize interrupts
                // (pending or force) and mask and level
    masked_ir = ( r[PENDING].get()                                        or        // pending
                  0xFFFF & r[PROC_IR_FORCE(i_cpu)].b[IRQMP_PROC_IR_FORCE_IF] ) and  // upper half of force reg contains force clear
                  r[PROC_IR_MASK(i_cpu)].get()                                 and  // mask
                  0xFFFF & r[LEVEL].b[IRQMP_IR_LEVEL_IL];                           // level 1
    for (high_ir=15; high_ir>=0; high_ir--) {
      if (masked_ir[high_ir]) {
        continue;
      }
    }

    // If no IR on level 1, check level 0.
    if (high_ir == 0) {
                  // (pending or force) and mask and not level
      masked_ir = ( r[PENDING].get()                                        or           //pending
                    0xFFFF & r[PROC_IR_FORCE(i_cpu)].b[IRQMP_PROC_IR_FORCE_IF] ) and     //force
                    r[PROC_IR_MASK(i_cpu)].get()                                 and not //mask
                    0xFFFF & r[LEVEL].b[IRQMP_IR_LEVEL_IL];                              //level 0
      for (high_ir=15; high_ir>=0; high_ir++) {
        if (masked_ir[high_ir]) {
          continue;
        }
      }
    }
    irq_req.write(1<<i_cpu, 0xF & high_ir);
  } // foreach cpu
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
  unsigned int cleared_vector = r[PENDING].get() and not r[CLEAR].get();
  r[PENDING].set(cleared_vector);
}

/// callback registered on interrupt force registers
void CIrqmp::clear_forced_ir() {

  for (int i_cpu=0; i_cpu<ncpu; i_cpu++) {
    unsigned int ir_force_reg = r[IRQMP_PROC_IR_FORCE(i_cpu)].get();
    //write mask clears IFC bits:
    //IF && !IFC && write_mask
    ir_force_reg = ir_force_reg and not (ir_force_reg >> 16) and IRQMP_PROC_IR_FORCE_IF;
    //unsigned int new_force = static_cast<unsigned int>(ir_force_reg);
    r[IRQMP_PROC_IR_FORCE(i_cpu)].set(ir_force_reg);
  }
}


/// process sensitive to irqi
void CIrqmp::clear_acknowledged_irq(const uint32_t &cleared_irq, const unsigned int &i_cpu, signalkit::signal_in_if<uint32_t> *signal, signalkit::signal_out_if<uint32_t> *sender, const sc_core::sc_time &time) {
#if 0
#ifdef COUT_TIMING
    cout << endl << "clear_acknowledged_irq called by trigger at:" << sc_time_stamp();
#endif
    t += sc_core::sc_time(CLOCK_PERIOD, SC_NS);
#endif
    
#ifdef COUT_TIMING
    cout << endl << "clear_acknowledged_irq called after delay at:" << sc_time_stamp();
#endif
    short int high_ir;
    unsigned int masked_ir;

    //extended IR handling: Identify highest pending EIR
    if (eirq != 0 && cleared_irq == (unsigned int)eirq) {
      masked_ir = ( r[PENDING].get() and r[PROC_IR_MASK(i_cpu)].get() ); //force and level not supported for EIRQs
      for (high_ir=31; high_ir>=16; high_ir--) {
        if (masked_ir & (1 << high_ir)) {
          return;
        }
      }
      //write EIR ID Register
      unsigned int set = static_cast<unsigned int>( IRQMP_PROC_EXTIR_ID_EID and high_ir );
      r[PROC_EXTIR_ID(i_cpu)].set(set);
      //clear EIR from pending register
      bool false_var = false;
      r[PENDING].bit_set(high_ir, false_var);
    }

    //clear interrupt from pending and force register
    bool false_var = false;
    r[PENDING].bit_set( static_cast<unsigned int>(cleared_irq), false_var);
    if (r[BROADCAST].bit_get(cleared_irq)) {
      r[PROC_IR_FORCE(i_cpu)].bit_set( static_cast<unsigned int>(cleared_irq), false_var);
    }
}

/// reset cpus after write to cpu status register
/// callback registered on mp status register
void CIrqmp::mpstat_write() {
  cpu_rst.write(0xFFFFFFFF, true);
}

///
/// Registers can be read by software.
/// The TLM transport function will read the value from the register container.
/// Prior to a read access, no changes to the registers are necessary.
///
/// callback registered on all registers
void CIrqmp::register_read() {

}

/// @}

#endif
