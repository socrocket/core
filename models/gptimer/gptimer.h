/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       gptimer.h                                               */
/*             header file containing the definition of the gptimer    */
/*             model. Due to the fact that the gptimer class is a      */
/*             template class it includes its implementation from      */
/*             gptimer.tpp                                             */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef TIMER_H
#define TIMER_H

#include <boost/config.hpp>
#include <systemc>
#include <greenreg.h>
#include <greenreg_ambasocket.h>

#include "greencontrol/all.h"
#include "multisignalhandler.h"

#include "gptimersignals.h"
#include "gptimerregisters.h"

#include <string>
#include <ostream>
#include <vector>

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

    void start();
    void stop();

};

template <int gpindex, int gpaddr, int gpmask, int gpirq, int gsepirq, int gsbits, int gnbits, int gwdog>
class Timer : public gs::reg::gr_device, public MultiSignalSender, public MultiSignalTarget<Timer<gpindex, gpaddr, gpmask, gpirq, gsepirq, gsbits, gnbits, gwdog> > {
  public:
    typedef gs::socket::config<gs_generic_signal_protocol_types> target_config;
    /* Slave socket with delayed switch*/
    gs::reg::greenreg_socket< gs::amba::amba_slave<32> > bus; 
    
    gs_generic_signal::target_signal_multi_socket<
      Timer<gpindex, gpaddr, gpmask, gpirq, gsepirq, gsbits, gnbits, gwdog> > in;
    gs_generic_signal::initiator_signal_multi_socket                          out;
    
    unsigned int conf_defaults;

    sc_core::sc_time lasttime;
    unsigned int  lastvalue;

    unsigned int ticks;
    
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
    void tick_calc();
    void ticking();

    void scaler_read();
    void scaler_write();
    void screload_write();
    void conf_read();

    void do_reset(unsigned int &id, gs_generic_signal_payload& trans, sc_core::sc_time& delay);
    void do_dhalt(unsigned int &id, gs_generic_signal_payload& trans, sc_core::sc_time& delay);

    void diag();

    void clk(sc_core::sc_clock &clk);
    void clk(sc_core::sc_time &period);
    void clk(double period, sc_core::sc_time_unit base);

    inline int valueof(sc_core::sc_time t, int offset, sc_core::sc_time cycletime) const;
    inline int numberofticksbetween(sc_core::sc_time a, sc_core::sc_time b, int counter, sc_core::sc_time cycletime);
    
    inline unsigned char get_dhalt() {
      return in.get_last_value(dhalt::ID);
    }
    
    inline unsigned char get_rst() {
      return in.get_last_value(rst::ID);
    }
};


#include "gptimer.tpp"

#endif // TIMER_H
