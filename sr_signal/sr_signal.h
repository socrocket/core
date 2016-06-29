// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_signal
/// @{
/// @file sr_signal.h
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///
/// sr_signal enables the user to read and write signal like channels.
/// Unlike the signal implementation is SystemC the signals are not using the SystemC kernel.
/// Therefore they are not scheduled. They are direct transmittet as function calls.
/// sr_signal is designed to complete the idea behind TLM to use direct fast communication.
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


#ifndef SR_SIGNAL_H_
#define SR_SIGNAL_H_
#include "sr_signal_module.h"
#include "sr_signal_adapter.h"
#include "sr_signal_connect.h"

#endif  // SR_SIGNAL_H_
/// @}
