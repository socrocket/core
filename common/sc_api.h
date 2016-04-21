// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file sc_api.h
/// @date 2013-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Marcus Bartholomeu (GreenSocs Ltd)
/// @author Rolf Meyer
#ifndef COMMON_SC_API_H_
#define COMMON_SC_API_H_

#include <systemc>

#define SYSTEMC_VENDOR_ACCELLERA 0
#define SYSTEMC_VENDOR_MENTOR 1
#define SYSTEMC_VENDOR_CADENCE 2

#ifdef SYSTEMC_API
#if SYSTEMC_API != 210 && SYSTEMC_API != 220 && SYSTEMC_API != 230 && SYSTEMC_API != 231
#error "The SYSTEMC_API macro is set to an unknown value. Accepted values are 210, 220, 230 or 231."
#error "Please, see the source code gs_sc_api_detection.h (greenlib) for more information."
#endif
#else

#ifdef NC_SYSTEMC
#error "NC_SYSTEMC"

//#if IEEE_1666_SYSTEMC == 201101L
#define SYSTEMC_API 230
#define SYSTEMC_VENDOR 2
//#endif

#else // Accellera SystemC

#define SYSTEMC_VENDOR 0
// OSCI SystemC 2.1
#if SYSTEMC_VERSION == 20050714
#define SYSTEMC_API 210
#endif
// OSCI SystemC 2.2.0 and 2.2.05jun06
#if SYSTEMC_VERSION == 20070314 || SYSTEMC_VERSION == 20060505
#define SYSTEMC_API 220
#endif
// OSCI SystemC 2.3.0
#if SYSTEMC_VERSION == 20111121 || SYSTEMC_VERSION == 20120701
#define SYSTEMC_API 230
#endif
// OSCI SystemC 2.3.1
#if SYSTEMC_VERSION == 20140417
#define SYSTEMC_API 231
#endif
// SystemC API to use must be defined at this point
// include the detection above for other SystemC vendors to
// define the SYSTEMC_API macro
#ifndef SYSTEMC_API
#error "The SYSTEMC_API macro is not set and could not be detected."
#error "Please, see the source code gs_sc_api_detection.h for more information."
#endif
#endif
#endif

// Extend SystemC with our custom functions.
#include "core/common/sc_find.h"

#endif  // COMMON_SC_API_H_
/// @}
