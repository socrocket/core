/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       signalkit_h/in.h:                                       */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef TLM_SIGNAL_IN_H
#define TLM_SIGNAL_IN_H

#include "signalkit_h/base.h"
#include "signalkit_h/ifs.h"

namespace signalkit {

template<class TYPE, class MODULE>
class signal_in : public signal_base<TYPE, MODULE>, public signal_in_if<TYPE> {
  public:
    typedef void(MODULE::*t_callback)(const TYPE &value, signal_in_if<TYPE> *signal, signal_out_if<TYPE> *sender, const sc_core::sc_time &time);
   
    signal_in(MODULE *module, sc_core::sc_module_name mn = NULL)
      : signal_base<TYPE, MODULE>::signal_base(module, mn), m_callback(NULL) {}

    signal_in(MODULE *module, t_callback callback, sc_core::sc_module_name mn = NULL)
      : signal_base<TYPE, MODULE>::signal_base(module, mn), m_callback(callback) {}

    virtual ~signal_in() {}
    virtual void bind(signal_out_bind_if<TYPE> &sender, const unsigned int &channel = 0) {}
    
    virtual void update(signal_out_if<TYPE> *sender, const sc_core::sc_time &time) {
      this->m_value = sender->read();
      if(m_callback) {
        (this->m_module->*m_callback)(this->m_value, this, sender, time);
      }
    }

    virtual operator TYPE() const {
      return this->m_value;
    }

    virtual void operator () (signal_out_bind_if<TYPE> &sender) {
      this->bind(sender);
      sender.bind(*this);
    }

  private:
    t_callback m_callback;
};

} // signalkit

#endif // TLM_SIGNAL_IN_H
