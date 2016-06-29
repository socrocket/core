// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_signal
/// @{
/// @file base.h
/// This file contains the base class for all sr_signal signals
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef SR_SIGNAL_BASE_H
#define SR_SIGNAL_BASE_H

#include <systemc>

namespace sr_signal {

/// Base class for all sr_signal signals.
/// This is needed to hide the direct pointer to the parent model.
/// Each instantiated member signal needs a pointer to it's parent class to
/// provide an instance pointer for callback functions.
///
/// To provide these pointer we abuse the sc_object api.
/// sc_object provides a mechanism to get an sc_object instance of the parent object.
/// This is casted back to the type MODULE (template parameter).
template<class TYPE, class MODULE>
class signal_base : public sc_core::sc_object {
    public:
        /// Default constructor
        signal_base(sc_core::sc_module_name mn = NULL) :
            sc_core::sc_object(mn) {
        }

        /// default destructor.
        /// Only declared for virtual functions.
        virtual ~signal_base() {
        }
    protected:
        /// Returns the MODULE instance of the parent object.
        /// It only works if the parent object is an sc_object and from type MODULE.
        virtual MODULE *get_module() {
            sc_core::sc_object *obj = get_parent_object();
            MODULE *mod = dynamic_cast<MODULE *> (obj);
            if (!mod) {
                // Error Message: MODULE is not an sc_object/systemc_module.
                return NULL;
            }
            return mod;
        }
};

};  // namespace sr_signal

#endif  // SR_SIGNAL_BASE_H
/// @}
