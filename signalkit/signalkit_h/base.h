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
// Title:      signalkit_h/base.h
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

#ifndef SIGNALKIT_BASE_H
#define SIGNALKIT_BASE_H

#include <systemc>

/// @addtogroup signalkit
/// @{

namespace signalkit {

template<class TYPE, class MODULE>
class signal_base : public sc_core::sc_object {
    public:
        signal_base(sc_core::sc_module_name mn = NULL) :
            sc_core::sc_object() {
        }

        virtual ~signal_base() {
        }
    protected:
        virtual MODULE *get_module() {
            sc_core::sc_object *obj = get_parent();
            MODULE *mod = dynamic_cast<MODULE *> (obj);
            if (!mod) {
                // Error Message: MODULE is not an sc_object/systemc_module.
                return NULL;
            }
            return mod;
        }
};

}
; // signalkit

/// @}

#endif // SIGNALKIT_BASE_H
