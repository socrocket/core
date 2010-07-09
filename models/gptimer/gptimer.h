#ifndef TIMER_H
#define TIMER_H

#include <boost/config.hpp>
#include <systemc>
#include <greenreg.h>
#include <greenreg_ambasocket.h>

#include "greencontrol/all.h"

#include <string>
#include <ostream>
#include <vector>

struct gptimer_in_type {
    bool dhalt;
    bool extclk;
};

inline std::ostream& operator<<(std::ostream& os, const gptimer_in_type& a) {
  os << "( dhalt: " << std::boolalpha << a.dhalt << ", extclk: " << std::boolalpha << a.extclk << ")";
   return os;
}
inline void sc_trace(sc_core::sc_trace_file *file, const gptimer_in_type &a, const std::string &name) {
  sc_trace(file, a.dhalt, name + ".dhalt");
  sc_trace(file, a.extclk, name + ".extclk");
}
inline int operator== (const gptimer_in_type& left, const gptimer_in_type& right) {
  return left.dhalt == right.dhalt && left.extclk == right.extclk;
}

struct gptimer_out_type {
    sc_dt::sc_uint<8> tick;
    sc_dt::sc_uint<32> timer1;
    bool wdogn;
    bool wdog;
};

inline std::ostream& operator<<(std::ostream& os, const gptimer_out_type& a) { 
  os << "( tick: " << std::hex << a.tick << ", timer1: " << std::hex << a.timer1 
     << ", wdogn: " << std::boolalpha << a.wdogn 
     << ", wdog: " << std::boolalpha << a.wdog << ")";
  return os;
}

inline void sc_trace(sc_core::sc_trace_file *file, const gptimer_out_type &a, const std::string &name) {
  sc_trace(file, a.tick, name + ".tick");
  sc_trace(file, a.timer1, name + ".timer1");
  sc_trace(file, a.wdogn, name + ".wdogn");
  sc_trace(file, a.wdog, name + ".wdog");
}

inline int operator== (const gptimer_out_type& left, const gptimer_out_type& right) { return 0; }

template <int gpindex = 0, int gpaddr = 0, int gpmask = 4095, int gpirq = 0, int gsepirq = 0, int gsbits = 16, int gnbits = 32, int gwdog = 0>
class Timer;

template <int gpindex = 0, int gpaddr = 0, int gpmask = 4095, int gpirq = 0, int gsepirq = 0, int gsbits = 16, int gnbits = 32, int gwdog = 0>
class Counter : public gs::reg::gr_subdevice {
  public:
    Timer<gpindex, gpaddr, gpmask, gpirq, gsepirq, gsbits, gnbits, gwdog> &p;
    //Timer &p;
    bool m_pirq;

    /* Times to calculate value and wait times. */
    sc_core::sc_time lasttime;
    unsigned int lastvalue;
    sc_core::sc_time zerofactor;
    sc_core::sc_event e_wait;
    unsigned int nr;
    
    bool stopped;
    bool chain_run;

    GC_HAS_CALLBACKS();
    SC_HAS_PROCESS(Counter);
	
    Counter(Timer<gpindex, gpaddr, gpmask, gpirq, gsepirq, gsbits, gnbits, gwdog> &_parent, unsigned int nr, sc_core::sc_module_name name);
    ~Counter();
  
    void end_of_elaboration();
    void do_reset();

    void ctrl_read();
    void ctrl_write();
    void value_read();
    void value_write();
    void chaining();
    void ticking();
    sc_core::sc_time nextzero();
    sc_core::sc_time cycletime();
    void calculate();
    void dhalt();

    void start();
    void stop();
    sc_core::sc_signal<sc_dt::sc_uint<32> > pirq;
};

template <int gpindex, int gpaddr, int gpmask, int gpirq, int gsepirq, int gsbits, int gnbits, int gwdog>
class Timer : public gs::reg::gr_device {
  public:
    /* Slave socket with delayed switch*/
    gs::reg::greenreg_socket< gs::amba::amba_slave<32> > bus; 
    sc_core::sc_in<bool>                 reset;
    
    sc_core::sc_in<gptimer_in_type>      gpti;
    sc_core::sc_out<gptimer_out_type>    gpto;

    sc_core::sc_in<sc_dt::sc_uint<32> >  pirqi;
    sc_core::sc_out<sc_dt::sc_uint<32> > pirqo;
    sc_core::sc_out<sc_dt::sc_uint<32> > pconfig_0;
    sc_core::sc_out<sc_dt::sc_uint<32> > pconfig_1;
    sc_core::sc_out<sc_dt::sc_uint<16> > pindex;

    unsigned int conf_defaults;

    sc_core::sc_time lasttime;
    unsigned int  lastvalue;
    
    sc_core::sc_time clockcycle;
    sc_core::sc_event e_tick;
    
    typedef Counter<gpindex, gpaddr, gpmask, gpirq, gsepirq, gsbits, gnbits, gwdog> counter_type;
    std::vector<counter_type *> counter;
    //std::vector<Counter *> counter;

    GC_HAS_CALLBACKS();
    SC_HAS_PROCESS(Timer);

    /**
     * @param: pirq    Defines which APB interupt the timers will generate. Default is 0.
     * @param: sepirq  If set to 1, each timer will drive an individual interrupt line, 
     *                 starting with interrupt irq. If set to 0, all timers will drive 
     *                 the same interrupt line (irq).
     * @param: ntimers Defines the number of timers in the unit. Default is 1. Max is 7.
     * @param: nbits   Defines the number of bits in the timers. Default is 32.
     * @param: sbits   Defines the number of bits in the scaler. Default is 16.
     * @param: wdog    Watchdog reset value. When set to a non-zero value, the
     *                 last timer will be enabled and pre-loaded with this value
     *                 at reset. When the timer value reaches 0, the WDOG output
     *                 is driven active.
     */
    Timer(sc_core::sc_module_name name, unsigned int ntimers = 1);
    ~Timer();
    
    void end_of_elaboration();
    void ticking();
    void irqjoin();
    void tickjoin();
    void do_dhalt();
    void scaler_read();
    void scaler_write();
    void screload_write();
    void conf_read();
    void do_reset();
    void tick_calc();
    void diag();

    void clk(sc_core::sc_clock &clk);
    void clk(sc_core::sc_time &period);
    void clk(double period, sc_core::sc_time_unit base);

    int valueof(sc_core::sc_time t, int offset, sc_core::sc_time cycletime);
    int numberofticksbetween(sc_core::sc_time a, sc_core::sc_time b, int counter, sc_core::sc_time cycletime);
#ifdef DEBUGOUT
    sc_core::sc_signal<bool> tick[8];
#endif
};

#include "gptimer.tpp"

#endif // TIMER_H
