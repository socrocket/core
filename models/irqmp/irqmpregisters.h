/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       irqmpreg.h                                              */
/*             defines addresses, write masks, and default values of   */
/*             all registers of the IRQMP interrupt controller         */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef IRQMP_REGISTER_H
#define IRQMP_REGISTER_H

/// @addtogroup irqmp
/// @{

/***AMBA bus addresses (32 bit)***/

#define IRQMP_AMBA_BASE                (0x00000000)

#define IRQMP_IR_LEVEL                 (IRQMP_AMBA_BASE + 0x00)                    // Prioritization                (internal)
#define IRQMP_IR_PENDING               (IRQMP_AMBA_BASE + 0x04)                    // IR Monitoring / Distribution  
#define IRQMP_IR_FORCE                 (IRQMP_AMBA_BASE + 0x08)                    // IR Monitoring / Distribution  
#define IRQMP_IR_CLEAR                 (IRQMP_AMBA_BASE + 0x0C)                    // IR Monitoring                 
#define IRQMP_MP_STAT                  (IRQMP_AMBA_BASE + 0x10)                    // Processor Control             
#define IRQMP_BROADCAST                (IRQMP_AMBA_BASE + 0x14)                    // IR Distribution               (internal)
#define IRQMP_PROC_IR_MASK(CPU_INDEX)  (IRQMP_AMBA_BASE + 0x40 + 0x4 * CPU_INDEX)  // IR Distribution               
#define IRQMP_PROC_IR_FORCE(CPU_INDEX) (IRQMP_AMBA_BASE + 0x80 + 0x4 * CPU_INDEX)  // IR Monitoring / Distribution  
#define IRQMP_PROC_EXTIR_ID(CPU_INDEX) (IRQMP_AMBA_BASE + 0xC0 + 0x4 * CPU_INDEX)  // IR Monitoring                 


/// register contents (config bit masks)

/// interrupt level register

/// interrupt priority level (0 or 1)
#define IRQMP_IR_LEVEL_IL       (0x0000FFFE) 

///interrupt pending register

/// extended interrupt pending (true or false)
#define IRQMP_IR_PENDING_EIP    (0xFFFE0000) 

/// interrupt pending (true or false)
#define IRQMP_IR_PENDING_IP     (0x0000FFFE) 


/// interrupt force register

/// force interrupt (true or false)
#define IRQMP_IR_FORCE_IF       (0x0000FFFE) 

/// interrupt clear register

/// n=1 to clear interrupt n
#define IRQMP_IR_CLEAR_IC       (0x0000FFFE) 

/// multiprocessor status register

/// number of CPUs in the system
#define IRQMP_MP_STAT_NCPU      (0xF0000000) 

/// interrupt number used for extended interrupts
#define IRQMP_MP_STAT_EIRQ      (0x000F0000)

/// power down status of CPUs (1 = power down)
#define IRQMP_MP_STAT_STAT      (0x00000000 or ncpu) 

/// broadcast register (applicable if NCPU>1)

/// broadcast mask: if n=1, interrupt n is broadcasted
#define IRQMP_BROADCAST_BM      (0x0000FFFE) 

///processor mask register

/// interrupt mask for extended interrupts
#define IRQMP_PROC_MASK_EIM     (0xFFFE0000) 

/// interrupt mask (0 = masked)
#define IRQMP_PROC_MASK_IM      (0x0000FFFE) 

/// processor interrupt force register

/// interrupt force clear
#define IRQMP_PROC_IR_FORCE_IFC (0xFFFE0000) 

/// interrupt force
#define IRQMP_PROC_IR_FORCE_IF  (0x0000FFFE) 

/// extended interrupt identification register

/// ID of the acknowledged extended interrupt (16..31)
#define IRQMP_PROC_EXTIR_ID_EID (0x0000001F) 


/// register default values

/// interrupt level register
#define IRQMP_LEVEL_DEFAULT      (0x00000000) 

/// interrupt pending register
#define IRQMP_PENDING_DEFAULT    (0x00000000) 

/// interrupt force register
#define IRQMP_FORCE_DEFAULT      (0x00000000) 

/// interrupt clear register
#define IRQMP_CLEAR_DEFAULT      (0x00000000) 

/// multiprocessor status register
#define IRQMP_MP_STAT_DEFAULT    (0x00000001) 

/// broadcast register
#define IRQMP_BROADCAST_DEFAULT  (0x00000000) 

/// interrupt mask register
#define IRQMP_MASK_DEFAULT       (0xFFFFFFFE) 

/// processor interrupt force register
#define IRQMP_PROC_FORCE_DEFAULT (0x00000000) 

/// extended interrupt identification register
#define IRQMP_EXTIR_ID_DEFAULT   (0x00000000) 

/// @}
#endif

