// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file sc_api.h
///
///
/// @date 2013-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author
///

#ifndef COMMON_SC_API_H_
#define COMMON_SC_API_H_

#include <systemc>

#ifdef SYSTEMC_API
#if SYSTEMC_API != 210 && SYSTEMC_API != 220 && SYSTEMC_API != 230
#error "The SYSTEMC_API macro is set to an unknown value. Accepted values are 210 or 220."
#error "Please, see the source code gs_sc_api_detection.h for more information."
#endif
#else
// OSCI SystemC 2.1
#if SYSTEMC_VERSION == 20050714
#define SYSTEMC_API 210
#endif
// OSCI SystemC 2.2.0 and 2.2.05jun06
#if SYSTEMC_VERSION == 20070314 || SYSTEMC_VERSION == 20060505
#define SYSTEMC_API 220
#endif
// OSCI SystemC 2.3.0
#if SYSTEMC_VERSION == 20120701
#define SYSTEMC_API 230
#endif
// SystemC API to use must be defined at this point
// include the detection above for other SystemC vendors to
// define the SYSTEMC_API macro
#ifndef SYSTEMC_API
#error "The SYSTEMC_API macro is not set and could not be detected."
#error "Please, see the source code gs_sc_api_detection.h for more information."
#endif
#endif

// Extend SystemC with our custom functions.
#include "common/sc_find.h"

#endif  // COMMON_SC_API_H_
/// @}
