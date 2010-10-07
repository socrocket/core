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
// Title:      dcio_payload_extension.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Defines the payload
//             extension class for communication with dcio socket of
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

#ifndef __DCIO_PAYLOAD_EXTENSION_H__
#define __DCIO_PAYLOAD_EXTENSION_H__

#include "tlm.h"

/// @brief Payload extensions for TLM data cache target socket
class dcio_payload_extension : public tlm::tlm_extension<dcio_payload_extension> {

 public:

  typedef tlm::tlm_base_protocol_types::tlm_payload_type tlm_payload_type;
  typedef tlm::tlm_base_protocol_types::tlm_phase_type   tlm_phase_type;

  /// constructor
  dcio_payload_extension(void);

  /// destructor
  ~dcio_payload_extension(void);

  void copy_from(const tlm_extension_base &extension);
  tlm::tlm_extension_base * clone(void) const;

  // extensions
  // ----------
  /// address space identifier
  unsigned int asi;
  /// flush data cache
  unsigned int flush;
  /// flush data cache line
  unsigned int flushl;
  /// lock cache line
  unsigned int lock;
  /// debug information
  unsigned int * debug;

};

#endif // __DCIO_PAYLOAD_EXTENSION_H__
