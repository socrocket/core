
/* Template boot-code for LEON3 test benches */

#include "config.h"

#ifndef conf_stacksize
#define conf_stacksize 0x00020000
#endif
#ifndef conf_ramsize
#define conf_ramsize   0x00100000
#endif
#ifndef conf_mcfg1
#define conf_mcfg1     0x10380033
#endif
#ifndef conf_mcfg2
#define conf_mcfg2     0xe6B86e60
#endif
#ifndef conf_mcfg3
#define conf_mcfg3     0x000ff000
#endif
#ifndef conf_sdcfg
#define conf_sdcfg     0xe6A06e60
#endif

	.seg	"text"
	.proc	0
	.align	4
	.global start
start:

	flush
	set 0x10e0, %g1		! init IU
	mov %g1, %psr
	mov %g0, %wim
	mov %g0, %tbr
	mov %g0, %y
	mov %g0, %asr16
	nop
	set  0x81000f, %g1
	sta %g1, [%g0] 2
	mov %g0, %g2
	nop
	nop
	nop
	nop
	nop
	or %g2, %g2, %g0
	nop
	nop
	nop
	nop
	nop
#ifdef DSUADDR
	set	DSUADDR, %g2
	st	%g0, [%g2]
	st	%g0, [%g2+0x08]
	st	%g0, [%g2+0x20]
	st	%g0, [%g2+0x24]
	st	%g0, [%g2+0x40]
	st	%g0, [%g2+0x44]
	st	%g0, [%g2+0x50]
	st	%g0, [%g2+0x54]
	st	%g0, [%g2+0x58]
	st	%g0, [%g2+0x5C]
	st	%g0, [%g2+0x54]
#endif

2:
	mov %asr17, %g3
	and %g3, 0x1f, %g3
	mov %g0, %g4
	mov %g0, %g5
	mov %g0, %g6
	mov %g0, %g7
1:
	mov %g0, %l0
	mov %g0, %l1
	mov %g0, %l2
	mov %g0, %l3
	mov %g0, %l4
	mov %g0, %l5
	mov %g0, %l6
	mov %g0, %l7
	mov %g0, %o0
	mov %g0, %o1
	mov %g0, %o2
	mov %g0, %o3
	mov %g0, %o4
	mov %g0, %o5
	mov %g0, %o6
	mov %g0, %o7
	subcc %g3, 1, %g3
	bge 1b
	save

	mov	2, %g1
	mov	%g1, %wim
	set 0x10e0, %g1		! enable traps
	mov %g1, %psr
	nop; nop; nop;

	mov %psr, %g1
	srl %g1, 12, %g1
	andcc %g1, 1, %g0
	be 1f
	nop

	set _fsrxx, %g3
	ld [%g3], %fsr
	ldd [%g3], %f0
	ldd [%g3], %f2
	ldd [%g3], %f4
	ldd [%g3], %f6
	ldd [%g3], %f8
	ldd [%g3], %f10
	ldd [%g3], %f12
	ldd [%g3], %f14
	ldd [%g3], %f16
	ldd [%g3], %f18
	ldd [%g3], %f20
	ldd [%g3], %f22
	ldd [%g3], %f24
	ldd [%g3], %f26
	ldd [%g3], %f28
	ldd [%g3], %f30
	nop
	nop
	nop
	nop
	nop
	faddd %f0, %f2, %f4
	nop
	nop
	nop
	nop
	ba  1f	
	nop
	
	
.align	8
_fsrxx:
	.word 0
	.word 0

1:
	mov %asr17, %g3
	srl %g3, 28, %g3
	andcc %g3, 0x0f, %g3
	bne 1f
!     MEMCFG APB BASE
	set ((conf_apbctrl_ioaddr&conf_apbctrl_iomask)<<20)|((conf_memctrl_apb_addr&conf_memctrl_apb_mask)<<8), %g1
	set conf_mcfg1, %g2
	st  %g2, [%g1]
	set conf_mcfg2, %g2
	st  %g2, [%g1+4]
	set conf_mcfg3, %g2
	st  %g2, [%g1+8]
!     IRQMP APB BASE
!	set ((conf_apbctrl_ioaddr&conf_apbctrl_iomask)<<20)|((conf_irqmp_addr&conf_irqmp_mask)<<8), %g1
!	set 0x0ffff, %g2
!	st  %g2, [%g1+0x10]

! Check if vendor and device id is higher than 0x1009!
	set (((conf_ahbctrl_ioaddr&conf_ahbctrl_iomask)<<20)|((conf_ahbctrl_cfgaddr&conf_ahbctrl_cfgmask)<<8)|(conf_memctrl_ahb_index<<5)), %g1
	ld  [%g1], %g2
	srl %g2, 12, %g2
	set 0x01009, %g1
	subcc %g1, %g2, %g0
	bne 1f

! if newer device ASDCFG == MCFG1 set register to
	set ((conf_apbctrl_ioaddr&conf_apbctrl_iomask)<<20)|((conf_memctrl_apb_addr&conf_memctrl_apb_mask)<<8), %g1
!	set ASDCFG, %g1
	set conf_sdcfg, %g2
	st  %g2, [%g1]

	! %g3 = cpu index
1:	set conf_stacksize, %g2
        mov %g0, %g1
2:	subcc %g3, 0, %g0
	be 3f
	nop
	add %g1, %g2, %g1
	ba 2b
	sub %g3, 1, %g3

#       RAMSTART
3:	set ((conf_memctrl_ram_addr&conf_memctrl_ram_mask)<<20)+ conf_ramsize-32, %fp
	sub %fp, %g1, %fp
	sub %fp, 96, %sp
#     RAMSTART
	set ((conf_memctrl_ram_addr&conf_memctrl_ram_mask)<<20), %g1

	jmp %g1
	nop

.align	32
