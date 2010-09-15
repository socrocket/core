/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       signalkit_h/base.h:                                     */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef SIGNALKIT_BASE_H
#define SIGNALKIT_BASE_H

#include <systemc>

/// @addtogroup signalkit
/// @{

namespace signalkit {

template<class TYPE, class MODULE>
class signal_base : public sc_core::sc_object {
  public:
    signal_base(sc_core::sc_module_name mn = NULL)
      : sc_core::sc_object() {}
    
    virtual ~signal_base() {}
  protected:
    virtual MODULE *get_module() {
      sc_core::sc_object *obj = get_parent();
      MODULE *mod = dynamic_cast<MODULE *>(obj);
      if(!mod) {
        // Error Message: MODULE is not an sc_object/systemc_module.
        return NULL;
      } 
      return mod;
    }
};

}; // signalkit

/// @}

#endif // SIGNALKIT_BASE_H
