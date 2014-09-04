// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup signalkit
/// @{
/// @file inout.h
/// 
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef SIGNALKIT_INOUT_H
#define SIGNALKIT_INOUT_H

#include <stdint.h>
#include "core/common/signalkit_h/out.h"
#include "core/common/signalkit_h/in.h"

namespace signalkit {

/// Signalkit input output signal.
/// This class implements a TLM Signal abstraction of an bidirectional signal.
/// The signal stores the value and can execute a callback.
template<class TYPE, class MODULE>
class signal_inout : public signal_out<TYPE, MODULE> , public signal_in<TYPE,
        MODULE> {
    public:
        /// Inout signal callback type.
        /// The callback has two parameter.
        ///
        /// @param value The new value of the signal.
        /// @param time  The delay from sc_timestamp() of the propagation.
        typedef void(MODULE::*t_callback)(const TYPE &value,
                                          const sc_core::sc_time &time);

        /// Constructor without callback.
        ///
        /// @param mn Signal name.
        signal_inout(sc_core::sc_module_name mn = NULL) :
            signal_out<TYPE, MODULE>::signal_out(mn),
                    signal_in<TYPE, MODULE>::signal_in(mn) {
        }

        /// Constructor with callback.
        ///
        /// @param callback A base class function which gets executed every time the value is chaned.
        /// @param mn       Signal name.
        signal_inout(t_callback callback, sc_core::sc_module_name mn = NULL) :
            signal_out<TYPE, MODULE>::signal(mn),
                    signal_in<TYPE, MODULE>::signal_in(callback, mn) {
        }

        /// Virtual destructor.
        virtual ~signal_inout() {
        }
};

}  // signalkit

#endif  // SIGNALKIT_INOUT_H
/// @}
