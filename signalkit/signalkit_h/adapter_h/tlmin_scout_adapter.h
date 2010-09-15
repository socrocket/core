/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       tlmin_scout_adapter.h:                                  */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef TLMIN_SCOUT_ADAPTER_H
#define TLMIN_SCOUT_ADAPTER_H

#include "signalkit_h/module.h"
#include <systemc>

/// @addtogroup signalkit
/// @{

namespace signalkit {

template<class INTYPE, class OUTTYPE = INTYPE>
class tlmin_scout_adapter : public signal_module<tlmin_scout_adapter<INTYPE, OUTTYPE> >, public sc_core::sc_module {
  public:
    typename signal_module<tlmin_scout_adapter<INTYPE, OUTTYPE> >::template signal<INTYPE>::in in;
    sc_core::sc_out<OUTTYPE > out;

    tlmin_scout_adapter(sc_core::sc_module_name mn)
      : sc_core::sc_module(mn)
      , in(&tlmin_scout_adapter::oninput, "IN")
      , out("OUT") {}

  private:
    void oninput(const INTYPE &value, signal_in_if<INTYPE> *signal, signal_out_if<INTYPE> *sender, const sc_core::sc_time &time) {
      OUTTYPE o = value;
      out.write(o);
    }

};

} // signalkit

/// @}

#endif // TLMIN_SCOUT_ADAPTER_H
