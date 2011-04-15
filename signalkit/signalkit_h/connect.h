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
// Title:      signalkit_h/connect.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    This file contains connect methods of all kind of 
//             signalkit connections: in<->out, kit<->sysc
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

#ifndef SIGNALKIT_CONNECT_H
#define SIGNALKIT_CONNECT_H

#include "signalkit_h/ifs.h"
#include "signalkit_h/adapter.h"
#include <stdint.h>
namespace signalkit {

/// @addtogroup signalkit
/// @{

/// Connects an input signal with an output signal
template<class TYPE>
void connect(signal_in_if<TYPE> &in, signal_out_bind_if<TYPE> &out, int channel = 0) {
    in.bind(*out.bind(in, channel), channel);
}

/// Connects an input signal with an output signal
template<class TYPE>
void connect(signal_out_bind_if<TYPE> &out, signal_in_if<TYPE> &in, int channel = 0) {
    in.bind(*out.bind(in, channel), channel);
}

/// Connects an input signal with an output signal
template<class TYPE>
void connect(signal_in_if<TYPE> &in, signal_out_bind_if<TYPE> &out, int inchannel, int outchannel) {
    in.bind(*out.bind(in, outchannel), inchannel);
}

/// Connects an input signal with an output signal
template<class TYPE>
void connect(signal_out_bind_if<TYPE> &out, signal_in_if<TYPE> &in, int outchannel, int inchannel) {
    in.bind(*out.bind(in, outchannel), inchannel);
}

/// Special connect mehtod to connect an signalkit output with an systemc input signal
template<class INTYPE, class OUTTYPE, class MODULE>
sc_core::sc_module *connect(signal_out<INTYPE, MODULE> &in, sc_core::sc_signal<OUTTYPE> &out, int channel = 0) {
    tlmin_scout_adapter<INTYPE, OUTTYPE> *result = new tlmin_scout_adapter<
            INTYPE, OUTTYPE> (sc_core::sc_gen_unique_name("adapter"));
    in.bind(result->in, channel);
    result->in.bind(in, channel);
    result->out(out);
    return result;
}

/// Special connect mehtod to connect an signalkit output with an systemc sc_in
template<class INTYPE, class OUTTYPE, class MODULE>
sc_core::sc_module *connect(signal_out<INTYPE, MODULE> &in, sc_core::sc_in<OUTTYPE> &out, int channel = 0) {
    sc_core::sc_signal<OUTTYPE> signal(sc_core::sc_gen_unique_name("signal"));
    tlmin_scout_adapter<INTYPE, OUTTYPE> *result = new tlmin_scout_adapter<
            INTYPE, OUTTYPE> (sc_core::sc_gen_unique_name("adapter"));
    out(signal);
    in.bind(result->in, channel);
    result->in.bind(in, channel);
    result->out(signal);
    return result;
}


/// Special connect mehtod to connect an signalkit input with an systemc output signal
template<class INTYPE, class OUTTYPE, class MODULE>
sc_core::sc_module *connect(sc_core::sc_signal<INTYPE> &in, signal_in<OUTTYPE, MODULE> &out, int channel = 0) {
    scin_tlmout_adapter<INTYPE, OUTTYPE> *result = new scin_tlmout_adapter<
            INTYPE, OUTTYPE> (sc_core::sc_gen_unique_name("adapter"));
    result->in(in);
    result->out.bind(out, channel);
    out.bind(result->out, channel);
    return result;
}

/// Special connect mehtod to connect an signalkit input with an systemc sc_out
template<class INTYPE, class OUTTYPE, class MODULE>
sc_core::sc_module *connect(sc_core::sc_out<INTYPE> &in, signal_in<OUTTYPE, MODULE> &out, uint32_t channel = 0) {
    sc_core::sc_signal<INTYPE> signal(sc_core::sc_gen_unique_name("signal"));
    scin_tlmout_adapter<INTYPE, OUTTYPE> *result = new scin_tlmout_adapter<
            INTYPE, OUTTYPE> (sc_core::sc_gen_unique_name("adapter"));
    in(signal);
    result->in(signal);
    result->out.bind(out, channel);
    out.bind(result->out, channel);
    return result;
}

/// @}

} // signalkit

#endif // SIGNALKIT_CONNECT_H
