/***********************************************************************/
/* Project:    Hardware-Software Co-Simulation SoC Validation Platform */
/*                                                                     */
/* File:       irqmpreg.h                                              */
/*             defines addresses, write masks, and default values of   */
/*             all registers of the IRQMP interrupt controller         */
/*                                                                     */
/* Date:       18.05.2010                                              */
/* Revision:   1                                                       */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef IRQMP_REGISTER_H
#define IRQMP_REGISTER_H


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


/***register contents (config bit masks)***/

//interrupt level register
#define IRQMP_IR_LEVEL_IL       (0x0000FFFE) // interrupt priority level (0 or 1)

//interrupt pending register
#define IRQMP_IR_PENDING_EIP    (0xFFFE0000) // extended interrupt pending (true or false)
#define IRQMP_IR_PENDING_IP     (0x0000FFFE) // interrupt pending (true or false)

//interrupt force register
#define IRQMP_IR_FORCE_IF       (0x0000FFFE) // force interrupt (true or false)

//interrupt clear register
#define IRQMP_IR_CLEAR_IC       (0x0000FFFE) // n=1 to clear interrupt n

//multiprocessor status register
#define IRQMP_MP_STAT_NCPU      (0xF0000000) // number of CPUs in the system
#define IRQMP_MP_STAT_EIRQ      (0x000F0000) // interrupt number used for extended interrupts
#define IRQMP_MP_STAT_STAT      (0x00000000 or ncpu) // power down status of CPUs (1 = power down)

//broadcast register (applicable if NCPU>1)
#define IRQMP_BROADCAST_BM      (0x0000FFFE) // broadcast mask: if n=1, interrupt n is broadcasted

//processor mask register
#define IRQMP_PROC_MASK_EIM     (0xFFFE0000) // interrupt mask for extended interrupts
#define IRQMP_PROC_MASK_IM      (0x0000FFFE) // interrupt mask (0 = masked)

//processor interrupt force register
#define IRQMP_PROC_IR_FORCE_IFC (0xFFFE0000) // interrupt force clear
#define IRQMP_PROC_IR_FORCE_IF  (0x0000FFFE) // interrupt force

//extended interrupt identification register
#define IRQMP_PROC_EXTIR_ID_EID (0x0000001F) // ID of the acknowledged extended interrupt (16..31)


/***register default values***/
#define IRQMP_LEVEL_DEFAULT      (0x00000000) // interrupt level register
#define IRQMP_PENDING_DEFAULT    (0x00000000) // interrupt pending register
#define IRQMP_FORCE_DEFAULT      (0x00000000) // interrupt force register
#define IRQMP_CLEAR_DEFAULT      (0x00000000) // interrupt clear register
#define IRQMP_MP_STAT_DEFAULT    (0x00000001) // multiprocessor status register
#define IRQMP_BROADCAST_DEFAULT  (0x00000000) // broadcast register
#define IRQMP_MASK_DEFAULT       (0xFFFFFFFE) // interrupt mask register
#define IRQMP_PROC_FORCE_DEFAULT (0x00000000) // processor interrupt force register
#define IRQMP_EXTIR_ID_DEFAULT   (0x00000000) // extended interrupt identification register

#endif


/*
Offene Frage:
 o Gibt es einen Unterschied zwischen dem "processor interrupt mask register" und dem "processor 1 interrupt mask register"?
 o Gleiche Frage für "interrupt force register"
 o Warum gibt es kein "processor extended interrupt identification register" an Adresse 0xC0?
 o Welchen Sinn hat das "interrupt force register" in einem Einprozessorsystem (NCPU = 0)?
    o Warum gibt es ein eigenes IR force reg für NCPU=0 an Adresse 0x08? Schneller durch Lokalität?
*/

