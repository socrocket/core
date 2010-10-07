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
// Title:      ext_erase.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    generic payload extension indicating memory to be erased
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Dennis Bode
// Reviewed:
//*********************************************************************

#ifndef EXT_ERASE
#define EXT_ERASE

#include <systemc.h>
#include <tlm.h>

//simple payload extension for erasing memory
struct ext_erase : public tlm::tlm_extension<ext_erase> {
public:
  ext_erase() {erase_flag = 0;}
  bool erase_flag;

  //must_override pure virtual clone method
  virtual tlm::tlm_extension_base* clone() const {
    ext_erase* t = new ext_erase;
    t->erase_flag = this->erase_flag;
    return t;
  }

  //must override pure virtual copy_from method
  virtual void copy_from (tlm::tlm_extension_base const &ext) {
    erase_flag = static_cast<ext_erase const &>(ext).erase_flag;
  }
};

#endif
