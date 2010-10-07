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

#ifndef SIGNALKIT_CONNECT_H
#define SIGNALKIT_CONNECT_H

#include "signalkit_h/ifs.h"
#include "signalkit_h/adapter.h"

/// @addtogroup signalkit
/// @{

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

/// @}

#endif // SIGNALKIT_CONNECT_H
