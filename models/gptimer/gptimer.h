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

#ifndef GPTIMER_H
#define GPTIMER_H

#include <boost/config.hpp>
#include <systemc>
#include <greenreg.h>
#include <greenreg_ambasocket.h>

#include "greencontrol/all.h"
#include "signalkit.h"

#include "gptimerregisters.h"
#include "grlibdevice.h"

#include <string>
#include <ostream>
#include <vector>

/// @addtogroup gptimer GPTimer
/// @{
class Timer;

/// @brief This class implements an internal counter of a gptimer.
class Counter : public gs::reg::gr_subdevice {
  public:
    
    /// A pointer to the parent GPTimer. This is needed to acces common functions and register.
    Timer &p;
    //Timer &p;

    /// The interrupt state of the counter. Might be get deprecated
    bool m_pirq;

    /// The Time when lastvalue was set.
    /// @see lastvalue
    sc_core::sc_time lasttime;

    /// A defined value to calculate the value of the value register.
    /// It gets set whether a write of the value register happens or stop() gets executet
    /// @see lasttime
    /// @see stop()
    unsigned int lastvalue;

    /// ???
    sc_core::sc_time zerofactor;

    /// The event which implements the corefunctionality.
    /// It gets set by calculate() and ticking() is waiting with it.
    /// @see calculate()
    /// @see ticking()
    sc_core::sc_event e_wait;

    /// The number of the counter. This variable is needed to calculate the right delay slot 
    /// and to find the corresponding registers.
    unsigned int nr;
    
    /// Stores wether a timer is stoped or not.
    bool stopped;

    /// Stores wether a timer is running in chain mode or not.
    bool chain_run;

    GC_HAS_CALLBACKS();
    SC_HAS_PROCESS(Counter);
	
    Counter(Timer &_parent, unsigned int nr, sc_core::sc_module_name name);
    ~Counter();
  
    /// Execute the callback registering when systemc reaches the end of elaboration.
    void end_of_elaboration();

    /// Performs the reset code for the Counter. This function is executed by the Timer::do_reset() function
    void do_reset();

    /// This is a callback which gets executed before the control register is read.
    /// It updates the control register with the current values.
    void ctrl_read();

    /// This is a callback which gets executed after the control register is written.
    /// It applies the changes to the current state of the Counter.
    /// If recalculation of the waiting time is needed it calle calculate().
    void ctrl_write();

    /// This is a callback wich gets executed before the value register is read.
    /// It calculates the current value of the register. lasttime and lastvalue are used as base.
    /// Other functions are using this function to trigger an update of the value register.
    ///
    /// @see lastvalue
    /// @see lasttime
    void value_read();

    /// This function is a callback wich gets executed after the value register is written.
    /// It stores the current time and value into the lasttime and lastvalue attributes and
    /// Triggers a recalculation of the waiting time.
    ///
    /// @see lastvalue
    /// @see lasttime
    /// @see calculate()
    void value_write();

    /// This function prepares the Counter for chaining.
    void chaining();

    /// This function contains the core functionality of the Counter.
    /// It is a SC_THREAD which triggers the interupt and waits for the e_tick event.
    void ticking();

    sc_core::sc_time nextzero();
    sc_core::sc_time cycletime();

    void calculate();

    void start();
    void stop();

};

/// @brief This class is a tlm model of the gaisler aeroflex grlib gptimer.
///   
class Timer
    : public gs::reg::gr_device
    , public signalkit::signal_module<Timer>
    , public GrlibDevice {
  public:
    /// Slave socket with delayed switchi
    gs::reg::greenreg_socket< gs::amba::amba_slave<32> > bus; 
    
    signal<bool>::in      rst;
    signal<bool>::in      dhalt;
    signal<uint8_t>::out  tick;
    signal<uint32_t>::out irq;
    signal<uint32_t>::out wdog;
   
    /// Stores the default config register value 
    /// TODO: Might be deprecated.
    unsigned int conf_defaults;

    /// Stores the time when the value of the prescaler was known.
    sc_core::sc_time lasttime;
    
    /// Stores the last known prescaler value.
    ///
    /// A prescaler value is known when it gets set by the prescaler value register 
    /// or when its get calculatet befor a complete stop.
    ///
    /// It gets always set with the lasttime. Both define the prescaler function 
    /// implemented in valueof.
    unsigned int  lastvalue;

    /// The current state of the tick signal.
    ///
    /// The tick signal is a sum of multible bits. Each is set by a differen Counter or by the prescaler.
    /// Therefore the accumulated state gets stored in this attribute to modify only one bit at a time when a counter changes the signal.
    ///
    unsigned int ticks;
    
    /// The clock cycle length
    ///
    /// To calculate the right delays and waiting times we need to store the length of one clock cycle.
    ///
    sc_core::sc_time clockcycle;

    /// The tick event.
    ///
    /// This event gets set to calculate and produce the tick value for the prescaler underflow.
    /// The value gets calculated either when the prescaler reset or value register change or when the undeflow just happend.
    /// See tick_calc for calculation and ticking for the event wait statement.
    ///
    sc_core::sc_event e_tick;
    
    /// A vector of Counter classes, each representate an internal counter.
    std::vector<Counter *> counter;

    GC_HAS_CALLBACKS(); 
    SC_HAS_PROCESS(Timer);

    /// Creates an instance of an GPTimer.
    ///
    /// @param name    The name of the instance. It's needed for debunging.
    /// @param ntimers Defines the number of timers in the unit. Default is 1. Max is 7.
    /// @param pirq    Defines which APB interupt the timers will generate. Default is 0.
    /// @param sepirq  If set to 1, each timer will drive an individual interrupt line, 
    ///                starting with interrupt irq. If set to 0, all timers will drive 
    ///                the same interrupt line (irq).
    /// @param ntimers Defines the number of timers in the unit. Default is 1. Max is 7.
    /// @param nbits   Defines the number of bits in the timers. Default is 32.
    /// @param sbits   Defines the number of bits in the scaler. Default is 16.
    /// @param wdog    Watchdog reset value. When set to a non-zero value, the
    ///                last timer will be enabled and pre-loaded with this value
    ///                at reset. When the timer value reaches 0, the WDOG output
    ///                is driven active.
    Timer(sc_core::sc_module_name name, unsigned int ntimers = 1, int gpindex = 0, int gpaddr = 0, int gpmask = 4095, int gpirq = 0, int gsepirq = 0, int gsbits = 16, int gnbits = 32, int gwdog = 0);

    /// Free all counter and unregister all callbacks.
    ~Timer();
    
    /// Execute the callback registering when systemc reaches the end of elaboration.
    void end_of_elaboration();

    /// Calculate the time of the next prescaler underflow to create the debunging output for the presacler ticks.
    void tick_calc();

    /// Create the prescaler tick when the prescaler underflow happens.
    void ticking();

    /// Execute before the prescaler value register gets read, to calculate the current value.
    void scaler_read();

    /// Execute after the presacler value is written, to recalculate the counter and ticking functions.
    void scaler_write();

    /// Execute after the presacler reset is written, to recalculate the counter and ticking functions.
    void screload_write();

    /// Execute before the prescaler config register gets read, to calculate the current value.
    void conf_read();

    /// Execute a reset on the timer when the rst signal arrives.
    void do_reset(const bool &value, signalkit::signal_in_if<bool> *signal, signalkit::signal_out_if<bool> *sender, const sc_core::sc_time &time);

    /// Performs the stopping and starting of all counter when dhalt arrives.
    void do_dhalt(const bool &value, signalkit::signal_in_if<bool> *signal, signalkit::signal_out_if<bool> *sender, const sc_core::sc_time &time);

    /// Diagnostic SC_THREAD
    ///
    ///  diag is a diagnostic SC_THREAD it gets triggert once in a clockcycle. But this module does not receive 
    ///  a clock it calculates the clockcycle from the values set by a clk function.
    ///  In this function are debuging commands to read current signals or register values.
    ///
    void diag();

    /// Set the clockcycle length.
    ///
    ///  With this function you can set the clockcycle length of the gptimer instance.
    ///  The clockcycle is useed to calculate internal delays and waiting times to trigger the timer core functionality.
    ///
    /// @param clk An sc_clk instance. The function will extract the clockcycle length from the instance.
    void clk(sc_core::sc_clock &clk);

    /// Set the clockcycle length.
    ///
    ///  With this function you can set the clockcycle length of the gptimer instance.
    ///  The clockcycle is useed to calculate internal delays and waiting times to trigger the timer core functionality.
    ///
    /// @param period An sc_time variable which holds the clockcycle length.
    void clk(sc_core::sc_time &period);

    /// Set the clockcycle length.
    ///
    ///  With this function you can set the clockcycle length of the gptimer instance.
    ///  The clockcycle is useed to calculate internal delays and waiting times to trigger the timer core functionality.
    ///
    /// @param period A double wich holds the clockcycle length in a unit stored in base.
    /// @param base   The unit of the clockcycle length stored in period.
    void clk(double period, sc_core::sc_time_unit base);

    /// The time to value function of the prescaler or the counters.
    ///
    ///  This is the fundamental function which defines the connection between a given time and the value.
    ///  All calculations in the Timer and Counter class are based on this function.
    ///  The function itself is defined through the two attributes lastvalue and lasttime.
    ///
    /// @param t         The Time for which is needed the prescaler value.
    /// @param offset    An offset, defined to calculate a value which is influenced by the counter number.
    ///                  This is needed due to the fact that the rtl model decrements the counters one after one,
    ///                  Each in a clockcycle if the prescaler undeflows.
    /// @param cycletime The cycle length which decrements the value.
    /// @return          The value to time t.
    ///
    /// @see lastvalue
    /// @see lasttime
    /// @see numberofticksbetween()
    ///
    inline int valueof(sc_core::sc_time t, int offset, sc_core::sc_time cycletime) const;

    /// @brief
    ///
    inline int numberofticksbetween(sc_core::sc_time a, sc_core::sc_time b, int counter, sc_core::sc_time cycletime);
};

/// @}

#include "gptimer.tpp"

#endif // GPTIMER_H
