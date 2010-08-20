/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       signalkit_h/out.h:                                      */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef SIGNALKIT_OUT_H
#define SIGNALKIT_OUT_H

#include "signalkit_h/base.h"
#include "signalkit_h/ifs.h"

namespace signalkit {

template<class TYPE, class MODULE>
class signal_out : public signal_base<TYPE, MODULE>, public signal_out_if<TYPE> {
  protected:
    typedef std::vector<signal_in_if<TYPE> *> t_receiver;
    
  public:
    signal_out(MODULE *module, sc_core::sc_module_name mn = NULL)
      : signal_base<TYPE, MODULE>::signal_base(module, mn) {}
      
    virtual ~signal_out() {}
  
    virtual void bind(signal_in_if<TYPE> &receiver, const unsigned int &channel = 0) {
      // TODO: Double insert possible
      //if(m_receiver.find(receiver) != m_receiver.end()) {
      m_receiver.push_back(&receiver);
      //}
    }
    
    virtual void write(const TYPE &value, const sc_core::sc_time &time = sc_core::SC_ZERO_TIME) {
      this->m_value = value;
      for(typename t_receiver::iterator i = m_receiver.begin(); i != m_receiver.end(); i++) {
        (*i)->update((signal_out_if<TYPE> *)this, time);
      }
    }
    
    void operator()(signal_in_if<TYPE> &receiver) {
      this->bind(receiver);
      receiver.bind(*this);
    }
    
    TYPE operator=(const TYPE &t) {
      write(t);
      return this->m_value;
    }
    
    TYPE operator=(const signal_if<TYPE> &t) {
      TYPE value = t;
      write(value);
      return this->m_value;
    }
  protected:
    t_receiver m_receiver;
};

} // signalkit

#endif // SIGNALKIT_OUT_H
