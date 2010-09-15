/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       gpCGPCounter.cpp                                           */
/*             source file defining the implementation of the gptimer  */
/*             model. Due to the fact that the gptimer class is a      */
/*             template class it gets included by its definition in    */
/*             gptimer.h                                               */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#include <string>
#include "gpcounter.h"
#include "gptimer.h"

#define SCALER            (0x00)
#define SCRELOAD          (0x04)
#define CONF              (0x08)
#define VALUE             (0x10*(nr+1)+0x0)
#define RELOAD            (0x10*(nr+1)+0x4)
#define CTRL              (0x10*(nr+1)+0x8)

/// @addtogroup gptimer
/// @{

CGPCounter::CGPCounter(CGPTimer &_parent, unsigned int _nr, sc_core::sc_module_name name)
  : gr_subdevice(name, _parent), p(_parent), nr(_nr), stopped(true), chain_run(false) {  
  SC_THREAD(ticking);
  //do_reset();
}

CGPCounter::~CGPCounter() {
  GC_UNREGISTER_CALLBACKS();
}
  
void CGPCounter::end_of_elaboration() {
  GR_FUNCTION(CGPCounter, ctrl_read);
  GR_SENSITIVE(p.r[CTRL].add_rule(gs::reg::PRE_READ, 
                                  gen_unique_name("ctrl_read", false), 
                                  gs::reg::NOTIFY));

  GR_FUNCTION(CGPCounter, ctrl_write);
  GR_SENSITIVE(p.r[CTRL].add_rule(gs::reg::POST_WRITE, 
                                  gen_unique_name("ctrl_write", false), 
                                  gs::reg::NOTIFY));

  GR_FUNCTION(CGPCounter, value_read);
  GR_SENSITIVE(p.r[VALUE].add_rule(gs::reg::PRE_READ, 
                                   gen_unique_name("value_read", false), 
                                   gs::reg::NOTIFY));

  GR_FUNCTION(CGPCounter, value_write);
  GR_SENSITIVE(p.r[VALUE].add_rule(gs::reg::POST_WRITE, 
                                   gen_unique_name("value_write", false), 
                                   gs::reg::NOTIFY));

  p.r[VALUE].enable_events();
}

void CGPCounter::ctrl_read() {
  p.r[CTRL].b[TIM_CTRL_DH] = (p.dhalt.read() != 0);
  p.r[CTRL].b[TIM_CTRL_IP] = m_pirq; 
}

void CGPCounter::ctrl_write() {
  p.r[CTRL].b[TIM_CTRL_DH] = (p.dhalt.read() != 0);
  std::cout << " CGPTimer " << nr << " write " << std::endl;

  /* Clean irq if desired */
  bool old_pirq = m_pirq;

  m_pirq = p.r[CTRL].b[TIM_CTRL_IP] and m_pirq;
  if(old_pirq && !m_pirq) {
    unsigned int irqnr = (p.r[CONF] >> 3) & 0xF;
    if(p.r[CONF].b[TIM_CONF_SI]) {
       irqnr += nr;
    }
    p.irq.write(p.irq.read() & ~(1<<irqnr));
  }

  /* Prepare for chainging */
  if(p.r[CTRL].b[TIM_CTRL_CH]) {
    chain_run = false;
    stop();
  }

  /* Load */
  if(p.r[CTRL].b[TIM_CTRL_LD]) {
    unsigned int reload = p.r[RELOAD]; 
    p.r[VALUE].set(reload);
    value_write();
    p.r[CTRL].b[TIM_CTRL_LD] = false;
  }

  /* Enable */
  //std::cout << "StartStop_" << nr << std::endl;
  if(p.r[CTRL].b[TIM_CTRL_EN] && stopped) {
    std::cout << "Start_" << nr << std::endl;
    start();
  } else if((!p.r[CTRL].b[TIM_CTRL_EN])&&!stopped) {
    std::cout << "Stop_" << nr << std::endl;
    stop();
  }
}

void CGPCounter::value_read() {
  if(!stopped) {
    sc_core::sc_time now = sc_core::sc_time_stamp();
    int reload = p.r[RELOAD] + 1;
    int dticks;
    if(p.r[CTRL].b[TIM_CTRL_CH]) {
      dticks = p.numberofticksbetween(lasttime, now, 0, p.counter[nr-1]->cycletime());
    } else {
      dticks = p.numberofticksbetween(lasttime, now, nr+2, p.clockcycle);
    }
    int value = ((int)lastvalue - dticks) % reload;
    
    if(value<0) {
      p.r[VALUE] = reload + value;
    } else {
      p.r[VALUE] = value;
    }
  }
}

void CGPCounter::value_write() {
  lastvalue = p.r[VALUE];
  lasttime  = sc_core::sc_time_stamp();
  if(!stopped) {
    calculate();
  }
}

void CGPCounter::chaining() {
  chain_run = true;
  //std::cout << "Chaining_" << nr << std::endl;
  start();
}

void CGPCounter::ticking() {
  while(1) {
    /* calculate sleep time, CGPCounter timeout */
    calculate();
  
    #ifdef DEBUG  
    std::cout << std::endl << "CGPCounter_" << nr << " is wait" << std::endl;
    #endif
    wait(e_wait);
    #ifdef DEBUG
    std::cout << std::endl << "CGPCounter_" << nr << " is rockin'" << std::endl;
    #endif
    
    /* Send interupt and set outputs */
    if(p.r[CTRL].b[TIM_CTRL_IE]) {
        // p.r[CTRL].b[TIM_CTRL_SI] // seperatet interupts
        // APBIRQ addresse beachten -.-
      unsigned int irqnr = (p.r[CONF] >> 3) & 0xF;
      #ifdef DEBUG
      std::cout << "IRQ: " << irqnr << " CONF "<< p.r[CONF] << std::endl;
      #endif
      if(p.r[CONF].b[TIM_CONF_SI]) {
        irqnr += nr;
      }
      p.irq.write(p.irq.read() | (1<<irqnr));
    }
#ifdef DEBUGOUT
    if(p.rst) {
      p.tick.write(p.tick.read() | (1<<(nr + 1)));
    }
#endif
    m_pirq = true;
    wait(p.clockcycle);
#ifdef DEBUGOUT
    if(p.rst) {
      p.tick.write(p.tick.read() & ~(1<<(nr + 1)));
    }
#endif
    
    unsigned int nrn = (nr+1<p.counter.size())? nr+1 : 0;
    if(p.r[TIM_CTRL(nrn)].b[TIM_CTRL_CH]) {
      p.counter[(nrn) % p.counter.size()]->chaining();
    }

    // Enable value becomes restart value
    p.r[CTRL].b[TIM_CTRL_EN].set(p.r[CTRL].b[TIM_CTRL_RS].get());
  }
}

void CGPCounter::do_reset() {
  lastvalue   = 0;
  lasttime    = sc_core::sc_time_stamp();
  p.r[VALUE]  = 0;
  p.r[RELOAD] = 0;
  p.r[CONF]   = 0;
  stopped     = true;
  chain_run   = false;
}


/**
 * Gets the time to the end of the next zero hit.
 */
sc_core::sc_time CGPCounter::nextzero() {
  sc_core::sc_time t; /* cycle time of foundation (other CGPCounter or the cloccycle) */
  int x; /* Per cycle */
  if(p.r[CTRL].b[TIM_CTRL_CH]) { /* We depend on the cycle time of the last CGPCounter. */
    if(nr) {
      //p.counter[nr-1]->value_read();
      //t = p.counter[nr-1]->cycletime();
      x = p.r[TIM_VALUE(nr-1)];
    } else {
      p.counter[p.counter.size()-1]->value_read();
      t = p.counter[p.counter.size()-1]->cycletime();
      x = p.r[TIM_VALUE(p.counter.size()-1)];
    }
  } else { /* We only depend on the prescaler */
    p.scaler_read();
    t = p.clockcycle;
    x = p.r[SCALER];
  } 
  return t * (x + 2);
}

/**
 * Gets the Cycletime of the CCGPCounter.
 */
sc_core::sc_time CGPCounter::cycletime() {
  sc_core::sc_time t; /* cycle time of foundation (other CGPCounter or the cloccycle) */
  int m; /* Per cycle */
  if(p.r[CTRL].b[TIM_CTRL_CH]) { /* We depend on the cycle time of the last CGPCounter. */
    if(nr) {
      t = p.counter[nr-1]->cycletime();
      m = p.r[TIM_RELOAD(nr-1)];
    } else {
      t = p.counter[p.counter.size()]->cycletime();
      m = p.r[TIM_RELOAD(p.counter.size())];
    }
  } else 
  { /* We only depend on the prescaler */
    t = p.clockcycle;
    m = p.r[TIM_SCRELOAD];
    //m = 1;
  }
  return t * (m + 1);
  //return t * (m + 1);
}

/* Recalculate sleeptime and send notification */
void CGPCounter::calculate() {
  e_wait.cancel();
  value_read();
  int value = p.r[VALUE];
  sc_core::sc_time time;
  // Calculate with currentime, and lastvalue updates
  if(p.r[CTRL].b[TIM_CTRL_EN]) {
    sc_core::sc_time zero = this->nextzero();
    sc_core::sc_time cycle = this->cycletime();
    #ifdef DEBUG
    std::cout << " calc_" << nr << " Zero: " << zero << " Cycle: " << cycle << " Value: " << value << std::endl;
    #endif
    time = zero;
    time += (cycle * value) + (nr + 1) * p.clockcycle;
    #ifdef DEBUG
    std::cout << " calc_" << nr << ": " << time << std::endl;
    #endif
    e_wait.notify(time);
  }
}

/* Start counting imideately.
 * For example for enable, !dhalt, e_chain
 */
void CGPCounter::start() {
  std::cout << "start_" << nr << " stopped: " 
            << stopped << "-" 
            << p.r[CTRL].b[TIM_CTRL_EN] << "-" 
            << (p.dhalt.read()!=0) << "-" 
            << (!p.r[CTRL].b[TIM_CTRL_CH] || (p.r[CTRL].b[TIM_CTRL_CH] && chain_run)) 
            << std::endl;
  if(stopped && p.r[CTRL].b[TIM_CTRL_EN] /*&& (p.dhalt.read()!=0)*/ && 
    (!p.r[CTRL].b[TIM_CTRL_CH] || (p.r[CTRL].b[TIM_CTRL_CH] && chain_run))) {
    std::cout << "startnow_" << nr << std::endl;
    
    lasttime  = sc_core::sc_time_stamp();
    calculate();
    stopped = false;
  }
}

/* Stoping counting imideately.
 * For example for disableing, dhalt, e_chain
 */
void CGPCounter::stop() {
  if(!stopped) {
    //std::cout << "stop_" << nr << std::endl;
    e_wait.cancel();
    value_read();
    lastvalue = p.r[VALUE];
    lasttime  = sc_core::sc_time_stamp();
    stopped = true;
  }
}

/// @}
