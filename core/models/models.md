IP Models {#models_p}
=====================

One oft the goals were to develop several TLM IP models. 
All mandatory IP models:
Name                                   | Description
-------------------------------------- | ------------------------------------------------------------
@subpage ahbctrl_p "AHBCtrl"           | Aeroflex Gaisler AMBA AHB Controler
@subpage mctrl_p "MCtrl"               | Aeroflex Gaisler GRLIB MCTRL Memory Controller or equivalent
@subpage memory_p "Memory"             | %A memory model working with IP 2
@subpage mmu_cache_p "MMU Cache "      | %A Harvard L1 cache (including support of cache coherence protocols, snooping and write invalidate) and a SPARCv8 MMU or equivalent
@subpage gptimer_p "GPTimer"           | Aeroflex Gaisler GPTIMER General Purpose Timer Unit or equivalent
@subpage irqmp_p "IRQMP"               | Aeroflex Gaisler IRQMP Interrupt Controller or equivalent
@subpage ahbspacewire_p "AHBSpaceWire" | A module which integrates the CSpaceWire Model and the SoCRocket Framework

In addition the following supplementary IP models where designed to make the platform simulation more accurate and usable:

Name                             | Description
-------------------------------- | ------------------------------------------------------------
@subpage apbctrl_p "APBCtrl"     | AMBA AHB to APB Bridge
@subpage ahbmem_p "AHBMem"       | Aeroflex Gaisler GRLIB AHB Memory
@subpage apbuart_p "APBUART"     | Aeroflex Gaisler GRLIB APBUART
@subpage ahbprof_p "AHBProf"     | SystemC AHB System Profiler

On top of that the following IP models were build for lecturing purpose:

Name                                 | Description
------------------------------------ | ----------------------------------------------------------------
@subpage ahbin_p "AHBIn"             | %A simple AHB Inut Device to demonstrate how to build a AHB Master
@subpage ahbout_p "AHBOut"           | %A Device that demonstrates how to build a simple AHB Slave
@startcomment
@subpage ahbcamera_p "AHBCamera"     | %A Video Input Device for Lecture purpose
@subpage ahbdisplay_p "AHBDisplay"   | %A Vidoe Output Device for Lectures
@subpage ahbshuffler_p "AHBShuffler" | %A Picture Shuffler for Lectures
@endcomment
