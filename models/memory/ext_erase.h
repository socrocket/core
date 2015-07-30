// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup memory
/// @{
/// @file ext_erase.h
/// generic payload extension indicating memory to be erased
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Dennis Bode
///

#ifndef EXT_ERASE
#define EXT_ERASE

#include <tlm.h>
#include "core/common/systemc.h"

//simple payload extension for erasing memory
struct ext_erase : public tlm::tlm_extension<ext_erase> {
    public:
//        ext_erase() {
//            erase_flag = 0;
//        }
//        bool erase_flag;

        //must_override pure virtual clone method
        virtual tlm::tlm_extension_base* clone() const {
            ext_erase* t = new ext_erase;
//            t->erase_flag = this->erase_flag;
            return t;
        }

        //must override pure virtual copy_from method
        virtual void copy_from(tlm::tlm_extension_base const &ext) {
//            erase_flag = static_cast<ext_erase const &> (ext).erase_flag;
        }
};

#endif
/// @}