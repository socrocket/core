#ifndef CONFIG_H
#define CONFIG_H

#define conf 
#define conf_sys 
#define conf_sys_lt_at true
#define conf_sys_clock 10
#define conf_sys_gdb true
#define conf_sys_timing true
#define conf_sys_power false
#define conf_ahbctrl 
#define conf_ahbctrl_ioaddr 0xFFF
#define conf_ahbctrl_iomask 0xFFF
#define conf_ahbctrl_cfgaddr 0xFF0
#define conf_ahbctrl_cfgmask 0xFF0
#define conf_ahbctrl_rrobin false
#define conf_ahbctrl_defmast 0
#define conf_ahbctrl_ioen true
#define conf_ahbctrl_fixbrst false
#define conf_ahbctrl_split false
#define conf_ahbctrl_fpnpen true
#define conf_ahbctrl_mcheck true
#define conf_apbctrl 
#define conf_apbctrl_haddr 0x800
#define conf_apbctrl_hmask 0xFFF
#define conf_apbctrl_index 2
#define conf_apbctrl_check true
#define conf_mmu_cache 
#define conf_mmu_cache_icen false
#define conf_mmu_cache_dcen false
#define conf_mmu_cache_ilram false
#define conf_mmu_cache_dlram false
#define conf_mmu_cache_cached 65535
#define conf_mmu_cache_index 0
#define conf_mmu_cache_mmu_en false
#define conf_ahbmem true
#define conf_ahbmem_index 1
#define conf_ahbmem_addr 0xA00
#define conf_ahbmem_mask 0xFFF
#define conf_gptimer true
#define conf_gptimer_addr 0x003
#define conf_gptimer_mask 0xFFF
#define conf_gptimer_index 3
#define conf_gptimer_pirq 8
#define conf_gptimer_sepirq true
#define conf_gptimer_ntimers 7
#define conf_gptimer_sbits 16
#define conf_gptimer_nbits 32
#define conf_gptimer_wdog 0
#define conf_memctrl 
#define conf_memctrl_index 0
#define conf_memctrl_apb 
#define conf_memctrl_apb_addr 0
#define conf_memctrl_apb_mask 4095
#define conf_memctrl_apb_index 0
#define conf_memctrl_prom true
#define conf_memctrl_prom_addr 0
#define conf_memctrl_prom_mask 3584
#define conf_memctrl_prom_asel 28
#define conf_memctrl_prom_banks 2
#define conf_memctrl_prom_bsize 256
#define conf_memctrl_prom_width 32
#define conf_memctrl_io false
#define conf_memctrl_ram 
#define conf_memctrl_ram_addr 1024
#define conf_memctrl_ram_mask 3072
#define conf_memctrl_ram_wprot false
#define conf_memctrl_ram_asel 29
#define conf_memctrl_ram_sram true
#define conf_memctrl_ram_sram_banks 4
#define conf_memctrl_ram_sram_bsize 128
#define conf_memctrl_ram_sram_width 32
#define conf_memctrl_ram_sdram true
#define conf_memctrl_ram_sdram_banks 2
#define conf_memctrl_ram_sdram_bsize 256
#define conf_memctrl_ram_sdram_width 32
#define conf_memctrl_ram_sdram_cols 16
#define conf_memctrl_ram8 true
#define conf_memctrl_ram16 true
#define conf_memctrl_sden true
#define conf_memctrl_sepbus false
#define conf_memctrl_sdbits 32
#define conf_memctrl_mobile 0
#define conf_socwire false
#define conf_inputdev false
#define conf_irqmp 
#define conf_irqmp_addr 0x002
#define conf_irqmp_mask 0xFFF
#define conf_irqmp_index 2
#define conf_irqmp_ncpu 1
#define conf_irqmp_eirq 4
#define conf_uart_addr 0x001
#define conf_uart_mask 0xFFF


#endif // CONFIG_H
