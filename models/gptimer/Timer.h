#ifndef TIMER_H
#define TIMER_H

#include <boost/config.hpp>
#include <systemc>
#include <greenreg.h>
#include <greenreg_socket.h>

#include "greencontrol/all.h"

#include <vector>

#define SHOW(name, msg) \
{ std::printf("@%-5s /%-2d %20s: ", sc_core::sc_time_stamp().to_string().c_str(), (unsigned)sc_core::sc_delta_count(), name); std::cout << msg << std::endl; }

class Timer;

class Counter : public gs::reg::gr_subdevice {
  public:
    Timer &p;
    //sc_core::sc_out<sc_dt::sc_logic > pirq, wdog;
    bool m_pirq;

    /* Events for register changes */
    //gs::reg::gr_event *e_value, *e_reload, *e_ctrl, *e_conf, *e_scaler;

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
};

class Timer : public gs::reg::gr_device {
  public:

    /* Slave socket with delayed switch*/
    gs::reg::greenreg_socket< gs::gp::generic_slave> bus; 
    sc_core::sc_in<bool>   reset;
    sc_core::sc_in<sc_dt::sc_logic>   dhalt;
    sc_core::sc_out<sc_dt::sc_logic>  wdog, wdogn;
    sc_core::sc_out<sc_dt::sc_lv<8> > tick;

    unsigned int conf_defaults;

    sc_core::sc_time lasttime;
    unsigned int  lastvalue;
    
    sc_core::sc_time clockcycle;
    
    std::vector<Counter*> counter;

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
    void do_dhalt();
    void scaler_read(gs::reg::transaction_type* &tr, const sc_core::sc_time& delay);
    void scaler_write(gs::reg::transaction_type* &tr, const sc_core::sc_time& delay);
    void conf_read();
    void do_reset();

    int valueof(sc_core::sc_time t);
    int numberofticksbetween(sc_core::sc_time a, sc_core::sc_time b);
};

#endif // TIMER_H
