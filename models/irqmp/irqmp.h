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
// Title:      irqmp.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file defining the irqmp module template
//             includes implementation file irqmp.tpp at the bottom
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Dennis Bode
// Reviewed:
//*********************************************************************

#ifndef IRQMP_H
#define IRQMP_H

#define CLOCK_PERIOD 2

#include <boost/config.hpp>
#include <systemc.h>
#include <greenreg.h>
#include <greenreg_ambasocket.h>

#include "greencontrol/all.h"

#include "signalkit.h"
/// @addtogroup irqmp IRQMP
/// @{

class CIrqmp : public gs::reg::gr_device, public signalkit::signal_module<
        CIrqmp> {
    public:
        /// Slave socket with delayed switch; responsible for all bus communication
        gs::reg::greenreg_socket<gs::amba::amba_slave<32> > bus;

        /// Reset input signal
        signal<bool>::in rst;

        /// CPU reset out signals
        signal<bool>::selector cpu_rst;

        /// IRQ Request out signals
        signal<uint32_t>::selector irq_req;

        /// IRQ Acknowledge input signals
        signal<uint32_t>::infield irq_ack;

        /// IRQ input signals from other devices
        signal<uint32_t>::infield irq_in;

        GC_HAS_CALLBACKS();

        /// Constructor. Takes vhdl generics as parameters
        CIrqmp(sc_core::sc_module_name name, int _paddr = 0,
               int _pmask = 0xFFF, int _ncpu = 2, int _eirq = 1); // interrupt cascade for extended interrupts
        ~CIrqmp();

        //function prototypes
        void end_of_elaboration();
        void reset_registers(const bool &value, const sc_core::sc_time &time);

        /// bus communication

        /// Write to IR clear register
        void clear_write();

        /// Write to IFC bits of IR force register
        void clear_forced_ir();

        /// Write to MP status register
        void mpstat_write();

        /// One read function for all registers
        void register_read();

        /// Processor communication
        void register_irq(const uint32_t &cleared_irq,
                          const unsigned int &i_cpu, 
                          const sc_core::sc_time &time);

        /// Bus and processor communication

        ///processor communication
        void launch_irq();

        ///processor communication
        void clear_acknowledged_irq(const uint32_t &cleared_irq,
                                    const unsigned int &i_cpu,
                                    const sc_core::sc_time &time);

    private:
        const int ncpu;
        const int eirq;

    public:
        //---register address offset
        static const uint32_t IRQMP_IR_LEVEL = 0x00;
        static const uint32_t IRQMP_IR_PENDING = 0x04;
        static const uint32_t IRQMP_IR_FORCE = 0x08;
        static const uint32_t IRQMP_IR_CLEAR = 0x0C;
        static const uint32_t IRQMP_MP_STAT = 0x10;
        static const uint32_t IRQMP_BROADCAST = 0x14;
        static const uint32_t IRQMP_PROC_IR_MASK(int CPU_INDEX) {
            return (0x40 + 0x4 * CPU_INDEX);
        }
        static const uint32_t IRQMP_PROC_IR_FORCE(int CPU_INDEX) {
            return (0x80 + 0x4 * CPU_INDEX);
        }
        static const uint32_t IRQMP_PROC_EXTIR_ID(int CPU_INDEX) {
            return (0xC0 + 0x4 * CPU_INDEX);
        }

        /// register contents (config bit masks)

        /// interrupt level register

        /// interrupt priority level (0 or 1)
        static const uint32_t IRQMP_IR_LEVEL_IL = 0x0000FFFE;

        ///interrupt pending register

        /// extended interrupt pending (true or false)
        static const uint32_t IRQMP_IR_PENDING_EIP = 0xFFFE0000;

        /// interrupt pending (true or false)
        static const uint32_t IRQMP_IR_PENDING_IP = 0x0000FFFE;

        /// interrupt force register

        /// force interrupt (true or false)
        static const uint32_t IRQMP_IR_FORCE_IF = 0x0000FFFE;

        /// interrupt clear register

        /// n=1 to clear interrupt n
        static const uint32_t IRQMP_IR_CLEAR_IC = 0x0000FFFE;

        /// multiprocessor status register

        /// number of CPUs in the system
        static const uint32_t IRQMP_MP_STAT_NCPU = 0xF0000000;

        /// interrupt number used for extended interrupts
        static const uint32_t IRQMP_MP_STAT_EIRQ = 0x000F0000;

        /// power down status of CPUs (1 = power down)
        inline uint32_t IRQMP_MP_STAT_STAT() const {
            return (0x00000000 or ncpu);
        }

        /// broadcast register (applicable if NCPU>1)

        /// broadcast mask: if n=1, interrupt n is broadcasted
        static const uint32_t IRQMP_BROADCAST_BM = 0x0000FFFE;

        ///processor mask register

        /// interrupt mask for extended interrupts
        static const uint32_t IRQMP_PROC_MASK_EIM = 0xFFFE0000;

        /// interrupt mask (0 = masked)
        static const uint32_t IRQMP_PROC_MASK_IM = 0x0000FFFE;

        /// processor interrupt force register

        /// interrupt force clear
        static const uint32_t IRQMP_PROC_IR_FORCE_IFC = 0xFFFE0000;

        /// interrupt force
        static const uint32_t IRQMP_PROC_IR_FORCE_IF = 0x0000FFFE;

        /// extended interrupt identification register

        /// ID of the acknowledged extended interrupt (16..31)
        static const uint32_t IRQMP_PROC_EXTIR_ID_EID = 0x0000001F;

        /// register default values

        /// interrupt level register
        static const uint32_t IRQMP_LEVEL_DEFAULT = 0x00000000;

        /// interrupt pending register
        static const uint32_t IRQMP_PENDING_DEFAULT = 0x00000000;

        /// interrupt force register
        static const uint32_t IRQMP_FORCE_DEFAULT = 0x00000000;

        /// interrupt clear register
        static const uint32_t IRQMP_CLEAR_DEFAULT = 0x00000000;

        /// multiprocessor status register
        static const uint32_t IRQMP_MP_STAT_DEFAULT = 0x00000001;

        /// broadcast register
        static const uint32_t IRQMP_BROADCAST_DEFAULT = 0x00000000;

        /// interrupt mask register
        static const uint32_t IRQMP_MASK_DEFAULT = 0xFFFFFFFE;

        /// processor interrupt force register
        static const uint32_t IRQMP_PROC_FORCE_DEFAULT = 0x00000000;

        /// extended interrupt identification register
        static const uint32_t IRQMP_EXTIR_ID_DEFAULT = 0x00000000;

};

/// @}

#endif
