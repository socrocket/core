//   GreenConfig framework
//
// LICENSETEXT
//
//   Copyright (C) 2009 : GreenSocs Ltd
// 	 http://www.greensocs.com/ , email: info@greensocs.com
//
//   Developed by :
//   
//   Christian Schroeder <schroeder@eis.cs.tu-bs.de>,
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
//
//
// The contents of this file are subject to the licensing terms specified
// in the file LICENSE. Please consult this file for restrictions and
// limitations that may apply.
// 
// ENDLICENSETEXT


#ifndef __CONFIG1_H__
#define __CONFIG1_H__

//
// This file is the recommended file to include the basic
// GreenConfig service which is located in the namespace
// gs::cnf.
//
// The user simply needs to #include greencontrol/config.h
//
#define BEGIN_GS_CONFIG_NAMESPACE namespace gs { namespace cnf {

#define END_GS_CONFIG_NAMESPACE   }  }

#include "greencontrol/config.h"
#include "gs_config.h"     // parameters
#include "gs_config_class.h"
#endif
