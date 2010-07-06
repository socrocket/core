/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       defines.h - Header file with global defines, to be      */
/*             used by the current testbench.                          */
/*                                                                     */
/* Modified on $Date$                                                  */
/*          at $Revision$                                              */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#ifndef __DEFINES_H__

#define DUMP(name, msg) std::cout<<"@"<<sc_core::sc_time_stamp()<<" /"<<(unsigned)sc_core::sc_delta_count()<<" ("<<name  <<"): "<<msg<<std::endl

#define LOCAL_CLOCK 10
#define CACHE_MASTER_ID 1

#endif // __DEFINES_H__
