/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       multisignalhandler.h                                    */
/*             header file defines an abstraction on top of the        */
/*             greensocs signal socets. It implements Sender and       */
/*             Target classes to send a signal in one line and         */
/*             register callbacks to signals                           */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TU Braunschweig              */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include "utils/stl_ext/cpp_callbacks.h"
#include "utils/stl_ext/cpp_void_callbacks.h"
#include "tlm.h"
#include "green_signal.h"
#include <map>

#define CB_HANDLER(guard, class_name, method_name) \
  { \
    gs::reg_utils::void_cpp_callback_3<class_name, unsigned int, gs_generic_signal_payload, sc_core::sc_time> *_m_current_callback = \
      new gs::reg_utils::void_cpp_callback_3< class_name, unsigned int, gs_generic_signal_payload, sc_core::sc_time>(); \
    _m_current_callback->bind(this, method_name); \
    this->addCallback(guard::ID, _m_current_callback); \
  }


#define SIGNAL_SET_SOC(socket, name, data) \
  SIGNAL_SET_INT(socket, name, data, false);

#define SIGNAL_SET(name, data) \
  SIGNAL_SET_INT(*m_socket, name, data, false);

#define SIGNAL_SET_ACK(name, data) \
  SIGNAL_SET_INT(*m_socket, name, data, true);

#define SIGNAL_SET_INT(socket, name, data, flag) \
  { \
    gs_generic_signal::initiator_signal_multi_socket &__socket = socket; \
    gs_generic_signal_payload* __p = __socket.get_transaction(); \
    __p->free_all_extensions(); \
    /*signal_phase  __ph;*/ \
    sc_core::sc_time __t = sc_core::SC_ZERO_TIME; \
    unsigned int __d = (data); \
    /*bool __f = flag;*/ \
    __p->set_data_ptr((unsigned char *)&__d); \
    /*p->set_ack_requirement(__f);*/ \
    __socket.validate_extension<name>(*__p); \
    /*__socket->nb_transport_fw(*p,ph,t);*/ \
    __socket->b_transport(*__p,__t); \
    __socket.invalidate_extension<name>(*__p); \
    __socket.release_transaction(__p); \
  }

using namespace gs_generic_signal;

class MultiSignalSender {
  public:
    typedef gs::socket::config<gs_generic_signal_protocol_types> sender_config;

    void register_sender_socket(gs_generic_signal::initiator_signal_multi_socket &socket, sender_config &conf) {
      m_socket = &socket;
      socket.set_config(conf);
    }

    

  protected:
    gs_generic_signal::initiator_signal_multi_socket *m_socket;
    
};

template <class T>
class MultiSignalTarget {
  public:
    typedef gs::reg_utils::callback_3<void, unsigned int, gs_generic_signal_payload, sc_core::sc_time > SignalCallback;
    typedef gs::socket::config<gs_generic_signal_protocol_types> target_config;

    void register_target_socket(T *me, target_signal_multi_socket<T> &socket, target_config &conf) {
      //socket.register_nb_transport_fw(me, &T::transporttocallback);
      socket.register_b_transport(me, &T::transporttocallback);
      socket.set_config(conf);
    }
  
  protected:
    bool addCallback(unsigned int id, SignalCallback *callback) {
      if(m_map.find(id) != m_map.end()) {
        return false;
      }
      m_map.insert(make_pair(id, callback));
      return true;
    }

    //tlm::tlm_sync_enum transporttocallback(unsigned int id, gs_generic_signal_payload& trans, signal_phase& ph, sc_core::sc_time& delay) {
    void transporttocallback(unsigned int id, gs_generic_signal_payload& trans, sc_core::sc_time& delay) {
      for(map<unsigned int, SignalCallback *>::iterator iter = m_map.begin(); iter != m_map.end(); ++iter ) {
        if(trans.get_extension(iter->first)) {
          //if(trans.is_ack_required()==true) {
          //  ph=gs_generic_signal::ACK;
          //}
          iter->second->execute(id, trans, delay);
        }
      }
      //return tlm::TLM_COMPLETED;
    }
  private:
    map<unsigned int, SignalCallback *> m_map;
};

#endif // SIGNALHANDLER_H
