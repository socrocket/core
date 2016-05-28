// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup reset_irqmp IRQMP Reset sender
/// @{
/// @file reset_irqmp.cpp
///
/// @date 2016-2016
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer

#include "gaisler/reset_irqmp/reset_irqmp.h"
#include "core/common/sr_registry.h"

SR_HAS_MODULE(ResetIrqmp);

ResetIrqmp::ResetIrqmp(sc_core::sc_module_name mn)
  : sc_core::sc_module(mn), rst("rst") {

  }

void ResetIrqmp::start_of_simulation() {
  rst.write(0);
  rst.write(1);
}


