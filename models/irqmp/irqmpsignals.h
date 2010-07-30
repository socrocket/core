/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       irqmpsignals.h                                          */
/*             header file defining the irqmp signal socket guards     */
/*             usable by all components and models communicating with  */
/*             an IRQMP interrupt controller model                     */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef IRQMPSIGNALS_H
#define IRQMPSIGNALS_H

#include "globalsignals.h"

GS_GUARD_ONLY_EXTENSION(irq0);
GS_GUARD_ONLY_EXTENSION(irq1);
GS_GUARD_ONLY_EXTENSION(irq2);
GS_GUARD_ONLY_EXTENSION(irq3);
GS_GUARD_ONLY_EXTENSION(irq4);
GS_GUARD_ONLY_EXTENSION(irq5);
GS_GUARD_ONLY_EXTENSION(irq6);
GS_GUARD_ONLY_EXTENSION(irq7);
GS_GUARD_ONLY_EXTENSION(irq8);
GS_GUARD_ONLY_EXTENSION(irq9);
GS_GUARD_ONLY_EXTENSION(irq10);
GS_GUARD_ONLY_EXTENSION(irq11);
GS_GUARD_ONLY_EXTENSION(irq12);
GS_GUARD_ONLY_EXTENSION(irq13);
GS_GUARD_ONLY_EXTENSION(irq14);
GS_GUARD_ONLY_EXTENSION(irq15);

GS_GUARD_ONLY_EXTENSION(rst0);
GS_GUARD_ONLY_EXTENSION(rst1);
GS_GUARD_ONLY_EXTENSION(rst2);
GS_GUARD_ONLY_EXTENSION(rst3);
GS_GUARD_ONLY_EXTENSION(rst4);
GS_GUARD_ONLY_EXTENSION(rst5);
GS_GUARD_ONLY_EXTENSION(rst6);
GS_GUARD_ONLY_EXTENSION(rst7);
GS_GUARD_ONLY_EXTENSION(rst8);
GS_GUARD_ONLY_EXTENSION(rst9);
GS_GUARD_ONLY_EXTENSION(rst10);
GS_GUARD_ONLY_EXTENSION(rst11);
GS_GUARD_ONLY_EXTENSION(rst12);
GS_GUARD_ONLY_EXTENSION(rst13);
GS_GUARD_ONLY_EXTENSION(rst14);
GS_GUARD_ONLY_EXTENSION(rst15);
#endif
