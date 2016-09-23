#ifndef CORE_SR_ISS_INTSINSICS_PLATFORMINSTRINSIC_H
#define CORE_SR_ISS_INTSINSICS_PLATFORMINSTRINSIC_H

#include "core/base/systemc.h"
#include "core/trapgen/modules/abi_if.hpp"
#include "core/sr_iss/intrinsics/intrinsicbase.h"

///Base class for each emulated system call;
///Operator () implements the behaviour of the
///emulated call
template<class wordSize>
class PlatformIntrinsic
#ifndef SWIG
  : public sc_core::sc_object
#endif
{
  public:
#ifndef SWIG
    trap::ABIIf<wordSize> *m_processor;
    IntrinsicBase *m_manager;
    sc_time latency;
    /// SWIG needs a default constructor
    PlatformIntrinsic(sc_core::sc_module_name mn = "intrinsics") : sc_core::sc_object(mn), m_processor(NULL), m_manager(NULL), latency(SC_ZERO_TIME) {}
    virtual ~PlatformIntrinsic() {}

    virtual void setProcessor(trap::ABIIf<wordSize> *processor) { this->m_processor = processor; }
    virtual void setManager(IntrinsicBase *manager) { this->m_manager = manager; }
    virtual bool operator()() { return false; };
#endif
    virtual void setLatency(sc_time &latency) { this->latency = latency; }
};

#define SR_HAS_INTRINSIC_GENERATOR(type, factory, isinstance) \
  static SrModuleRegistry __sr_module_registry_##type##__("PlatformIntrinsic", #type, &factory, &isinstance, __FILE__); \
  volatile SrModuleRegistry *__sr_module_registry_##type = &__sr_module_registry_##type##__;

#define SR_HAS_INTRINSIC(type) \
    sc_core::sc_object *create_##type(sc_core::sc_module_name mn) { \
      return new type(mn); \
    } \
    bool isinstance_of_##type(sc_core::sc_object *obj) { \
      return dynamic_cast<type *>(obj) != NULL; \
    } \
    SR_HAS_INTRINSIC_GENERATOR(type, create_##type, isinstance_of_##type);

#endif  // CORE_SR_ISS_INTSINSICS_PLATFORMINSTRINSIC_H
