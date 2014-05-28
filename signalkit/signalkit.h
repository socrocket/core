// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup signalkit
/// @{
/// @file signalkit.h
/// 
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

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
/// @}