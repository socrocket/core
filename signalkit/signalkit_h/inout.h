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
// Title:      signalkit_h/inout.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef SIGNALKIT_INOUT_H
#define SIGNALKIT_INOUT_H

#include "signalkit_h/out.h"
#include "signalkit_h/in.h"
#include <stdint.h>

namespace signalkit {

/// @addtogroup signalkit
/// @{

/// Signalkit input output signal.
/// This class implements a TLM Signal abstraction of an bidirectional signal.
/// The signal stores the value and can execute a callback.
template<class TYPE, class MODULE>
class signal_inout : public signal_out<TYPE, MODULE> , public signal_in<TYPE,
        MODULE> {
    public:
        /// Inout signal callback type.
        /// The callback has two parameter.
        ///
        /// @param value The new value of the signal.
        /// @param time  The delay from sc_timestamp() of the propagation.
        typedef void(MODULE::*t_callback)(const TYPE &value,
                                          const sc_core::sc_time &time);

        /// Constructor without callback.
        ///
        /// @param mn Signal name.
        signal_inout(sc_core::sc_module_name mn = NULL) :
            signal_out<TYPE, MODULE>::signal_out(mn),
                    signal_in<TYPE, MODULE>::signal_in(mn) {
        }

        /// Constructor with callback.
        ///
        /// @param callback A base class function which gets executed every time the value is chaned.
        /// @param mn       Signal name.
        signal_inout(t_callback callback, sc_core::sc_module_name mn = NULL) :
            signal_out<TYPE, MODULE>::signal(mn),
                    signal_in<TYPE, MODULE>::signal_in(callback, mn) {
        }

        /// Virtual destructor.
        virtual ~signal_inout() {
        }
};

/// @}

} // signalkit

#endif // SIGNALKIT_INOUT_H
