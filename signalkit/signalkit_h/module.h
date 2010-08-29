/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       signalkit_h/module.h:                                   */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

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
