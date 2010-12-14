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

// instruction port service machine
void cpu_lt_rtl_adapter::cache_transactor() {

  icache_in_type itmp;
  dcache_in_type dtmp;

  while (1) {

    if (rst == SC_LOGIC_0) {

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

      ici.write(itmp);

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

      dci.write(dtmp);

    } else {

      // write instruction and data port
      ici.write(ival);
      dci.write(dval);

      if (clk.posedge()) {

	if ((ico.read().hold == SC_LOGIC_1)&&(dco.read().hold == SC_LOGIC_1)) {

	  // done - next instruction
	  cache_ready.notify();
	  
	}
      }
    }

    wait();

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

  // wait for instruction cycle to be finished
  wait(cache_ready);

}


void cpu_lt_rtl_adapter::dread(unsigned int address, unsigned int * data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock) {

  dcache_in_type dtmp;

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
  
}

void cpu_lt_rtl_adapter::dwrite(unsigned int address, unsigned int * data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock) {

  dcache_in_type dtmp;

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
      
}

