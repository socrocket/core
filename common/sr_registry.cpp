// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file sr_registry.cpp
/// 
/// @date 2013-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
#include "core/common/sr_registry.h"

SrModuleRegistry::map_t SrModuleRegistry::reg = SrModuleRegistry::map_t();

SrModuleRegistry::SrModuleRegistry(std::string type, SrModuleRegistry::generator_f funct, std::string file) : m_funct(funct), m_file(file) {
  SrModuleRegistry::reg.insert(std::make_pair(type, this));
};

sc_core::sc_object *SrModuleRegistry::create_object_by_name(std::string type, std::string name) {
  SrModuleRegistry::map_t::iterator iter = SrModuleRegistry::reg.find(type);
  if(iter != SrModuleRegistry::reg.end()) {
    return iter->second->m_funct(name.c_str());
  }
  return NULL;
}

std::set<std::string> SrModuleRegistry::get_module_files() {
  std::set<std::string> result;
  for(SrModuleRegistry::map_t::iterator iter = SrModuleRegistry::reg.begin(); iter != SrModuleRegistry::reg.end(); ++iter) {
    result.insert(iter->second->m_file);
  }
  return result;
}

/// @}

