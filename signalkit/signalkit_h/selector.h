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
// Title:      signalkit_h/selector.h
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

#ifndef SIGNALKIT_SELECTOR_H
#define SIGNALKIT_SELECTOR_H

#include "signalkit_h/base.h"
#include "signalkit_h/ifs.h"
#include "signalkit_h/out.h"
#include <stdint.h>

#ifdef EN_HASH
#define MAP MAP
#include <ext/MAP>
namespace std { using namespace __gnu_cxx; }
#else
#define MAP map
#include <map>
#endif

namespace signalkit {

/// @addtogroup signalkit
/// @{

/// Signalkit selector signal.
/// This class implements a TLM Signal abstraction of an outgoing signal with multiple channels.
/// The signal stores the value and triggers the update function of all receivers.
/// Each channel can be updated seperately.
/// The channels are addressed as bitmasks. This makes it possible to update multiple channels at once.
template<class TYPE, class MODULE>
class signal_selector : public signal_base<TYPE, MODULE> ,
                        public signal_out_bind_if<TYPE> {
    public:
        /// Constructor
        ///
        /// @param mn Signal name.
        signal_selector(sc_core::sc_module_name mn = NULL) :
            signal_base<TYPE, MODULE>::signal_base(mn) {
        }

        /// Virtual destructor
        virtual ~signal_selector() {
        }
        
        /// Selector signal bind method.
        /// This method implements the binding mechanism with an input signal on a specific channel.
        ///
        /// @param receiver Input interface to bind with.
        /// @param channel  The channel which has to be bind.
        virtual signal_out_bind_if<TYPE> *bind(signal_in_if<TYPE> &receiver,
                          const uint32_t &channel) {
            // TODO: Make work multipel selector<->infield channels
            signal_out<TYPE, MODULE> *item = NULL;
            typename t_map::iterator iter = outs.find(channel);
            if (iter != outs.end()) {
                item = iter->second;
            } else {
                item = new signal_out<TYPE, MODULE> (
                        sc_core::sc_gen_unique_name("port", true));
                outs.insert(std::make_pair(channel, item));
            }
            item->bind(receiver);
            return item;
        }

        /// Write the value of a signal.
        /// Stores the value in corresponding channel and triggers an updat in all receivers.
        ///
        /// @param mask  The write mask with all channels to write to.
        /// @param value The new value of the signal.
        /// @param time The delay from sc_timestamp() at propagation.
        virtual void write(const uint32_t &mask, const TYPE &value,
                           const sc_core::sc_time &time = sc_core::SC_ZERO_TIME) {
            for (typename t_map::iterator i = outs.begin(); i != outs.end() && (mask >= (uint32_t)(1 << i->first)); i++) {
                if(mask & (1 << i->first)) {
                    i->second->write(value, time);
                }
            }
        }

        /*
         void operator[](const unsigned int &mask, const TYPE &value) {
         this->write(mask, value)
         }*/

        /// Connecting a channel with an input signal.
        /// Calls caller and receiver bind methods.
        ///
        /// @param receiver The input signal to connect with.
        /// @param channel  The channel which gets connected.
        void operator()(signal_in_if<TYPE> &receiver,
                        const uint32_t &channel) {
            this->bind(receiver, channel);
            receiver.bind(*this);
        }

    private:
        /// Channels to outputs list type.
        typedef std::MAP<uint32_t, signal_out<TYPE, MODULE> *> t_map;
        /// Stores the output list
        t_map outs;
};

/// @}

} // signalkit

#endif // SIGNALKIT_SELECTOR_H
