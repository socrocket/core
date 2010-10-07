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
// Title:      signalkit_h/ifs.h
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

#ifndef SIGNALKIT_IFS_H
#define SIGNALKIT_IFS_H

/// @addtogroup signalkit
/// @{

namespace signalkit {

template<class TYPE>
class signal_if {
  public:
    virtual ~signal_if() {}

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
class signal_in_if;

template<class TYPE>
class signal_out_bind_if {
  public:
    virtual ~signal_out_bind_if() {}
    virtual void bind(signal_in_if<TYPE> &t, const unsigned int &channel = 0) = 0;
};

template<class TYPE>
class signal_out_if : virtual public signal_if<TYPE>, public signal_out_bind_if<TYPE> {
  public:

    virtual ~signal_out_if() {}
    virtual void write(const TYPE &value, const sc_core::sc_time &time = sc_core::SC_ZERO_TIME) = 0;

    virtual TYPE operator=(const TYPE &t) {
      write(t);
      return this->m_value;
    }

    virtual TYPE operator=(const signal_if<TYPE> &t);
};

template<class TYPE>
class signal_in_if : virtual public signal_if<TYPE> {
  public:
    virtual ~signal_in_if() {}

    virtual void bind(signal_out_bind_if<TYPE> &t, const unsigned int &channel = 0) = 0;
    virtual void update(signal_out_if<TYPE> *sender, const sc_core::sc_time &time = sc_core::SC_ZERO_TIME) = 0;

};

template<class TYPE>
TYPE signal_out_if<TYPE>::operator=(const signal_if<TYPE> &t) {
  TYPE value = t;
  write(value);
  return this->m_value;
}

} // signalkit

/// @}

#endif // SIGNALKIT_IFS_H
