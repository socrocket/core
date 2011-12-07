//*********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is 
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      clkdevice.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file defining the clkdevice base class
//             The clkdevice is the base for all devices using an internal clk.
//
// Modified on $Date: 2011-09-06 18:39:06 +0200 (Tue, 06 Sep 2011) $
//          at $Revision: 500 $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef CLKDEVICE_H
#define CLKDEVICE_H

#include <systemc.h>
#include "signalkit.h"

class CLKDevice {
    public:
        SIGNALMODULE(CLKDevice);

        /// Reset input signal
        signal<bool>::in rst;
    
        /// Clock input signal
        signal<sc_time>::in clk;
    
        CLKDevice();
        virtual ~CLKDevice();
       // Signal Callbacks 
        /// Reset Callback
        ///
        ///  This function is called when the reset signal is triggert.
        ///  The reset whill reset all registers and bring the IRQ controler in a valid state.
        ///
        /// @param value Value of the reset signal the reset is active as long the signal is false.
        ///              Therefore the reset is done on the transition from false to true.
        /// @param time  Delay to the current simulation time. Is not used in this callback.
        virtual void onrst(const bool &value, const sc_time &time);

        /// Clock Callback
        ///
        ///  This function is called when ever the clock is chaning.
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
        void set_clk(sc_core::sc_time &period);

        /// Set the clockcycle length.
        ///
        ///  With this function you can set the clockcycle length of the gptimer instance.
        ///  The clockcycle is useed to calculate internal delays and waiting times to trigger the timer core functionality.
        ///
        /// @param period A double wich holds the clockcycle length in a unit stored in base.
        /// @param base   The unit of the clockcycle length stored in period.
        void set_clk(double period, sc_core::sc_time_unit base);


  protected:
    sc_time clock_cycle;

    virtual void dorst() = 0;
    virtual void clkcng() {};
};

#endif // CLKDEVICE_H
