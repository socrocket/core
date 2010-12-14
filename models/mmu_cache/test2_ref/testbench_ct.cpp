#include "testbench_ct.h"

// main stimuli process
void testbench_ct::initiator_thread(void) {

  unsigned int instr;
  unsigned int data;
  
  int i;

  while(1) {

    std::cout << "initiator thread" << v::endl;

    if(rst.read()==1) {

      std::cout << "reset" << std::endl;

      instr_gen(0,1,1,0);
      data_gen(0xb,0,0,2,0,0,1,0,1,0,0);

      wait(100, SC_NS);

      //instr_gen(0,0,1,0);

      //wait(10, SC_NS);

      //instr_gen(4,0,1,0);

      //wait(100, SC_NS);

      //instr_gen(8,0,1,0);

      //wait(100, SC_NS);

      //instr_gen(0xc,0,1,0);

      //wait(100, SC_NS);

      //instr_gen(0x10,0,1,0);

      sc_stop();
    }

    wait();
  }
}

void testbench_ct::ico_listener() {

  while(1) {

    wait();

  }

}

void testbench_ct::dco_listener() {

  while(1) {

    wait();

  }
}

// helper function for writing the ici output port
void testbench_ct::instr_gen(unsigned int fpc, bool inull, bool su, bool flush) {

    icache_in_type ival;

    ival.rpc = 0;
    ival.fpc = fpc;
    ival.dpc = 0;
    ival.rbranch = SC_LOGIC_0;
    ival.fbranch = SC_LOGIC_0;
    ival.inull = (inull) ? SC_LOGIC_1 : SC_LOGIC_0; 
    ival.su = (su) ? SC_LOGIC_1 : SC_LOGIC_0;
    ival.flush = (flush) ? SC_LOGIC_1 : SC_LOGIC_0;
    ival.flushl = SC_LOGIC_0;
    ival.fline = 0;
    ival.pnull = SC_LOGIC_0;

    ici.write(ival);
}

// helper function for writing the dci output port
void testbench_ct::data_gen(unsigned int asi, 
			unsigned int address, 
			unsigned int data, 
			unsigned int size, 
			bool enaddr,
			bool eenaddr,
			bool nullify,
			bool lock,
			bool read,
			bool write,
			bool flush) {

    dcache_in_type dval;

    dval.asi = asi;
    dval.maddress = address;
    dval.eaddress = address;
    dval.edata = data;
    dval.size = size;
    dval.enaddr = (enaddr) ? SC_LOGIC_1 : SC_LOGIC_0;
    dval.eenaddr = (eenaddr) ? SC_LOGIC_1 : SC_LOGIC_0;
    dval.nullify = (nullify) ? SC_LOGIC_1 : SC_LOGIC_0;
    dval.lock = (lock) ? SC_LOGIC_1 : SC_LOGIC_0;
    dval.read = (read) ? SC_LOGIC_1 : SC_LOGIC_0;
    dval.write = (write) ? SC_LOGIC_1 : SC_LOGIC_0;
    dval.flush = (flush) ? SC_LOGIC_1 : SC_LOGIC_0;
    dval.flushl = SC_LOGIC_0;
    dval.dsuen = SC_LOGIC_0;
    dval.msu = SC_LOGIC_1;
    dval.esu = SC_LOGIC_1;
    dval.intack = SC_LOGIC_0;

    dci.write(dval);
}

// creates a sc_signal clock for driving the hdl components
void testbench_ct::clock_gen_thread() {

  while(1) {

    signal_clk.write(SC_LOGIC_0);
    wait();
    signal_clk.write(SC_LOGIC_1);
    wait();

  }
}

// reset control
void testbench_ct::reset_gen_thread() {

  std::cout << "Reset process" << std::endl;

  icache_in_type ival;
  dcache_in_type dval;

  // init ici
  ival.rpc = 0;
  ival.fpc = 0;
  ival.dpc = 0;
  ival.rbranch = SC_LOGIC_0;
  ival.fbranch = SC_LOGIC_0;
  ival.inull = SC_LOGIC_0;
  ival.su = SC_LOGIC_0;
  ival.flush = SC_LOGIC_0;
  ival.flushl = SC_LOGIC_0;
  ival.fline = 0;
  ival.pnull = SC_LOGIC_0;

  ici.write(ival);

  // init dci
  dval.asi = 0;
  dval.maddress = 0;
  dval.eaddress = 0;
  dval.edata = 0;
  dval.size = 0;
  dval.enaddr = 0;
  dval.eenaddr = 0;
  dval.nullify = 0;
  dval.lock = 0;
  dval.read = 0;
  dval.write = 0;
  dval.flush = 0;
  dval.flushl = 0;
  dval.dsuen = 0;
  dval.msu = 0;
  dval.esu = 0;
  dval.intack = 0;

  dci.write(dval);

  // init reset
  rst.write(0);
  rst_scl.write(SC_LOGIC_0);

  // wait for a while
  wait(95, sc_core::SC_NS);

  // remove reset
  rst.write(1);
  rst_scl.write(SC_LOGIC_1);

}

