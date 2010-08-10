// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       icio_payload_extension.h - Defines the payload          *
// *             extension class for communication with icio socket of   *
// *             mmu_cache.                                              *
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision$                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************

#ifndef __ICIO_PAYLOAD_EXTENSION_H__
#define __ICIO_PAYLOAD_EXTENSION_H__

#include "tlm.h"

/// @brief Payload extensions for TLM instruction cache target socket
class icio_payload_extension : public tlm::tlm_extension<icio_payload_extension> {

 public:
  
  typedef tlm::tlm_base_protocol_types::tlm_payload_type tlm_payload_type;
  typedef tlm::tlm_base_protocol_types::tlm_phase_type   tlm_phase_type;

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
