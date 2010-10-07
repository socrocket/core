//*********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is 
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      gptimerregisters.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file containing the naming macros of all gptimer
//             registers.
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef TIMER_REGISTER_H
#define TIMER_REGISTER_H

/// @addtogroup gptimer
/// @{

#define TIM_AHB_BASE      (0x00000000)
#define TIM_SCALER        (TIM_AHB_BASE+0x00)
#define TIM_SCRELOAD      (TIM_AHB_BASE+0x04)
#define TIM_CONF          (TIM_AHB_BASE+0x08)
#define TIM_VALUE(nr)     (TIM_AHB_BASE+0x10*(nr+1)+0x0)
#define TIM_RELOAD(nr)    (TIM_AHB_BASE+0x10*(nr+1)+0x4)
#define TIM_CTRL(nr)      (TIM_AHB_BASE+0x10*(nr+1)+0x8)

#define TIM_CONF_DF       9
#define TIM_CONF_SI       8
#define TIM_CONF_IQ_MA    0x000000F8
#define TIM_CONF_IQ_OS    3
#define TIM_CONF_NR_MA    0x00000007
#define TIM_CONF_NR_OS    0

#define TIM_CTRL_DH       6
#define TIM_CTRL_CH       5
#define TIM_CTRL_IP       4
#define TIM_CTRL_IE       3
#define TIM_CTRL_LD       2
#define TIM_CTRL_RS       1
#define TIM_CTRL_EN       0

/// @}

#endif
