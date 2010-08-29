/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       signalkit_h/selector.h:                                 */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef SIGNALKIT_SELECTOR_H
#define SIGNALKIT_SELECTOR_H

#include <map>
#include "signalkit_h/base.h"
#include "signalkit_h/ifs.h"
#include "signalkit_h/out.h"

/// @addtogroup signalkit
/// @{

namespace signalkit {

template<class TYPE, class MODULE>
class signal_selector : public signal_base<TYPE, MODULE>, public signal_out_bind_if<TYPE> {
  public:
    signal_selector(MODULE *module, sc_core::sc_module_name mn = NULL)
      : signal_base<TYPE, MODULE>::signal_base(module, mn) {}
      
    virtual void bind(signal_in_if<TYPE> &receiver, const unsigned int &channel) {
      // TODO: Make work multipel selector<->infield channels
      signal_out<TYPE, MODULE> *item = NULL;
      typename t_map::iterator iter = outs.find(channel);
      if(iter != outs.end()) {
        item = iter->second;
      } else {
        item = new signal_out<TYPE, MODULE>(this->m_module, sc_core::sc_gen_unique_name("port", true));
        outs.insert( std::make_pair(channel, item));
      }  
      item->bind(receiver);
    }
      
    virtual void write(const unsigned int &mask, const TYPE &value, const sc_core::sc_time &time = sc_core::SC_ZERO_TIME) {
      for(typename t_map::iterator i = outs.begin(); i != outs.end(); i++) {
        if(mask&(1<<i->first)) {
          i->second->write(value, time);
        }
      }
    }
   
    /* 
    void operator[](const unsigned int &mask, const TYPE &value) {
      this->write(mask, value)
    }*/
    
    void operator()(signal_in_if<TYPE> &receiver, const unsigned int &channel) {
      this->bind(receiver, channel);
      receiver.bind(*this);
    }
    
  private:
    typedef std::map<unsigned int, signal_out<TYPE, MODULE> *> t_map;
    t_map outs;
};


} // signalkit

/// @}

#endif // SIGNALKIT_SELECTOR_H
