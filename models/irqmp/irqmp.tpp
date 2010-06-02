/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       irqmp.tpp                                               */
/*             implementation of irqmp module                          */
/*             is included by irqmp.h template header file             */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

/*  2-DO

- take care of pindex / pconfig signals (apbo: PnP)
- take care of pwd and fpen signals (irqi)
- take care of rst, run, and rstvec signals (irqo)

long term:
- add LT / AT timing information

*/

#ifndef IRQMP_TPP
#define IRQMP_TPP

#define COUT_TIMING

#include "irqmp.h"
#include "irqmpreg.h"
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
  rst("rst") // signal naming
//  pindex("PINDEX"),
//  conf_defaults((sepirq << 8) | ((pirq & 0xF) << 3) | (ntimers & 0x7)),
//  clockcycle(10.0, sc_core::SC_NS)
                                     {

  /* create register */
  r.create_register( "level", "Interrupt Level Register", // name + description
         /* offset */ 0x00,
         /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
     /* init value */ 0x00000000,
     /* write mask */ IRQMP_IR_LEVEL_IL,
      /* reg width */ 32,          /* Maximum register with is 32bit sbit must be less than 32. */
      /* lock mask */ 0x00         /* Not implementet has to be zero. */
                   );
  r.create_register( "pending", "Interrupt Pending Register", 
         /* offset */ 0x04,
         /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
     /* init value */ 0x00000000, 
     /* write mask */ IRQMP_IR_PENDING_EIP | IRQMP_IR_PENDING_IP,
      /* reg width */ 32,
      /* lock mask */ 0x00
                   );
  /*Following register is part of the manual, but will never be used.
   * 1) A system with 0 cpus will never be implemented
   * 2) If there were 0 cpus, no cpu would need an IR force register
   * 3) The IR force register for the first CPU ('CPU 0') will always be located at address 0x80
   */
  if (ncpu == 0) {
    r.create_register( "force", "Interrupt Force Register", 
           /* offset */ 0x08,
           /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
       /* init value */ 0x00000000, 
       /* write mask */ IRQMP_IR_FORCE_IF,
        /* reg width */ 32, 
        /* lock mask */ 0x00
                     );
  }
  r.create_register( "clear", "Interrupt Clear Register", 
         /* offset */ 0x0C,
         /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
     /* init value */ 0x00000000, 
     /* write mask */ IRQMP_IR_CLEAR_IC,
      /* reg width */ 32,
      /* lock mask */ 0x00
                   );
  r.create_register( "mpstat", "Multiprocessor Status Register", 
         /* offset */ 0x10,
         /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
     /* init value */ 0x00000000, 
     /* write mask */ IRQMP_MP_STAT_NCPU | IRQMP_MP_STAT_EIRQ | IRQMP_MP_STAT_STAT,
      /* reg width */ 32,
      /* lock mask */ 0x00
                   );
  r.create_register( "broadcast", "Interrupt broadcast Register", 
         /* offset */ 0x14,
         /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
     /* init value */ 0x00000000, 
     /* write mask */ IRQMP_BROADCAST_BM,
      /* reg width */ 32,
      /* lock mask */ 0x00
                   );

  for(unsigned int i=0; i<ncpu; ++i) {
    r.create_register( gen_unique_name("mask", false), "Interrupt Mask Register", 
          /* offset */ 0x40 + 0x4 * i,
          /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
      /* init value */ 0xFFFFFFFF, 
      /* write mask */ IRQMP_PROC_MASK_EIM | IRQMP_PROC_MASK_IM, 
       /* reg width */ 32, 
       /* lock mask */ 0x00
                     );
    r.create_register( gen_unique_name("force", false), "Interrupt Force Register", 
          /* offset */ 0x80 + 0x4 * i,
          /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
      /* init value */ 0x00000000, 
      /* write mask */ IRQMP_PROC_IR_FORCE_IFC | IRQMP_IR_FORCE_IF,
       /* reg width */ 32, 
       /* lock mask */ 0x00
                     );
    r.create_register( gen_unique_name("eir_id", false), "Extended Interrupt Identification Register", 
          /* offset */ 0xC0 + 0x4 * i,
          /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
      /* init value */ 0x00000000, 
      /* write mask */ IRQMP_PROC_EXTIR_ID_EID, 
       /* reg width */ 32, 
       /* lock mask */ 0x00
                     );
  }

  //process registration
  SC_METHOD(reset_registers);
//  dont_initialize();
  sensitive << rst.neg();

  SC_METHOD(register_irq);
  dont_initialize();
  sensitive << apbi_pirq;

  SC_METHOD(clear_acknowledged_irq);
  for (int i_cpu = 0; i_cpu<ncpu; i_cpu++) {
    dont_initialize();
    sensitive << irqi[i_cpu];
  }
}

//destructor
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
   Irqmp <pindex, paddr, pmask, ncpu, eirq>::~Irqmp() {
  GC_UNREGISTER_CALLBACKS();
}




/*Hook up callback functions to registers*/
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
  void Irqmp <pindex, paddr, pmask, ncpu, eirq>::end_of_elaboration() {   
/*
  GR_FUNCTION(Irqmp, pending_write);                      // args: module name, callback function name
  GR_SENSITIVE(r[PENDING].add_rule( gs::reg::POST_WRITE,  // function to be called after register write
                                    "pending_write",      // function name
                                     gs::reg::NOTIFY));   // notification on every register access
*/                                                 //SEE ALSO: GreenReg-Introduction, slide 68 (58 in ToC)

  // send interrupts to processors after write to pending / force regs
  GR_FUNCTION(Irqmp, launch_irq);
  GR_SENSITIVE(r[PENDING].add_rule( gs::reg::POST_WRITE,
                                    "launch_irq",
                                    gs::reg::NOTIFY));
  for (int i_cpu=0; i_cpu<ncpu;i_cpu++) {
    GR_FUNCTION(Irqmp, launch_irq);
    GR_SENSITIVE(r[PROC_IR_FORCE(i_cpu)].add_rule( gs::reg::POST_WRITE, 
                                     gen_unique_name("force_write", false), 
                                     gs::reg::NOTIFY));
  }

  // unset pending / force bits of cleared interrupts
  GR_FUNCTION(Irqmp, clear_write);
  GR_SENSITIVE(r[CLEAR].add_rule( gs::reg::POST_WRITE, 
                                   "clear_write", 
                                   gs::reg::NOTIFY));

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
                                     gen_unique_name("mask_write", false), 
                                     gs::reg::NOTIFY));
    GR_SENSITIVE(r[PROC_IR_FORCE(i_cpu)].add_rule( gs::reg::POST_WRITE, 
                                     gen_unique_name("force_write", false), 
                                     gs::reg::NOTIFY));
    GR_SENSITIVE(r[PROC_EXTIR_ID(i_cpu)].add_rule( gs::reg::POST_WRITE, 
                                     gen_unique_name("extir_id_write", false), 
                                     gs::reg::NOTIFY));
  }

}


/***process implementation***/

/*
 * reset registers to default values
 */
//process sensitive to reset signal
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
  void Irqmp <pindex, paddr, pmask, ncpu, eirq>::reset_registers() {

  static bool delay = true;

  //either model the timing

  if (delay) {
#ifdef COUT_TIMING
    cout << endl << "reset_registers called by trigger at:" << sc_time_stamp();
#endif
    delay = false;
    next_trigger(CLOCK_PERIOD, SC_NS);
  }
  //or model the functionality
  else {

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

    delay = true;
  } //else

}


/*
 * register irq
 *  o watch interrupt bus signals (apbi.pirq)
 *  o write incoming interrupts into pending or force registers
 */
//process sensitive to apbi.pirq
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
  void Irqmp <pindex, paddr, pmask, ncpu, eirq>::register_irq() {

  static bool delay = true;

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
#ifdef COUT_TIMING
    cout << endl << "register_irq called after delay at:" << sc_time_stamp();
#endif
    //set pending register
    unsigned int set = static_cast<unsigned int>( apbi_pirq.read() and not r[BROADCAST].get() );
    r[PENDING].set( set );

    //set force registers for broadcasted interrupts
    for (int i_cpu; i_cpu<ncpu; i_cpu++) {                                                   // EIRs cannot be forced
      unsigned int set = static_cast<unsigned int>( apbi_pirq.read() and r[BROADCAST].get() and IRQMP_PROC_IR_FORCE_IF );
      r[PROC_IR_FORCE(i_cpu)].set( set );
    }
    delay = true;
  }

  /* FIXME:
   *  Pending and force regs are set now. Is an explicit call of the launch_irq function required here?
   *  To be checken in simulation.
   */

}


/*
 * launch irq:
 *  o combine pending, force, and mask register
 *  o prioritize pending interrupts
 *  o send highest priority IR to processor via irqo port
 */
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
      if (masked_ir[high_ir]) continue;
    }

    // If no IR on level 1, check level 0.
    if (high_ir == 0) {
                  // (pending or force) and mask and not level
      masked_ir = ( r[PENDING].get()                                        or           //pending
                    0xFFFF & r[PROC_IR_FORCE(i_cpu)].b[IRQMP_PROC_IR_FORCE_IF] ) and     //force
                    r[PROC_IR_MASK(i_cpu)].get()                                 and not //mask
                    0xFFFF & r[LEVEL].b[IRQMP_IR_LEVEL_IL];                              //level 0
      for (high_ir=15; high_ir>=0; high_ir++) {
        if (masked_ir[high_ir]) continue;
      }
    }
    temp = irqo[i_cpu].read();
    temp.irl = static_cast<sc_uint<4> > (0x0000000F and high_ir);
    irqo[i_cpu].write(temp); 
  } // foreach cpu
}

/*
 * clear acknowledged IRQs                                         (two processes)
 *  o interrupts can be cleared
 *    - by software writing to the IR_Clear register               (process 1: clear_write)
 *    - by processor sending irqi.intack signal and irqi.irl data  (process 2: clear_acknowledged_irq)
 *  o remove IRQ from pending / force registers
 *  o in case of eirq, release eirq ID in eirq ID register
 */
// callback registered on interrupt clear register
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
    void Irqmp<pindex, paddr, pmask, ncpu, eirq>::clear_write() {
  // pending reg only: forced IRs cannot be cleared by software
  unsigned int cleared_vector = r[PENDING].get() and not r[CLEAR].get();
  r[PENDING].set(cleared_vector);
}

//process sensitive to irqi
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
  void Irqmp <pindex, paddr, pmask, ncpu, eirq>::clear_acknowledged_irq() {
  static bool delay = true;

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
#ifdef COUT_TIMING
    cout << endl << "clear_acknowledged_irq called after delay at:" << sc_time_stamp();
#endif
    short int high_ir;
    sc_uint<32> masked_ir;

    for (int i_cpu; i_cpu<ncpu; i_cpu++) {
      if (irqi[i_cpu].read().intack == 1) {
        sc_uint<4> cleared_irq = irqi[i_cpu].read().irl;

        //extended IR handling: Identify highest pending EIR and write EIR ID register
        if (eirq == 0 && cleared_irq == eirq) {
          masked_ir = ( r[PENDING].get() and r[PROC_IR_MASK(i_cpu)].get() ); //force and level not supported for EIRQs
          for (high_ir=31; high_ir>=16; high_ir--) {
            if (masked_ir[high_ir]) continue;
          }
          unsigned int set = static_cast<unsigned int>( IRQMP_PROC_EXTIR_ID_EID and high_ir );
          r[PROC_EXTIR_ID(i_cpu)].set(set);
        }

        //clear interrupt from pending and force register
        bool true_var = true;
        r[PENDING].bit_set( static_cast<unsigned int>(cleared_irq), true_var);
        if (r[BROADCAST].bit_get(cleared_irq)) {
          r[PROC_IR_FORCE(i_cpu)].bit_set( static_cast<unsigned int>(cleared_irq), true_var);
        }
      } //if intack
    } //for i_cpu
    delay = true;
  } //else delay
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
  }
}

/*
 * Registers can be read by software.
 * The TLM transport function will read the value from the register container.
 * Prior to a read access, no changes to the registers are necessary.
 */
//callback registered on all registers
template <int pindex, int paddr, int pmask, int ncpu, int eirq>
  void Irqmp <pindex, paddr, pmask, ncpu, eirq>::register_read() {

}



#endif
