// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup signalkit
/// @{
/// @file tlmin_scout_adapter.h
/// 
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef TLMIN_SCOUT_ADAPTER_H
#define TLMIN_SCOUT_ADAPTER_H

#include <systemc>
#include "signalkit/signalkit_h/module.h"

namespace signalkit {

template<class INTYPE, class OUTTYPE = INTYPE>
class tlmin_scout_adapter : public signal_module<tlmin_scout_adapter<INTYPE,
        OUTTYPE> > , public sc_core::sc_module {
    public:
        typename signal_module<tlmin_scout_adapter<INTYPE, OUTTYPE> >::template signal<
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

}  // namesapce signalkit

#endif  // TLMIN_SCOUT_ADAPTER_H
/// @}
