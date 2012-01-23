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
// Title:      gpcounter.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file containing the definition of the gptimer
//             model. Due to the fact that the gptimer class is a
//             template class it includes its implementation from
//             gptimer.tpp
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef GPCOUNTER_H
#define GPCOUNTER_H

#include <greenreg.h>
#include <systemc>
#include <boost/config.hpp>

#include "greencontrol/all.h"
#include "signalkit.h"

#include <string>
#include <ostream>
#include <vector>

/// @addtogroup gptimer GPTimer
/// @{
class GPTimer;

/// @brief This class implements an internal counter of a gptimer.
class GPCounter : public gs::reg::gr_subdevice {
    public:

        /// A pointer to the parent GPTimer. This is needed to acces common functions and register.
        GPTimer &p;

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
        SC_HAS_PROCESS( GPCounter);

        /// Creates a new Counter instance. Only usfull if its called from inside of a GPTimer instance.
        GPCounter(GPTimer &_parent, unsigned int nr,
                   sc_core::sc_module_name name);

        /// Destroies a Counter instance
        ~GPCounter();

        /// Execute the callback registering when systemc reaches the end of elaboration.
        void end_of_elaboration();

        /// Execute the callback registering when systemc reaches the end of simulation.
        void end_of_simulation();

      // Register Callbacks
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

      // Functions
        /// Performs the reset code for the Counter. This function is executed by the Timer::do_reset() function
        void do_reset();

        /// This function prepares the Counter for chaining.
        void chaining();

        sc_core::sc_time nextzero();
        sc_core::sc_time cycletime();

        /// Function calculating the waiting time for the next timer event.
        void calculate();

        /// Start a Counter from dhalt or to enable it etc.
        void start();
        
        /// Stop a Counter from dhalt or to enable it etc.
        void stop();
        
      // Threads
        /// This function contains the core functionality of the Counter.
        /// It is a SC_THREAD which triggers the interupt and waits for the e_tick event.
        void ticking();
    private:
        uint64_t m_underflows;


};

/// @}
//
#endif // GPCOUNTER_H
