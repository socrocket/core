// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup signalkit
/// @{
/// @file tlmsc_inout_adapter.h
/// 
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef TLMSC_INOUT_ADAPTER_H
#define TLMSC_INOUT_ADAPTER_H

#include "signalkit_h/module.h"
#include <systemc>

namespace signalkit {

/// @addtogroup signalkit
/// @{

template<class INTYPE, class OUTTYPE = INTYPE>
class tlmsc_inout_adapter : public signal_module<tlmsc_inout_adapter<INTYPE,
        OUTTYPE> > , public sc_core::sc_module {
    public:
        typename signal_module<tlmsc_inout_adapter<INTYPE, OUTTYPE> >::template signal<
                INTYPE>::inout tlm;
        sc_core::sc_inout<OUTTYPE> sc;

        SC_HAS_PROCESS( tlmsc_inout_adapter);

        tlmsc_inout_adapter(sc_core::sc_module_name mn) :
            sc_core::sc_module(mn), tlm(&tlmsc_inout_adapter::ontlm, "TLM"),
                    sc("SC") {
            SC_THREAD(onsc);
            dont_initialize();
            sensitive << sc;
        }

    private:
        void ontlm(const INTYPE &value, const sc_core::sc_time &time) {
            OUTTYPE o = value;
            sc.write(o);
        }

        void onsc() {
            OUTTYPE o = sc.read();
            INTYPE i = o;
            tlm = i;
        }
};

/// @}

} // signalkit

#endif // TLMSC_INOUT_ADAPTER_H
/// @}