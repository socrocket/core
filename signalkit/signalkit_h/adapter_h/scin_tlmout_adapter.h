/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       scin_tlmout_adapter.h:                                  */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef SCIN_TLMOUT_ADAPTER_H
#define SCIN_TLMOUT_ADAPTER_H

#include "signalkit_h/module.h"
#include <systemc>

/// @addtogroup signalkit
/// @{

namespace signalkit {

template<class INTYPE, class OUTTYPE = INTYPE>
class scin_tlmout_adapter : public signal_module<scin_tlmout_adapter<INTYPE, OUTTYPE> >, public sc_core::sc_module {
public:
  typename signal_module<scin_tlmout_adapter<INTYPE, OUTTYPE> >::template signal<OUTTYPE>::out out;
  sc_core::sc_in<INTYPE>     in;

  SC_HAS_PROCESS(scin_tlmout_adapter);
  
  scin_tlmout_adapter(sc_core::sc_module_name mn)
    : sc_core::sc_module(mn)
    , out(this,"OUT")
    , in("IN") {
      
    SC_THREAD(oninput);
    dont_initialize();
    sensitive << in;
  }

private:
  void oninput() {
    INTYPE i = in.read();
    OUTTYPE o = i;
    out = o;
  }
};

} // signalkit

/// @}

#endif // SCIN_TLMOUT_ADAPTER_H
