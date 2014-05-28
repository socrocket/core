// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file clkdevice.h
/// header file defining the clkdevice base class The clkdevice is the base for
/// all devices using an internal clk.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef CLKDEVICE_H
#define CLKDEVICE_H

#include <systemc.h>
#include "signalkit.h"

class CLKDevice {
    public:
        SK_HAS_SIGNALS(CLKDevice);

        /// Reset input signal
        signal<bool>::in rst;

        /// Clock input signal
        signal<sc_time>::in clk;

        CLKDevice();
        virtual ~CLKDevice();
       // Signal Callbacks
        /// Reset Callback
        ///
        ///  This function is called when the reset signal is triggerd.
        ///  The reset whill reset all registers and bring the IRQ controler in a valid state.
        ///
        /// @param value Value of the reset signal the reset is active as long the signal is false.
        ///              Therefore the reset is done on the transition from false to true.
        /// @param time  Delay to the current simulation time. Is not used in this callback.
        virtual void onrst(const bool &value, const sc_time &time);

        /// Clock Callback
        ///
        ///  This function is called when ever the clock is changing.
        ///  An internal variable called clock_cycle will be set to the exact value..
        ///
        /// @param value Value of the clock.
        /// @param time  Delay to the current simulation time. Is not used in this callback.
        virtual void onclk(const sc_time &value, const sc_time &time);


        /// Set the clockcycle length.
        ///
        ///  With this function you can set the clockcycle length of the gptimer instance.
        ///  The clockcycle is useed to calculate internal delays and waiting times to trigger the timer core functionality.
        ///
        /// @param clk An sc_clk instance. The function will extract the clockcycle length from the instance.
        void set_clk(sc_core::sc_clock &clk);

        /// Set the clockcycle length.
        ///
        ///  With this function you can set the clockcycle length of the gptimer instance.
        ///  The clockcycle is useed to calculate internal delays and waiting times to trigger the timer core functionality.
        ///
        /// @param period An sc_time variable which holds the clockcycle length.
        void set_clk(sc_core::sc_time period);

        /// Set the clockcycle length.
        ///
        ///  With this function you can set the clockcycle length of the gptimer instance.
        ///  The clockcycle is useed to calculate internal delays and waiting times to trigger the timer core functionality.
        ///
        /// @param period A double wich holds the clockcycle length in a unit stored in base.
        /// @param base   The unit of the clockcycle length stored in period.
        void set_clk(double period, sc_core::sc_time_unit base);

        virtual void dorst();
        virtual void clkcng() {};

  protected:
    sc_time clock_cycle;
};

#endif // CLKDEVICE_H
/// @}