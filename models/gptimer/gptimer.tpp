/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       gptimer.tpp                                             */
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

#define SCALER            (0x00)
#define SCRELOAD          (0x04)
#define CONF              (0x08)
#define VALUE             (0x10*(nr+1)+0x0)
#define RELOAD            (0x10*(nr+1)+0x4)
#define CTRL              (0x10*(nr+1)+0x8)

Counter::Counter(Timer &_parent, unsigned int _nr, sc_core::sc_module_name name)
  : gr_subdevice(name, _parent), p(_parent), nr(_nr), stopped(true), chain_run(false) {  
  SC_THREAD(ticking);
  //do_reset();
}

Counter::~Counter() {
  GC_UNREGISTER_CALLBACKS();
}
  
void Counter::end_of_elaboration() {
  GR_FUNCTION(Counter, ctrl_read);
  GR_SENSITIVE(p.r[CTRL].add_rule(gs::reg::PRE_READ, 
                                  gen_unique_name("ctrl_read", false), 
                                  gs::reg::NOTIFY));

  GR_FUNCTION(Counter, ctrl_write);
  GR_SENSITIVE(p.r[CTRL].add_rule(gs::reg::POST_WRITE, 
                                  gen_unique_name("ctrl_write", false), 
                                  gs::reg::NOTIFY));

  GR_FUNCTION(Counter, value_read);
  GR_SENSITIVE(p.r[VALUE].add_rule(gs::reg::PRE_READ, 
                                   gen_unique_name("value_read", false), 
                                   gs::reg::NOTIFY));

  GR_FUNCTION(Counter, value_write);
  GR_SENSITIVE(p.r[VALUE].add_rule(gs::reg::POST_WRITE, 
                                   gen_unique_name("value_write", false), 
                                   gs::reg::NOTIFY));

  p.r[VALUE].enable_events();
}

void Counter::ctrl_read() {
  p.r[CTRL].b[TIM_CTRL_DH] = (p.dhalt.read() != 0);
  p.r[CTRL].b[TIM_CTRL_IP] = m_pirq; 
}

void Counter::ctrl_write() {
  p.r[CTRL].b[TIM_CTRL_DH] = (p.dhalt.read() != 0);
  std::cout << " Timer " << nr << " write " << std::endl;

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

void Counter::value_read() {
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

void Counter::value_write() {
  lastvalue = p.r[VALUE];
  lasttime  = sc_core::sc_time_stamp();
  if(!stopped) {
    calculate();
  }
}

void Counter::chaining() {
  chain_run = true;
  //std::cout << "Chaining_" << nr << std::endl;
  start();
}

void Counter::ticking() {
  while(1) {
    /* calculate sleep time, counter timeout */
    calculate();
  
    #ifdef DEBUG  
    std::cout << std::endl << "Counter_" << nr << " is wait" << std::endl;
    #endif
    wait(e_wait);
    #ifdef DEBUG
    std::cout << std::endl << "Counter_" << nr << " is rockin'" << std::endl;
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

void Counter::do_reset() {
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
sc_core::sc_time Counter::nextzero() {
  sc_core::sc_time t; /* cycle time of foundation (other counter or the cloccycle) */
  int x; /* Per cycle */
  if(p.r[CTRL].b[TIM_CTRL_CH]) { /* We depend on the cycle time of the last Counter. */
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
 * Gets the Cycletime of the Ccounter.
 */
sc_core::sc_time Counter::cycletime() {
  sc_core::sc_time t; /* cycle time of foundation (other counter or the cloccycle) */
  int m; /* Per cycle */
  if(p.r[CTRL].b[TIM_CTRL_CH]) { /* We depend on the cycle time of the last Counter. */
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
void Counter::calculate() {
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
void Counter::start() {
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
void Counter::stop() {
  if(!stopped) {
    //std::cout << "stop_" << nr << std::endl;
    e_wait.cancel();
    value_read();
    lastvalue = p.r[VALUE];
    lasttime  = sc_core::sc_time_stamp();
    stopped = true;
  }
}

Timer::Timer(sc_core::sc_module_name name, unsigned int ntimers, int gpindex, int gpaddr, int gpmask, int gpirq, int gsepirq, int gsbits, int gnbits, int gwdog)
  : gr_device(name, gs::reg::ALIGNED_ADDRESS, 4*(1+ntimers), NULL)
  , GrlibDevice(0x1, 0x11, 0, 0, gpirq, GrlibBAR(APBIO, gpmask, false, false, gpaddr))
  , bus( "bus", r, 0x0, 0xFFFFFFFF, ::amba::amba_APB, ::amba::amba_LT, false)
  , rst(this, &Timer::do_reset, "RESET")
  , dhalt(this, &Timer::do_dhalt, "DHALT")
  , tick(this, "TICK"), irq(this, "IRQ"), wdog(this, "WDOG")
  , conf_defaults((gsepirq << 8) | ((gpirq & 0xF) << 3) | (ntimers & 0x7))
  , lasttime(0, sc_core::SC_NS), lastvalue(0)
  , clockcycle(10.0, sc_core::SC_NS) {

  /* create register */
  r.create_register( "scaler", "Scaler Value", 
         /* offset */ 0x00,
         /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
     /* init value */ 0xFFFFFFFF,
     /* write mask */ (2<<(gsbits))-1, /* sbits defines the width of the value. Any unused most significant bits are reserved Always read as 0s. */ 
      /* reg width */ 32,                            /* Maximum register with is 32bit sbit must be less than 32. */
      /* lock mask */ 0x00                           /* Not implementet has to be zero. */
                   );
  r.create_register( "screload", "Scaler Reload Value", 
         /* offset */ 0x04,
         /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
     /* init value */ 0xFFFFFFFF, 
     /* write mask */ static_cast<unsigned int>((1ULL<<gsbits)-1), /* sbits defines the width of the reload. Any unused most significant bits are reserved Always read as 0s. */ 
      /* reg width */ 32, 
      /* lock mask */ 0x00
                   );
  r.create_register( "conf", "Configuration Register", 
         /* offset */ 0x08,
         /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
     /* init value */ conf_defaults, 
     /* write mask */ 0x00003FFF, 
      /* reg width */ 32, 
      /* lock mask */ 0x00
                   );

  for(unsigned int i=0;i<ntimers;++i) {
    Counter *c = new Counter(*this, i, gen_unique_name("counter", true));
    counter.push_back(c);
    r.create_register( gen_unique_name("value", false), "Counter Value Register", 
          /* offset */ TIM_VALUE(i),
          /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
      /* init value */ 0x00000000, 
      /* write mask */ static_cast<unsigned int>((1ULL<<gnbits)-1), /* nbits defines the width of the value. Any unused most significant bits are reserved Always read as 0s. */ 
       /* reg width */ 32, 
       /* lock mask */ 0x00
                     );
    r.create_register( gen_unique_name("reload", false), "Reload Value Register", 
          /* offset */ TIM_RELOAD(i),
          /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
      /* init value */ 0x00000001, 
      /* write mask */ static_cast<unsigned int>((1ULL<<gnbits)-1), /* nbits defines the width of the reload. Any unused most significant bits are reserved Always read as 0s. */ 
       /* reg width */ 32, 
       /* lock mask */ 0x00
                     );
    r.create_register( gen_unique_name("ctrl", false), "Controle Register", 
          /* offset */ TIM_CTRL(i),
          /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
      /* init value */ 0x00000000, 
      /* write mask */ 0x0000007F, 
       /* reg width */ 32, 
       /* lock mask */ 0x00
                     );
  }
 
  #ifdef DEBUG
  SC_THREAD(diag);
  #endif

  #ifdef DEBUGOUT  
  SC_THREAD(ticking);
  #endif
}

Timer::~Timer() {
  for(std::vector<Counter *>::iterator iter=counter.begin();iter!=counter.end();iter++) {
  //for(counter.iterator iter=counter.begin();iter!=counter.end();iter++) {
    delete *iter;
  }
  GC_UNREGISTER_CALLBACKS();
}
  
void Timer::end_of_elaboration() {   
  GR_FUNCTION(Timer, scaler_read);
  GR_SENSITIVE(r[SCALER].add_rule( gs::reg::PRE_READ, 
                                   "scaler_read", 
                                   gs::reg::NOTIFY));
  
  GR_FUNCTION(Timer, scaler_write);
  GR_SENSITIVE(r[SCALER].add_rule( gs::reg::POST_WRITE, 
                                   "scaler_write", 
                                   gs::reg::NOTIFY));
  GR_FUNCTION(Timer, screload_write);
  GR_SENSITIVE(r[SCRELOAD].add_rule( gs::reg::POST_WRITE, 
                                     "scaler_reload_write", 
                                     gs::reg::NOTIFY));

  GR_FUNCTION(Timer, conf_read);
  GR_SENSITIVE(r[CONF].add_rule( gs::reg::PRE_READ, 
                                 "conf_read", 
                                 gs::reg::NOTIFY));
  
}

#ifdef DEBUGOUT
void Timer::tick_calc() {
  e_tick.cancel();
  scaler_read();
  int scaler = r[SCALER] + 1;
  int reload = r[SCRELOAD] + 1;
  //sc_core::sc_time now = sc_core::sc_time_stamp();
  //int scaler = valueof(now) - reload;
  if(reload) {
    scaler = (scaler) % reload;
  }
  //std::cout << "Scaler: " << scaler << std::endl;
  //std::cout << "Reload: " << reload << std::endl;
  //if(((unsigned )scaler != 0xFFFFFFFF) || (scaler == 0 && reload == 0)) {
  //  e_tick.notify(clockcycle * (((scaler)? scaler: (reload - 1)) - 1));
  //}  
  //if(reload != 0) {
    e_tick.notify(clockcycle * (scaler));
  //}
}


void Timer::ticking() {
  while(1) {
    tick_calc();
    wait(e_tick);
    if(rst.read() == 1) {
      tick.write(tick.read() | 1);
    }
    wait(clockcycle);
    if(rst.read() == 1) {
      tick.write(tick.read() & ~1);
    }
  }   
}
#endif

/* Disable or enable Counter when DHALT arrives 
 * The value will be directly fetched in conf_read to show the right value in the conf registers.
 */
void Timer::do_dhalt(const bool &value, signalkit::signal_in_if<bool> *signal, signalkit::signal_out_if<bool> *sender, const sc_core::sc_time &time) {
  if(r[TIM_CONF].b[TIM_CONF_DF]) {
    if(value) {
      for(std::vector<Counter *>::iterator iter=counter.begin();iter!=counter.end();iter++) {
        (*iter)->stop();
      }
    } else {
      for(std::vector<Counter *>::iterator iter=counter.begin();iter!=counter.end();iter++) {
        (*iter)->start();
      }
    }
  }
}

void Timer::scaler_read() {
  sc_core::sc_time now = sc_core::sc_time_stamp();
  int reload = r[SCRELOAD] + 1;
  int value  = valueof(now, 0, clockcycle) - (reload);
//  std::cout << " pure: " << std::dec << valueof(now) << "->" << value;

//  if(value<0) {
  if(reload) {
    value = value % (reload);
  }
  r[SCALER] = reload + value -1 ;
//  std::cout << " reg: " << std::dec << value << " +reload: " << reload + value << std::endl;
//  } else {
//  if(reload + 1) {
//    value = value % (reload + 1);
//  }
//    r[SCALER] = value;
//    std::cout << "unten" << std::dec << value << std::endl;
//  }
  // lastvalue = r[SCALER];
  // lasttime  = now;
}
void Timer::screload_write() {
  uint32_t reload = r[SCRELOAD];
  r[SCALER] = reload;
//  std::cout << "!Reload: " << reload << std::endl;
  scaler_write();
}

void Timer::scaler_write() {
  lasttime = sc_core::sc_time_stamp();
  lastvalue = r[SCALER];
//  std::cout << "!Scaler: " << lastvalue << std::endl;
  #ifdef DEBUGOUT
  tick_calc();
  #endif
}

void Timer::conf_read() {
  r[CONF] = (r[CONF] & 0x0000030) | (conf_defaults & 0x0000000F);
}

void Timer::do_reset(const bool &value, signalkit::signal_in_if<bool> *signal, signalkit::signal_out_if<bool> *sender, const sc_core::sc_time &time) {
  if(!value) {
    r[SCALER]   = 0xFFFFFFFF;
    r[SCRELOAD] = 0xFFFFFFFF;
    r[CONF]   = conf_defaults;
    lastvalue = 0;
    tick = 0;
    wdog = 0;
    lasttime = sc_core::sc_time_stamp();
    scaler_write();
    scaler_read();
    
    for(std::vector<Counter *>::iterator iter=counter.begin();iter!=counter.end();iter++) {
      (*iter)->do_reset();
    }
  }
}

inline int Timer::valueof(sc_core::sc_time t, int offset, sc_core::sc_time cycletime) const {
  return (int)(lastvalue - ((t - lasttime - (1+offset)* cycletime) / cycletime) + 1);
}
 
inline int Timer::numberofticksbetween(sc_core::sc_time a, sc_core::sc_time b, int counter, sc_core::sc_time cycletime) {
  int reload = r[SCRELOAD] + 1;
  int val_a = valueof(a, 0, cycletime);
  int val_b = valueof(b, counter, cycletime);
  int num_a = val_a / reload + (val_a > 0 && val_b < 0);
  int num_b = val_b / reload;
  return std::abs(num_a - num_b);
}

void Timer::clk(sc_core::sc_clock &clk) {
  clockcycle = clk.period();
}

void Timer::clk(sc_core::sc_time &period) {
  clockcycle = period;
}

void Timer::clk(double period, sc_core::sc_time_unit base) {
  clockcycle = sc_core::sc_time(period, base);
}

#ifdef DEBUG
#define SHOWCOUNTER(n) \
    counter[n]->value_read(); \
    std::cout << " Timer"#n << ":{ v:" << r[TIM_VALUE(n)] << ", r:" << r[TIM_RELOAD(n)] << "}";

void Timer::diag() {
  while(1) {
    std::printf("\n@%-7s /%-4d: ", sc_core::sc_time_stamp().to_string().c_str(), (unsigned)sc_core::sc_delta_count());
    scaler_read();
    std::cout << "Scaler:{ v:" << r[TIM_SCALER] << ", r:" << r[TIM_SCRELOAD] << "}";
    SHOWCOUNTER(0);
    SHOWCOUNTER(1);
    SHOWCOUNTER(2);
    wait(clockcycle);
  }
}
#endif
