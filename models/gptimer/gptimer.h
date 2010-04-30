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

inline std::ostream& operator<<(std::ostream& os, const gptimer_in_type& a) { return os; }
inline void sc_trace(sc_core::sc_trace_file *, const gptimer_in_type&, const std::string &) {}
inline int operator== (const gptimer_in_type& left, const gptimer_in_type& right) { return 0; }

struct gptimer_out_type {
    sc_dt::sc_uint<8> tick;
    sc_dt::sc_uint<32> timer1;
    bool wdogn;
    bool wdog;
};

inline std::ostream& operator<<(std::ostream& os, const gptimer_out_type& a) { return os; }
inline void sc_trace(sc_core::sc_trace_file *, const gptimer_out_type&, const std::string &) {}
inline int operator== (const gptimer_out_type& left, const gptimer_out_type& right) { return 0; }

//template <int pindex = 0, int paddr = 0, int pmask = 4095, int pirq = 0, int sepirq = 0, int sbits = 16, int nbits = 32, int wdog = 0>
class Timer;

//template <int pindex = 0, int paddr = 0, int pmask = 4095, int pirq = 0, int sepirq = 0, int sbits = 16, int nbits = 32, int wdog = 0>
class Counter : public gs::reg::gr_subdevice {
  public:
    //Timer<pindex, paddr, pmask, pirq, sepirq, sbits, nbits, wdog> &p;
    Timer &p;
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
	
    Counter(Timer &_parent, unsigned int nr, sc_core::sc_module_name name, unsigned int nbits = 32);
    ~Counter();
  
    void end_of_elaboration();
    void do_reset();

    void ctrl_read();
    void ctrl_write();
    void value_read();
    void value_write();
    void chaining();
    void ticking();
    void calculate();
    void dhalt();

    void start();
    void stop();
    sc_core::sc_signal<sc_dt::sc_uint<32> > pirq;
};

//template <int pindex = 0, int paddr = 0, int pmask = 4095, int pirq = 0, int sepirq = 0, int sbits = 16, int nbits = 32, int wdog = 0>
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
    
    //std::vector<Counter<pindex, paddr, pmask, pirq, sepirq, sbits, nbits, wdog> *> counter;
    std::vector<Counter *> counter;

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
    Timer(sc_core::sc_module_name name, unsigned int pirq = 0, bool sepirq = false, unsigned int ntimers = 1, unsigned int nbits = 32, unsigned int sbits = 16, unsigned int wdog = 0);
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
    void clk(double &period, sc_core::sc_time_unit &base);

    int valueof(sc_core::sc_time t, int offset = 0);
    int numberofticksbetween(sc_core::sc_time a, sc_core::sc_time b, int counter);
    sc_core::sc_signal<bool> tick[8];
    sc_core::sc_trace_file *wave;
};

#endif // TIMER_H
