// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup mmu_cache
/// @{
/// @file dcio_payload_extension.h
/// Defines the payload extension class for communication with dcio socket of
/// mmu_cache.
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef __DCIO_PAYLOAD_EXTENSION_H__
#define __DCIO_PAYLOAD_EXTENSION_H__

#include "tlm.h"

/// @brief Payload extensions for TLM data cache target socket
class dcio_payload_extension : public tlm::tlm_extension<dcio_payload_extension> {

    public:

        typedef tlm::tlm_base_protocol_types::tlm_payload_type tlm_payload_type;
        typedef tlm::tlm_base_protocol_types::tlm_phase_type tlm_phase_type;

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
        /// intended failure
        bool fail;

};

#endif // __DCIO_PAYLOAD_EXTENSION_H__
/// @}