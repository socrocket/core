// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_signal
/// @{
/// @file ifs.h
/// This file contains abstract interfaces to provide tlm signal behaviour in
/// the signal kit. The archivement of the interfaces is to deal with abstract
/// template types as payload.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef SR_SIGNAL_IFS_H
#define SR_SIGNAL_IFS_H

#include <stdint.h>
#include <typeinfo>

namespace sr_signal {

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

/// Abstract bind interface
/// The interface allows to check the type of the template classes without
/// knowwing the instacnes (tamplete parameters)
class signal_bind_if {
    public:
        /// Describes weather the socket is initiator (true) or receiver (false).
        virtual bool signal_direction() = 0;
        
        /// String to compare member type
        virtual const char *signal_type() = 0;

        virtual bool signal_bind(signal_bind_if *, int = 0, int = 0) = 0;
        
        virtual ~signal_bind_if() {}
};

/// Signal output bind interface.
/// This interface implements the bind functionality of an output signal.
/// It's seperated from the signal output interface due to a second need in
/// the selector signal. The selector has to implement the same bind interface
/// but only a subclass implements the output interface for each channel.
/// @see selector
template<class TYPE>
class signal_out_bind_if : virtual public signal_bind_if {
    public:
        /// Virtual destructor
        virtual ~signal_out_bind_if() {
        }

        /// Define that we are an outgoing interface
        virtual bool signal_direction() {
            return true;
        }

        /// Return type as a string
        virtual const char *signal_type() {
            return typeid(TYPE).name();
        }

        /// Uncertian bind function.
        /// If type and direction is a macht we will bind
        /// To bind we use the teplate specific methods.
        /// Therefore it will be bound on the in interface not here.
        virtual bool signal_bind(signal_bind_if *other, int outchannel = 0, int inchannel = 0) {
            return other->signal_bind(this, inchannel, outchannel);
        }

        /// Abstract bind method interface.
        /// This method has to be implemented by each output signal to make it
        /// bind with an input.
        ///
        /// @param t       Input interface to bind with.
        /// @param channel The channel which has to be bind.
        virtual signal_out_bind_if<TYPE> *bind(signal_in_if<TYPE> &t, const unsigned int &channel = 0) = 0;


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
class signal_in_if : virtual public signal_if<TYPE>, virtual public signal_bind_if {
    public:
        /// Virtual destructor
        virtual ~signal_in_if() {
        }

        /// Define that we are an outgoing interface
        virtual bool signal_direction() {
            return false;
        }

        /// Return type as a string
        virtual const char *signal_type() {
            return typeid(TYPE).name();
        }

        /// Uncertian bind function.
        /// If type and direction is a macht we will bind
        /// To bind we use the teplate specific methods.
        virtual bool signal_bind(signal_bind_if *other, int inchannel = 0, int outchannel = 0) {
            if(other->signal_direction() && (std::strcmp(other->signal_type(), this->signal_type()) == 0)) {
                signal_out_bind_if<TYPE> *out = dynamic_cast<signal_out_bind_if<TYPE> *>(other);
                if(out) {
                    this->bind(*out->bind(*this, outchannel), inchannel);
                }
            }
            return false;
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

} // sr_signal

#ifdef EN_HASH
#include <ext/hash_map>

namespace __gnu_cxx {
template<>
struct hash<sr_signal::signal_out_if<int> *> {
  size_t operator()(sr_signal::signal_out_if<int> *__x) const {
    return (size_t)(__x);
  }
};
} // __gnu_cxx
#endif

#endif // SR_SIGNAL_IFS_H
/// @}
