// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file waf.cpp
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <fstream>
#include <string>
#include <utility>
#include <map>
#include "core/common/waf.h"

#ifdef __linux__
#define SYS_PLATFORM "linux2"
#elif __APPLE__
#define SYS_PLATFORM "darwin"
#elif __WIN32__
#define SYS_PLATFORM "win32"
#endif

#if not (defined(MTI_SYSTEMC) and defined(NC_SYSTEMC))
std::map<std::string, std::string> *wafConfigMap = NULL;
#endif

boost::filesystem::path findPath(std::string filename, std::string start) {

  boost::filesystem::path file(filename);
  #if (BOOST_VERSION < 104600)
    boost::filesystem::path path = boost::filesystem::path(start).parent_path();
  #else
    boost::filesystem::path path = boost::filesystem::absolute(boost::filesystem::path(start).parent_path());
  #endif

  while(!boost::filesystem::exists(path/file) && !path.empty()) {
    path = path.parent_path();
  }
  return path/file;
}

#if not (defined(MTI_SYSTEMC) and defined(NC_SYSTEMC))
std::map<std::string, std::string> *readLockFile(std::string top) {
    std::string lockfile(".lock-waf_" SYS_PLATFORM "_build");
    boost::filesystem::path lockpath = findPath(lockfile, top);
    std::ifstream lockstream(lockpath.string().c_str(), std::ios::in);
    std::string lockcontent;
    boost::regex expression("^(#)*?([^#=]*?)\\ =\\ (.*?)$");
    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;
    std::string::const_iterator start, end;
    std::map<std::string, std::string> *result = new std::map<std::string, std::string>();

    lockstream.seekg(0, std::ios::end);
    lockcontent.reserve(lockstream.tellg());
    lockstream.seekg(0, std::ios::beg);

    lockcontent.assign((std::istreambuf_iterator<char>(lockstream)),
                        std::istreambuf_iterator<char>());
    start = lockcontent.begin();
    end = lockcontent.end();
    while(regex_search(start, end, what, expression, flags)) { 
        // what[0] contains the whole string
        // what[1] Comment?
        // what[2] Key
        // what[3] Value
        std::string key(what[2].first, what[2].second);
        std::string val(what[3].first, what[3].second);
        result->insert(std::pair<std::string, std::string>(key, val));
        // update search position:
        start = what[0].second;
        // update flags:
        flags |= boost::match_prev_avail;
        flags |= boost::match_not_bob;
    } 
    return result;
}


std::map<std::string, std::string> *getWafConfig(std::string top) {
    if(!wafConfigMap && top != "") {
        wafConfigMap = readLockFile(top);
    }

    return wafConfigMap;
}
#endif

/// @}

