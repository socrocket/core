// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file vmap.h
///
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef COMMON_VMAP_H_
#define COMMON_VMAP_H_

#ifdef __GNUC__
#ifdef __GNUC_MINOR__
#if ((__GNUC__ >= 4 && __GNUC_MINOR__ >= 3) && !defined(MTI_SYSTEMC))
#include <tr1/unordered_map>
#define vmap std::tr1::unordered_map
#else
#include <ext/hash_map>
#define  vmap __gnu_cxx::hash_map
#endif
#else
#include <ext/hash_map>  // NOLINT(build/include)
#define  vmap __gnu_cxx::hash_map
#endif
#else  // ifdef __GNUC__
#ifdef _WIN32
#include <hash_map>
#define  vmap stdext::hash_map
#else
#include <map>
#define  vmap std::map
#endif
#endif

#endif  // COMMON_VMAP_H_
/// @}
