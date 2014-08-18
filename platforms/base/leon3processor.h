// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup platforms
/// @{
/// @file leon3processor.h
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
#ifndef PLATFORM_BASE_LEON3PROCESSOR_H_
#define PLATFORM_BASE_LEON3PROCESSOR_H_
#include "platforms/base/ambabase.h"
#include "models/utils/clkdevice.h"
#include "common/systemc.h"

#include "models/mmu_cache/lib/mmu_cache.h"
#include "leon3.funclt.h"
#include "leon3.funcat.h"

class Leon3Processor : public sc_core::sc_module, public CLKDevice {
  public:
    Leon3Processor(
        sc_core::sc_module_name mn,

        bool pow_mon = false,              ///< Enable power monitoring in AHBCtrl and APBCtrl
        amba::amba_layer_ids ambaLayer = amba::amba_LT) : 
      sc_core::sc_module(nm) {}
    ~Leon3mpPlatform() {}

    void clkcng() {
    }

    void dorst() {
    }

};
#endif  // PLATFORM_BASE_LEON3PROCESSOR_H_
///@}

