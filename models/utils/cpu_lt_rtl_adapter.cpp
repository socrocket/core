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
// Title:      cpu_lt_rtl_adapter.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Implementation of cpu_lt_rtl_adapter.
//             
//
// Method:
//
// Modified on $Date: 2010-10-07 19:25:02 +0200 (Thu, 07 Oct 2010) $
//          at $Revision $
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#include "cpu_lt_rtl_adapter.h"

// TLM forward transport function for icio socket
void cpu_lt_rtl_adapter::icio_custom_b_transport(tlm::tlm_generic_payload& tran, sc_core::sc_time &delay) {

  // extract payload
  tlm::tlm_command cmd = tran.get_command();
  sc_dt::uint64 addr = tran.get_address();
  unsigned char * ptr = tran.get_data_ptr();

  // extract payload extension
  icio_payload_extension * iext;
  tran.get_extension(iext);

  unsigned int flush = iext->flush;

  if (cmd == tlm::TLM_READ_COMMAND) {

    v::debug << sc_time_stamp() << " Transactor iread from address " << hex << addr << std::endl;

    iread((unsigned int)addr,(unsigned int*)ptr, flush);
    // Setting gp response status to OK
    tran.set_response_status(tlm::TLM_OK_RESPONSE);
    return;    


  } else {

    v::error << "tlm command not allowed on instruction socket" << v::endl;
    // Setting gp response status to OK
    tran.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    return;

  }

}

// 
// TLM forward transport function for dcio socket
void cpu_lt_rtl_adapter::dcio_custom_b_transport(tlm::tlm_generic_payload& tran, sc_core::sc_time &delay) {

  // extract payload
  tlm::tlm_command cmd = tran.get_command();
  sc_dt::uint64 addr = tran.get_address();
  unsigned char * ptr = tran.get_data_ptr();
  unsigned int len = tran.get_data_length();

  // extract payload extension
  dcio_payload_extension * dext;
  tran.get_extension(dext);

  unsigned int asi = dext->asi;
  unsigned int * debug = dext->debug;
  unsigned int flush = dext->flush;
  unsigned int flushl = dext->flushl;
  unsigned int lock = dext->lock;

  if (cmd == tlm::TLM_READ_COMMAND) {

    v::info << sc_time_stamp() << " Transactor dread from address " << hex << addr << v::endl;

    dread((unsigned int)addr, (unsigned int*)ptr, len, asi, flush, flushl, lock);
    // Setting gp response status to OK
    tran.set_response_status(tlm::TLM_OK_RESPONSE);
    return;

  } else if (cmd == tlm::TLM_WRITE_COMMAND) {

    v::info << sc_time_stamp() << " Transactor dwrite to address " << hex << addr << v::endl;

    dwrite((unsigned int)addr,(unsigned int*)ptr, len, asi, flush, flushl, lock);
    // Setting gp response status to OK
    tran.set_response_status(tlm::TLM_OK_RESPONSE);
    return;   

  
  } else {

    v::error << "Invalid TLM command" << v::endl;
    // Set gp response status 
    tran.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    return;

  }

}

void cpu_lt_rtl_adapter::iread(unsigned int address, unsigned int * data, unsigned int flush) {

  icache_in_type itmp;

  itmp.rpc = address;
  itmp.fpc = address;
  itmp.dpc = address;
  itmp.rbranch = SC_LOGIC_0;
  itmp.fbranch = SC_LOGIC_0;
  itmp.inull = SC_LOGIC_0;
  itmp.su = SC_LOGIC_1;
  itmp.flush = (bool)flush;
  itmp.fline = 0;
  itmp.pnull = SC_LOGIC_0;

  // data to signal
  ival.write(itmp);

  // async reset transactor
  state = idle;

  // trigger iread
  iread_pending.write(true);

  // wait for instruction cycle to be finished
  wait(iread_done);

  // result to tlm
  *data = ico_data_reg;

  // iread done
  iread_pending.write(false);

  // reset to default
  itmp.rpc = 0;
  itmp.fpc = 0;
  itmp.dpc = 0;
  itmp.rbranch = SC_LOGIC_0;
  itmp.fbranch = SC_LOGIC_0;
  itmp.inull = SC_LOGIC_1;
  itmp.su = SC_LOGIC_1;
  itmp.flush = 0;
  itmp.fline = 0;
  itmp.pnull = SC_LOGIC_0;

  ival.write(itmp);

}


void cpu_lt_rtl_adapter::dread(unsigned int address, unsigned int * data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock) {

  dcache_in_type dtmp;

  dtmp.asi = asi;
  dtmp.maddress = address;
  dtmp.eaddress = address;

  dtmp.edata = 0;

  // The RTL model expects the length parameter
  // not as the number of bytes.
  switch (length) {

    // byte
    case 1: dtmp.size = 0;
      break;
    // short
    case 2: dtmp.size = 1;
      break;
    // word
    case 4: dtmp.size = 2;
      break;
    // dword
    case 8: dtmp.size = 3;
      break;
    default: dtmp.size = 2;
      v::warn << " Invalid access size " << length << v::endl;
  }

  dtmp.enaddr = SC_LOGIC_1;
  dtmp.eenaddr = SC_LOGIC_1;
  dtmp.nullify = SC_LOGIC_0;
  dtmp.lock = (bool)lock;
  dtmp.read = SC_LOGIC_1;
  dtmp.write = SC_LOGIC_0;
  dtmp.flush = (bool)flush;
  dtmp.flushl = (bool)flushl;
  dtmp.dsuen = SC_LOGIC_0;
  dtmp.msu = SC_LOGIC_0;
  dtmp.esu = SC_LOGIC_0;
  dtmp.intack = SC_LOGIC_0;

  // data to signal
  dval.write(dtmp);

  // async reset transactor state
  state = idle;

  // trigger dread
  dread_pending.write(true);

  // wait for instruction cycle to be finished
  wait(dread_done);

  // result to tlm
  *data = dco_data_reg;

  // dread done
  dread_pending.write(false);

  // reset to default
  dtmp.asi = 0xb;
  dtmp.maddress = 0;
  dtmp.eaddress = 0;
  dtmp.edata = 0;
  dtmp.size = 2;
  dtmp.enaddr = SC_LOGIC_0;
  dtmp.eenaddr = SC_LOGIC_0;
  dtmp.nullify = SC_LOGIC_0;
  dtmp.lock = 0;
  dtmp.read = SC_LOGIC_1;
  dtmp.write = SC_LOGIC_0;
  dtmp.flush = 0;
  dtmp.flushl = 0;
  dtmp.dsuen = SC_LOGIC_0;
  dtmp.msu = SC_LOGIC_0;
  dtmp.esu = SC_LOGIC_0;
  dtmp.intack = SC_LOGIC_0;

  dval.write(dtmp);
 
}



void cpu_lt_rtl_adapter::dwrite(unsigned int address, unsigned int * data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock) {

  dcache_in_type dtmp;

  dtmp.asi = asi;
  dtmp.maddress = address;
  dtmp.eaddress = address;
  dtmp.edata = *data;

  // The RTL model expects the length parameter
  // not as the number of bytes.
  switch (length) {

    // byte
    case 1: dtmp.size = 0;
      break;
    // short
    case 2: dtmp.size = 1;
      break;
    // word
    case 4: dtmp.size = 2;
      break;
    // dword
    case 8: dtmp.size = 3;
      break;
    default: dtmp.size = 2;
      v::warn << " Invalid access size " << length << v::endl;

  }

  dtmp.enaddr = SC_LOGIC_1;
  dtmp.eenaddr = SC_LOGIC_1;
  dtmp.nullify = SC_LOGIC_0;
  dtmp.lock = (bool)lock;
  dtmp.read = SC_LOGIC_0;
  dtmp.write = SC_LOGIC_1;
  dtmp.flush = (bool)flush;
  dtmp.flushl = (bool)flushl;
  dtmp.dsuen = SC_LOGIC_0;
  dtmp.msu = SC_LOGIC_0;
  dtmp.esu = SC_LOGIC_0;
  dtmp.intack = SC_LOGIC_0;

  // data to signal
  dval.write(dtmp);

  // async reset transactor
  state = idle;

  // trigger dwrite
  dwrite_pending.write(true);

  // wait for dwrite_thread to finish
  wait(dwrite_done);

  // dwrite done
  dwrite_pending.write(false);

  // reset to default
  dtmp.asi = 0xb;
  dtmp.maddress = 0;
  dtmp.eaddress = 0;
  dtmp.edata = 0;
  dtmp.size = 2;
  dtmp.enaddr = SC_LOGIC_0;
  dtmp.eenaddr = SC_LOGIC_0;
  dtmp.nullify = SC_LOGIC_0;
  dtmp.lock = 0;
  dtmp.read = SC_LOGIC_1;
  dtmp.write = SC_LOGIC_0;
  dtmp.flush = 0;
  dtmp.flushl = 0;
  dtmp.dsuen = SC_LOGIC_0;
  dtmp.msu = SC_LOGIC_0;
  dtmp.esu = SC_LOGIC_0;
  dtmp.intack = SC_LOGIC_0;

  // data to signal
  dval.write(dtmp);
      
}
   
// state transition at clock tick
void cpu_lt_rtl_adapter::fsm_clock_tick() {

  while(1) {

    if (clk.posedge()) {

      if (rst==SC_LOGIC_1) {

	state = nextstate;

      }
      
    }

    wait();
  }
}

// determine next state (asynchr)
void cpu_lt_rtl_adapter::fsm_next_state() {

  dcache_in_type dtmp;

  while(1) {

    switch(state) {

      case idle:
	
	ici.write(ival);
	dci.write(dval);

        // instruction read
        if (iread_pending && (ico.read().hold == SC_LOGIC_1) && (dco.read().hold == SC_LOGIC_1)) {

	  nextstate = ireadaddr;
      
	// data read
        } else if (dread_pending && (ico.read().hold == SC_LOGIC_1) && (dco.read().hold == SC_LOGIC_1)) {

   	  nextstate = dreadaddr;

	// data write
        } else if (dwrite_pending) {

  	  nextstate = dwriteaddr;

        } else {

	  nextstate = idle;

	}

        break;

      case ireadaddr:

	if ((ico.read().hold==SC_LOGIC_1)&&(iread_pending)) {
	  
	  ico_data_reg=ico.read().data[ico.read().set.to_uint()].to_uint();
	  nextstate=idle;

	  // ready for next instruction
	  iread_done.notify(5, SC_NS);

	} else {

	  nextstate=ireadmiss;

	  // not ready for next instruction
	  iread_done.cancel();
	}

	ico_data_reg=ico.read().data[ico.read().set.to_uint()].to_uint();
   
        break;

      case ireadmiss:

	if ((ico.read().mds==SC_LOGIC_0)&&(iread_pending)) {

	  ico_data_reg=ico.read().data[0].to_uint();
	  nextstate=idle;
	  
	  // ready for next instruction
	  iread_done.notify(5, SC_NS);

	} else {

	  nextstate=ireadmiss;

	  // not ready for next instruction
	  iread_done.cancel();

	}

	break;

      case dreadaddr:

	if ((dco.read().hold==SC_LOGIC_1)&&(dread_pending)) {

	  dco_data_reg=dco.read().data[dco.read().set.to_uint()].to_uint();
	  nextstate=idle;

	  // ready for next instruction
	  dread_done.notify(5, SC_NS);

	} else {

	  nextstate=dreadmiss;

	  // not ready for next instruction
	  dread_done.cancel();

	}

        break;

      case dreadmiss:
	
	if ((dco.read().mds==SC_LOGIC_0)&&(dread_pending)) {

	  dco_data_reg=dco.read().data[0].to_uint();
	  nextstate=idle;

	  // ready for next instruction
	  dread_done.notify(5, SC_NS);

	} else {

	  nextstate=dreadmiss;

	  // remove address enables
	  dtmp = dval.read();
	  dtmp.enaddr = SC_LOGIC_0;
	  dtmp.eenaddr = SC_LOGIC_0;
	  dci.write(dval);
	  
	  // not ready for next instruction
	  dread_done.cancel();

	}

	break;

      case dwriteaddr:
	
	if ((dco.read().hold==SC_LOGIC_1)&&(dwrite_pending)) {

	  nextstate=idle;
	  
	  // ready for next instruction
	  dwrite_done.notify(5,SC_NS);

	} else {

	  nextstate=dwritemiss;
	  dwrite_done.cancel();

        }

	break;

      case dwritemiss:

	// remove address and write enables
	dtmp = dval.read();
	dtmp.enaddr = SC_LOGIC_0;
	dtmp.eenaddr = SC_LOGIC_0;
	dtmp.write = SC_LOGIC_0;
	dci.write(dtmp);

	if ((dco.read().hold==SC_LOGIC_1)&&(dwrite_pending)) {

	  nextstate=idle;

	  // ready for next instruction
	  dwrite_done.notify(5,SC_NS);

	} else {

	  nextstate=dwritemiss;
	  dwrite_done.cancel();

        }

        break;

      default:

        break;

    }

    wait();

  }
}


void cpu_lt_rtl_adapter::start_of_simulation() {

  // initialize signals and output ports
  icache_in_type itmp;
  dcache_in_type dtmp;

  itmp.rpc = 0;
  itmp.fpc = 0;
  itmp.dpc = 0;
  itmp.rbranch = SC_LOGIC_0;
  itmp.fbranch = SC_LOGIC_0;
  itmp.inull = SC_LOGIC_1;
  itmp.su = SC_LOGIC_1;
  itmp.flush = 0;
  itmp.fline = 0;
  itmp.pnull = SC_LOGIC_0;

  dtmp.asi = 0xb;
  dtmp.maddress = 0;
  dtmp.eaddress = 0;
  dtmp.edata = 0;
  dtmp.size = 2;
  dtmp.enaddr = SC_LOGIC_0;
  dtmp.eenaddr = SC_LOGIC_0;
  dtmp.nullify = SC_LOGIC_0;
  dtmp.lock = 0;
  dtmp.read = SC_LOGIC_1;
  dtmp.write = SC_LOGIC_0;
  dtmp.flush = 0;
  dtmp.flushl = 0;
  dtmp.dsuen = SC_LOGIC_0;
  dtmp.msu = SC_LOGIC_0;
  dtmp.esu = SC_LOGIC_0;
  dtmp.intack = SC_LOGIC_0;

  ival.write(itmp);
  dval.write(dtmp);

  ici.write(itmp);
  dci.write(dtmp);

}
