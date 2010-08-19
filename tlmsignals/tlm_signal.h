/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       tlm_signal.h - An alternative signal implementation.    */
/*             This signal implementation is not routet through the    */
/*             systemc kernel and does not need any sheeduling.        */
/*             It is designed with a LT TLM Modeling in mind. Due to   */
/*             some important toplevel signals it's sometimes needet   */
/*             to provide the ability to route single signals.         */
/*             Normaly TLM abstractions would route them as a          */
/*             Bus extension or use C++ Function calls insteed. But    */
/*             this is undynamic and complicated to use                */
/*             Other solutiions like green-signal-sockets build on top */
/*             Of TLM Sockets. This archieves the same goal but is     */
/*             complicated to use and heavy in weight.                 */
/*             This implementation relyes on C++ Callbacks and         */
/*             therefor it's very light weight. It's interface is      */
/*             designed to be very close to the systemc signal IO      */
/*             interface.                                              */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef TLM_SIGNAL_H
#define TLM_SIGNAL_H

#include "systemc.h"
#include <vector>
#include <map>

//namespace tlm_signals {

template<class TYPE>
class tlm_signal_container {
  public:
    TYPE operator=(const tlm_signal_container<TYPE> &t);
    TYPE operator=(const TYPE &t);
    bool operator==(const TYPE &t);
  
  private:
    TYPE value;
};

template<class MODULE>
class tlm_module;

template<class TYPE, class MODULE>
class tlm_signal_base : public sc_core::sc_object {
  public:
    tlm_signal_base(MODULE *module, sc_core::sc_module_name mn = NULL)
      : sc_core::sc_object(), m_module(module) {}
    
  protected: 
    MODULE *m_module;
};

template<class TYPE>
class tlm_signal_if {
  public:
    virtual ~tlm_signal_if() {}
  
    virtual const TYPE &read() {
      return m_value;
    }
    
    operator TYPE() const {
      return m_value;
    }
  
    bool operator==(const TYPE &t) const {
      return (m_value == t);
    }
    
  protected:
    TYPE m_value;
};

template<class TYPE>
class tlm_signal_in_if;
  
template<class TYPE>
class tlm_signal_out_if : public tlm_signal_if<TYPE> {
  public:

    virtual ~tlm_signal_out_if() {}
    
    virtual void write(const TYPE &value, const sc_core::sc_time &time = sc_core::SC_ZERO_TIME) = 0;
  
    TYPE operator=(const TYPE &t) {
      write(t);
      return this->m_value;
    }

    TYPE operator=(const tlm_signal_if<TYPE> &t);
};

template<class TYPE>
class tlm_signal_in_if {
  public:
    virtual ~tlm_signal_in_if() {}
  
    virtual void update(tlm_signal_out_if<TYPE> *sender, const sc_core::sc_time &time = sc_core::SC_ZERO_TIME) = 0;
    
};

template<class TYPE>
TYPE tlm_signal_out_if<TYPE>::operator=(const tlm_signal_if<TYPE> &t) {
  TYPE value = t;
  write(value);
  return this->m_value;
}
    
template<class TYPE, class MODULE>
class tlm_signal_in : public tlm_signal_base<TYPE, MODULE>, public tlm_signal_in_if<TYPE>, public tlm_signal_if<TYPE> {
  public:
    typedef void(MODULE::*t_callback)(const TYPE &value, tlm_signal_in_if<TYPE> *signal, tlm_signal_out_if<TYPE> *sender, const sc_core::sc_time &time);
   
    tlm_signal_in(MODULE *module, sc_core::sc_module_name mn = NULL)
      : tlm_signal_base<TYPE, MODULE>::tlm_signal_base(module, mn), m_callback(NULL) {}

    tlm_signal_in(MODULE *module, t_callback callback, sc_core::sc_module_name mn = NULL)
      : tlm_signal_base<TYPE, MODULE>::tlm_signal_base(module, mn), m_callback(callback) {}

    virtual ~tlm_signal_in() {}
    
    virtual void update(tlm_signal_out_if<TYPE> *sender, const sc_core::sc_time &time) {
      this->m_value = sender->read();
      if(m_callback) {
        (this->m_module->*m_callback)(this->m_value, this, sender, time);
      }
    }

    operator TYPE() const {
      return this->m_value;
    }

  private:
    t_callback m_callback;
};

template<class TYPE, class MODULE>
class tlm_signal_out : public tlm_signal_base<TYPE, MODULE>, tlm_signal_out_if<TYPE> {
  protected:
    typedef std::vector<tlm_signal_in_if<TYPE> *> t_receiver;
    
  public:
    tlm_signal_out(MODULE *module, sc_core::sc_module_name mn = NULL)
      : tlm_signal_base<TYPE, MODULE>::tlm_signal_base(module, mn) {}
      
    virtual ~tlm_signal_out() {}
  
    virtual void write(const TYPE &value, const sc_core::sc_time &time = sc_core::SC_ZERO_TIME) {
      this->m_value = value;
      for(typename t_receiver::iterator i = m_receiver.begin(); i != m_receiver.end(); i++) {
        (*i)->update((tlm_signal_out_if<TYPE> *)this, time);
      }
    }
    
    void operator()(tlm_signal_in_if<TYPE> &receiver) {
      // TODO: Double insert possible
      //if(m_receiver.find(receiver) != m_receiver.end()) {
        m_receiver.push_back(&receiver);
      //}
    }
    TYPE operator=(const TYPE &t) {
      write(t);
      return this->m_value;
    }
    
    TYPE operator=(const tlm_signal_if<TYPE> &t) {
      TYPE value = t;
      write(value);
      return this->m_value;
    }
  protected:
    t_receiver m_receiver;
};

template<class TYPE, class MODULE>
class tlm_signal_inout : public tlm_signal_out<TYPE, MODULE>, public tlm_signal_in<TYPE, MODULE> {
  public:
    typedef void(MODULE::*t_callback)(const TYPE &value, tlm_signal_in_if<TYPE> *signal, tlm_signal_out_if<TYPE> *sender, const sc_core::sc_time &time);
   
    tlm_signal_inout(MODULE *module, sc_core::sc_module_name mn = NULL)
      : tlm_signal_out<TYPE, MODULE>::tlm_signal_out(module, mn)
      , tlm_signal_in<TYPE, MODULE>::tlm_signal_in(module, mn) {}

    tlm_signal_inout(MODULE *module, t_callback callback, sc_core::sc_module_name mn = NULL)
      : tlm_signal_out<TYPE, MODULE>::tlm_signal(module, mn)
      , tlm_signal_in<TYPE, MODULE>::tlm_signal_in(module, callback, mn) {}

    virtual ~tlm_signal_inout() {}
    
  private:
};

template<class TYPE, class MODULE>
class tlm_signal_selector : public tlm_signal_base<TYPE, MODULE> {
  public:
    tlm_signal_selector(MODULE *module, sc_core::sc_module_name mn = NULL)
      : tlm_signal_base<TYPE, MODULE>::tlm_signal_base(module, mn) {}
      
    virtual void write(const unsigned long long &mask, const TYPE &value, const sc_core::sc_time &time = sc_core::SC_ZERO_TIME) {
      for(typename t_map::iterator i = outs.begin(); i != outs.end(); i++) {
        if(mask&(1<<i->first)) {
          i->second.write(value, time);
        }
      }
    }
    
    //TYPE operator[](const unsigned int &mask, const TYPE &t);
    void operator()(tlm_signal_in_if<TYPE> &receiver, unsigned int channel) {
      typename t_map::iterator item = outs.insert( std::make_pair<unsigned int, tlm_signal_out<TYPE, MODULE> >(channel, tlm_signal_out<TYPE, MODULE>(this->m_module, sc_core::sc_gen_unique_name("port", true))));
      item->second(receiver);
    }
    
  private:
    typedef std::map<unsigned int, tlm_signal_out<TYPE, MODULE> > t_map;
    t_map outs;
};

template<class TYPE, class MODULE>
class tlm_signal_infield : public tlm_signal_base<TYPE, MODULE>, public tlm_signal_in_if<TYPE> {
  public:
    typedef void(MODULE::*t_callback)(const TYPE &value, const unsigned int &channel, tlm_signal_in_if<TYPE> *signal, tlm_signal_out_if<TYPE> *sender, const sc_core::sc_time &time);
    
    tlm_signal_infield(MODULE *module, sc_core::sc_module_name mn = NULL)
      : tlm_signal_base<TYPE, MODULE>::tlm_signal_base(module, mn), m_callback(NULL) {}

    tlm_signal_infield(MODULE *module, t_callback callback, sc_core::sc_module_name mn = NULL)
      : tlm_signal_base<TYPE, MODULE>::tlm_signal_base(module, mn), m_callback(callback) {}

    virtual ~tlm_signal_infield() {}
    
    virtual void update(tlm_signal_out_if<TYPE> *sender, const sc_core::sc_time &time) {
      typename std::map<tlm_signal_out_if<TYPE> *, unsigned int>::iterator channel = this->m_channel.find(sender);
      if(channel==m_channel.end()) {
        return;
      }
      typename std::map<unsigned int, TYPE>::iterator value = this->m_value.find(channel->second);
      if(value==m_value.end()) {
        return;
      }
      value->second = sender->read();
      if(m_callback) {
        (this->m_module->*m_callback)(value->second, channel->second, this, sender, time);
      }
    }

    TYPE read(const unsigned int &channel) const {
      typename std::map<unsigned int, TYPE>::iterator item = this->m_value.find(channel);
      return item->second;
    }

    TYPE read(const unsigned int &channel, const TYPE &default_value) const {
      typename std::map<unsigned int, TYPE>::iterator item = this->m_value.find(channel);
      if(item!=this->m_value.end()) {
        return item->second;
      } else {
        return default_value;
      }
    }

    TYPE operator [](const unsigned int &channel) const {
      return read(channel);
    }

    void operator ()(tlm_signal_out_if<TYPE> &sender, unsigned int channel) {
      TYPE t;
      m_channel.insert(std::make_pair(&sender, channel));
      m_value.insert(std::make_pair(channel, t));
      sender(*this);
    }

  private:
    std::map<tlm_signal_out_if<TYPE> *, unsigned int> m_channel;
    std::map<unsigned int, TYPE> m_value;
    t_callback m_callback;
};

template<class MODULE>
class tlm_module {
  public:
    template<class TYPE>
    struct signal {
      typedef tlm_signal_in<TYPE, MODULE>       in;
      typedef tlm_signal_out<TYPE, MODULE>      out;
      typedef tlm_signal_inout<TYPE, MODULE>    inout;
      typedef tlm_signal_selector<TYPE, MODULE> selector;
      typedef tlm_signal_infield<TYPE, MODULE>  infield;
    };
};

//} // tlm_signals

#endif // TLM_SIGNAL_H

#if 0
/// How to use tlm_signals
class IRQMP : public tlm_signals<ModuleB> {
  public:
    tlm_in<bool>       rst;
    tlm_out<int>       irq;
    tlm_selector<char> sel;

    ModuleB()
      : rst(this)
      , irq(this)
      , sel(this) {}

    void get_irq() {
      if(irq == m_irq) {
        sel[5] = irq;
      }
    }
  private:
    int m_irq;
};

class CPU : public tlm_signals<ModuleA> {
  public:
    tlm_in<int>    irq;
    tlm_out<bool>  rst;
    tlm_inout<int> ack;
    
    void irq_cb() {
      if(irq.value() == 20) {
        ack |= 0xFF;
      }
    }

    ModuleA()
      : irq(this, &ModuleA::irq_cb)
      , rst(this)
      , ack(this) {}
};

int main(int argc, char *argv[]) {
  CPU   cpu0;
  IRQMP irq;

  cpu.rst( irq.rst);
  irq.sel( cpu.irq, 5);
}

#endif

