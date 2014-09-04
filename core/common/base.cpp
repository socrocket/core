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

#include "core/common/base.h"
#include "core/models/utils/apbdevice.h"

template<>
SCBaseModule<DefaultBase>::SCBaseModule(ModuleName mn, uint32_t register_count) :
  DefaultBase(mn) {
}

template<>
SCBaseModule<APBDevice<DefaultBase> >::SCBaseModule(ModuleName mn, uint32_t register_count) :
  APBDevice<DefaultBase>(mn, register_count) {
}

#ifndef MTI_SYSTEMC

template<>
SCBaseModule<RegisterBase>::SCBaseModule(ModuleName mn, uint32_t register_count) :
  RegisterBase(mn, gs::reg::ALIGNED_ADDRESS, register_count, NULL) {
}

template<>
SCBaseModule<APBDevice<RegisterBase> >::SCBaseModule(ModuleName mn, uint32_t register_count) :
  APBDevice<RegisterBase>(mn, register_count) {
}

#endif

/// @}
