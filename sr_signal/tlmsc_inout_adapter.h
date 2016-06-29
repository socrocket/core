// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_signal
/// @{
/// @file tlmsc_inout_adapter.h
/// @date 2010-2015
/// @author Rolf Meyer
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
#ifndef TLMSC_INOUT_ADAPTER_H
#define TLMSC_INOUT_ADAPTER_H

#include <systemc>
#include "sr_signal_module.h"

namespace sr_signal {

template<class INTYPE, class OUTTYPE = INTYPE>
class tlmsc_inout_adapter : public sr_signal_module<tlmsc_inout_adapter<INTYPE,
        OUTTYPE> > , public sc_core::sc_module {
    public:
        typename sr_signal_module<tlmsc_inout_adapter<INTYPE, OUTTYPE> >::template signal<
                INTYPE>::inout tlm;
        sc_core::sc_inout<OUTTYPE> sc;

        SC_HAS_PROCESS(tlmsc_inout_adapter);

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

}  // namesapce sr_signal

#endif  // TLMSC_INOUT_ADAPTER_H
/// @}
