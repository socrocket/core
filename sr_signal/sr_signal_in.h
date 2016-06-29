// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_signal
/// @{
/// @file in.h
/// @date 2010-2015
/// @author Rolf Meyer
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
#ifndef TLM_SIGNAL_IN_H
#define TLM_SIGNAL_IN_H

#include <stdint.h>
#include "sr_signal_base.h"
#include "sr_signal_ifs.h"

namespace sr_signal {

/// sr_signal input signal.
/// This class implements a TLM Signal abstraction of an incomming signal.
/// The signal stores the value and can execute a callback.
template<class TYPE, class MODULE>
class signal_in : public signal_base<TYPE, MODULE> , public signal_in_if<TYPE> {
    public:
        /// Input signal callback type.
        /// The callback has two parameter.
        ///
        /// @param value The new value of the signal.
        /// @param time  The delay from sc_timestamp() of the propagation.
        typedef void(MODULE::*t_callback)(const TYPE &value,
                                          const sc_core::sc_time &time);

        /// Constructor without callback.
        ///
        /// @param mn Signal name.
        signal_in(sc_core::sc_module_name mn = NULL) :
            signal_base<TYPE, MODULE>::signal_base(mn), m_callback(NULL) {
        }

        /// Constructor with callback.
        ///
        /// @param callback A base class function which gets executed every time the value is chaned.
        /// @param mn       Signal name.
        signal_in(t_callback callback, sc_core::sc_module_name mn = NULL) :
            signal_base<TYPE, MODULE>::signal_base(mn), m_callback(callback) {
        }

        /// Virtual destructor.
        virtual ~signal_in() {
        }

        /// Input signal bind method.
        /// This method implements the binding mechanism with an output signal.
        ///
        /// @param sender Output interface to bind with.
        /// @param channel The channel which has to be bind.
        virtual void bind(signal_out_bind_if<TYPE> &sender,
                          const uint32_t &channel = 0) {
            // No work to do.
        }

        /// This method implements the update meachnism.
        /// It sets the new value and calls the callback if needed.
        ///
        /// @param sender Which calls update.
        /// @param time   Delay from sc_timestamp() after which the signal is set.
        //                To delay further actions.
        virtual void update(signal_out_if<TYPE> *sender,
                            const sc_core::sc_time &time) {
            this->m_value = sender->read();
            if (m_callback) {
                MODULE *mod = this->get_module();
                if (mod) {
                    (mod->*m_callback)(this->m_value, time);
                }
            }
        }

        /// Gets the current signal value.
        virtual operator TYPE() const {
            return this->m_value;
        }

        /// Bind the input signal to an output signal.
        /// Calls both bind methods.
        ///
        /// @param sender The output signal.
        virtual void operator ()(signal_out_bind_if<TYPE> &sender) {
            this->bind(sender);
            sender.bind(*this);
        }

    private:
        /// Stores the callback.
        t_callback m_callback;
};

}  // namespace sr_signal

#endif  // TLM_SIGNAL_IN_H
/// @}
