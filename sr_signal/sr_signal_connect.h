// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_signal
/// @{
/// @file connect.h
/// This file contains connect methods of all kind of sr_signal connections:
/// in<->out, kit<->sysc
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef SR_SIGNAL_CONNECT_H
#define SR_SIGNAL_CONNECT_H

#include <stdint.h>
#include "sr_signal_ifs.h"
#include "sr_signal_adapter.h"
namespace sr_signal {

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

/// Special connect mehtod to connect an sr_signal output with an systemc input signal
template<class INTYPE, class OUTTYPE, class MODULE>
sc_core::sc_module *connect(signal_out<INTYPE, MODULE> &in, sc_core::sc_signal<OUTTYPE> &out, int channel = 0) {
    tlmin_scout_adapter<INTYPE, OUTTYPE> *result = new tlmin_scout_adapter<
            INTYPE, OUTTYPE> (sc_core::sc_gen_unique_name("adapter"));
    in.bind(result->in, channel);
    result->in.bind(in, channel);
    result->out(out);
    return result;
}

/// Special connect mehtod to connect an sr_signal output with an systemc sc_in
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


/// Special connect mehtod to connect an sr_signal input with an systemc output signal
template<class INTYPE, class OUTTYPE, class MODULE>
sc_core::sc_module *connect(sc_core::sc_signal<INTYPE> &in, signal_in<OUTTYPE, MODULE> &out, int channel = 0) {
    scin_tlmout_adapter<INTYPE, OUTTYPE> *result = new scin_tlmout_adapter<
            INTYPE, OUTTYPE> (sc_core::sc_gen_unique_name("adapter"));
    result->in(in);
    result->out.bind(out, channel);
    out.bind(result->out, channel);
    return result;
}

/// Special connect mehtod to connect an sr_signal input with an systemc sc_out
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

} // namespace sr_signal

#endif // SR_SIGNAL_CONNECT_H
/// @}
