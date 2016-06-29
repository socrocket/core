// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_signal
/// @{
/// @file module.h
/// @date 2010-2015
/// @author Rolf Meyer
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// This file implements a base class for all tlm module which need to use
/// sr_signal signals. It provides all neccessary classes to work with signals.
#ifndef SR_SIGNAL_MODULE_H
#define SR_SIGNAL_MODULE_H

#include "sr_signal_in.h"
#include "sr_signal_out.h"
#include "sr_signal_inout.h"
#include "sr_signal_selector.h"
#include "sr_signal_infield.h"

namespace sr_signal {

/// sc_signal module base class.
/// Derive from this class to use the sr_signal.
template<class MODULE>
class sr_signal_module {
    protected:
        /// Defines all signal types without the need to tell each of it the type of the base class
        template<class TYPE>
        struct signal {
                /// Input signal
                typedef signal_in<TYPE, MODULE> in;

                /// Output signal
                typedef signal_out<TYPE, MODULE> out;

                /// Inout signal
                typedef signal_inout<TYPE, MODULE> inout;

                /// Selector signal
                typedef signal_selector<TYPE, MODULE> selector;

                /// Infield signal
                typedef signal_infield<TYPE, MODULE> infield;
        };
};

#define SR_HAS_SIGNALS(name) \
        template<class TYPE> \
        struct signal { \
                typedef sr_signal::signal_in<TYPE, name> in; \
                typedef sr_signal::signal_out<TYPE, name> out; \
                typedef sr_signal::signal_inout<TYPE, name> inout; \
                typedef sr_signal::signal_selector<TYPE, name> selector; \
                typedef sr_signal::signal_infield<TYPE, name> infield; \
        };

#define SIGNALMODULE(name) \
  SR_HAS_SIGNALS(name)

}  // namespace sr_signal

#endif  // SR_SIGNAL_MODULE_H
/// @}
