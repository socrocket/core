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
// Title:      signalkit_h/ifs.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    This file contains abstract interfaces to provide tlm 
//             signal behaviour in the signal kit.
//             The archivement of the interfaces is to deal with abstract 
//             template types as payload.
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

#ifndef SIGNALKIT_IFS_H
#define SIGNALKIT_IFS_H

namespace signalkit {

/// @addtogroup signalkit
/// @{

/// Main signal interface.
/// This interface implements the basic read functionality of a signal value
/// This is needed for input and output signals.
template<class TYPE>
class signal_if {
    public:
        /// Virtual destructor
        virtual ~signal_if() {
        }

        /// Read the current signal value.
        virtual const TYPE &read() {
            return m_value;
        }

        /// Call operator to get the current signal value
        operator TYPE() const {
            return m_value;
        }

        /// Comparing the current signal value with a variable of the same type TYPE.
        bool operator==(const TYPE &t) const {
            return (m_value == t);
        }

    protected:
        /// The signal value.
        TYPE m_value;
};

// Signal input interface.
// Forward declaration.
template<class TYPE>
class signal_in_if;

/// Signal output bind interface.
/// This interface implements the bind functionality of an output signal.
/// It's seperated from the signal output interface due to a second need in
/// the selector signal. The selector has to implement the same bind interface
/// but only a subclass implements the output interface for each channel.
/// @see selector
template<class TYPE>
class signal_out_bind_if {
    public:
        /// Virtual destructor
        virtual ~signal_out_bind_if() {
        }

        /// Abstract bind method interface.
        /// This method has to be implemented by each output signal to make it 
        /// bind with an input.
        ///
        /// @param t       Input interface to bind with.
        /// @param channel The channel which has to be bind.
        virtual void bind(signal_in_if<TYPE> &t, const unsigned int &channel = 0) = 0;
};

/// Signal output interface
/// This interface provides abstract function declarations to write on a signal.
template<class TYPE>
class signal_out_if : virtual public signal_if<TYPE> ,
                      public signal_out_bind_if<TYPE> {
    public:
        /// Virtual destructor
        virtual ~signal_out_if() {
        }

        /// Write method.
        /// This abstract method has to be implemented by each output signal.
        /// It handles the write behaviour of the signal.
        /// Therefore it will propagate the written value to the other ends 
        /// of the signal (input signals).
        ///
        /// @param value The new value of the signal. Which is to set.
        /// @param time  Delay from sc_timestamp() after which the signal is set.
        virtual void write(const TYPE &value, const sc_core::sc_time &time =
                sc_core::SC_ZERO_TIME) = 0;

        /// Set operator.
        /// The value of the signal is changed to a new value.
        /// @param t The new value.
        /// @return The new value.
        virtual TYPE operator=(const TYPE &t) {
            write(t);
            return this->m_value;
        }

        /// Set operator.
        /// Thets the signal to a value from another signal.
        /// @param t Signal to read the value from.
        /// @return The new value.
        virtual TYPE operator=(const signal_if<TYPE> &t);
};

/// Signal input interface.
/// This interface implements the bind functionality of an input signal and
/// it provides an abstract function declarations to update the signal.
/// The signal has to be updated by an output signal.
/// If the write function of an output signal is called all connected input signals
/// shall call their update functions.
template<class TYPE>
class signal_in_if : virtual public signal_if<TYPE> {
    public:
        /// Virtual destructor
        virtual ~signal_in_if() {
        }

        /// Abstract bind method interface.
        /// This method has to be implemented by each input signal to make it 
        /// bind with an output.
        ///
        /// @param t Input interface to bind with.
        /// @param channel The channel which has to be bind.
        virtual void bind(signal_out_bind_if<TYPE> &t,
                          const unsigned int &channel = 0) = 0;

        /// Abstract update method interface.
        /// This method has to be implemented by each input signal to update the
        /// the signal value and call all callbacks.
        ///
        /// @param sender Which calls update.
        /// @param time   Delay from sc_timestamp() after which the signal is set.
        //                To delay further actions.
        virtual void update(signal_out_if<TYPE> *sender,
                            const sc_core::sc_time &time =
                                    sc_core::SC_ZERO_TIME) = 0;

};

// Output interface set operator.
template<class TYPE>
TYPE signal_out_if<TYPE>::operator=(const signal_if<TYPE> &t) {
    TYPE value = t;
    write(value);
    return this->m_value;
}

/// @}

} // signalkit

#endif // SIGNALKIT_IFS_H
