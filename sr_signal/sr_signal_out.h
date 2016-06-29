// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_signal
/// @{
/// @file out.h
/// @date 2010-2015
/// @author Rolf Meyer
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
#ifndef SR_SIGNAL_OUT_H
#define SR_SIGNAL_OUT_H

#include "sr_signal_base.h"
#include "sr_signal_ifs.h"
#include <stdint.h>

namespace sr_signal {

/// sc_signal output signal.
/// This class implements a TLM Signal abstraction of an outgoing signal.
/// The signal stores the value and triggers the update function of all receivers.
template<class TYPE, class MODULE>
class signal_out : public signal_base<TYPE, MODULE> ,
                   public signal_out_if<TYPE> {
    protected:
        /// Type of a list of receivers
        typedef std::vector<signal_in_if<TYPE> *> t_receiver;

    public:
        /// Constructor
        ///
        /// @param mn Signal name.
        signal_out(sc_core::sc_module_name mn = NULL) :
            signal_base<TYPE, MODULE>::signal_base(mn) {
        }

        /// Virtual destructor
        virtual ~signal_out() {
        }

        /// Output signal bind method.
        /// This method implements the binding mechanism with an input signal.
        ///
        /// @param receiver Input interface to bind with.
        /// @param channel  The channel which has to be bind.
        virtual signal_out_bind_if<TYPE> *bind(signal_in_if<TYPE> &receiver,
                          const uint32_t &channel = 0) {
            for (typename t_receiver::iterator iter = m_receiver.begin(); iter
                    != m_receiver.end(); iter++) {
                if (*iter == &receiver) {
                    return this;
                }
            }
            m_receiver.push_back(&receiver);
            return this;
        }

        /// Write the value of a signal.
        /// Stores the value and triggers an update in all receivers.
        ///
        /// @param value The new value of the signal.
        /// @param time The delay from sc_timestamp() at propagation.
        virtual void write(const TYPE &value, const sc_core::sc_time &time =
                sc_core::SC_ZERO_TIME) {
            this->m_value = value;
            for (typename t_receiver::iterator i = m_receiver.begin();
                 i != m_receiver.end(); i++) {
                (*i)->update((signal_out_if<TYPE> *)this, time);
            }
        }

        /// Connecting the Signal with an input signal.
        /// Calls caller and receiver bind methods.
        ///
        /// @param receiver The input signal to connect with.
        void operator()(signal_in_if<TYPE> &receiver) {
            this->bind(receiver);
            receiver.bind(*this);
        }

        /// Set the value and call updates on all receivern.
        TYPE operator=(const TYPE &t) {
            write(t);
            return this->m_value;
        }

        /// Sets the value from another signal.
        TYPE operator=(const signal_if<TYPE> &t) {
            TYPE value = t;
            write(value);
            return this->m_value;
        }
    protected:
        /// List of receivers.
        t_receiver m_receiver;
};

}  // namespace sr_signal

#endif  // SR_SIGNAL_OUT_H
/// @}
