// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup signalkit
/// @{
/// @file scin_tlmout_adapter.h
/// 
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef SCIN_TLMOUT_ADAPTER_H
#define SCIN_TLMOUT_ADAPTER_H

#include <systemc>
#include "signalkit/signalkit_h/module.h"

namespace signalkit {

template<class INTYPE, class OUTTYPE = INTYPE>
class scin_tlmout_adapter : public signal_module<scin_tlmout_adapter<INTYPE,
        OUTTYPE> > , public sc_core::sc_module {
    public:
        typename signal_module<scin_tlmout_adapter<INTYPE, OUTTYPE> >::template signal<
                OUTTYPE>::out out;
        sc_core::sc_in<INTYPE> in;

        SC_HAS_PROCESS( scin_tlmout_adapter);

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

}  // namespace signalkit

#endif  // SCIN_TLMOUT_ADAPTER_H
/// @}
