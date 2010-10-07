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
// Title:      signalkit_h/module.h
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

#ifndef SIGNALKIT_MODULE_H
#define SIGNALKIT_MODULE_H

#include "signalkit_h/in.h"
#include "signalkit_h/out.h"
#include "signalkit_h/inout.h"
#include "signalkit_h/selector.h"
#include "signalkit_h/infield.h"

/// @addtogroup signalkit
/// @{

namespace signalkit {

template<class MODULE>
class signal_module {
  public:
    template<class TYPE>
    struct signal {
      typedef signal_in<TYPE, MODULE>       in;
      typedef signal_out<TYPE, MODULE>      out;
      typedef signal_inout<TYPE, MODULE>    inout;
      typedef signal_selector<TYPE, MODULE> selector;
      typedef signal_infield<TYPE, MODULE>  infield;
    };
};

} // signalkit

/// @}

#endif // TLM_MODULE_H
