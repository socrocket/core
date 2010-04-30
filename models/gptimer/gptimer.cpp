#include "gptimer.h"
#include "timreg.h"
#include <string>

#define SCALER            (0x00)
#define SCRELOAD          (0x04)
#define CONF              (0x08)
#define VALUE             (0x10*(nr+1)+0x0)
#define RELOAD            (0x10*(nr+1)+0x4)
#define CTRL              (0x10*(nr+1)+0x8)

Counter::Counter(Timer &_parent, unsigned int _nr, sc_core::sc_module_name name, unsigned int nbits)
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
  p.r[CTRL].b[TIM_CTRL_DH] = (p.gpti.read().dhalt == 1);
  p.r[CTRL].b[TIM_CTRL_IP] = m_pirq; 
}

void Counter::ctrl_write() {
  p.r[CTRL].b[TIM_CTRL_DH] = (p.gpti.read().dhalt == 1);

  /* Clean irq if desired */
  m_pirq = p.r[CTRL].b[TIM_CTRL_IP] and m_pirq;

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
  //std::cout << "Start_" << nr << std::endl;
    start();
  } else if((!p.r[CTRL].b[TIM_CTRL_EN])&&!stopped) {
    //std::cout << "Stop_" << nr << std::endl;
    stop();
  }
}

void Counter::value_read() {
  if(!stopped) {
    sc_core::sc_time now = sc_core::sc_time_stamp();
    int reload = p.r[RELOAD] + 1;
    int ticks = p.numberofticksbetween(lasttime, now, nr+2);
    int value = ((int)lastvalue - ticks) % reload;
    
    if(value<0) {
      p.r[VALUE] = reload + value;
    } else {
      p.r[VALUE] = value;
    }
    //lastvalue = p.r[VALUE];
    //lasttime  = now;
    //p.r[VALUE] = 3;
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
  bool val;
  while(1) {
    /* calculate sleep time, counter timeout */
    calculate();
    
    std::cout << std::endl << "Counter_" << nr << " is wait" << std::endl;
    wait(e_wait);
    std::cout << std::endl << "Counter_" << nr << " is rockin'" << std::endl;
    
    /* Send interupt and set outputs */
    if(p.r[CTRL].b[TIM_CTRL_IE]) {
        // p.r[CTRL].b[TIM_CTRL_SI] // seperatet interupts
        // APBIRQ addresse beachten -.-
      unsigned int irqnr = (p.r[CONF] >> 3) & 0xF, irq;
      std::cout << "IRQ: " << irqnr << " CONF "<< p.r[CONF] << std::endl;
      if(p.r[CONF].b[TIM_CONF_SI]) {
        irqnr += nr;
      }
      irq = pirq.read();
      irq &= (1<<irqnr);
      val = p.tick[nr+1].read();
      val = true;
      if(p.reset==1) {
        p.tick[nr+1].write(val);
        pirq.write(irqnr);
      }
      m_pirq = true;
      wait(p.clockcycle);
      irq = pirq.read();
      irq &= ~(1<<irqnr);
      val = p.tick[nr+1].read();
      val = false;
      if(p.reset==1) {
        p.tick[nr+1].write(val);
        pirq.write(irqnr);
      }
    }
    
    if(p.r[TIM_CTRL(nr+1)].b[TIM_CTRL_CH]) {
      p.counter[(nr+1) % p.counter.size()]->chaining();
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
    
/* Recalculate sleeptime and send notification */
void Counter::calculate() {
  e_wait.cancel();
  // Calculate with currentime, and lastvalue updates
  if(p.r[CTRL].b[TIM_CTRL_EN]) {
    p.scaler_read();
    value_read();
    int scaler   = p.r[SCALER];
    int screload = p.r[SCRELOAD];
    int value    = p.r[VALUE];
    if(value==0) {
      value      = p.r[RELOAD];
    }

    sc_core::sc_time time = (p.clockcycle * (scaler + 2)) + (p.clockcycle * screload * (value+1));
    std::cout << " calc_" << nr << ": " << time << std::endl;
    //sc_core::sc_time time = (p.clockcycle * p.lastvalue + p.lasttime) * lastvalue + lasttime + p.clockcycle * (nr+1);
    e_wait.notify(time);
  }
}

/* Start counting imideately.
 * For example for enable, !dhalt, e_chain
 */
void Counter::start() {
  //std::cout << "start_" << nr << " stopped:" << stopped << "-" << (!p.r[CTRL].b[TIM_CTRL_CH] || (p.r[CTRL].b[TIM_CTRL_CH] && chain_run)) << std::endl;
  if(stopped && p.r[CTRL].b[TIM_CTRL_EN] && (p.gpti.read().dhalt != 1) && 
    (!p.r[CTRL].b[TIM_CTRL_CH] || (p.r[CTRL].b[TIM_CTRL_CH] && chain_run))) {
    //std::cout << "start_" << nr << std::endl;
    
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

Timer::Timer(sc_core::sc_module_name name, unsigned int pirq, bool sepirq, unsigned int ntimers, unsigned int nbits, unsigned int sbits, unsigned int wdog)
  : gr_device(name, gs::reg::ALIGNED_ADDRESS, 4*(1+ntimers), NULL)
  , bus( "bus", r, 0x0, 0xFFFFFFFF, ::amba::amba_APB, ::amba::amba_LT, false)
  , reset("reset"), gpti("GPTIMER_IN"), gpto("GPTIMER_OUT"), pirqi("GPTIMER_IRQ_IN"), pirqo("GPTIMER_IRQ_OUT")
  , pconfig_0("PCONFIG_0"), pconfig_1("PCONFIG_1"), pindex("PINDEX")
  , conf_defaults((sepirq << 8) | ((pirq & 0xF) << 3) | (ntimers & 0x7))
  , lasttime(0, sc_core::SC_NS), lastvalue(0)
  , clockcycle(10.0, sc_core::SC_NS) {

  /* create register */
  r.create_register( "scaler", "Scaler Value", 
         /* offset */ 0x00,
         /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
     /* init value */ 0xFFFFFFFF,
     /* write mask */ (2<<(sbits))-1, /* sbits defines the width of the value. Any unused most significant bits are reserved Always read as 0s. */ 
      /* reg width */ 32,                            /* Maximum register with is 32bit sbit must be less than 32. */
      /* lock mask */ 0x00                           /* Not implementet has to be zero. */
                   );
  r.create_register( "screload", "Scaler Reload Value", 
         /* offset */ 0x04,
         /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
     /* init value */ 0xFFFFFFFF, 
     /* write mask */ (2<<(sbits))-1, /* sbits defines the width of the reload. Any unused most significant bits are reserved Always read as 0s. */ 
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
    Counter *c = new Counter(*this, i, gen_unique_name("counter", true), nbits);
    counter.push_back(c);
    r.create_register( gen_unique_name("value", false), "Counter Value Register", 
          /* offset */ TIM_VALUE(i),
          /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
      /* init value */ 0x00000000, 
      /* write mask */ (2<<(nbits+1))-1, /* nbits defines the width of the value. Any unused most significant bits are reserved Always read as 0s. */ 
       /* reg width */ 32, 
       /* lock mask */ 0x00
                     );
    r.create_register( gen_unique_name("reload", false), "Reload Value Register", 
          /* offset */ TIM_RELOAD(i),
          /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
      /* init value */ 0x00000001, 
      /* write mask */ (2<<(nbits+1))-1, /* nbits defines the width of the reload. Any unused most significant bits are reserved Always read as 0s. */ 
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
 
  SC_THREAD(diag);
  
  SC_THREAD(ticking);

  SC_THREAD(tickjoin);
  sensitive << tick[0] << tick[1] << tick[2] << tick[3] << tick[4] << tick[5] << tick[6] << tick[7];

  SC_THREAD(irqjoin);
  for(unsigned int i=0;i<counter.size();i++) {
    sensitive << counter[i]->pirq;
  }

  SC_THREAD(do_reset);
  sensitive << reset.pos(); 

  SC_THREAD(do_dhalt);
  sensitive << gpti;
}

Timer::~Timer() {
  for(std::vector<Counter *>::iterator iter=counter.begin();iter!=counter.end();iter++) {
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
  //  e_tick.notify(clockcycle * (((scaler)? scaler: (reload-1))-1));
  //}  
  //if(reload != 0) {
    e_tick.notify(clockcycle * (scaler));
  //}
}

void Timer::ticking() {
  bool val;
  while(1) {
    tick_calc();
    wait(e_tick);
    val = tick[0].read();
    val = true;
    if(reset==1) {
      tick[0].write(val);
    }
    wait(clockcycle);
    val = tick[0].read();
    val = false;
    if(reset==1) {
      tick[0].write(val);
    }
  }   
}

void Timer::tickjoin() {
  gptimer_out_type val;
  while(1) {
    val = gpto.read();
    val.tick = 0;
    for(int i=0;i<8;i++) {
      bool t = tick[i].read();
      val.tick |= (t<<(7-i));
    }
    gpto.write(val);
    wait();
  }
}

void Timer::irqjoin() {
  unsigned int irq = 0;
  while(1) {
    for(unsigned int i=0;i<counter.size();i++) {
      irq |= counter[i]->pirq.read();
    }
    pirqo.write(irq);
    wait();
  }
}

/* Disable or enable Counter when DHALT arrives 
 * The value will be directly fetched in conf_read to show the right value in the conf registers.
 */
void Timer::do_dhalt() {
  while(1) {
    if(r[TIM_CONF].b[TIM_CONF_DF]) {
      if(gpti.read().dhalt == 1) {
        for(std::vector<Counter *>::iterator iter=counter.begin();iter!=counter.end();iter++) {
          (*iter)->stop();
        }
      } else {
        for(std::vector<Counter *>::iterator iter=counter.begin();iter!=counter.end();iter++) {
          (*iter)->start();
        }
      }
    }
    wait();
  }
}

void Timer::scaler_read() {
  sc_core::sc_time now = sc_core::sc_time_stamp();
  int reload = r[SCRELOAD] +1;
  int value  = valueof(now) - (reload);
//  std::cout << " pure: " << std::dec << valueof(now)<< "->" << value;

//  if(value<0) {
  if(reload) {
    value = value % (reload);
  }
  r[SCALER] = reload + value -1 ;
//  std::cout << " reg: " << std::dec << value << " +reload: " << reload + value << std::endl;
//  } else {
//  if(reload+1) {
//    value = value % (reload+1);
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
  tick_calc();
}

void Timer::conf_read() {
  r[CONF] = (r[CONF] & 0x0000030) | (conf_defaults & 0x0000000F);
}

void Timer::do_reset() {
  gptimer_out_type val;
  while(1) {
    wait();
    if(reset==0) {
      r[SCALER]   = 0xFFFFFFFF;
      r[SCRELOAD] = 0xFFFFFFFF;
      r[CONF]   = conf_defaults;
      lastvalue = 0;
      val.tick  = 0;
      val.timer1 = 0;
      val.wdogn  = true;
      val.wdog   = false;
      gpto.write(val);
      lasttime = sc_core::sc_time_stamp();
      scaler_write();
      scaler_read();
      
      for(std::vector<Counter *>::iterator iter=counter.begin();iter!=counter.end();iter++) {
        (*iter)->do_reset();
      }
    }
  }
}

int Timer::valueof(sc_core::sc_time t, int offset) {
  return (int)(lastvalue - ((t - lasttime - (1+offset)* clockcycle) / clockcycle)+1);
}
 
int Timer::numberofticksbetween(sc_core::sc_time a, sc_core::sc_time b, int counter) {
  int reload = r[SCRELOAD] + 1;
  int val_a = valueof(a);
  int val_b = valueof(b, (counter));
  int num_a = val_a / reload + (val_a>0&&val_b<0);
  int num_b = val_b / reload;
//  std::cout << std::dec << " val_a: " << std::dec << val_a << " val_b: " << std::dec << val_b << " num_a " << std::dec << num_a << " num_b " << std::dec << num_b;
  return std::abs(num_a - num_b);
}

void Timer::clk(sc_core::sc_clock &clk) {
  clockcycle = clk.period();
}

void Timer::clk(sc_core::sc_time &period) {
  clockcycle = period;
}

void Timer::clk(double &period, sc_core::sc_time_unit &base) {
  clockcycle = sc_core::sc_time(period, base);
}

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
