// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_signal
/// @{
/// @file sr_signal.i
///
/// @date 2013-2016
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
%module sr_signal

%include "std_string.i"
%include "stdint.i"
%include "usi.i"

USI_REGISTER_MODULE(sr_signal)

%begin %{
#include "sr_signal.h"
%}

%inline %{
class USISignal {
  public:
    USISignal(sc_core::sc_object *obj) : m_obj(NULL) {
      m_obj = dynamic_cast<sr_signal::signal_bind_if *>(obj);
    }

    bool signal_direction() {
      return m_obj? m_obj->signal_direction() : false;
    }

    const char *signal_type() {
      return m_obj? m_obj->signal_type() : "";
    }

    bool signal_bind(sc_core::sc_object *obj, int channel = 0) {
      USISignal other(obj);
      if(m_obj && other.m_obj) {
          return m_obj->signal_bind(other.m_obj, channel, channel);
      }
      return false;
    }

    bool signal_bind(sc_core::sc_object *obj, int channel1, int channel2) {
      USISignal other(obj);
      if(m_obj && other.m_obj) {
          return m_obj->signal_bind(other.m_obj, channel1, channel2);
      }
      return false;
    }

#ifndef SWIG
  private:
    sr_signal::signal_bind_if *m_obj;
  friend USIObject find_USISignal_object(sc_core::sc_object *obj, std::string name);
#endif
};

%}

%{
USIObject find_USISignal_object(sc_core::sc_object *obj, std::string name) {
  PyObject *result = NULL;
  USISignal *instance = new USISignal(obj);
  if(instance) {
    if(instance->m_obj) {
      result = SWIG_NewPointerObj(SWIG_as_voidptr(instance), SWIGTYPE_p_USISignal, 1);
    } else {
      delete instance;
      instance = NULL;
    }
  }
  return result;
}

USI_REGISTER_OBJECT_GENERATOR(find_USISignal_object);
%}

