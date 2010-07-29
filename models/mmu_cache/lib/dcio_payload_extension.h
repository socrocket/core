/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       dcio_payload_extension.h - Defines the payload          */
/*             extension class for communication with dcio socket of   */
/*             mmu_cache.                                              */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#ifndef __DCIO_PAYLOAD_EXTENSION_H__
#define __DCIO_PAYLOAD_EXTENSION_H__

#include "tlm.h"

class dcio_payload_extension : public tlm::tlm_extension<dcio_payload_extension> {

 public:
  
  typedef tlm::tlm_base_protocol_types::tlm_payload_type tlm_payload_type;
  typedef tlm::tlm_base_protocol_types::tlm_phase_type   tlm_phase_type;

  // constructor
  dcio_payload_extension(void);

  // destructor
  ~dcio_payload_extension(void);

  void copy_from(const tlm_extension_base &extension);
  tlm::tlm_extension_base * clone(void) const;

  // extensions
  // ----------
  // address space identifier
  unsigned int asi;
  // flush data cache
  unsigned int flush;
  // flush data cache line
  unsigned int flushl;
  // lock cache line
  unsigned int lock;
  

};

#endif // __DCIO_PAYLOAD_EXTENSION_H__
