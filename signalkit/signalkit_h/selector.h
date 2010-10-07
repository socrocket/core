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

#include <map>
#include "signalkit_h/base.h"
#include "signalkit_h/ifs.h"
#include "signalkit_h/out.h"

/// @addtogroup signalkit
/// @{

namespace signalkit {

template<class TYPE, class MODULE>
class signal_selector : public signal_base<TYPE, MODULE> ,
                        public signal_out_bind_if<TYPE> {
    public:
        signal_selector(sc_core::sc_module_name mn = NULL) :
            signal_base<TYPE, MODULE>::signal_base(mn) {
        }

        virtual void bind(signal_in_if<TYPE> &receiver,
                          const unsigned int &channel) {
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
        }

        virtual void write(const unsigned int &mask, const TYPE &value,
                           const sc_core::sc_time &time = sc_core::SC_ZERO_TIME) {
            for (typename t_map::iterator i = outs.begin(); i != outs.end(); i++) {
                if (mask & (1 << i->first)) {
                    i->second->write(value, time);
                }
            }
        }

        /*
         void operator[](const unsigned int &mask, const TYPE &value) {
         this->write(mask, value)
         }*/

        void operator()(signal_in_if<TYPE> &receiver,
                        const unsigned int &channel) {
            this->bind(receiver, channel);
            receiver.bind(*this);
        }

    private:
        typedef std::map<unsigned int, signal_out<TYPE, MODULE> *> t_map;
        t_map outs;
};

} // signalkit

/// @}

#endif // SIGNALKIT_SELECTOR_H
