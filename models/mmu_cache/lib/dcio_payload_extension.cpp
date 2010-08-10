/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       dcio_payload_extension.cpp - Implements the payload     */
/*             extensions for communication with dcio socket of        */
/*             mmu_cache.                                              */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/


#include "dcio_payload_extension.h"

// constructor
dcio_payload_extension::dcio_payload_extension(void) : asi(0), flush(0), flushl(0), lock(0) {}

// destructor
dcio_payload_extension::~dcio_payload_extension(void) {}

// override virtual copy_from method
void dcio_payload_extension::copy_from(const tlm_extension_base &extension) {

  asi    = static_cast<dcio_payload_extension const &>(extension).asi;
  flush  = static_cast<dcio_payload_extension const &>(extension).flush;
  flushl = static_cast<dcio_payload_extension const &>(extension).flushl;
  lock   = static_cast<dcio_payload_extension const &>(extension).lock;
  debug  = static_cast<dcio_payload_extension const &>(extension).debug;
}

// override virtual clone method
tlm::tlm_extension_base * dcio_payload_extension::clone(void) const {

  return new dcio_payload_extension(*this);

}
