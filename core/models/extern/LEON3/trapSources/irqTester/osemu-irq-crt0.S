/*
 * C startup code for the Fujitsu SPARClite demo board
 *
 * Copyright (c) 1995, 1996 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
#include "asm.h"

    .text
    .align 8

    .globl SYM(_start)
SYM(_start):
    .globl SYM(start)
SYM(start):
        /* Initialization of all the global registers to 0 a part from PSR and WIM
        */
        set     __PSR_INIT, %g1
        wr      %g1, %g0, %psr
        mov     %g0, %tbr
        mov     %g0, %wim
        mov     %g0, %y

        flush

        clr %g1
        clr %g2
        clr %g3
        clr %g4
        clr %g5
        clr %g6
        /* Initialization of all the registers of the registers windows and,
         * finally, initialization of the WIM register itself
        */
        set __WINSIZE, %g2
wl0:
        clr %i0
        clr %i1
        clr %i2
        clr %i3
        clr %i4
        clr %i5
        clr %i6
        clr %i7
        clr %l0
        clr %l1
        clr %l2
        clr %l3
        clr %l4
        clr %l5
        clr %l6
        clr %l7
        save
        subcc   %g2, 1, %g2
        bne wl0
        nop

        wr  %g0, __PSR_INIT, %psr
        set __WIM_INIT, %g3
        mov %g3, %wim

        /* see if the stack is already setup. if not, then default
        *  to the __stack symbol
        */
        cmp     %g0, %sp
        bne      1f
        sethi   %hi(__stack), %g1
        or      %g1, %lo(__stack), %g1
        mov %g1, %sp                ! set the stack pointer
        add     %sp, -1024, %fp
        mov     %fp, %sp
        /*add     %fp, -128, %sp*/
1:

    /* zero the bss section */
        sethi %hi(__bss_start), %g2
        or    %g2, %lo(__bss_start), %g2      ! start of bss
        sethi %hi(_end), %g3
        or    %g3, %lo(_end), %g3         ! end of bss
        mov   %g0, %g1               ! so std has two zeros
zerobss:
        std    %g0, [%g2]
        add    %g2, 8, %g2
        cmp    %g2, %g3
        bleu,a zerobss
        nop

/*
 * Initializes the trap vectors
 */
    set 0xfff8, %l1
    tst %l1                 ! Set condition codes

    set 0x0, %l0                ! Base of new trap vector
    mov %l0, %tbr               ! Install the new tbr
    mov     1, %l1
    sll     %l1, __WINSIZE-1, %l1 ! set the winsize limit
    wr  %g0, %l1, %wim
    nop

    set SYM(win_ovf_trap), %l1      ! Setup window overflow trap
    ldd [%l1], %l2
    std %l2, [%l0 + 5 * 16]
    ldd [%l1 + 8], %l2
    std %l2, [%l0 + 5 * 16 + 8]

    set SYM(win_unf_trap), %l1      ! Setup window underflow trap
    ldd [%l1], %l2
    std %l2, [%l0 + 6 * 16]
    ldd [%l1 + 8], %l2
    std %l2, [%l0 + 6 * 16 + 8]

    set SYM(_start), %l1      ! Setup of the reset vector
    ldd [%l1], %l2
    std %l2, [%l0]
    ldd [%l1 + 8], %l2
    std %l2, [%l0 + 8]

    /* Now lets set up interrupt vectors; all of them use the same interrupt routine */
    set 0xf, %l4
setIrqLoop:
    sub   %l4, 1, %l4
    set SYM(irq_trap), %l1
    ldd [%l1], %l2
    add %l4, %l0, %l4
    sll %l4, 4, %l5
    std %l2, [%l5 + 0x11 * 16]
    ldd [%l1 + 8], %l2
    std %l2, [%l5 + 0x11 * 16 + 8]
    cmp %l4, %g0
    bne setIrqLoop
    nop

/*
 * Try enabling the FPU by setting EF.  If that causes a trap, then we probably
 * don't have an FPU.
 */

    set SYM(no_fpu_trap), %l1       ! Install new trap routine
    ldd [%l1], %l2
    std %l2, [%l0 + 2 * 16]

    mov %psr, %l0
    sethi %hi(0x1000), %l1
    bset %l1, %l0


/*
 * initialize target specific stuff. Only execute these
 * functions it they exist.
 */
init:
    sethi   %hi(SYM(hardware_init_hook)), %g1
        or      %g1,%lo(SYM(hardware_init_hook)),%g1
    cmp %g0,%g1
    be  1f
    nop
    call    SYM(hardware_init_hook)
    nop

1:
    sethi   %hi(SYM(software_init_hook)), %g1
        or      %g1,%lo(SYM(software_init_hook)),%g1
    cmp %g0, %g1
    nop

    /* Initialize the program status register */
    mov %psr, %l0
    or  %l0, 0x020, %l0
    wr  %l0, %psr

    be  2f
    nop
    call    SYM(software_init_hook)
    nop
2:
    /*Enable TRAP, by lowering the PIL*/
    mov %psr, %l0
    and  %l0, 0xFFFFF0FF, %l0
    wr  %l0, %psr
    nop
    call SYM(main)
    nop

    /* call exit from the C library so atexit gets called, and the
     * C++ destructors get run. This calls our exit routine below
     * when it's done.
     */
    call    SYM(exit)
    nop
    call    SYM(_exit)

/*
 * Trap handlers.
 */

    .align 8

SYM(win_ovf_trap):
    /*sethi %hi(SYM(win_ovf)), %l3
    jmpl %lo(SYM(win_ovf))+%l3, %g0
    mov %wim, %l0
    nop*/
	rd  %psr, %l0
	sethi %hi(0), %l4 //sethi  %hi(SYM(win_ovf)), %l4
	jmp  %l4 + SYM(win_ovf)	!  <win_ovf>
 	mov  5, %l3
    nop

    .align 8

SYM(win_unf_trap):
    /*sethi %hi(SYM(win_unf)), %l3
    jmpl %lo(SYM(win_unf))+%l3, %g0
    mov %wim, %l0
    nop*/
 	rd  %psr, %l0
 	sethi  %hi(0), %l4//sethi  %hi(SYM(win_unf)), %l4
 	jmp  %l4 + SYM(win_unf)	! 4000157c <window_underflow_trap_handler>
 	mov  6, %l3
    nop

    .align 8

SYM(irq_trap):
    nop
    sethi  %hi(0), %l4
    jmp  %l4 + SYM(irq_routine)
    nop

    .align 8

SYM(no_fpu_trap):           ! Come here when no fpu exists.
    jmpl %l2, %g0           ! This just skips the
    rett %l2+4          ! offending instruction.
    nop

   .align 8

.globl    .__nop_busy_loop
.__nop_busy_loop:
    b .__nop_busy_loop
