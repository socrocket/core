/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       tlm_signal_adapter.h - This file provides adapter to    */
/*             connect tlm_signals with SystemC signals.               */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef TLM_SIGNAL_ADAPTER
#define TLM_SIGNAL_ADAPTER

#include "tlm_signal.h"
#include <systemc.h>

template<class INTYPE, class OUTTYPE = INTYPE>
class tlmin_scout_adapter : public tlm_module<tlmin_scout_adapter<INTYPE, OUTTYPE> >, public sc_core::sc_module {
  public:
    typename tlm_module<tlmin_scout_adapter<INTYPE, OUTTYPE> >::template signal<INTYPE>::in in;
    sc_out<OUTTYPE > out;

    tlmin_scout_adapter(sc_core::sc_module_name mn)
      : sc_core::sc_module(mn)
      , in(this, &tlmin_scout_adapter::oninput, "IN")
      , out("OUT") {}

  private:
    void oninput(const INTYPE &value, tlm_signal_in_if<INTYPE> *signal, tlm_signal_out_if<INTYPE> *sender, const sc_core::sc_time &time) {
      OUTTYPE o = value;
      out.write(o);
    }

};

template<class INTYPE, class OUTTYPE = INTYPE>
class scin_tlmout_adapter : public tlm_module<scin_tlmout_adapter<INTYPE, OUTTYPE> >, public sc_core::sc_module {
public:
  typename tlm_module<scin_tlmout_adapter<INTYPE, OUTTYPE> >::template signal<OUTTYPE>::out out;
  sc_in<INTYPE>     in;

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


template<class INTYPE, class OUTTYPE = INTYPE>
class tlmsc_inout_adapter : public tlm_module<tlmsc_inout_adapter<INTYPE, OUTTYPE> >, public sc_core::sc_module {
public:
  typename tlm_module<tlmsc_inout_adapter<INTYPE, OUTTYPE> >::template signal<INTYPE>::inout tlm;
  sc_inout<OUTTYPE>     sc;

  SC_HAS_PROCESS(tlmsc_inout_adapter);
  
  tlmsc_inout_adapter(sc_core::sc_module_name mn)
    : sc_core::sc_module(mn)
    , tlm(this, &tlmsc_inout_adapter::ontlm, "TLM")
    , sc("SC") {
    SC_THREAD(onsc);
    dont_initialize();
    sensitive << sc;
  }

private:
  void ontlm(const INTYPE &value, tlm_signal_in_if<INTYPE> *signal, tlm_signal_out_if<INTYPE> *sender, const sc_core::sc_time &time) {
    OUTTYPE o = value;
    sc.write(o);
  }

  void onsc() {
    OUTTYPE o = sc.read();
    INTYPE i = o;
    tlm = i;
  }
};

template<class INTYPE, class OUTTYPE, class MODULE>
sc_core::sc_module *signal_connect(tlm_signal_out<INTYPE, MODULE> &in, sc_core::sc_signal<OUTTYPE> &out) {
  tlmin_scout_adapter<INTYPE, OUTTYPE> *result = new tlmin_scout_adapter<INTYPE, OUTTYPE>(sc_core::sc_gen_unique_name("adapter"));
  in(result->in);
  result->out(out);
  return result;
}

template<class INTYPE, class OUTTYPE, class MODULE>
sc_core::sc_module *signal_connect(sc_core::sc_signal<INTYPE> &in, tlm_signal_in<OUTTYPE, MODULE> &out) {
  scin_tlmout_adapter<INTYPE, OUTTYPE> *result = new scin_tlmout_adapter<INTYPE, OUTTYPE>(sc_core::sc_gen_unique_name("adapter"));
  result->in(in);
  result->out(out);
  return result;
}
// namespace tlm_signals

// } // tlm_signals

#endif // TLM_SIGNAL_ADAPTER

