// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_signal
/// @{
/// @file scin_tlmout_adapter.h
/// @date 2010-2015
/// @author Rolf Meyer
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
#ifndef SCIN_TLMOUT_ADAPTER_H
#define SCIN_TLMOUT_ADAPTER_H

#include <systemc>
#include "sr_signal_module.h"

namespace sr_signal {

template<class INTYPE, class OUTTYPE = INTYPE>
class scin_tlmout_adapter : public sr_signal_module<scin_tlmout_adapter<INTYPE,
        OUTTYPE> > , public sc_core::sc_module {
    public:
        typename sr_signal_module<scin_tlmout_adapter<INTYPE, OUTTYPE> >::template signal<
                OUTTYPE>::out out;
        sc_core::sc_in<INTYPE> in;

        SC_HAS_PROCESS(scin_tlmout_adapter);

        scin_tlmout_adapter(sc_core::sc_module_name mn) :
            sc_core::sc_module(mn), out("OUT"), in("IN") {

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

}  // namespace sr_signal

#endif  // SCIN_TLMOUT_ADAPTER_H
/// @}
