// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file waf.h
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
#ifndef COMMON_WAF_H_
#define COMMON_WAF_H_

#include <string>
#include <map>

boost::filesystem::path findPath(std::string file, std::string start);
std::map<std::string, std::string> *readLockFile(std::string top);
std::map<std::string, std::string> *getWafConfig(std::string top = "");

#endif  // COMMON_WAF_H_
/// @}
