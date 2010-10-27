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
// Title:      scin_tlmout_adapter.h
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

#ifndef SCIN_TLMOUT_ADAPTER_H
#define SCIN_TLMOUT_ADAPTER_H

#include "signalkit_h/module.h"
#include <systemc>

namespace signalkit {

/// @addtogroup signalkit
/// @{

template<class INTYPE, class OUTTYPE = INTYPE>
class scin_tlmout_adapter : public signal_module<scin_tlmout_adapter<INTYPE,
        OUTTYPE> > , public sc_core::sc_module {
    public:
        typename signal_module<scin_tlmout_adapter<INTYPE, OUTTYPE> >::template signal<
                OUTTYPE>::out out;
        sc_core::sc_in<INTYPE> in;

        SC_HAS_PROCESS( scin_tlmout_adapter);

        scin_tlmout_adapter(sc_core::sc_module_name mn) :
            sc_core::sc_module(mn), out("OUT"), in("IN") {

            SC_THREAD(oninput);
            dont_initialize();
            sensitive << in;
        }

    private:
        void oninput() {
            INTYPE i = in.read();
            OUTTYPE o = i;
            out = o;
        }
};

/// @}

} // signalkit

#endif // SCIN_TLMOUT_ADAPTER_H
