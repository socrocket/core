// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_registry
/// @{
/// @file sr_registry.i
/// @date 2013-2015
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
%module sr_registry

%include "usi.i"
%include "std_string.i"
%include "std_set.i"



%{
USI_REGISTER_MODULE(sr_registry);
%}

namespace std {
   %template(set_string) set<std::string>;
}

sc_core::sc_object *create_object_by_name(std::string group, std::string type, std::string name);
bool is_type(std::string group, std::string type, sc_core::sc_object *obj);
std::set<std::string> get_module_files(std::string group);
std::set<std::string> get_module_names(std::string group);
std::set<std::string> get_group_names();
bool load(std::string name);
bool unload(std::string name);

%{
#include "sr_registry.h"

sc_core::sc_object *create_object_by_name(std::string group, std::string type, std::string name) {
  return SrModuleRegistry::create_object_by_name(group, type, name);
}

bool is_type(std::string group, std::string type, sc_core::sc_object *obj) {
  return SrModuleRegistry::is_type(group, type, obj);
}

std::set<std::string> get_module_files(std::string group) {
  return SrModuleRegistry::get_module_files(group);
}

std::set<std::string> get_module_names(std::string group) {
  return SrModuleRegistry::get_module_names(group);
}

std::set<std::string> get_group_names() {
  return SrModuleRegistry::get_group_names();
}

bool load(std::string name) {
  return SrModuleRegistry::load(name);
}

bool unload(std::string name) {
  return SrModuleRegistry::unload(name);
}

%}


