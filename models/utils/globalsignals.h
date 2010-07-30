/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       globalsignals.h                                         */
/*             header file defining the generic signal socket guards   */
/*             usable by all components and models                     */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TU Braunschweig              */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef GLOBALSIGNALS_H
#define GLOBALSIGNALS_H

#include "multisignalhandler.h"

GS_GUARD_ONLY_EXTENSION(rst);
GS_GUARD_ONLY_EXTENSION(set_irq);
GS_GUARD_ONLY_EXTENSION(clr_irq);

#endif
