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

#ifndef TLMSC_INOUT_ADAPTER_H
#define TLMSC_INOUT_ADAPTER_H

#include "signalkit_h/module.h"
#include <systemc>

/// @addtogroup signalkit
/// @{

namespace signalkit {

template<class INTYPE, class OUTTYPE = INTYPE>
class tlmsc_inout_adapter : public signal_module<tlmsc_inout_adapter<INTYPE, OUTTYPE> >, public sc_core::sc_module {
public:
  typename signal_module<tlmsc_inout_adapter<INTYPE, OUTTYPE> >::template signal<INTYPE>::inout tlm;
  sc_core::sc_inout<OUTTYPE>     sc;

  SC_HAS_PROCESS(tlmsc_inout_adapter);
  
  tlmsc_inout_adapter(sc_core::sc_module_name mn)
    : sc_core::sc_module(mn)
    , tlm(&tlmsc_inout_adapter::ontlm, "TLM")
    , sc("SC") {
    SC_THREAD(onsc);
    dont_initialize();
    sensitive << sc;
  }

private:
  void ontlm(const INTYPE &value, signal_in_if<INTYPE> *signal, signal_out_if<INTYPE> *sender, const sc_core::sc_time &time) {
    OUTTYPE o = value;
    sc.write(o);
  }

  void onsc() {
    OUTTYPE o = sc.read();
    INTYPE i = o;
    tlm = i;
  }
};

} // signalkit

/// @}

#endif // TLMSC_INOUT_ADAPTER_H
