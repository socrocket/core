// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file propertyconfig.h
/// 
///
/// @date 2013-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author 
///


#ifndef COMMON_GS_CONFIG_H_
#define COMMON_GS_CONFIG_H_

//
// This file is the recommended file to include the basic
// GreenConfig service which is located in the namespace
// gs::cnf.
//
// The user simply needs to #include greencontrol/config.h
//
#define BEGIN_GS_CONFIG_NAMESPACE namespace gs { namespace cnf {

#define END_GS_CONFIG_NAMESPACE   }  }

#include <greencontrol/config.h>
#include "common/gs_config/gs_config.h"     // parameters
#include "common/gs_config/gs_config_class.h"
#endif  // COMMON_GS_CONFIG_H_
/// @}
