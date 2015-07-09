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
    typedef sc_core::sc_object *(*factory_f)(sc_core::sc_module_name);
    typedef bool (*isinstance_f)(sc_core::sc_object *obj);
    SrModuleRegistry(std::string group, std::string type, factory_f factory, isinstance_f isinstance, std::string file);
    void included();
    static sc_core::sc_object *create_object_by_name(std::string group, std::string type, std::string name);
    static std::string get_type_of(sc_core::sc_object *obj);
    static std::set<std::string> get_module_files(std::string group);
    static std::set<std::string> get_module_names(std::string group);
    static std::set<std::string> get_group_names();

  private:
    static map_t &get_group(std::string group);
    static map_map_t *m_members;
    factory_f m_factory;
    isinstance_f m_isinstance;
    const std::string m_file;
};


#define \
  SR_HAS_MODULE_GENERATOR(type, factory, isinstance) \
  static SrModuleRegistry __sr_module_registry_##type##__("module", #type, &factory, &isinstance, __FILE__); \
  volatile SrModuleRegistry *__sr_module_registry_##type = &__sr_module_registry_##type##__;

#define \
  SR_HAS_MODULE(type) \
    sc_core::sc_object *create_##type(sc_core::sc_module_name mn) { \
      return new type(mn); \
    } \
    bool isinstance_of_##type(sc_core::sc_object *obj) { \
      return dynamic_cast<type *>(obj) != NULL; \
    } \
    SR_HAS_MODULE_GENERATOR(type, create_##type, isinstance_of_##type);

#define \
  SR_INCLUDE_MODULE(type) \
  extern SrModuleRegistry *__sr_module_registry_##type; \
  (__sr_module_registry_##type)->included();

#endif  // COMMON_SR_REGISTRY_H_
/// @}
