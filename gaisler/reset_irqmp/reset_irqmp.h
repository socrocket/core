// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup reset_irqmp IRQMP Reset sender
/// @{
/// @file reset_irqmp.h
///
/// @date 2016-2016
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
#ifndef MODELS_RESET_IRQMP_RESET_IRQMP_H_
#define MODELS_RESET_IRQMP_RESET_IRQMP_H_

#include "core/common/systemc.h"
#include "core/common/base.h"
#include "core/common/sr_signal.h"

class ResetIrqmp : public DefaultBase {
  public:
    SR_HAS_SIGNALS(ResetIrqmp);
    signal<bool>::out rst;
    ResetIrqmp(sc_core::sc_module_name mn);
    void start_of_simulation();
};

#endif  // MODELS_RESET_IRQMP_RESET_IRQMP_H_
