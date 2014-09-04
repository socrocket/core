/* Window size */
#include "core/models/extern/LEON3/trapSources/irqTester/asm.h"

    .text

    .align 8
/*Function for the initialization of the processor: they get called if different
from NULL: you can define your own here if you need*/
.globl hardware_init_hook
hardware_init_hook = 0

    .align 8
.globl SYM(software_init_hook)
SYM(software_init_hook):
    save  %sp, -104, %sp
    nop
    call    SYM(_init)
    nop
    restore
    retl
    nop
    nop

    .align 8
.globl SYM(win_ovf)
        /* a SAVE instruction caused a trap */
SYM(win_ovf):
    rd  %wim, %l0
/*    mov  %g1, %l7
    srl  %l3, 1, %g1
    sll  %l3, 7, %l4
    or  %l4, %g1, %g1
    save
    mov  %g1, %wim
    nop
    nop
    nop
    std  %l0, [ %sp ]
    std  %l2, [ %sp + 8 ]
    std  %l4, [ %sp + 0x10 ]
    std  %l6, [ %sp + 0x18 ]
    std  %i0, [ %sp + 0x20 ]
    std  %i2, [ %sp + 0x28 ]
    std  %i4, [ %sp + 0x30 ]
    std  %fp, [ %sp + 0x38 ]
    restore
    nop
    mov  %l7, %g1
    jmp  %l1
    rett  %l2
    nop
    nop
    nop*/
    mov %g1, %l3        ! Save g1, we use it to hold the wim
    srl %l0, 1, %g1     ! Rotate wim right
    sll %l0, __WINSIZE-1, %l0
    or  %l0, %g1, %g1

    mov %g0, %wim       ! disable wim
    nop
    nop
    nop
    save    %g0, %g0, %g0       ! Slip into next window
    nop
    nop
    std %l0, [%sp + 0 * 4]  ! save L & I registers
    std %l2, [%sp + 2 * 4]
    std %l4, [%sp + 4 * 4]
    std %l6, [%sp + 6 * 4]

    std %i0, [%sp + 8 * 4]
    std %i2, [%sp + 10 * 4]
    std %i4, [%sp + 12 * 4]
    std %i6, [%sp + 14 * 4]

    restore             ! Go back to trap window.
    nop
    nop
    nop
    mov %g1, %wim       ! Install the new wim
    mov %l3, %g1        ! Restore %g1
    nop
    jmpl    %l1,  %g0
    rett    %l2
    nop
    nop
/*        .end win_ovf*/

    .align 8

.globl SYM(win_unf)
        /* a RESTORE instruction caused a trap */
SYM(win_unf):
    rd  %wim, %l0
/*    sll  %l3, 1, %l4
    srl  %l3, 7, %l5
    or  %l5, %l4, %l5
    mov  %l5, %wim
    nop
    nop
    nop
    restore
    nop
    nop
    nop
    restore
    ldd  [ %sp ], %l0
    ldd  [ %sp + 8 ], %l2
    ldd  [ %sp + 0x10 ], %l4
    ldd  [ %sp + 0x18 ], %l6
    ldd  [ %sp + 0x20 ], %i0
    ldd  [ %sp + 0x28 ], %i2
    ldd  [ %sp + 0x30 ], %i4
    ldd  [ %sp + 0x38 ], %fp
    save
    nop
    nop
    nop
    save
    nop
    nop
    jmp  %l1
    rett  %l2
    nop
    nop
    nop*/
    sll %l0, 1, %l3     ! Rotate wim left
    srl %l0, __WINSIZE-1, %l0
    or  %l0, %l3, %l0

    mov %l0, %l3
    mov %g0, %wim       ! disable wim

    nop
    nop
    restore             ! User's window
    nop
    nop
    nop
    restore             ! His caller's window

    ldd [%sp + 0 * 4], %l0  ! restore L & I registers
    ldd [%sp + 2 * 4], %l2
    ldd [%sp + 4 * 4], %l4
    ldd [%sp + 6 * 4], %l6

    ldd [%sp + 8 * 4], %i0
    ldd [%sp + 10 * 4], %i2
    ldd [%sp + 12 * 4], %i4
    ldd [%sp + 14 * 4], %i6

    save    %g0, %g0, %g0       ! Back to trap window
    nop
    nop
    nop
    save    %g0, %g0, %g0
    nop
    nop
    nop
    mov %l3, %wim       ! Install the new wim
    nop
    nop
    jmpl    %l1,  %g0
    rett    %l2
    nop
    nop
/*        .end win_unf*/

    .align 8

/* interrupt handler: it simply calls the isrHandler which has to be defines in the C file*/
.globl SYM(irq_routine)
SYM(irq_routine):
    nop
    nop
    mov %tbr, %g5
    mov SYM(myData), %l4
    and  %g5, 0xf0, %g5
    sra  %g5, 4, %g5
    add  %g5, 0x30, %g5
    stb  %g5, [ %l4 ]
    sll  %g5, 0x18, %g5
    sra  %g5, 0x18, %g5
    cmp  %g5, 0x39
    ble  goAndPrint
    nop
    add  %g5, 7, %g5
    stb  %g5, [ %l4 ]
goAndPrint:
    mov  2, %o0
    mov SYM(myData), %l4
    add %l4, 4, %o1
    mov  1, %o2
    call  SYM(write)
    nop
    mov  2, %o0
    mov SYM(myData), %o1
    mov  1, %o2
    call  SYM(write)
    nop
    mov  2, %o0
    mov SYM(myData), %l4
    add %l4, 4, %o1
    mov  1, %o2
    call  SYM(write)
    nop
    nop
    nop
    jmpl    %l1,  %g0
    rett    %l2
    nop
    nop
    nop
    nop
    nop
    nop
    nop

SYM(myData):
.long   0
.byte '\n'
.byte '\n'
.byte '\n'
.byte '\n'

.align 8
