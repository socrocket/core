// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file sr_registry.h
/// 
/// @date 2013-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
#ifndef COMMON_SR_REGISTRY_H_
#define COMMON_SR_REGISTRY_H_

#include <string>
#include <map>
#include <set>
#include "core/common/systemc.h"

class SrModuleRegistry {
  public:
    typedef std::map<std::string, SrModuleRegistry *> map_t;
    typedef std::map<std::string, map_t> map_map_t;
    typedef sc_core::sc_object *(*generator_f)(sc_core::sc_module_name);
    SrModuleRegistry(std::string group, std::string type, generator_f funct, std::string file);
    static sc_core::sc_object *create_object_by_name(std::string group, std::string type, std::string name);
    static std::set<std::string> get_module_files(std::string group);

  private:
    static map_map_t m_members;
    generator_f m_funct;
    const std::string m_file;
};


#define \
  SR_HAS_MODULE_GENERATOR(type, funct) \
  static SrModuleRegistry __sr_module_registry_##funct##__("module", type, &funct, __FILE__); \
  volatile SrModuleRegistry *__sr_module_registry_##funct = &__sr_module_registry_##funct##__;

#define \
  SR_HAS_MODULE(type) \
    sc_core::sc_object *create_##type(sc_core::sc_module_name mn) { \
      return new type(mn); \
    } \
    SR_HAS_MODULE_GENERATOR(#type, create_##type);

#endif  // COMMON_SR_REGISTRY_H_
/// @}
