// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_signal
/// @{
/// @file inout.h
/// @date 2010-2015
/// @author Rolf Meyer
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
#ifndef SR_SIGNAL_INOUT_H
#define SR_SIGNAL_INOUT_H

#include <stdint.h>
#include "sr_signal_out.h"
#include "sr_signal_in.h"

namespace sr_signal {

/// sr_signal input output signal.
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

}  // sr_signal

#endif  // SR_SIGNAL_INOUT_H
/// @}
