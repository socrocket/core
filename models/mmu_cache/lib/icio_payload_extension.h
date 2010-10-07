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
// Title:      icio_payload_extension.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Defines the payload
//             extension class for communication with icio socket of
//             mmu_cache.
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#ifndef __ICIO_PAYLOAD_EXTENSION_H__
#define __ICIO_PAYLOAD_EXTENSION_H__

#include "tlm.h"

/// @brief Payload extensions for TLM instruction cache target socket
class icio_payload_extension : public tlm::tlm_extension<icio_payload_extension> {

    public:

        typedef tlm::tlm_base_protocol_types::tlm_payload_type tlm_payload_type;
        typedef tlm::tlm_base_protocol_types::tlm_phase_type tlm_phase_type;

        /// constructor
        icio_payload_extension(void);

        /// destructor
        ~icio_payload_extension(void);

        void copy_from(const tlm_extension_base &extension);
        tlm::tlm_extension_base * clone(void) const;

        // extensions
        // ----------
        /// flush instruction cache
        unsigned int flush;
        /// flush instruction cache line
        unsigned int flushl;
        /// line offset in cache flush
        unsigned int fline;
        /// debug information
        unsigned int * debug;
};

#endif // __ICIO_PAYLOAD_EXTENSION_H__
