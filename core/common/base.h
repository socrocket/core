// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common Common
/// @{
/// @file base.h
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
#ifndef COMMON_BASE_H_
#define COMMON_BASE_H_

#include "core/common/systemc.h"
#include "core/common/gs_config.h"
#include "core/common/sr_report.h"

typedef sc_core::sc_module_name ModuleName;
typedef sc_core::sc_module DefaultBase;

typedef gs::cnf::cnf_api ParameterAPI;
typedef gs::cnf::gs_param_array ParameterArray;

//template<typename TYPE>
//using Parameter<TYPE> = gs::cnf::gs_config<TYPE>;

template<class BASE = DefaultBase>
class BaseModule : public BASE {
  public:
    BaseModule(ModuleName mn) :
        BASE(mn),
        m_generics("generics"),
        m_counters("counters"),
        m_power("power") {
      // m_api = gs::cnf::GCnf_Api::getApiInstance(self);
      DefaultBase *self = dynamic_cast<DefaultBase *>(this);
      if(self) {
        m_api = gs::cnf::GCnf_Api::getApiInstance(self);
      } else {
        srError("ConfigBaseModule")
          ("A ConfigBaseModule instance must also inherit from srBaseModule when it gets instantiated.");
      }
      init_generics();
      init_registers();
      init_counters();
      init_power();
    }
    virtual ~BaseModule() {}

    virtual void init_generics() {};
    virtual void init_registers() {};
    virtual void init_counters() {};
    virtual void init_power() {};
    
  protected:
    /// Internal module gs param api instance
    ParameterAPI *m_api;

    /// Configuration generic container
    ParameterArray m_generics;

    /// Performance counter container
    ParameterArray m_counters;

    /// Power counters container
    ParameterArray m_power;
};

#endif  // COMMON_BASE_H_
