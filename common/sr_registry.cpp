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

SrModuleRegistry::map_map_t *SrModuleRegistry::m_members = NULL;

SrModuleRegistry::map_t &SrModuleRegistry::get_group(std::string group) {
  if(!SrModuleRegistry::m_members) {
    SrModuleRegistry::m_members = new map_map_t();
  }
  SrModuleRegistry::map_map_t::iterator iter = SrModuleRegistry::m_members->find(group);
  if(iter == SrModuleRegistry::m_members->end()) {
    SrModuleRegistry::m_members->insert(std::make_pair(group, SrModuleRegistry::map_t()));
    iter = SrModuleRegistry::m_members->find(group);
  }
  return iter->second;
}

SrModuleRegistry::SrModuleRegistry(std::string groupname, std::string type, SrModuleRegistry::generator_f funct, std::string file) : 
  m_funct(funct), m_file(file) {
  SrModuleRegistry::map_t &group = SrModuleRegistry::get_group(groupname);
  group.insert(std::make_pair(type, this));
};

sc_core::sc_object *SrModuleRegistry::create_object_by_name(std::string groupname, std::string type, std::string name) {
  SrModuleRegistry::map_t &group = SrModuleRegistry::get_group(groupname);
  SrModuleRegistry::map_t::iterator item = group.find(type);
  if(item != group.end()) {
    return item->second->m_funct(name.c_str());
  }
  return NULL;
}

std::set<std::string> SrModuleRegistry::get_module_files(std::string groupname) {
  std::set<std::string> result;
  SrModuleRegistry::map_t &group = SrModuleRegistry::get_group(groupname);
  for(SrModuleRegistry::map_t::iterator item = group.begin(); item != group.end(); ++item) {
    result.insert(item->second->m_file);
  }
  return result;
}

/// @}

