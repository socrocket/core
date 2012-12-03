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
// Title:      apbuart.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef APBUART_H
#define APBUART_H

#include "apbdevice.h"
#include "clkdevice.h"

#include <greencontrol/all.h>
#include <greenreg_ambasockets.h>
#include <systemc>
#include <boost/config.hpp>

#include "signalkit.h"
#include "verbose.h"

#include "io_if.h"

#include <string>
#include <ostream>
#include <vector>

/// @addtogroup apbuart APBUART
/// @{

/// @brief This class is a TLM 2.0 Model of the Aeroflex Gaisler GRLIB APBUART.
/// Further informations to the original VHDL Modle are available in the GRLIB IP Core User's Manual Section 16
class APBUART : public gs::reg::gr_device, public APBDevice, public CLKDevice {
    public:
        SC_HAS_PROCESS(APBUART);
        SK_HAS_SIGNALS(APBUART);
        GC_HAS_CALLBACKS();
        /// APB Slave socket for all bus communication
        gs::reg::greenreg_socket<gs::amba::amba_slave<32> > bus;

        signal<bool>::out irq;

        sc_event e_irq;

        io_if *m_backend;
        //signal<bool>::selector irq;

        APBUART(sc_core::sc_module_name name, io_if *backend, uint16_t pindex = 0, 
                uint16_t paddr = 0, uint16_t pmask = 4095, int pirq = 0, 
                bool console = false, 
                //bool parity = false, bool flow = false, int fifosize = 0, 
                 bool powmon = false);

        /// Free all counter and unregister all callbacks.
        ~APBUART();

        /// Execute the callback registering when systemc reaches the end of elaboration.
        void end_of_elaboration();

        
      // Register Callbacks
        void data_read();

        void data_write();

        void status_read();

        void send_irq();

      // Signal Callbacks
        virtual void dorst();

        const uint32_t powermon;

        static const uint32_t DATA            = 0x00000000;
        static const uint32_t STATUS          = 0x00000004;
        static const uint32_t CONTROL         = 0x00000008;
        static const uint32_t SCALER          = 0x0000000C;
        
        static const uint32_t DATA_DEFAULT    = 0x0;
        static const uint32_t STATUS_DEFAULT  = 0x00000006;
        static const uint32_t CONTROL_DEFAULT = 0x80000000;
        static const uint32_t SCALER_DEFAULT  = 0x0;

        static const uint32_t DATA_MASK       = 0x000000FF;
        static const uint32_t STATUS_MASK     = 0x00000000;
        static const uint32_t CONTROL_MASK    = 0x00007FFF;
        static const uint32_t SCALER_MASK     = 0x00000FFF;
};

/// @}

#endif // APBUART_H
