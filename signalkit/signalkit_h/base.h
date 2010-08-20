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

namespace signalkit {

template<class TYPE, class MODULE>
class signal_base : public sc_core::sc_object {
  public:
    signal_base(MODULE *module, sc_core::sc_module_name mn = NULL)
      : sc_core::sc_object(), m_module(module) {}
    
  protected: 
    MODULE *m_module;
};

}; // signalkit

#endif // SIGNALKIT_BASE_H
