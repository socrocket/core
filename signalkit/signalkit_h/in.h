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

/// @addtogroup
/// @{

namespace signalkit {

template<class TYPE, class MODULE>
class signal_in : public signal_base<TYPE, MODULE> , public signal_in_if<TYPE> {
    public:
        typedef void(MODULE::*t_callback)(const TYPE &value,
                                          signal_in_if<TYPE> *signal,
                                          signal_out_if<TYPE> *sender,
                                          const sc_core::sc_time &time);

        signal_in(sc_core::sc_module_name mn = NULL) :
            signal_base<TYPE, MODULE>::signal_base(mn), m_callback(NULL) {
        }

        signal_in(t_callback callback, sc_core::sc_module_name mn = NULL) :
            signal_base<TYPE, MODULE>::signal_base(mn), m_callback(callback) {
        }

        virtual ~signal_in() {
        }
        virtual void bind(signal_out_bind_if<TYPE> &sender,
                          const unsigned int &channel = 0) {
        }

        virtual void update(signal_out_if<TYPE> *sender,
                            const sc_core::sc_time &time) {
            this->m_value = sender->read();
            if (m_callback) {
                MODULE *mod = this->get_module();
                if (mod) {
                    (mod->*m_callback)(this->m_value, this, sender, time);
                }
            }
        }

        virtual operator TYPE() const {
            return this->m_value;
        }

        virtual void operator ()(signal_out_bind_if<TYPE> &sender) {
            this->bind(sender);
            sender.bind(*this);
        }

    private:
        t_callback m_callback;
};

} // signalkit

/// @}

#endif // TLM_SIGNAL_IN_H
