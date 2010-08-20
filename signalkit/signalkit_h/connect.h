/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       signalkit_h/connect.h:                                  */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef SIGNALKIT_CONNECT_H
#define SIGNALKIT_CONNECT_H

#include "signalkit_h/ifs.h"
#include "signalkit_h/adapter.h"

namespace signalkit {

template<class TYPE>
void connect(signal_in_if<TYPE> &in, signal_out_if<TYPE> &out) {
  in.bind(out);
  out.bind(in);
}

template<class INTYPE, class OUTTYPE, class MODULE>
sc_core::sc_module *connect(signal_out<INTYPE, MODULE> &in, sc_core::sc_signal<OUTTYPE> &out) {
  tlmin_scout_adapter<INTYPE, OUTTYPE> *result = new tlmin_scout_adapter<INTYPE, OUTTYPE>(sc_core::sc_gen_unique_name("adapter"));
  in(result->in);
  result->out(out);
  return result;
}

template<class INTYPE, class OUTTYPE, class MODULE>
sc_core::sc_module *connect(sc_core::sc_signal<INTYPE> &in, signal_in<OUTTYPE, MODULE> &out) {
  scin_tlmout_adapter<INTYPE, OUTTYPE> *result = new scin_tlmout_adapter<INTYPE, OUTTYPE>(sc_core::sc_gen_unique_name("adapter"));
  result->in(in);
  result->out(out);
  return result;
}

} // signalkit

#endif // SIGNALKIT_CONNECT_H
