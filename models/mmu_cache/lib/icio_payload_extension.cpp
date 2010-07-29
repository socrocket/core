/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       icio_payload_extension.cpp - Implements the payload     */
/*             extensions for communication with icio socket of        */
/*             mmu_cache.                                              */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#include "icio_payload_extension.h"

// constructor
icio_payload_extension::icio_payload_extension(void) : flush(0), flushl(0), fline(0) { }

// destructor
icio_payload_extension::~icio_payload_extension(void) { }

// override virtual copy_from method
void icio_payload_extension::copy_from(const tlm_extension_base &extension) {

  flush  = static_cast<icio_payload_extension const &>(extension).flush;
  flushl = static_cast<icio_payload_extension const &>(extension).flushl;
  fline  = static_cast<icio_payload_extension const &>(extension).fline;
}

// override virtual clone method
tlm::tlm_extension_base * icio_payload_extension::clone(void) const {

  return new icio_payload_extension(*this);

}
