// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_signal
/// @{
/// @file tlmin_scout_adapter.h
/// @date 2010-2015
/// @author Rolf Meyer
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
#ifndef TLMIN_SCOUT_ADAPTER_H
#define TLMIN_SCOUT_ADAPTER_H

#include <systemc>
#include "sr_signal_module.h"

namespace sr_signal {

template<class INTYPE, class OUTTYPE = INTYPE>
class tlmin_scout_adapter : public sr_signal_module<tlmin_scout_adapter<INTYPE,
        OUTTYPE> > , public sc_core::sc_module {
    public:
        typename sr_signal_module<tlmin_scout_adapter<INTYPE, OUTTYPE> >::template signal<
                INTYPE>::in in;
        sc_core::sc_out<OUTTYPE> out;

        tlmin_scout_adapter(sc_core::sc_module_name mn) :
            sc_core::sc_module(mn), in(&tlmin_scout_adapter::oninput, "IN"),
                    out("OUT") {
        }

    private:
        void oninput(const INTYPE &value,
                     const sc_core::sc_time &time) {
            OUTTYPE o = value;
            out.write(o);
        }

};

}  // namesapce sr_signal

#endif  // TLMIN_SCOUT_ADAPTER_H
/// @}
