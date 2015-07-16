#ifndef CORE_COMMON_SR_ISS_INTSINSICS_PLATFORMINSTRINSIC_H
#define CORE_COMMON_SR_ISS_INTSINSICS_PLATFORMINSTRINSIC_H

#include "core/common/systemc.h"
#include "core/common/trapgen/ABIIf.hpp"
#include "core/common/sr_iss/intrinsics/intrinsicbase.h"

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

#endif  // CORE_COMMON_SR_ISS_INTSINSICS_PLATFORMINSTRINSIC_H
