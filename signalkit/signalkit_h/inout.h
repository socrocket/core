/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       signalkit_h/inout.h:                                    */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef SIGNALKIT_INOUT_H
#define SIGNALKIT_INOUT_H

#include "signalkit_h/out.h"
#include "signalkit_h/in.h"

/// @addtogroup signalkit
/// @{

namespace signalkit {

template<class TYPE, class MODULE>
class signal_inout : public signal_out<TYPE, MODULE>, public signal_in<TYPE, MODULE> {
  public:
    typedef void(MODULE::*t_callback)(const TYPE &value, signal_in_if<TYPE> *signal, signal_out_if<TYPE> *sender, const sc_core::sc_time &time);
   
    signal_inout(MODULE *module, sc_core::sc_module_name mn = NULL)
      : signal_out<TYPE, MODULE>::signal_out(module, mn)
      , signal_in<TYPE, MODULE>::signal_in(module, mn) {}

    signal_inout(MODULE *module, t_callback callback, sc_core::sc_module_name mn = NULL)
      : signal_out<TYPE, MODULE>::signal(module, mn)
      , signal_in<TYPE, MODULE>::signal_in(module, callback, mn) {}

    virtual ~signal_inout() {}
};

} // signalkit

/// @}

#endif // SIGNALKIT_INOUT_H
