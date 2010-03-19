#include "Timer.h"
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
  GR_SENSITIVE(p.r[CTRL].add_rule(gs::reg::PRE_READ, gen_unique_name("ctrl_read", false), gs::reg::NOTIFY));
  GR_FUNCTION(Counter, ctrl_write);
  GR_SENSITIVE(p.r[CTRL].add_rule(gs::reg::POST_WRITE, gen_unique_name("ctrl_write", false), gs::reg::NOTIFY));
  GR_FUNCTION(Counter, value_read);
  GR_SENSITIVE(p.r[VALUE].add_rule(gs::reg::PRE_READ, gen_unique_name("value_read", false), gs::reg::NOTIFY));
  GR_FUNCTION(Counter, value_write);
  GR_SENSITIVE(p.r[VALUE].add_rule(gs::reg::POST_WRITE, gen_unique_name("value_write", false), gs::reg::NOTIFY));
  p.r[VALUE].enable_events();
}

void Counter::ctrl_read() {
  p.r[CTRL].b[TIM_CTRL_DH] = (p.dhalt == 1);
  p.r[CTRL].b[TIM_CTRL_IP] = m_pirq; 
}

void Counter::ctrl_write() {
  p.r[CTRL].b[TIM_CTRL_DH] = (p.dhalt == 1);

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
  std::cout << "StartStop_" << nr << std::endl;
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
    int ticks = p.numberofticksbetween(lasttime, now);
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
  std::cout << "Chaining_" << nr << std::endl;
  start();
}

void Counter::ticking() {
  while(1) {
    /* calculate sleep time, counter timeout */
    calculate();
    
    std::cout << std::endl << "Counter_"<< nr << " is wait" << std::endl;
    wait(e_wait);
    std::cout << std::endl << "Counter_" << nr << " is rockin'" << std::endl;
    
    /* Send interupt and set outputs */
    if(p.r[CTRL].b[TIM_CTRL_IE]) {
        // p.r[CTRL].b[TIM_CTRL_SI] // seperatet interupts
        // APBIRQ addresse beachten -.-
        //pirq = sc_dt::sc_logic_1;
      m_pirq = true;
    }
    
    if(p.r[TIM_CTRL(nr+1)].b[TIM_CTRL_CH]) {
      p.counter[(nr+1)%p.counter.size()]->chaining();
    }

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
  // Calculate with currentime, and lastvalue updates
  if(p.r[CTRL].b[TIM_CTRL_EN]) {
    e_wait.cancel();
    sc_core::sc_time time = (p.clockcycle * p.lastvalue + p.lasttime) * lastvalue + lasttime + p.clockcycle * (nr+1);
    e_wait.notify(time);
  }
}

/* Start counting imideately.
 * For example for enable, !dhalt, e_chain
 */
void Counter::start() {
  std::cout << "start_" << nr << " stopped:" << stopped << "-" << (!p.r[CTRL].b[TIM_CTRL_CH] || (p.r[CTRL].b[TIM_CTRL_CH] && chain_run)) << std::endl;
  if(stopped && p.r[CTRL].b[TIM_CTRL_EN] && (p.dhalt != 1) && 
    (!p.r[CTRL].b[TIM_CTRL_CH] || (p.r[CTRL].b[TIM_CTRL_CH] && chain_run))) {
    std::cout << "start_" << nr << std::endl;
    
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
    std::cout << "stop_" << nr << std::endl;
    e_wait.cancel();
    value_read();
    lastvalue = p.r[VALUE];
    lasttime  = sc_core::sc_time_stamp();
    stopped = true;
  }
}

Timer::Timer(sc_core::sc_module_name name, unsigned int pirq, bool sepirq, unsigned int ntimers, unsigned int nbits, unsigned int sbits, unsigned int wdog)
  : gr_device(name, gs::reg::ALIGNED_ADDRESS, 4*(1+ntimers), NULL)
  , bus( "bus", r, 0x0, 0xFFFFFFFF)
  , conf_defaults((sepirq << 8) | ((pirq & 0xF) << 3) | (ntimers & 0x7))
  , lasttime(0, sc_core::SC_NS), lastvalue(0)
  , clockcycle(1.0, sc_core::SC_NS) {

  std::cout << "conf_defaults: " << std::hex << conf_defaults << std::endl;
  
  /* create register */
  r.create_register( "scaler", "Scaler Value", 
         /* offset */ 0x00,
         /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
     /* init value */ 0x00000000,
     /* write mask */ (2<<(sbits))-1, /* sbits defines the width of the value. Any unused most significant bits are reserved Always read as 0s. */ 
      /* reg width */ 32,                            /* Maximum register with is 32bit sbit must be less than 32. */
      /* lock mask */ 0x00                           /* Not implementet has to be zero. */
                   );
  r.create_register( "screload", "Scaler Reload Value", 
         /* offset */ 0x04,
         /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
     /* init value */ 0x00000000, 
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
    r.create_register( gen_unique_name("value", true), "Counter Value Register", 
          /* offset */ TIM_VALUE(i),
          /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
      /* init value */ 0x00000000, 
      /* write mask */ (2<<(nbits+1))-1, /* nbits defines the width of the value. Any unused most significant bits are reserved Always read as 0s. */ 
       /* reg width */ 32, 
       /* lock mask */ 0x00
                     );
    r.create_register( gen_unique_name("reload", true), "Reload Value Register", 
          /* offset */ TIM_RELOAD(i),
          /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
      /* init value */ 0x00000001, 
      /* write mask */ (2<<(nbits+1))-1, /* nbits defines the width of the reload. Any unused most significant bits are reserved Always read as 0s. */ 
       /* reg width */ 32, 
       /* lock mask */ 0x00
                     );
    r.create_register( gen_unique_name("ctrl", true), "Controle Register", 
          /* offset */ TIM_CTRL(i),
          /* config */ gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
      /* init value */ 0x00000000, 
      /* write mask */ 0x0000007F, 
       /* reg width */ 32, 
       /* lock mask */ 0x00
                     );
  }
  
  SC_THREAD(do_reset);
  sensitive << reset.pos(); 
}

Timer::~Timer() {
  for(std::vector<Counter *>::iterator iter=counter.begin();iter!=counter.end();iter++) {
    delete *iter;
  }
  GC_UNREGISTER_CALLBACKS();
}
  
void Timer::end_of_elaboration() { 
  SC_THREAD(do_dhalt);
  sensitive << dhalt;
  
  GR_FUNCTION_PARAMS(Timer, scaler_read);
  GR_DELAYED_SENSITIVE(r[SCALER].add_rule( gs::reg::PRE_READ, "scaler read", gs::reg::NOTIFY), sc_core::sc_time( 0, sc_core::SC_NS));
  
  GR_FUNCTION_PARAMS(Timer, scaler_write);
  GR_DELAYED_SENSITIVE(r[SCALER].add_rule( gs::reg::POST_WRITE, "scaler read", gs::reg::NOTIFY), sc_core::sc_time( 0, sc_core::SC_NS));

  GR_FUNCTION(Timer, conf_read);
  GR_SENSITIVE(r[CONF].add_rule( gs::reg::PRE_READ, "conf read", gs::reg::NOTIFY));
  
}

/* Disable or enable Counter when DHALT arrives 
 * The value will be directly fetched in conf_read to show the right value in the conf registers.
 */
void Timer::do_dhalt() {
  while(1) {
    if(r[TIM_CONF].b[TIM_CONF_DF]) {
      if(dhalt == 1) {
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

void Timer::scaler_read(gs::reg::transaction_type* &tr, const sc_core::sc_time& delay) {
  sc_core::sc_time now = sc_core::sc_time_stamp();
  int reload = r[SCRELOAD]+1;
  int value  = valueof(now) % reload;
  if(value<0) {
    r[SCALER] = reload + value;
  } else {
    r[SCALER] = value;
  }
  // lastvalue = r[SCALER];
  // lasttime  = now;
}

void Timer::scaler_write(gs::reg::transaction_type* &tr, const sc_core::sc_time& delay) {
  lasttime = sc_core::sc_time_stamp();
  lastvalue = r[SCALER];
}

void Timer::conf_read() {
  r[CONF] = (r[CONF] & 0x0000030) | (conf_defaults & 0x0000000F);
}

void Timer::do_reset() {
  r[SCALER]   = 0;
  r[SCRELOAD] = 0;
  r[CONF]   = conf_defaults;
  lastvalue = 0;
  lasttime = sc_core::sc_time_stamp();
  
  for(std::vector<Counter *>::iterator iter=counter.begin();iter!=counter.end();iter++) {
    (*iter)->do_reset();
  }
}

int Timer::valueof(sc_core::sc_time t) {
  return (int)(lastvalue - ((t - lasttime) / clockcycle));
}
 
int Timer::numberofticksbetween(sc_core::sc_time a, sc_core::sc_time b) {
  int reload = r[SCRELOAD]+1;
  int val_a = valueof(a);
  int val_b = valueof(b);
  int num_a = val_a / reload + (val_a>1&&val_b<1);
  int num_b = val_b / reload;
  return std::abs(num_a - num_b);
}

