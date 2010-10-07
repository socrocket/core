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
// Title:      apbtestbench.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file defining the a generic APB testbench
//             template to use for systemc or vhdl simulation
//             all implementations are included for
//             maximum inline optiisatzion
//
// Method:
//
// Modified on $Date: 2010-08-30 00:40:19 +0200 (Mon, 30 Aug 2010) $
//          at $Revision: 84 $
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TU Braunschweig
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#include "apbtestbench.h"

CAPBTestbench::CAPBTestbench(sc_core::sc_module_name nm)
: sc_core::sc_module(nm), master_sock("socket", amba::amba_APB, amba::amba_LT, false) {}

CAPBTestbench::~CAPBTestbench() {}
