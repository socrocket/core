//*********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is 
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      signalkit_h/infield.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef SIGNALKIT_INFIELD_H
#define SIGNALKIT_INFIELD_H

#include "signalkit_h/base.h"
#include "signalkit_h/ifs.h"
#include <map>

/// @addtogroup signalkit
/// @{

namespace signalkit {

template<class TYPE, class MODULE>
class signal_infield : public signal_base<TYPE, MODULE>, public signal_in_if<TYPE> {
  public:
    typedef void(MODULE::*t_callback)(const TYPE &value, const unsigned int &channel, signal_in_if<TYPE> *signal, signal_out_if<TYPE> *sender, const sc_core::sc_time &time);

    signal_infield(sc_core::sc_module_name mn = NULL)
      : signal_base<TYPE, MODULE>::signal_base(mn), m_callback(NULL) {}

    signal_infield(t_callback callback, sc_core::sc_module_name mn = NULL)
      : signal_base<TYPE, MODULE>::signal_base(mn), m_callback(callback) {}

    virtual ~signal_infield() {}
    virtual void bind(signal_out_bind_if<TYPE> &sender, const unsigned int &channel = 0) {
      // TODO: Make it work with multible sender per channel
      TYPE v = TYPE();
      signal_out_if<TYPE> *out = static_cast<signal_out_if<TYPE> *>(&sender);
      if(out) {
        m_channel.insert(std::make_pair(out, channel));
        m_value.insert(std::make_pair(channel, v));
      }
    }

    virtual void update(signal_out_if<TYPE> *sender, const sc_core::sc_time &time) {
      typename std::map<signal_out_if<TYPE> *, unsigned int>::iterator channel = this->m_channel.find(sender);
      if(channel==m_channel.end()) {
        return;
      }
      typename std::map<unsigned int, TYPE>::iterator value = this->m_value.find(channel->second);
      if(value==m_value.end()) {
        return;
      }
      value->second = sender->read();
      if(m_callback) {
        MODULE *mod = this->get_module();
        if(mod) {
          (mod->*m_callback)(value->second, channel->second, this, sender, time);
        }
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

    TYPE operator[](const unsigned int &channel) const {
      return read(channel);
    }

    void operator()(signal_out_bind_if<TYPE> &sender, unsigned int channel) {
      this->bind(sender);
      sender.bind(*this);
    }

  private:
    std::map<signal_out_if<TYPE> *, unsigned int> m_channel;
    std::map<unsigned int, TYPE> m_value;
    t_callback m_callback;
};

} // signalkit

/// @}

#endif // SIGNALKIT_INFIELD_H
