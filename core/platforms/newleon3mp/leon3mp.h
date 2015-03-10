// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup platform
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
#include "core/platforms/base/ambabase.h"
#include "core/common/clkdevice.h"
#include "core/common/systemc.h"

#include "core/models/mmu_cache/lib/leon3_mmu_cache.h"
#include "core/models/mmu_cache/lib/mmu_cache.h"
#include "core/models/ahbin/ahbin.h"
#include "core/models/memory/memory.h"
#include "core/models/apbctrl/apbctrl.h"
#include "core/models/ahbmem/ahbmem.h"
#include "core/models/mctrl/mctrl.h"
#include "core/models/mmu_cache/lib/defines.h"
#include "core/models/gptimer/gptimer.h"
#include "core/models/apbuart/apbuart.h"
#include "core/models/apbuart/tcpio.h"
#include "core/models/apbuart/nullio.h"
#include "core/models/irqmp/irqmp.h"
#include "core/models/ahbctrl/ahbctrl.h"
#include "core/models/ahbprof/ahbprof.h"

#include "core/models/extern/LEON3/simulatorSources/leon3.funclt.h"

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
        uint32_t ahbctrl_ioaddr = 0xFFF,              ///< The MSB address of the AHBCtrl I/O area
        uint32_t ahbctrl_iomask = 0xFFF,              ///< The AHBCtrl I/O area address mask
        uint32_t ahbctrl_cfgaddr = 0xFF0,             ///< The MSB address of the AHBCtrl configuration area (PNP)
        uint32_t ahbctrl_cfgmask = 0xFF0,             ///< The address mask of the AHBCtrl configuration area
        bool ahbctrl_rrobin = false,                  ///< AHBCtrl arbitration: 1 - round robin, 0 - fixed priority arbitration (only AT)
        bool ahbctrl_split = false,                   ///< Enable support for AHB SPLIT response in the AHBCtrl (only AT)
        uint32_t ahbctrl_defmast = 0,                 ///< ID of the default master at the AHBCtrl
        bool ahbctrl_ioen = true,                     ///< AHBCtrl AHB I/O area enable
        bool ahbctrl_fixbrst = false,                 ///< Enable support for fixed-length bursts in the AHBCtrl
        bool ahbctrl_fpnpen = true,                   ///< Enable full decoding of PnP configuration records in the AHBCtrl.
        bool ahbctrl_mcheck = true,                   ///< Check if there are any intersections between core memory regions in the AHBCtrl.
        uint32_t apbctrl_haddr = 0xfff,               ///< The MSB address of the APBCtrl AHB area. Sets the 12 MSBs in the AHB address
        uint32_t apbctrl_hmask = 0,                   ///< The 12bit AHB area address mask of the APBCtrl
        bool apbctrl_mcheck = true,                   ///< Check if there are any intersections between APB slave memory regions in the APBCtrl
        uint32_t apbctrl_hindex = 0,                  ///< AHB bus index of the APBCtrl
        uint32_t irqmp_paddr = 0,                     ///< Upper 12bit of the Irqmp APB address.
        uint32_t irqmp_pmask = 0xFFF,                 ///< Upper 12bit of the Irqmp APB mask.
        uint32_t irqmp_ncpu = 2,                      ///< Number of CPU which receive interupts stored in Irqmp Status Register.
        uint32_t irqmp_eirq = 1,                      ///< Interrupt channel which hides all the extended interrupt channels.
        uint32_t irqmp_pindex = 0,                    ///< Irqmp APB slave index.
        bool leon3_mmu_cache_ic_en = true,            ///< instruction cache enable
        uint32_t leon3_mmu_cache_irepl = 1,           ///< instruction cache replacement strategy
        uint32_t leon3_mmu_cache_isets = 4,           ///< number of instruction cache sets
        uint32_t leon3_mmu_cache_ilinesize = 8,       ///< instruction cache line size (in bytes)
        uint32_t leon3_mmu_cache_isetsize = 8,        ///< size of an instruction cache set (in kbytes)
        uint32_t leon3_mmu_cache_isetlock = true,     ///< enable instruction cache locking
        uint32_t leon3_mmu_cache_dcen = true,         ///< data cache enable
        uint32_t leon3_mmu_cache_drepl = 1,           ///< data cache replacement strategy
        uint32_t leon3_mmu_cache_dsets = 2,           ///< number of data cache sets
        uint32_t leon3_mmu_cache_dlinesize = 4,       ///< data cache line size (in bytes)
        uint32_t leon3_mmu_cache_dsetsize = 8,        ///< size of a data cache set (in kbytes)
        bool leon3_mmu_cache_dsetlock = true,         ///< enable data cache locking
        bool leon3_mmu_cache_dsnoop = true,           ///< enable data cache snooping
        bool leon3_mmu_cache_ilram = false,           ///< enable instruction scratch pad
        uint32_t leon3_mmu_cache_ilramsize = 0x000,   ///< size of the instruction scratch pad (in kbytes)
        uint32_t leon3_mmu_cache_ilramstart = 0x000,  ///< start address of the instruction scratch pad
        uint32_t leon3_mmu_cache_dlram = false,       ///< enable data scratch pad
        uint32_t leon3_mmu_cache_dlramsize = 0x000,   ///< size of the data scratch pad (in kbytes)
        uint32_t leon3_mmu_cache_dlramstart = 0x000,  ///< start address of the data scratch pad
        uint32_t leon3_mmu_cache_cached = 0,          ///< fixed cacheability mask
        bool leon3_mmu_en = true,                     ///< mmu enable
        uint32_t leon3_mmu_cache_itlb_num = 8,        ///< number of instruction TLBs
        uint32_t leon3_mmu_cache_dtlb_num = 8,        ///< number of data TLBs
        uint32_t leon3_mmu_cache_tlb_type = 0,        ///< split or shared instruction and data TLBs
        uint32_t leon3_mmu_cache_tlb_rep = 1,         ///< TLB replacement strategy
        uint32_t leon3_mmu_cache_mmupgsz = 0,         ///< MMU page size
        uint32_t leon3_mmu_cache_hindex = 0,          ///< ID of the bus master
        bool pow_mon = false,                         ///< Enable power monitoring in AHBCtrl and APBCtrl
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
        ambaLayer) {

      connect(irqmp.irq_req, leon3.cpu.IRQ_port.irq_signal);
      connect(leon3.cpu.irqAck.initSignal, irqmp.irq_ack);
      connect(leon3.cpu.irqAck.run, irqmp.cpu_rst);
      connect(leon3.cpu.irqAck.status, irqmp.cpu_stat);
    }
    ~Leon3mpPlatform() {}

    void clkcng() {
      AMBABasePlatform::clkcng();
    }

    void dorst() {
      AMBABasePlatform::dorst();
    }
  private:
    leon3_mmu_cache leon3;

};
#endif  // PLATFORM_LEON3MP_LEON3MP_H_
///@}
