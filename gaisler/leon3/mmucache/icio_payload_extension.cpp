// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file icio_payload_extension.cpp
/// Implements the payload extensions for communication with icio socket of
/// mmu_cache.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include "gaisler/leon3/mmucache/icio_payload_extension.h"

// constructor
icio_payload_extension::icio_payload_extension(void) :
    flush(0), flushl(0), fline(0), debug(NULL), fail(0) {
}

// destructor
icio_payload_extension::~icio_payload_extension(void) {
}

// override virtual copy_from method
void icio_payload_extension::copy_from(const tlm_extension_base &extension) {

    flush = static_cast<icio_payload_extension const &> (extension).flush;
    flushl = static_cast<icio_payload_extension const &> (extension).flushl;
    fline = static_cast<icio_payload_extension const &> (extension).fline;
    debug = static_cast<icio_payload_extension const &> (extension).debug;
    fail = static_cast<icio_payload_extension const &> (extension).fail;
}

// override virtual clone method
tlm::tlm_extension_base * icio_payload_extension::clone(void) const {

    return new icio_payload_extension(*this);

}
/// @}