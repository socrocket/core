// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file ahbmaster.cpp
///
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include "models/utils/ahbmaster.h"

#ifndef MTI_SYSTEMC
// #include <greensocket/initiator/multi_socket.h>
#include <greenreg/greenreg.h>
#include <greenreg_ambasockets.h>
#endif
using namespace std;
using namespace sc_core;
using namespace tlm;


/// @}
