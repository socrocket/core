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

void SrModuleRegistry::included() {
}

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

SrModuleRegistry::SrModuleRegistry(std::string groupname, std::string type, SrModuleRegistry::factory_f factory, SrModuleRegistry::isinstance_f isinstance, std::string file) : 
  m_factory(factory), m_isinstance(isinstance), m_file(file) {
  SrModuleRegistry::map_t &group = SrModuleRegistry::get_group(groupname);
  group.insert(std::make_pair(type, this));
};

sc_core::sc_object *SrModuleRegistry::create_object_by_name(std::string groupname, std::string type, std::string name) {
  SrModuleRegistry::map_t &group = SrModuleRegistry::get_group(groupname);
  SrModuleRegistry::map_t::iterator item = group.find(type);
  if(item != group.end()) {
    return item->second->m_factory(name.c_str());
  }
  return NULL;
}

std::string SrModuleRegistry::get_type_of(sc_core::sc_object *obj) {
  if(!SrModuleRegistry::m_members) {
    SrModuleRegistry::m_members = new map_map_t();
  }
  for(SrModuleRegistry::map_map_t::iterator group = m_members->begin(); group != m_members->end(); ++group) {
    for(SrModuleRegistry::map_t::iterator item = group->second.begin(); item != group->second.end(); ++item) {
      if(item->second->m_isinstance(obj)) {
        return group->first + std::string(".") + item->first;
      }
    }
  }
  return std::string("");
}

std::set<std::string> SrModuleRegistry::get_module_files(std::string groupname) {
  std::set<std::string> result;
  SrModuleRegistry::map_t &group = SrModuleRegistry::get_group(groupname);
  for(SrModuleRegistry::map_t::iterator item = group.begin(); item != group.end(); ++item) {
    result.insert(item->second->m_file);
  }
  return result;
}

std::set<std::string> SrModuleRegistry::get_module_names(std::string groupname) {
  std::set<std::string> result;
  SrModuleRegistry::map_t &group = SrModuleRegistry::get_group(groupname);
  for(SrModuleRegistry::map_t::iterator item = group.begin(); item != group.end(); ++item) {
    result.insert(item->first);
  }
  return result;
}

std::set<std::string> SrModuleRegistry::get_group_names() {
  std::set<std::string> result;
  if(!SrModuleRegistry::m_members) {
    SrModuleRegistry::m_members = new map_map_t();
  }
  for(SrModuleRegistry::map_map_t::iterator item = m_members->begin(); item != m_members->end(); ++item) {
    result.insert(item->first);
  }
  return result;
}

/// @}

