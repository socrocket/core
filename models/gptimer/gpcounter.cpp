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
  GR_SENSITIVE(p.r[CGPTimer::CTRL(nr)].add_rule(gs::reg::PRE_READ, 
                                  gen_unique_name("ctrl_read", false), 
                                  gs::reg::NOTIFY));

  GR_FUNCTION(CGPCounter, ctrl_write);
  GR_SENSITIVE(p.r[CGPTimer::CTRL(nr)].add_rule(gs::reg::POST_WRITE, 
                                  gen_unique_name("ctrl_write", false), 
                                  gs::reg::NOTIFY));

  GR_FUNCTION(CGPCounter, value_read);
  GR_SENSITIVE(p.r[CGPTimer::VALUE(nr)].add_rule(gs::reg::PRE_READ, 
                                   gen_unique_name("value_read", false), 
                                   gs::reg::NOTIFY));

  GR_FUNCTION(CGPCounter, value_write);
  GR_SENSITIVE(p.r[CGPTimer::VALUE(nr)].add_rule(gs::reg::POST_WRITE, 
                                   gen_unique_name("value_write", false), 
                                   gs::reg::NOTIFY));

  p.r[CGPTimer::VALUE(nr)].enable_events();
}

void CGPCounter::ctrl_read() {
  p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_DH] = (p.dhalt.read() != 0);
  p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_IP] = m_pirq; 
}

void CGPCounter::ctrl_write() {
  p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_DH] = (p.dhalt.read() != 0);
  std::cout << " CGPTimer " << nr << " write " << std::endl;

  /* Clean irq if desired */
  bool old_pirq = m_pirq;

  m_pirq = p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_IP] and m_pirq;
  if(old_pirq && !m_pirq) {
    unsigned int irqnr = (p.r[CGPTimer::CONF] >> 3) & 0xF;
    if(p.r[CGPTimer::CONF].b[CGPTimer::CONF_SI]) {
       irqnr += nr;
    }
    p.irq.write(p.irq.read() & ~(1<<irqnr));
  }

  /* Prepare for chainging */
  if(p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH]) {
    chain_run = false;
    stop();
  }

  /* Load */
  if(p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_LD]) {
    unsigned int reload = p.r[CGPTimer::RELOAD(nr)]; 
    p.r[CGPTimer::VALUE(nr)].set(reload);
    value_write();
    p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_LD] = false;
  }

  /* Enable */
  //std::cout << "StartStop_" << nr << std::endl;
  if(p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_EN] && stopped) {
    std::cout << "Start_" << nr << std::endl;
    start();
  } else if((!p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_EN])&&!stopped) {
    std::cout << "Stop_" << nr << std::endl;
    stop();
  }
}

void CGPCounter::value_read() {
  if(!stopped) {
    sc_core::sc_time now = sc_core::sc_time_stamp();
    int reload = p.r[CGPTimer::RELOAD(nr)] + 1;
    int dticks;
    if(p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH]) {
      dticks = p.numberofticksbetween(lasttime, now, 0, p.counter[nr-1]->cycletime());
    } else {
      dticks = p.numberofticksbetween(lasttime, now, nr+2, p.clockcycle);
    }
    int value = ((int)lastvalue - dticks) % reload;
    
    if(value<0) {
      p.r[CGPTimer::VALUE(nr)] = reload + value;
    } else {
      p.r[CGPTimer::VALUE(nr)] = value;
    }
  }
}

void CGPCounter::value_write() {
  lastvalue = p.r[CGPTimer::VALUE(nr)];
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
    if(p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_IE]) {
        // p.r[CGPTimer::CTRL].b[CGPTimer::TIM_CTRL_SI] // seperatet interupts
        // APBIRQ addresse beachten -.-
      unsigned int irqnr = (p.r[CGPTimer::CONF] >> 3) & 0xF;
      #ifdef DEBUG
      std::cout << "IRQ: " << irqnr << " CONF "<< p.r[CGPTimer::CONF] << std::endl;
      #endif
      if(p.r[CGPTimer::CONF].b[CGPTimer::CONF_SI]) {
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
    if(p.r[CGPTimer::CTRL(nrn)].b[CGPTimer::CTRL_CH]) {
      p.counter[(nrn) % p.counter.size()]->chaining();
    }

    // Enable value becomes restart value
    p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_EN].set(p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_RS].get());
  }
}

void CGPCounter::do_reset() {
  lastvalue   = 0;
  lasttime    = sc_core::sc_time_stamp();
  p.r[CGPTimer::VALUE(nr)]  = 0;
  p.r[CGPTimer::RELOAD(nr)] = 0;
  p.r[CGPTimer::CTRL(nr)]   = 0;
  stopped     = true;
  chain_run   = false;
}


/**
 * Gets the time to the end of the next zero hit.
 */
sc_core::sc_time CGPCounter::nextzero() {
  sc_core::sc_time t; /* cycle time of foundation (other CGPCounter or the cloccycle) */
  int x; /* Per cycle */
  if(p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH]) { /* We depend on the cycle time of the last CGPCounter. */
    if(nr) {
      //p.counter[nr-1]->value_read();
      //t = p.counter[nr-1]->cycletime();
      x = p.r[CGPTimer::VALUE(nr-1)];
    } else {
      p.counter[p.counter.size()-1]->value_read();
      t = p.counter[p.counter.size()-1]->cycletime();
      x = p.r[CGPTimer::VALUE(p.counter.size()-1)];
    }
  } else { /* We only depend on the prescaler */
    p.scaler_read();
    t = p.clockcycle;
    x = p.r[CGPTimer::SCALER];
  } 
  return t * (x + 2);
}

/**
 * Gets the Cycletime of the CCGPCounter.
 */
sc_core::sc_time CGPCounter::cycletime() {
  sc_core::sc_time t; /* cycle time of foundation (other CGPCounter or the cloccycle) */
  int m; /* Per cycle */
  if(p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH]) { /* We depend on the cycle time of the last CGPCounter. */
    if(nr) {
      t = p.counter[nr-1]->cycletime();
      m = p.r[CGPTimer::RELOAD(nr-1)];
    } else {
      t = p.counter[p.counter.size()]->cycletime();
      m = p.r[CGPTimer::RELOAD(p.counter.size())];
    }
  } else 
  { /* We only depend on the prescaler */
    t = p.clockcycle;
    m = p.r[CGPTimer::SCRELOAD];
    //m = 1;
  }
  return t * (m + 1);
  //return t * (m + 1);
}

/* Recalculate sleeptime and send notification */
void CGPCounter::calculate() {
  e_wait.cancel();
  value_read();
  int value = p.r[CGPTimer::VALUE(nr)];
  sc_core::sc_time time;
  // Calculate with currentime, and lastvalue updates
  if(p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_EN]) {
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
            << p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_EN] << "-" 
            << (p.dhalt.read()!=0) << "-" 
            << (!p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH] || 
                (p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH] && chain_run)) 
            << std::endl;
  if(stopped && p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_EN] /*&& (p.dhalt.read()!=0)*/ && 
    (!p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH] || 
     (p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH] && chain_run))) {
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
    lastvalue = p.r[CGPTimer::VALUE(nr)];
    lasttime  = sc_core::sc_time_stamp();
    stopped = true;
  }
}

/// @}
