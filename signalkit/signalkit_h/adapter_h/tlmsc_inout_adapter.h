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
// Title:      tlmin_scout_adapter.h
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

#ifndef TLMSC_INOUT_ADAPTER_H
#define TLMSC_INOUT_ADAPTER_H

#include "signalkit_h/module.h"
#include <systemc>

namespace signalkit {

/// @addtogroup signalkit
/// @{

template<class INTYPE, class OUTTYPE = INTYPE>
class tlmsc_inout_adapter : public signal_module<tlmsc_inout_adapter<INTYPE,
        OUTTYPE> > , public sc_core::sc_module {
    public:
        typename signal_module<tlmsc_inout_adapter<INTYPE, OUTTYPE> >::template signal<
                INTYPE>::inout tlm;
        sc_core::sc_inout<OUTTYPE> sc;

        SC_HAS_PROCESS( tlmsc_inout_adapter);

        tlmsc_inout_adapter(sc_core::sc_module_name mn) :
            sc_core::sc_module(mn), tlm(&tlmsc_inout_adapter::ontlm, "TLM"),
                    sc("SC") {
            SC_THREAD(onsc);
            dont_initialize();
            sensitive << sc;
        }

    private:
        void ontlm(const INTYPE &value, const sc_core::sc_time &time) {
            OUTTYPE o = value;
            sc.write(o);
        }

        void onsc() {
            OUTTYPE o = sc.read();
            INTYPE i = o;
            tlm = i;
        }
};

/// @}

} // signalkit

#endif // TLMSC_INOUT_ADAPTER_H
