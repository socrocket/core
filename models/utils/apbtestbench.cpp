/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       apbtestbench.cpp                                        */
/*             header file defining the a generic APB testbench        */
/*             template to use for systemc or vhdl simulation          */
/*             all implementations are included for                    */
/*             maximum inline optiisatzion                             */
/*                                                                     */
/* Modified on $Date: 2010-08-30 00:40:19 +0200 (Mon, 30 Aug 2010) $   */
/*          at $Revision: 84 $                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TU Braunschweig              */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#include "apbtestbench.h"

CAPBTestbench::CAPBTestbench(sc_core::sc_module_name nm)
: sc_core::sc_module(nm), master_sock("socket", amba::amba_APB, amba::amba_LT, false) {}
    
CAPBTestbench::~CAPBTestbench() {}
