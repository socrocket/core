// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file dcio_payload_extension.cpp
/// Implements the payload extensions for communication with dcio socket of
/// mmu_cache.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///


#include "gaisler/leon3/mmucache/dcio_payload_extension.h"

// constructor
dcio_payload_extension::dcio_payload_extension(void) :
    asi(0), flush(0), flushl(0), lock(0) {
}

// destructor
dcio_payload_extension::~dcio_payload_extension(void) {
}

// override virtual copy_from method
void dcio_payload_extension::copy_from(const tlm_extension_base &extension) {

    asi = static_cast<dcio_payload_extension const &> (extension).asi;
    flush = static_cast<dcio_payload_extension const &> (extension).flush;
    flushl = static_cast<dcio_payload_extension const &> (extension).flushl;
    lock = static_cast<dcio_payload_extension const &> (extension).lock;
    debug = static_cast<dcio_payload_extension const &> (extension).debug;
    fail = static_cast<dcio_payload_extension const &> (extension).fail;
}

// override virtual clone method
tlm::tlm_extension_base * dcio_payload_extension::clone(void) const {

    return new dcio_payload_extension(*this);

}
/// @}