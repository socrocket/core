#ifndef VCOMMON_H
#define VCOMMON_H

#include "vendian.h"
#include "verbose.h"

#include <systemc.h>

inline vwait(sc_core::sc_time &delay) {
    if(delay!=sc_core::SC_ZERO_TIME) {
        sc_core::wait(delay);
    }
}

inline await(sc_core::sc_time time) {
    sc_core::wait(time - sc_time_stamp());
}

#endif // VCOMMON_H
