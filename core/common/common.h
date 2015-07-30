// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file common.h
///
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef COMMON_COMMON_H_
#define COMMON_COMMON_H_

#include "core/common/systemc.h"
#include "core/common/vendian.h"
#include "core/common/verbose.h"
inline void vwait(sc_core::sc_time &delay) {  // NOLINT(runtime/references)
  if (delay != sc_core::SC_ZERO_TIME) {
    sc_core::wait(delay);
  }
}

inline void await(sc_core::sc_time time) {
  sc_core::wait(time - sc_core::sc_time_stamp());
}

#if SYSTEMC_API == 210 || SYSTEMC_API == 220
namespace gs {
namespace cnf {
typedef void callback_return_type;
}
}
#define GC_RETURN_OK
#else
#define GC_RETURN_OK gs::cnf::return_nothing
#endif

#endif  // COMMON_COMMON_H_
/// @}
