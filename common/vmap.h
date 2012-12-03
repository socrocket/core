// ********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the
// European Space Agency or of TU-Braunschweig.
// ********************************************************************
// Title:      vmap.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
// ********************************************************************

#ifndef VMAP_H
#define VMAP_H

#ifdef __GNUC__
#  ifdef __GNUC_MINOR__
#    if ((__GNUC__ >= 4 && __GNUC_MINOR__ >= 3) && !defined(MTI_SYSTEMC))
#      include <tr1/unordered_map>
#      define vmap std::tr1::unordered_map
#    else
#      include <ext/hash_map>
#      define  vmap __gnu_cxx::hash_map
#    endif
#  else
#    include <ext/hash_map>
#    define  vmap __gnu_cxx::hash_map
#  endif
#else
#  ifdef _WIN32
#    include <hash_map>
#      define  vmap stdext::hash_map
#  else
#    include <map>
#    define  vmap std::map
#  endif
#endif

#endif
