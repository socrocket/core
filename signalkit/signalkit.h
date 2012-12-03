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
// Title:      signalkit.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef SIGNALKIT_H
#define SIGNALKIT_H

/// @defgroup signalkit SignalKit
///
/// The SignalKit enables the user to read and write signal like channels.
/// Unlike the signal implementation is SystemC the signals are not using the SystemC kernel.
/// Therefore they are not scheduled. They are direct transmittet as function calls.
/// The SignalKit is designed to complete the idea behind TLM to use direct fast communication.
/// But the use of single Signals should be avoided when possible.
///
/// Each incomming signal can trigger a callback, but at least it will change a value.
/// This happen instandly. Timing information can be attatched to a signal change.
/// The transport of timing information does only work in signal direction.
///
/// Signals can have multiple sender or receiver. But each receiver must be registerd at each sender.
///
/// Therefore unlike TLM sockets it is not possible to interact over signals directly between a master and slave.
/// Insted it is useful in a case of unidiectional communication: interrupts or resets.
///
/// @{

#include "signalkit_h/module.h"
#include "signalkit_h/adapter.h"
#include "signalkit_h/connect.h"

/// @}

#endif // SIGNALKIT_H
