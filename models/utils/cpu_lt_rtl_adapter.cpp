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

    std::cout << " transactor read from address " << hex << addr << std::endl;

    iread((unsigned int)addr,(unsigned int*)ptr, flush);

  } else {

    v::error << "tlm command not allowed on instruction socket" << v::endl;

  }

}

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

    dread((unsigned int)addr, (unsigned int*)ptr, len, asi, flush, flushl, lock);

  } else if (cmd == tlm::TLM_WRITE_COMMAND) {

    dwrite((unsigned int)addr,(unsigned int*)ptr, len, asi, flush, flushl, lock);
  
  } else {

    v::error << "tlm command not allowed on data socket" << v::endl;

  }

}

void cpu_lt_rtl_adapter::data_capture() {

  // init
  instr_mds = false;
  data_mds = false;

  while(1) {

    if (rst == SC_LOGIC_1) {

      if (clk.posedge()) {

	// instruction strobe
	if (ico.read().mds == SC_LOGIC_0) {

	  instr = ico.read().data[0].to_uint();
	  instr_mds = true;

        }

	// data strobe
	if (dco.read().mds == SC_LOGIC_0) {

	  data = dco.read().data[0].to_uint();
	  data_mds = true;

	}
      }
    }

    wait();
  }
}
      

// instruction port service machine
void cpu_lt_rtl_adapter::cache_transactor() {

  unsigned int state;

  while (1) {

    if (rst == SC_LOGIC_1) {

      switch (state) {

        case 0:

	  // write instruction and data port
	  ici.write(ival);
	  dci.write(dval);

	  // wait 1 cycle for address sampling
	  if (clk.posedge()) {

	    state = 1;

	  }
	  
	  break;
	
        default:

	  if ((ico.read().hold == SC_LOGIC_1)&&(dco.read().hold == SC_LOGIC_1)) {

	    if (!instr_mds) {
	    
	      instr = ico.read().data[0].to_uint();

	    }

	    if (!data_mds) {

	      data = dco.read().data[0].to_uint();

	    }

	    // done - next instruction
	    cache_ready.notify();
	    state = 0;
	  }
      }
    }
	      
    wait();

  }
}

void cpu_lt_rtl_adapter::iread(unsigned int address, unsigned int * data, unsigned int flush) {

  icache_in_type itmp;

  // make sure we are behind posedge clock
  wait(1,SC_PS);

  instr = 0;
  instr_mds = 0;

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

  // wait for instruction cycle to be finished
  wait(cache_ready);

  // data to tlm
  *data = instr;

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

  data = 0;
  instr_mds = 0;

  // make sure we are behind posedge clock
  wait(1,SC_PS);

  dtmp.asi = asi;
  dtmp.maddress = address;
  dtmp.eaddress = address;
  dtmp.edata = *data;

  switch (length) {

    case 1: dtmp.size = 0;
      break;
    case 2: dtmp.size = 1;
      break;
    case 3: dtmp.size = 2;
      break;
    default: dtmp.size = 3;
      break;

  }

  dtmp.enaddr = SC_LOGIC_0;
  dtmp.eenaddr = SC_LOGIC_0;
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

  // wait for instruction cycle to be finished
  wait(cache_ready);

  // data to tlm
  *data = instr;

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

  // make sure we are behind posedge clock
  wait(1,SC_PS);

  dtmp.asi = asi;
  dtmp.maddress = address;
  dtmp.eaddress = address;
  dtmp.edata = *data;

  switch (length) {

    case 1: dtmp.size = 0;
      break;
    case 2: dtmp.size = 1;
      break;
    case 4: dtmp.size = 2;
      break;
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

  // wait for instruction cycle to be finished
  wait(cache_ready);

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
