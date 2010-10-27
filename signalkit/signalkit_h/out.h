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
// Title:      signalkit_h/out.h
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

#ifndef SIGNALKIT_OUT_H
#define SIGNALKIT_OUT_H

#include "signalkit_h/base.h"
#include "signalkit_h/ifs.h"

namespace signalkit {

/// @addtogroup signalkit
/// @{

/// Signalkit output signal.
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
        virtual void bind(signal_in_if<TYPE> &receiver,
                          const unsigned int &channel = 0) {
            for (typename t_receiver::iterator iter = m_receiver.begin(); iter
                    != m_receiver.end(); iter++) {
                if (*iter == &receiver) {
                    return;
                }
            }
            m_receiver.push_back(&receiver);
        }

        /// Write the value of a signal.
        /// Stores the value and triggers an updat in all receivers.
        ///
        /// @param value The new value of the signal.
        /// @param time The delay from sc_timestamp() at propagation.
        virtual void write(const TYPE &value, const sc_core::sc_time &time =
                sc_core::SC_ZERO_TIME) {
            this->m_value = value;
            for (typename t_receiver::iterator i = m_receiver.begin(); i
                    != m_receiver.end(); i++) {
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

/// @}

} // signalkit

#endif // SIGNALKIT_OUT_H
