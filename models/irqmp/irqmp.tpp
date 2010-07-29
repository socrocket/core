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

//constructor
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
         Irqmp<pindex, paddr, pmask, ncpu, eirq>::Irqmp(sc_core::sc_module_name name)
  :
  gr_device(
            name,                      //sc_module name
            gs::reg::ALIGNED_ADDRESS,  //address mode (options: aligned / indexed)
            48 + 4*ncpu,               //dword size (of register file)
            NULL                       //parent module
           ),
  bus( //greenreg_socket
      "bus",            //name
      r,                //register container
      0x0,              // ?
      0xFFFFFFFF,       // ?
      ::amba::amba_APB, // ?bus type?
      ::amba::amba_LT,  // ?communication type / abstraction level?
      false             // ?
     ),
  in("IN"), out("out") // signal naming
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

  for(unsigned int i=0; i<ncpu; ++i) {
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

  //process registration
  sender_config sconf;
  //conf.use_mandatory_extension<bar>();
  sconf.use_optional_extension<irq0>();
  sconf.use_optional_extension<irq1>();
  sconf.use_optional_extension<irq2>();
  sconf.use_optional_extension<irq3>();
  sconf.use_optional_extension<irq4>();
  sconf.use_optional_extension<irq5>();
  sconf.use_optional_extension<irq6>();
  sconf.use_optional_extension<irq7>();
  sconf.use_optional_extension<irq8>();
  sconf.use_optional_extension<irq9>();
  sconf.use_optional_extension<irq10>();
  sconf.use_optional_extension<irq11>();
  sconf.use_optional_extension<irq12>();
  sconf.use_optional_extension<irq13>();
  sconf.use_optional_extension<irq14>();
  sconf.use_optional_extension<irq15>();
  sconf.use_optional_extension<rst0>();
  sconf.use_optional_extension<rst1>();
  sconf.use_optional_extension<rst2>();
  sconf.use_optional_extension<rst3>();
  sconf.use_optional_extension<rst4>();
  sconf.use_optional_extension<rst5>();
  sconf.use_optional_extension<rst6>();
  sconf.use_optional_extension<rst7>();
  sconf.use_optional_extension<rst8>();
  sconf.use_optional_extension<rst9>();
  sconf.use_optional_extension<rst10>();
  sconf.use_optional_extension<rst11>();
  sconf.use_optional_extension<rst12>();
  sconf.use_optional_extension<rst13>();
  sconf.use_optional_extension<rst14>();
  sconf.use_optional_extension<rst15>();
  register_sender_socket(out, sconf);

  target_config tconf;
  tconf.use_mandatory_extension<rst>();
  tconf.use_optional_extension<irq0>();
  tconf.use_optional_extension<irq1>();
  tconf.use_optional_extension<irq2>();
  tconf.use_optional_extension<irq3>();
  tconf.use_optional_extension<irq4>();
  tconf.use_optional_extension<irq5>();
  tconf.use_optional_extension<irq6>();
  tconf.use_optional_extension<irq7>();
  tconf.use_optional_extension<irq8>();
  tconf.use_optional_extension<irq9>();
  tconf.use_optional_extension<irq10>();
  tconf.use_optional_extension<irq11>();
  tconf.use_optional_extension<irq12>();
  tconf.use_optional_extension<irq13>();
  tconf.use_optional_extension<irq14>();
  tconf.use_optional_extension<irq15>();
  tconf.use_optional_extension<set_irq>();
  //tconf.use_optional_extension<clr_irq>();
  register_target_socket(this, in, tconf);

  CB_HANDLER(rst, Irqmp, &Irqmp::reset_registers);
  CB_HANDLER(set_irq, Irqmp, &Irqmp::register_irq);
  //CB_HANDLER(clr_irq, Irqmp, &Irqmp::clrirq);
  CB_HANDLER(irq0, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq1, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq2, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq3, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq4, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq5, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq6, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq7, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq8, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq9, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq10, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq11, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq12, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq13, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq14, Irqmp, &Irqmp::clear_acknowledged_irq); 
  CB_HANDLER(irq15, Irqmp, &Irqmp::clear_acknowledged_irq);  
}

//destructor
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
   Irqmp <pindex, paddr, pmask, ncpu, eirq>::~Irqmp() {
  GC_UNREGISTER_CALLBACKS();
}


//Hook up callback functions to registers
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
  void Irqmp <pindex, paddr, pmask, ncpu, eirq>::end_of_elaboration() {   

  // send interrupts to processors after write to pending / force regs
  GR_FUNCTION(Irqmp, launch_irq);                         // args: module name, callback function name
  GR_SENSITIVE(r[PENDING].add_rule( gs::reg::POST_WRITE,  // function to be called after register write
                                    "launch_irq",         // function name
                                    gs::reg::NOTIFY));    // notification on every register access
  for (int i_cpu=0; i_cpu<ncpu; i_cpu++) {
    GR_FUNCTION(Irqmp, launch_irq);
    GR_SENSITIVE(r[PROC_IR_FORCE(i_cpu)].add_rule( gs::reg::POST_WRITE, 
                                     gen_unique_name("launch_irq", false), 
                                     gs::reg::NOTIFY));
  }

  // unset pending bits of cleared interrupts
  GR_FUNCTION(Irqmp, clear_write);
  GR_SENSITIVE(r[CLEAR].add_rule( gs::reg::POST_WRITE,
                                  "clear_write",
                                  gs::reg::NOTIFY));

  // unset pending bits of cleared interrupts
  GR_FUNCTION(Irqmp, clear_forced_ir);
  for (int i_cpu=0; i_cpu<ncpu; i_cpu++) {
    GR_SENSITIVE(r[PROC_IR_FORCE(i_cpu)].add_rule( gs::reg::POST_WRITE,
                                                   gen_unique_name("clear_forced_ir", false),
                                                   gs::reg::NOTIFY));
  }

  // manage cpu run / reset signals after write into MP status reg
  GR_FUNCTION(Irqmp, mpstat_write);
  GR_SENSITIVE(r[MP_STAT].add_rule( gs::reg::POST_WRITE, 
                                   "mpstat_write", 
                                   gs::reg::NOTIFY));

  // read registers
  GR_FUNCTION(Irqmp, register_read);
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
    GR_FUNCTION(Irqmp, register_read);
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

//reset registers to default values
//process sensitive to reset signal
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
  void Irqmp <pindex, paddr, pmask, ncpu, eirq>::reset_registers(unsigned int &id, gs_generic_signal_payload& trans, sc_core::sc_time& delay) {

  static bool delay = true;

  //either model the timing
#if 0
  if (delay) {
#ifdef COUT_TIMING
    cout << endl << "reset_registers called by trigger at:" << sc_time_stamp();
#endif
    delay = false;
    next_trigger(CLOCK_PERIOD, SC_NS);
  }
  //or model the functionality
  else {
#endif
  if(!in.get_last_value<rst>()) {
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
#if 0
    delay = true;
  } //else
#endif
  }
}


//
// register irq
//  o watch interrupt bus signals (apbi.pirq)
//  o write incoming interrupts into pending or force registers
//
//process sensitive to apbi.pirq
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
  void Irqmp <pindex, paddr, pmask, ncpu, eirq>::register_irq(unsigned int &id, gs_generic_signal_payload& trans, sc_core::sc_time& delay) {

  static bool delay = true;

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
#ifdef COUT_TIMING
    cout << endl << "register_irq called after delay at:" << sc_time_stamp();
#endif
    //set pending register
    unsigned int irq = 1 << in.get_last_value<set_irq>();
    unsigned int set = static_cast<unsigned int>( irq and not r[BROADCAST].get() );
    r[PENDING].set( set );

    //set force registers for broadcasted interrupts
    for (int i_cpu; i_cpu<ncpu; i_cpu++) {                                                   // EIRs cannot be forced
      unsigned int set = static_cast<unsigned int>( irq and r[BROADCAST].get() and IRQMP_PROC_IR_FORCE_IF );
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


//
// launch irq:
//  o combine pending, force, and mask register
//  o prioritize pending interrupts
//  o send highest priority IR to processor via irqo port
//
//callback registered on IR pending register,
//                       IR force registers
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
  void Irqmp <pindex, paddr, pmask, ncpu, eirq>::launch_irq() {
  short int high_ir;        // highest priority interrupt (to be launched)
  sc_uint<32> masked_ir;    // vector of pending, masked interrupts
  l3_irq_in_type temp;      // temp var required for write access to single signals inside the struct

  for (int i_cpu=0; i_cpu<ncpu; i_cpu++) {
    // process call might be triggered by acknowledge of forced IR by writing to IFC bits of IF register
    unsigned int cleared_force_reg =  r[PROC_IR_FORCE(i_cpu)].get()        and not   // keep previous values
                                     (r[PROC_IR_FORCE(i_cpu)].get() >> 16) and       // clear IRs as indicated by IFC
                                      IRQMP_PROC_IR_FORCE_IF;                        // clear IFC
    r[PROC_IR_FORCE(i_cpu)].set(cleared_force_reg);

    // masked = (pending || force) && mask
    masked_ir = (r[PENDING].get() or r[PROC_IR_FORCE(i_cpu)].get() ) and r[PROC_IR_MASK(i_cpu)].get();

    // any pending extended interrupts?
    if (eirq != 0) {
      sc_uint<32> eirq_pending = masked_ir and IRQMP_IR_PENDING_EIP;
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
    switch(i_cpu) {
      case 0: SIGNAL_SEND(irq0, (0x0000000F and high_ir)); break;
      case 1: SIGNAL_SEND(irq1, (0x0000000F and high_ir)); break;
      case 2: SIGNAL_SEND(irq2, (0x0000000F and high_ir)); break;
      case 3: SIGNAL_SEND(irq3, (0x0000000F and high_ir)); break;
      case 4: SIGNAL_SEND(irq4, (0x0000000F and high_ir)); break;
      case 5: SIGNAL_SEND(irq5, (0x0000000F and high_ir)); break;
      case 6: SIGNAL_SEND(irq6, (0x0000000F and high_ir)); break;
      case 7: SIGNAL_SEND(irq7, (0x0000000F and high_ir)); break;
      case 8: SIGNAL_SEND(irq8, (0x0000000F and high_ir)); break;
      case 9: SIGNAL_SEND(irq9, (0x0000000F and high_ir)); break;
      case 10: SIGNAL_SEND(irq10, (0x0000000F and high_ir)); break;
      case 11: SIGNAL_SEND(irq11, (0x0000000F and high_ir)); break;
      case 12: SIGNAL_SEND(irq12, (0x0000000F and high_ir)); break;
      case 13: SIGNAL_SEND(irq13, (0x0000000F and high_ir)); break;
      case 14: SIGNAL_SEND(irq14, (0x0000000F and high_ir)); break;
      case 15: SIGNAL_SEND(irq15, (0x0000000F and high_ir)); break;
    }
  } // foreach cpu
}

//
// clear acknowledged IRQs                                           (three processes)
//  o interrupts can be cleared
//    - by software writing to the IR_Clear register                 (process 1: clear_write)
//    - by software writing to the upper half of IR_Force registers  (process 2: Clear_forced_ir)
//    - by processor sending irqi.intack signal and irqi.irl data    (process 3: clear_acknowledged_irq)
//  o remove IRQ from pending / force registers
//  o in case of eirq, release eirq ID in eirq ID register
//
// callback registered on interrupt clear register
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
    void Irqmp<pindex, paddr, pmask, ncpu, eirq>::clear_write() {
  // pending reg only: forced IRs are cleared in the next function
  unsigned int cleared_vector = r[PENDING].get() and not r[CLEAR].get();
  r[PENDING].set(cleared_vector);
}

// callback registered on interrupt force registers
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
    void Irqmp<pindex, paddr, pmask, ncpu, eirq>::clear_forced_ir() {

  for (int i_cpu=0; i_cpu<ncpu; i_cpu++) {
    unsigned int ir_force_reg = r[IRQMP_PROC_IR_FORCE(i_cpu)].get();
    //write mask clears IFC bits:
    //IF && !IFC && write_mask
    ir_force_reg = ir_force_reg and not (ir_force_reg >> 16) and IRQMP_PROC_IR_FORCE_IF;
    //unsigned int new_force = static_cast<unsigned int>(ir_force_reg);
    r[IRQMP_PROC_IR_FORCE(i_cpu)].set(ir_force_reg);
  }
}


//process sensitive to irqi
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
  void Irqmp <pindex, paddr, pmask, ncpu, eirq>::clear_acknowledged_irq(unsigned int &id, gs_generic_signal_payload& trans, sc_core::sc_time& delay) {
  static bool delay = true;
#if 0
  //either model the timing
  if (delay) {
#ifdef COUT_TIMING
    cout << endl << "clear_acknowledged_irq called by trigger at:" << sc_time_stamp();
#endif
    delay = false;
    next_trigger(CLOCK_PERIOD, SC_NS);
  }
  //or model the functionality
  else {
#endif
#ifdef COUT_TIMING
    cout << endl << "clear_acknowledged_irq called after delay at:" << sc_time_stamp();
#endif
    short int high_ir;
    unsigned int masked_ir;

    int i_cpu = 0;
    int cleared_irq = 0;
    if(trans.get_extension<irq0>()) { i_cpu = 0; cleared_irq = in.get_last_value<irq0>();} else 
    if(trans.get_extension<irq1>()) { i_cpu = 1; cleared_irq = in.get_last_value<irq1>();} else
    if(trans.get_extension<irq2>()) { i_cpu = 2; cleared_irq = in.get_last_value<irq2>();} else
    if(trans.get_extension<irq3>()) { i_cpu = 3; cleared_irq = in.get_last_value<irq3>();} else
    if(trans.get_extension<irq4>()) { i_cpu = 4; cleared_irq = in.get_last_value<irq4>();} else
    if(trans.get_extension<irq5>()) { i_cpu = 5; cleared_irq = in.get_last_value<irq5>();} else
    if(trans.get_extension<irq6>()) { i_cpu = 6; cleared_irq = in.get_last_value<irq6>();} else
    if(trans.get_extension<irq7>()) { i_cpu = 7; cleared_irq = in.get_last_value<irq7>();} else
    if(trans.get_extension<irq8>()) { i_cpu = 8; cleared_irq = in.get_last_value<irq8>();} else
    if(trans.get_extension<irq9>()) { i_cpu = 9; cleared_irq = in.get_last_value<irq9>();} else
    if(trans.get_extension<irq10>()) { i_cpu = 10; cleared_irq = in.get_last_value<irq10>();} else
    if(trans.get_extension<irq11>()) { i_cpu = 11; cleared_irq = in.get_last_value<irq11>();} else
    if(trans.get_extension<irq12>()) { i_cpu = 12; cleared_irq = in.get_last_value<irq12>();} else
    if(trans.get_extension<irq13>()) { i_cpu = 13; cleared_irq = in.get_last_value<irq13>();} else
    if(trans.get_extension<irq14>()) { i_cpu = 14; cleared_irq = in.get_last_value<irq14>();} else
    if(trans.get_extension<irq15>()) { i_cpu = 15; cleared_irq = in.get_last_value<irq15>();}

    //extended IR handling: Identify highest pending EIR
    if (eirq != 0 && cleared_irq == eirq) {
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
      r[PENDING].bitset(high_ir, false_var);
    }

    //clear interrupt from pending and force register
    bool false_var = false;
    r[PENDING].bit_set( static_cast<unsigned int>(cleared_irq), false_var);
    if (r[BROADCAST].bit_get(cleared_irq)) {
      r[PROC_IR_FORCE(i_cpu)].bit_set( static_cast<unsigned int>(cleared_irq), false_var);
    }
#if 0
    delay = true;
  } //else delay
#endif
}

//reset cpus after write to cpu status register
//callback registered on mp status register
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
  void Irqmp <pindex, paddr, pmask, ncpu, eirq>::mpstat_write() {
  l3_irq_in_type temp;
  for (int i_cpu=ncpu-1; i_cpu>=0; i_cpu--) {
    temp = irqo[i_cpu].read();
    temp.rst = r[IRQMP_MP_STAT].bit_get(i_cpu);
    irqo[i_cpu].write(temp);
    switch(i_cpu) {
      case 0: SIGNAL_SEND(rst0, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 1: SIGNAL_SEND(rst1, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 2: SIGNAL_SEND(rst2, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 3: SIGNAL_SEND(rst3, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 4: SIGNAL_SEND(rst4, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 5: SIGNAL_SEND(rst5, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 6: SIGNAL_SEND(rst6, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 7: SIGNAL_SEND(rst7, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 8: SIGNAL_SEND(rst8, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 9: SIGNAL_SEND(rst9, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 10: SIGNAL_SEND(rst10, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 11: SIGNAL_SEND(rst11, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 12: SIGNAL_SEND(rst12, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 13: SIGNAL_SEND(rst13, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 14: SIGNAL_SEND(rst14, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
      case 15: SIGNAL_SEND(rst15, r[IRQMP_MP_STAT].bit_get(i_cpu)); break;
    }
  }
}

//
// Registers can be read by software.
// The TLM transport function will read the value from the register container.
// Prior to a read access, no changes to the registers are necessary.
//
//callback registered on all registers
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
  void Irqmp <pindex, paddr, pmask, ncpu, eirq>::register_read() {

}



#endif
