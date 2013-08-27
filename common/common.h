// ********************************************************************
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
// ********************************************************************
// Title:      common.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
// ********************************************************************

#ifndef VCOMMON_H
#define VCOMMON_H

#include "vendian.h"
#include "verbose.h"

#include <systemc.h>

inline void vwait(sc_core::sc_time &delay) {
    if(delay!=sc_core::SC_ZERO_TIME) {
        sc_core::wait(delay);
    }
}

inline void await(sc_core::sc_time time) {
    sc_core::wait(time - sc_time_stamp());
}

#if SYSTEMC_API == 210 || SYSTEMC_API == 220
namespace gs {
    namespace cnf {
        typedef void callback_return_type;
    }
}
#define GC_RETURN_OK
#else
#define GC_RETURN_OK gs::cnf::return_nothing 
#endif

#endif // VCOMMON_H
