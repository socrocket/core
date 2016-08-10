// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file sr_registry.cpp
///
/// @date 2013-2014
/// @author Rolf Meyer
/// @copyright
///   Licensed under the Apache License, Version 2.0 (the "License");
///   you may not use this file except in compliance with the License.
///   You may obtain a copy of the License at
///
///       http://www.apache.org/licenses/LICENSE-2.0
///
///   Unless required by applicable law or agreed to in writing, software
///   distributed under the License is distributed on an "AS IS" BASIS,
///   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
///   See the License for the specific language governing permissions and
///   limitations under the License.
#include "sr_registry.h"
#ifndef _WIN32
// Does not work under windows
#include <dlfcn.h>
#endif

SrModuleRegistry::map_map_t *SrModuleRegistry::m_members = NULL;
SrModuleRegistry::lib_map_t *SrModuleRegistry::m_libs = NULL;

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

bool SrModuleRegistry::is_type(std::string groupname, std::string type, sc_core::sc_object *obj) {
  SrModuleRegistry::map_t &group = SrModuleRegistry::get_group(groupname);
  SrModuleRegistry::map_t::iterator item = group.find(type);
  if(item != group.end()) {
    return item->second->m_isinstance(obj);
  }
  return false;
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

bool SrModuleRegistry::load(std::string name) {
#ifndef _WIN32
  if(!SrModuleRegistry::m_libs) {
    SrModuleRegistry::m_libs = new lib_map_t();
  }
  //void * handle = dlopen(name.c_str(), RTLD_NOW | RTLD_GLOBAL);
  void * handle = dlopen(name.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if(!handle) {
    std::cerr << "Could not open file: " << dlerror() << std::endl;
    return false;
  }
  SrModuleRegistry::m_libs->insert(std::make_pair(name, handle));
  return true;
#else
  return false;
#endif
}

bool SrModuleRegistry::unload(std::string name) {
#ifndef _WIN32
  if(!SrModuleRegistry::m_libs) {
    SrModuleRegistry::m_libs = new lib_map_t();
  }
  SrModuleRegistry::lib_map_t::iterator item = SrModuleRegistry::m_libs->find(name);
  if(item == SrModuleRegistry::m_libs->end()) {
    return false;
  }
  int ret = dlclose(item->second);
  if(!ret) {
    return false;
  }
  SrModuleRegistry::m_libs->erase(item);
  return true;
#else
  return false;
#endif
}

/// @}

