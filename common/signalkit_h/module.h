// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup signalkit
/// @{
/// @file module.h
/// This file implements a base class for all tlm module which need to use
/// signalkit signals. It provides all neccessary classes to work with signals.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef SIGNALKIT_MODULE_H
#define SIGNALKIT_MODULE_H

#include "signalkit/signalkit_h/in.h"
#include "signalkit/signalkit_h/out.h"
#include "signalkit/signalkit_h/inout.h"
#include "signalkit/signalkit_h/selector.h"
#include "signalkit/signalkit_h/infield.h"

namespace signalkit {

/// Signalkit module base class.
/// Derive from this class to use the signalkit.
template<class MODULE>
class signal_module {
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

#define SK_HAS_SIGNALS(name) \
        template<class TYPE> \
        struct signal { \
                typedef signalkit::signal_in<TYPE, name> in; \
                typedef signalkit::signal_out<TYPE, name> out; \
                typedef signalkit::signal_inout<TYPE, name> inout; \
                typedef signalkit::signal_selector<TYPE, name> selector; \
                typedef signalkit::signal_infield<TYPE, name> infield; \
        };

#define SIGNALMODULE(name) \
  SK_HAS_SIGNALS(name)

}  // namespace signalkit

#endif  // SIGNALKIT_MODULE_H
/// @}
