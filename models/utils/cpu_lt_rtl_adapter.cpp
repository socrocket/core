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


void cpu_lt_rtl_adapter::ico_demux() {

  icache_out_type ico_in;

  while(1) {

    ico_in = ico.read();
    
    ico_hold.write(ico_in.hold);
    ico_mds.write(ico_in.mds);

    wait();
  }
}

void cpu_lt_rtl_adapter::mds_capture() {

  icache_out_type ico_in;

  while(1) {

    ico_in = ico.read();

    ihrdata_reg = ico_in.data[0].to_uint();

    wait();
  }
}

void cpu_lt_rtl_adapter::dcache_removal() {

  dcache_in_type dci_out;

  while(1) {

    wait(clk.posedge_event());

    dci_out.asi = 9;
    dci_out.maddress = 0;
    dci_out.eaddress = 0;
    dci_out.edata = 0;
    dci_out.enaddr = 0;
    dci_out.eenaddr = 0;
    dci_out.read = SC_LOGIC_0;
    dci_out.write = SC_LOGIC_0;

    //dci.write(dci_out);
  }
}


void cpu_lt_rtl_adapter::instruction_gen() {

  icache_in_type ici_out;
  dcache_in_type dci_out;

  while(1) {

    ici_out.rbranch = 0;
    ici_out.fbranch = 0;
    ici_out.inull = 0;
    ici_out.su  = SC_LOGIC_1;
    ici_out.flush = 0;
    ici_out.fline = 0;
    ici_out.flushl = 0;
    ici_out.pnull = SC_LOGIC_0; 

    dci_out.asi = 0;
    dci_out.maddress = 0;
    dci_out.eaddress = 0;
    dci_out.edata = 0;
    dci_out.enaddr = 0;
    dci_out.eenaddr = 0;
    dci_out.size = 0;
    dci_out.nullify = 0;
    dci_out.lock = SC_LOGIC_0;
    dci_out.read = SC_LOGIC_0;
    dci_out.write = SC_LOGIC_0;
    dci_out.flush = 0;
    dci_out.flushl = 0;
    dci_out.dsuen = 0;
    dci_out.msu = 0;
    dci_out.esu = 0;
    dci_out.intack = 0;  

    // ------------------

    ici_out.rpc = 0xC0004;
    ici_out.fpc = 0xC0004;
    ici_out.dpc = 0xC0004;

    ici.write(ici_out);

    wait();

    // ------------------

    ici_out.rpc = 0xC0008;
    ici_out.fpc = 0xC0008;
    ici_out.dpc = 0xC0008;
    ici_out.flush = 1;

    ici.write(ici_out);

    dci_out.flush = 1;
    
    dci.write(dci_out);

    wait();

    // ------------------

    ici_out.rpc = 0xC0000;
    ici_out.fpc = 0xC0000;
    ici_out.dpc = 0xC0000;
    ici_out.flush = 0;

    ici.write(ici_out);
    
    dci_out.flush = 0;
    dci.write(dci_out);

    wait(1100);

    // ------------------

    dci_out.asi = 2;
    dci_out.maddress = 0;
    dci_out.eaddress = 0;
    dci_out.edata = 0xf;
    dci_out.enaddr = 1;
    dci_out.eenaddr = 1;
    dci_out.size = 2;
    dci_out.read = SC_LOGIC_0;
    dci_out.write = SC_LOGIC_1;

    dci.write(dci_out);

    //dcache_removal_event.notify();

    wait();

    // ------------------

    dci_out.asi = 9;
    dci_out.maddress = 0;
    dci_out.eaddress = 0;
    dci_out.edata = 0;
    dci_out.enaddr = 0;
    dci_out.eenaddr = 0;
    dci_out.size = 0;
    dci_out.read = SC_LOGIC_0;
    dci_out.write = SC_LOGIC_0;

    dci.write(dci_out);

    ici_out.rpc = 0xC0004;
    ici_out.fpc = 0xC0004;
    ici_out.dpc = 0xC0004;

    ici.write(ici_out);

    wait();

    // ------------------

    ici_out.rpc = 0xC0008;
    ici_out.fpc = 0xC0008;
    ici_out.dpc = 0xC0008;

    ici.write(ici_out);

    wait();

    // ------------------

    ici_out.rpc = 0xC0000;
    ici_out.fpc = 0xC0000;
    ici_out.dpc = 0xC0000;

    ici.write(ici_out);

    wait();

    // ------------------

    ici_out.rpc = 0xC0004;
    ici_out.fpc = 0xC0004;
    ici_out.dpc = 0xC0004;

    ici.write(ici_out);

    wait();

    // ------------------

    ici_out.rpc = 0xC0008;
    ici_out.fpc = 0xC0008;
    ici_out.dpc = 0xC0008;

    ici.write(ici_out);

    wait();

    // ------------------

    ici_out.rpc = 0xC0000;
    ici_out.fpc = 0xC0000;
    ici_out.dpc = 0xC0000;

    ici.write(ici_out);

    wait();

    // ------------------

    sc_stop();

  }
}

// instruction port service machine
void cpu_lt_rtl_adapter::icio_service_machine() {

  icache_in_type ici_out;
  icache_out_type ici_in;

  dcache_in_type dci_out;

  while (1) {

    if (!rst) {

      ici_out.rpc = 0xC0000;
      ici_out.fpc = 0xC0000;
      ici_out.dpc = 0xC0000;
      ici_out.rbranch = 0;
      ici_out.fbranch = 0;
      ici_out.inull = 0;
      ici_out.su  = SC_LOGIC_1;
      ici_out.flush = 0;
      ici_out.fline = 0;
      ici_out.flushl = 0;
      ici_out.pnull = SC_LOGIC_0;

      ici.write(ici_out);

      dci_out.asi = 0;
      dci_out.maddress = 0;
      dci_out.eaddress = 0;
      dci_out.edata = 0;
      dci_out.enaddr = 0;
      dci_out.eenaddr = 0;
      dci_out.size = 2;
      dci_out.nullify = 0;
      dci_out.lock = SC_LOGIC_0;
      dci_out.read = SC_LOGIC_0;
      dci_out.write = SC_LOGIC_0;
      dci_out.flush = 0;
      dci_out.flushl = 0;
      dci_out.dsuen = 0;
      dci_out.msu = 0;
      dci_out.esu = 0;
      dci_out.intack = 0;

      dci.write(dci_out);


    } else {

      if (clk.posedge()) {

	ici_in = ico.read();

	// assign next address
	if (ici_in.hold == SC_LOGIC_1) {

	  std::cout << sc_time_stamp() << " hold is 1" << std::endl;

	  next_instruction_event.notify();

	}
	else {

	  std::cout << sc_time_stamp() << " hold is not 1" << std::endl;
        }
      }
    }

   wait();

  }
}

void cpu_lt_rtl_adapter::iread(unsigned int address, unsigned int * data, unsigned int flush) {

  wait(1, SC_PS);

  icache_in_type ici_out;

  ici_out.rpc = address;
  ici_out.fpc = address;
  ici_out.dpc = address;
  ici_out.rbranch = SC_LOGIC_0;
  ici_out.fbranch = SC_LOGIC_0;
  ici_out.inull = SC_LOGIC_0;
  ici_out.su = SC_LOGIC_1;
  ici_out.flush = (bool)flush;
  ici_out.fline = 0;
  ici_out.pnull = SC_LOGIC_0;

  // drive rtl signals
  ici.write(ici_out);

  // we have an active request
  ici_request_pending = true;

  std::cout << sc_time_stamp() << "fuck written" << std::endl;

  // wait for data to become ready
  wait(icache_ready);

  *data = ihrdata_reg;

}

// data port service machine
void cpu_lt_rtl_adapter::dcio_service_machine() {

  sc_logic mds = SC_LOGIC_1;

  dcache_in_type dci_out;
  dcache_out_type dci_in;
  enum dcio_state { REQUEST, READHIT, READMISS, WRITEBUFFERED, WRITEUNBUFFERED } state;

  while (1) {

    if (dci_read_request_pending || dci_write_request_pending) {

      // read data cache outputs
      dci_in = dco.read();

      switch (state) {

      case REQUEST:

	std::cout << sc_time_stamp() << " DCACHE REQUEST " << std::endl;
	
	if (dci_read_request_pending) {
	  state = READHIT;
	} else {
	  state = WRITEBUFFERED;
	}
	
	break;

      case READHIT:

	std::cout << sc_time_stamp() << " DCACHE READHIT CHECK " << std::endl;

	// data is ready if hold signal does not go down
	if (dci_in.hold == SC_LOGIC_1) {

	  dci_read_request_pending = false;

	  dcache_ready.notify();

	  state = REQUEST;
	}
	break;

      case READMISS:

	std::cout << sc_time_stamp() << " DCACHE READMISS CHECK" << std::endl;

	// instruction is ready at posedge of memory data strobe
	if ((mds==SC_LOGIC_1)&&(dci_in.mds==SC_LOGIC_0)) {

	  std::cout << sc_time_stamp() << " READMISS (Edge detected)" << std::endl;

	  dci_read_request_pending = false;

	  dcache_ready.notify();

	  state = REQUEST;
	}
	break;

      case WRITEBUFFERED:

	std::cout << sc_time_stamp() << " DCACHE WRITEBUFFERED CHECK " << std::endl;	
 
	// write latency hidden if hold signal does not go down
	if (dci_in.hold == SC_LOGIC_1) {

	  dci_write_request_pending = false;

	  dcache_ready.notify();

	  state = REQUEST;

	} else {

	  state = WRITEUNBUFFERED;

	}
	break;

      case WRITEUNBUFFERED:

	std::cout << sc_time_stamp() << " DCACHE WRITEUNBUFFERED CHECK " << std::endl;

	// data is ready at posedge of memory data strobe
	if ((mds==SC_LOGIC_1)&&(dci_in.mds==SC_LOGIC_0)) {

	  std::cout << sc_time_stamp() << " READMISS (Edge detected)" << std::endl;

	  dci_write_request_pending = false;

	  state = REQUEST;

	}
	break;	  

      default:

	break;

      }
   
    } else {

      dci_out.asi = 9; // bit 3 of asi unsets dcache forcemiss signal
      dci_out.maddress = 0;
      dci_out.eaddress = 0;
      dci_out.edata = 0;
      dci_out.size = 0;
      dci_out.enaddr = SC_LOGIC_0;
      dci_out.eenaddr = SC_LOGIC_0;
      dci_out.nullify = 0;
      dci_out.lock = SC_LOGIC_0;
      dci_out.read = SC_LOGIC_0;
      dci_out.write = SC_LOGIC_0;
      dci_out.flush = SC_LOGIC_0;
      dci_out.flushl = SC_LOGIC_0;
      dci_out.dsuen = SC_LOGIC_0;
      dci_out.msu = SC_LOGIC_0;
      dci_out.esu = SC_LOGIC_0;
      dci_out.intack = SC_LOGIC_0;

      dci.write(dci_out);
    }

    mds = dci_in.mds;

    // next posedge clock
    wait();

  }

}

void cpu_lt_rtl_adapter::dread(unsigned int address, unsigned int * data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock) {

  wait(1,SC_PS);

  dcache_in_type dci_out;

  dci_out.asi = asi;
  dci_out.maddress = address;
  dci_out.eaddress = address;
  dci_out.edata = *data;

  switch (length) {

    case 1: dci_out.size = 0;
      break;
    case 2: dci_out.size = 1;
      break;
    case 3: dci_out.size = 2;
      break;
    default: dci_out.size = 3;
      break;

  }

  dci_out.enaddr = SC_LOGIC_0;
  dci_out.eenaddr = SC_LOGIC_0;
  dci_out.nullify = SC_LOGIC_0;
  dci_out.lock = (bool)lock;
  dci_out.read = SC_LOGIC_1;
  dci_out.write = SC_LOGIC_0;
  dci_out.flush = (bool)flush;
  dci_out.flushl = (bool)flushl;
  dci_out.dsuen = SC_LOGIC_0;
  dci_out.msu = SC_LOGIC_0;
  dci_out.esu = SC_LOGIC_0;
  dci_out.intack = SC_LOGIC_0;

  // write to icache
  dci.write(dci_out);

  dci_read_request_pending = true;

  wait(dcache_ready);

  *data = dhrdata_reg;
    
}

void cpu_lt_rtl_adapter::dwrite(unsigned int address, unsigned int * data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock) {

  wait(1,SC_PS);

  dcache_in_type dci_out;

  dci_out.asi = asi;
  dci_out.maddress = address;
  dci_out.eaddress = address;
  dci_out.edata = *data;

  switch (length) {

    case 1: dci_out.size = 0;
      break;
    case 2: dci_out.size = 1;
      break;
    case 3: dci_out.size = 2;
      break;
    default: dci_out.size = 3;
      break;

  }

  dci_out.enaddr = SC_LOGIC_1;
  dci_out.eenaddr = SC_LOGIC_1;
  dci_out.nullify = SC_LOGIC_0;
  dci_out.lock = dcio_lock;
  dci_out.read = SC_LOGIC_0;
  dci_out.write = SC_LOGIC_1;
  dci_out.flush = (bool)flush;
  dci_out.flushl = (bool)flushl;
  dci_out.dsuen = SC_LOGIC_0;
  dci_out.msu = SC_LOGIC_0;
  dci_out.esu = SC_LOGIC_0;
  dci_out.intack = SC_LOGIC_0;

  // write to dcache
  dci.write(dci_out);

  dci_write_request_pending = true;

  wait(dcache_ready);
      
}

