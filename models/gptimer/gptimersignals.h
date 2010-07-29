/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       gptimersignals.h                                        */
/*             header file defining the gptimer signal socket guards   */
/*             usable by all components and models communicating with  */
/*             an gptimer model                                        */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TU Braunschweig              */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef TIMER_SIGNALS_H
#define TIMER_SIGNALS_H

#include "globalsignals.h"

GS_GUARD_ONLY_EXTENSION(dhalt);
GS_GUARD_ONLY_EXTENSION(tick);
GS_GUARD_ONLY_EXTENSION(wdog);

#endif
