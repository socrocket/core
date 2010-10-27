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
// Title:      signalkit_h/in.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef TLM_SIGNAL_IN_H
#define TLM_SIGNAL_IN_H

#include "signalkit_h/base.h"
#include "signalkit_h/ifs.h"

namespace signalkit {

/// @addtogroup
/// @{

/// Signalkit input signal.
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
                          const unsigned int &channel = 0) {
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

/// @}

} // signalkit

#endif // TLM_SIGNAL_IN_H
