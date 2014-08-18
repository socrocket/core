// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup platforms
/// @{
/// @file leon3mp.h
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
#ifndef PLATFORM_LEON3MP_LEON3MP_H_
#define PLATFORM_LEON3MP_LEON3MP_H_
#include "platforms/base/ambabase.h"
#include "models/utils/clkdevice.h"
#include "common/systemc.h"

#include "models/mmu_cache/lib/mmu_cache.h"
#include "models/ahbin/ahbin.h"
#include "models/memory/memory.h"
#include "models/apbctrl/apbctrl.h"
#include "models/ahbmem/ahbmem.h"
#include "models/mctrl/mctrl.h"
#include "defines.h"
#include "models/gptimer/gptimer.h"
#include "models/apbuart/apbuart.h"
#include "models/apbuart/tcpio.h"
#include "models/apbuart/nullio.h"
#include "models/irqmp/irqmp.h"
#include "models/ahbctrl/ahbctrl.h"
#include "models/ahbprof/ahbprof.h"

#include "leon3.funclt.h"
#include "leon3.funcat.h"

#ifdef HAVE_SOCWIRE
#include "models/socwire/AHB2Socwire.h"
#endif
#ifdef HAVE_GRETH
#include "models/greth/greth/greth.h"
#include "vphy/tapdev.h"
#include "vphy/loopback.h"
#endif

#ifdef HAVE_PYSC
#include "pysc/pysc.h"
#endif


class Leon3mpPlatform : public AMBABasePlatform {
  public:
    Leon3mpPlatform(
        sc_core::sc_module_name mn,
        uint32_t ahbctrl_ioaddr = 0xFFF,   ///< The MSB address of the AHBCtrl I/O area
        uint32_t ahbctrl_iomask = 0xFFF,   ///< The AHBCtrl I/O area address mask
        uint32_t ahbctrl_cfgaddr = 0xFF0,  ///< The MSB address of the AHBCtrl configuration area (PNP)
        uint32_t ahbctrl_cfgmask = 0xFF0,  ///< The address mask of the AHBCtrl configuration area
        bool ahbctrl_rrobin = false,       ///< AHBCtrl arbitration: 1 - round robin, 0 - fixed priority arbitration (only AT)
        bool ahbctrl_split = false,        ///< Enable support for AHB SPLIT response in the AHBCtrl (only AT)
        uint32_t ahbctrl_defmast = 0,      ///< ID of the default master at the AHBCtrl
        bool ahbctrl_ioen = true,          ///< AHBCtrl AHB I/O area enable
        bool ahbctrl_fixbrst = false,      ///< Enable support for fixed-length bursts in the AHBCtrl
        bool ahbctrl_fpnpen = true,        ///< Enable full decoding of PnP configuration records in the AHBCtrl.
        bool ahbctrl_mcheck = true,        ///< Check if there are any intersections between core memory regions in the AHBCtrl.
        uint32_t apbctrl_haddr = 0xfff,    ///< The MSB address of the APBCtrl AHB area. Sets the 12 MSBs in the AHB address
        uint32_t apbctrl_hmask = 0,        ///< The 12bit AHB area address mask of the APBCtrl
        bool apbctrl_mcheck = true,        ///< Check if there are any intersections between APB slave memory regions in the APBCtrl
        uint32_t apbctrl_hindex = 0,       ///< AHB bus index of the APBCtrl
        uint32_t irqmp_paddr = 0,          ///< Upper 12bit of the Irqmp APB address.
        uint32_t irqmp_pmask = 0xFFF,      ///< Upper 12bit of the Irqmp APB mask.
        uint32_t irqmp_ncpu = 2,           ///< Number of CPU which receive interupts stored in Irqmp Status Register.
        uint32_t irqmp_eirq = 1,           ///< Interrupt channel which hides all the extended interrupt channels.
        uint32_t irqmp_pindex = 0,         ///< Irqmp APB slave index.
        bool pow_mon = false,              ///< Enable power monitoring in AHBCtrl and APBCtrl
        amba::amba_layer_ids ambaLayer = amba::amba_LT) : 
    AMBABasePlatform(
        mn,
        ahbctrl_ioaddr,
        ahbctrl_iomask,
        ahbctrl_cfgaddr,
        ahbctrl_cfgmask,
        ahbctrl_rrobin,
        ahbctrl_split,
        ahbctrl_defmast,
        ahbctrl_ioen,
        ahbctrl_fixbrst,
        ahbctrl_fpnpen,
        ahbctrl_mcheck,
        apbctrl_haddr,
        apbctrl_hmask,
        apbctrl_mcheck,
        apbctrl_hindex,
        irqmp_paddr,
        irqmp_pmask,
        irqmp_ncpu,
        irqmp_eirq,
        irqmp_pindex,
        pow_mon,
        ambaLayer) {}
    ~Leon3mpPlatform() {}

    void clkcng() {
      AMBABasePlatform::clkcng();
    }

    void dorst() {
      AMBABasePlatform::dorst();
    }

};
#endif  // PLATFORM_LEON3MP_LEON3MP_H_
///@}
