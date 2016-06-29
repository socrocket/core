// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_signal
/// @{
/// @file infield.h
/// This file implements the sr_signal infiels signal
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef SR_SIGNAL_INFIELD_H
#define SR_SIGNAL_INFIELD_H

#include <stdint.h>
#include "sr_signal_base.h"
#include "sr_signal_ifs.h"

#ifdef EN_HASH
#define SR_SIGNAL_MAP__ SR_SIGNAL_MAP__
#include <ext/SR_SIGNAL_MAP__>
namespace std { using namespace __gnu_cxx; }
#else
#define SR_SIGNAL_MAP__ map
#include <map>
#endif

namespace sr_signal {

/// This signal is an input signal which can be connected to multiple output signals.
/// But altered from the input signal the infield has knowlage about the source of the
/// arriving signal.
/// Each output signal can be bound to a infield channel.
/// Each hannel has its own value and the callback method can identiefy the seender.
template<class TYPE, class MODULE>
class signal_infield : public signal_base<TYPE, MODULE> , public signal_in_if<
        TYPE> {
    public:
        /// Callback method type
        /// The callback method must be a function with 3 parameters:
        ///
        /// @param value The new value of the signal.
        /// @param channel The input channel of the signal.
        /// @param time The delay of the signal.
        typedef void(MODULE::*t_callback)(const TYPE &value,
                                          const uint32_t &channel,
                                          const sc_core::sc_time &time);

        /// Constructor without callback
        ///
        /// @param mn SystemC Signal Name
        signal_infield(sc_core::sc_module_name mn = NULL) :
            signal_base<TYPE, MODULE>::signal_base(mn), m_callback(NULL) {
        }

        /// Constructor with callback
        ///
        /// @param callback Callback in the base class to be executed each time a
        ///                 value changes.
        /// @param mn SystemC Signal Name
        signal_infield(t_callback callback, sc_core::sc_module_name mn = NULL) :
            signal_base<TYPE, MODULE>::signal_base(mn), m_callback(callback) {
        }

        /// Virtual destructor
        virtual ~signal_infield() {
        }

        /// Bind method.
        /// This method binds a specific input channel to a corresponding output.
        /// Each channel can be bind multiple times.
        ///
        /// @param sender  Input interface to bind with.
        /// @param channel The channel which has to be bind.
        virtual void bind(signal_out_bind_if<TYPE> &sender,
                          const uint32_t &channel = 0) {
            // TODO: Make it work with multible sender per channel
            TYPE v = TYPE();
            signal_out_if<TYPE> *out =
                    static_cast<signal_out_if<TYPE> *> (&sender);
            if (out) {
                m_channel.insert(std::make_pair(out, channel));
                m_value.insert(std::make_pair(channel, v));
            }
        }

        /// Update method.
        /// Updates the value of a channel and executes the callback.
        ///
        /// @param sender Wich calls update.
        /// @param time  Delay from sc_timestamp() after which the signal is set.
        //               To delay further actions.
        virtual void update(signal_out_if<TYPE> *sender,
                            const sc_core::sc_time &time) {
            // The content of each channel is stored with it's channel number.
            // So first we have to get the channel number from the interface.
            // Then we have to update the value to the channel.
            typename std::SR_SIGNAL_MAP__<signal_out_if<TYPE> *, uint32_t>::iterator
                    channel = this->m_channel.find(sender);
            if (channel == m_channel.end()) {
                return;
            }
            typename std::SR_SIGNAL_MAP__<uint32_t, TYPE>::iterator value =
                    this->m_value.find(channel->second);
            if (value == m_value.end()) {
                return;
            }
            value->second = sender->read();
            if (m_callback) {
                MODULE *mod = this->get_module();
                if (mod) {
                    (mod->*m_callback)(value->second, channel->second, time);
                }
            }
        }

        /// Read the value of a specific channel.
        /// If the channel not exists --> error
        ///
        /// @param channel The channel to read from.
        /// @return        The value of the channel.
        TYPE read(const uint32_t &channel) const {
            typename std::SR_SIGNAL_MAP__<uint32_t, TYPE>::const_iterator item =
                    this->m_value.find(channel);
            if(item == this->m_value.end()) {
               //v::error << this->name() << "No value is register to channel number "
               //         << std::dec << channel << v::endl;
               return TYPE();
            }
            return item->second;
        }

        /// Safe read reads the value of a channel.
        /// If the channel not exists return default value.
        ///
        /// @param channel       The channel to read from.
        /// @param default_value The default value to read in case of an error.
        /// @return              The value of the channel.
        TYPE read(const uint32_t &channel, const TYPE &default_value) const {
            typename std::SR_SIGNAL_MAP__<uint32_t, TYPE>::iterator item =
                    this->m_value.find(channel);
            if (item != this->m_value.end()) {
                return item->second;
            } else {
                return default_value;
            }
        }

        /// Gets the value of a specific channel.
        TYPE operator[](const uint32_t &channel) const {
            return read(channel);
        }

        /// Connect the infield with an output on a specific channel.
        void operator()(signal_out_bind_if<TYPE> &sender, uint32_t channel) {
            this->bind(sender);
            sender.bind(*this);
        }

    private:
        /// Stores output interfaces to channel information.
        std::SR_SIGNAL_MAP__<signal_out_if<TYPE> *, uint32_t> m_channel;

        /// Stores the values of each channel.
        std::SR_SIGNAL_MAP__<uint32_t, TYPE> m_value;

        /// Stores the callback.
        t_callback m_callback;
};

}  // namespace sr_signal

#endif  // SR_SIGNAL_INFIELD_H
/// @}
