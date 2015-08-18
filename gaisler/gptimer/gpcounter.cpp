// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup gptimer
/// @{
/// @file gpcounter.cpp
/// source file defining the implementation of the gptimer model. Due to the
/// fact that the gptimer class is a template class it gets included by its
/// definition in gptimer.h
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#include "gaisler/gptimer/gpcounter.h"
#include "gaisler/gptimer/gptimer.h"
#include "core/common/verbose.h"

GPCounter::GPCounter(GPTimer *_parent, unsigned int _nr, ModuleName name) :
  DefaultBase(name), p(_parent), nr(_nr), stopped(true), chain_run(false),
  m_performance_counters("performance_counters"),
  m_underflows("undeflows", 0ull, m_performance_counters) {
  SC_THREAD(ticking);

  m_api = gs::cnf::GCnf_Api::getApiInstance(this);
}

GPCounter::~GPCounter() {
  GC_UNREGISTER_CALLBACKS();
}

void GPCounter::end_of_elaboration() {
}

void GPCounter::end_of_simulation() {
  v::report << name() << " ********************************************" << v::endl;
  v::report << name() << " * GPCounter Statistic:" << v::endl;
  v::report << name() << " * ----------------" << v::endl;
  v::report << name() << " * Counter Underflows: " << m_underflows << v::endl;
  v::report << name() << " ********************************************" << v::endl;
}

void GPCounter::ctrl_read() {
  p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_DH, 0);
  p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_IP, m_pirq);
}

void GPCounter::ctrl_write() {
  p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_DH, 0);

  m_pirq = p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_IP);

  // Prepare for chainging
  if (p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_CH)) {
    chain_run = false;
    stop();
  }

  // Load
  if (p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_LD)) {
    unsigned int reload = p->r[GPTimer::RELOAD(nr)];
    p->r[GPTimer::VALUE(nr)].write(reload);
    value_write();
    p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_LD, false);
  }

  // Enable
  if (p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_EN) && stopped) {
    v::debug << name() << "Start_" << nr << v::endl;
    start();
  } else if ((!p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_EN)) && !stopped) {
    v::debug << name() << "Stop_" << nr << v::endl;
    stop();
  }
}

void GPCounter::value_read() {
  if (!stopped) {
    sc_core::sc_time now = sc_core::sc_time_stamp();
    int64_t reload = (int64_t)((uint32_t)p->r[GPTimer::RELOAD(nr)]) + 1ll;

    int64_t dticks;
    if (p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_CH)) {
      dticks = p->numberofticksbetween(lasttime, now, 0, p->counter[nr - 1]->cycletime());
    } else {
      dticks = p->numberofticksbetween(lasttime, now, nr + 2, p->clock_cycle);
    }
    int64_t value = ((int64_t)lastvalue - dticks) % reload;

    if ((value < 0) & (p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_RS))) {
      p->r[GPTimer::VALUE(nr)] = (uint32_t)((reload + value));
    } else if  ((value < 0) & !(p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_RS))) {
      p->r[GPTimer::VALUE(nr)] = (static_cast<uint32_t> ((1ULL << p->g_sbits) - 1));
    } else {
      p->r[GPTimer::VALUE(nr)] = (uint32_t)value;
    }
    //v::info << name() << " value_read: value=" << v::uint64 << ((uint32_t)p->r[GPTimer::VALUE(nr)]) << " reload=" << v::uint64 << reload << v::endl;
  }
}

void GPCounter::value_write() {
    lastvalue = p->r[GPTimer::VALUE(nr)];
    lasttime = sc_core::sc_time_stamp();
    //v::info << name() << " value_write: lastvalue=" << v::uint32 << lastvalue << " lasttime=" << lasttime << v::endl;
    if (!stopped) {
        calculate();
    }
}

void GPCounter::chaining() {
    chain_run = true;
    v::debug << name() << "Chaining" << nr << v::endl;
    start();
}

void GPCounter::ticking() {
    unsigned int irqnr = 0;
    while (1) {
        // calculate sleep time, GPCounter timeout
        calculate();

        //v::info << name() << "GPCounter" << nr << " counting" << v::endl;
        wait(e_wait);

        // update performance counter
        m_underflows = m_underflows + 1;

        //v::info << name() << "GPCounter" << nr << " underflow" << v::endl;
        // Send interupt and set outputs
        if (p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_IE)) {
            // APBIRQ addresse beachten -.-
            irqnr = (p->r[GPTimer::CONF] >> 3) & 0x1F;
            value_read();
            //srInfo()
            //  ("IRQ", irqnr)
            //  ("CONF", (uint32_t)p->r[GPTimer::CONF])
            //  ("SCALER", (uint32_t)p->r[GPTimer::SCALER])
            //  ("SCRELOAD", (uint32_t)p->r[GPTimer::SCRELOAD])
            //  ("VALUE", (uint32_t)p->r[GPTimer::VALUE(nr)])
            //  ("RELOAD", (uint32_t)p->r[GPTimer::RELOAD(nr)])
            //  ("ticking...");

            if (p->r[GPTimer::CONF].bit(GPTimer::CONF_SI)) {
                irqnr += nr;
            }
            p->irq.write(std::pair<uint32_t, bool>(1 << irqnr, true));
            m_pirq = true;
        }
        if (p->counter.size()-1 == nr && p->g_wdog_length != 0) {
           p->wdog.write(true);
        }

        wait(p->clock_cycle);
        if (m_pirq && irqnr) {
            p->irq.write(std::pair<uint32_t, bool>(1 << irqnr, false));
        }
        if (p->counter.size()-1 == nr && p->g_wdog_length != 0) {
           p->wdog.write(false);
        }

        unsigned int nrn = (nr + 1 < p->counter.size())? nr + 1 : 0;
        if (p->r[GPTimer::CTRL(nrn)].bit(GPTimer::CTRL_CH)) {
            p->counter[nrn]->chaining();
        }

        // Enable value becomes restart value
        p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_EN, 
                p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_RS));

        if (!p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_RS)) {
          stop();
        }
    }
}

void GPCounter::do_reset() {
  lastvalue = 0;
  lasttime = sc_core::sc_time_stamp();
  p->r[GPTimer::VALUE(nr)] = 0;
  p->r[GPTimer::RELOAD(nr)] = 0;
  p->r[GPTimer::CTRL(nr)] = 0;
  stopped = true;
  chain_run = false;
}

// Gets the time to the end of the next zero hit.
sc_core::sc_time GPCounter::nextzero() {
  sc_core::sc_time t;                                   // cycle time of foundation (other GPCounter or the cloccycle)
  int64_t x;                                            // Per cycle
  if (p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_CH)) {  // We depend on the cycle time of the last GPCounter.
    if (nr) {
      p->counter[nr-1]->value_read();
      t = p->counter[nr-1]->cycletime();
      x = p->r[GPTimer::VALUE(nr - 1)];
    } else {
      p->counter[p->counter.size() - 1]->value_read();
      t = p->counter[p->counter.size() - 1]->cycletime();
      x = p->r[GPTimer::VALUE(p->counter.size() - 1)];
    }
  } else {                                           // We only depend on the prescaler
    p->scaler_read();
    t = p->clock_cycle;
    x = p->r[GPTimer::SCALER];
  }
  return t * (x + 2);
}


// Gets the Cycletime of the CGPCounter.
sc_core::sc_time GPCounter::cycletime() {
  sc_core::sc_time t;                                   // cycle time of foundation (other GPCounter or the cloccycle)
  int64_t m;                                            // Per cycle
  if (p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_CH)) {  // We depend on the cycle time of the last GPCounter.
    if (nr) {
        t = p->counter[nr - 1]->cycletime();
        m = p->r[GPTimer::RELOAD(nr - 1)];
    } else {
        t = p->counter[p->counter.size()]->cycletime();
        m = p->r[GPTimer::RELOAD(p->counter.size())];
    }
  } else {                                           // We only depend on the prescaler
    t = p->clock_cycle;
    m = p->r[GPTimer::SCRELOAD];
  }
  //v::debug << name() << "calc cycletime: clock_cycle: " << t << " SCRELOAD: " << m << " result: " << t*(m+1) << v::endl;
  return t * (m + 1);
}

// Recalculate sleeptime and send notification
void GPCounter::calculate() {
    e_wait.cancel();
    value_read();
    uint32_t value = p->r[GPTimer::VALUE(nr)];
    sc_core::sc_time time;
    // Calculate with currentime, and lastvalue updates
    if (p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_EN)) {
        sc_core::sc_time zero = this->nextzero();
        sc_core::sc_time cycle = this->cycletime();
        //v::info << name() << " calculate: " << nr
        //         << ": zero=" << zero << " cycle=" << cycle
        //         << " value=" << v::uint32 << value << v::endl;
        time = zero;
        //time += (cycle * value)+1 * p->clock_cycle;
        time += (cycle * value) + (nr + 1) * p->clock_cycle;
        //v::info << name() << " calculate: " << nr << ": time=" << time << v::endl;
        e_wait.notify(time);
    }
            //v::info << name() << "calculate" << 
            //  " CONF "<< hex << (uint32_t)p->r[GPTimer::CONF] << 
            //  " SCALER "<< hex << (uint32_t)p->r[GPTimer::SCALER] <<
            //  " SCRELOAD "<< hex << (uint32_t)p->r[GPTimer::SCRELOAD] <<
            //  " VALUE "<< hex << (uint32_t)p->r[GPTimer::VALUE(nr)] <<
            //  " CTRL "<< hex << (uint32_t)p->r[GPTimer::CTRL(nr)] <<
            //  " RELOAD "<< hex << (uint32_t)p->r[GPTimer::RELOAD(nr)] << v::endl;
}

// Start counting imideately.
// For example for enable, !dhalt, e_chain
void GPCounter::start() {
    v::debug << name() << "start: " << nr << ": stopped=" << stopped << "-"
            << static_cast<bool>(p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_EN)) << "-"
            << "-"
            << (!p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_CH)
                    || (p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_CH)
                            && chain_run)) << v::endl;
    if (stopped && p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_EN)
                    // && (p->dhalt.read()!=0)
                    && (!p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_CH)
                    || (p->r[GPTimer::CTRL(nr)].bit(GPTimer::CTRL_CH)
                            && chain_run))) {
        v::debug << name() << "startnow_" << nr << v::endl;

        lasttime = sc_core::sc_time_stamp();
        calculate();
        stopped = false;
    }
}

// Stoping counting imideately.
// For example for disableing, dhalt, e_chain
void GPCounter::stop() {
    if (!stopped) {
        v::debug << name() << "stop: " << nr << v::endl;
        e_wait.cancel();
        value_read();
        lastvalue = p->r[GPTimer::VALUE(nr)];
        lasttime = sc_core::sc_time_stamp();
        stopped = true;
    }
}

/// @}
