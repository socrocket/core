#include "testbench_ct.h"

void testbench_ct::initiator_thread() {

  unsigned int instr;
  unsigned int data;
  
  int i;

  dcache_in_type dval;
  icache_in_type ival;

  while(1) {

    v::info << "initiator thread" << v::endl;

    if(rst.read()==1) {

      std::cout << "reset" << std::endl;

      // read cache control register
      // address, *data, length, asi, lock

      // -----------------------------

      ival.rpc = 0x1000;
      ival.fpc = 0x1000;
      ival.dpc = 0x1000;
      ival.rbranch = SC_LOGIC_0;
      ival.fbranch = SC_LOGIC_0;
      ival.inull = SC_LOGIC_1;
      ival.su = SC_LOGIC_1;
      ival.flush = SC_LOGIC_0;
      ival.flushl = SC_LOGIC_0;
      ival.fline = 0;
      ival.pnull = SC_LOGIC_0;

      ici.write(ival);

      dval.asi = 0xb;
      dval.maddress = 0;
      dval.eaddress = 0;
      dval.edata = 0;
      dval.size = 2;
      dval.enaddr = SC_LOGIC_0;
      dval.eenaddr = SC_LOGIC_0;
      dval.nullify = SC_LOGIC_1;
      dval.lock = SC_LOGIC_0;
      dval.read = SC_LOGIC_1;
      dval.write = SC_LOGIC_0;
      dval.flush = SC_LOGIC_0;
      dval.flushl = SC_LOGIC_0;
      dval.dsuen = SC_LOGIC_0;
      dval.msu = SC_LOGIC_1;
      dval.esu = SC_LOGIC_1;
      dval.intack = SC_LOGIC_0;

      dci.write(dval);

      // ------------------------

      wait(130,sc_core::SC_NS);

      ival.rpc = 0x1000;
      ival.fpc = 0x1000;
      ival.dpc = 0x1000;
      ival.rbranch = SC_LOGIC_0;
      ival.fbranch = SC_LOGIC_0;
      ival.inull = SC_LOGIC_0;
      ival.su = SC_LOGIC_1;
      ival.flush = SC_LOGIC_0;
      ival.flushl = SC_LOGIC_0;
      ival.fline = 0;
      ival.pnull = SC_LOGIC_0;

      ici.write(ival);

      dval.asi = 0xb;
      dval.maddress = 0;
      dval.eaddress = 0;
      dval.edata = 0;
      dval.size = 2;
      dval.enaddr = SC_LOGIC_0;
      dval.eenaddr = SC_LOGIC_0;
      dval.nullify = SC_LOGIC_1;
      dval.lock = SC_LOGIC_0;
      dval.read = SC_LOGIC_1;
      dval.write = SC_LOGIC_0;
      dval.flush = SC_LOGIC_0;
      dval.flushl = SC_LOGIC_0;
      dval.dsuen = SC_LOGIC_0;
      dval.msu = SC_LOGIC_1;
      dval.esu = SC_LOGIC_1;
      dval.intack = SC_LOGIC_0;

      dci.write(dval);

      // ------------------------
      
      // ------------------------

      // ------------------------

      //iread(0x1000, &instr);
      wait();

      //dwrite(0x1000, 0x12345678, 4, 8, 0);


      for (i=0; i<1000; i++) {

	std::cout << "wait" << std::endl;
	wait();
      }

    }

    wait();
  }
}

void testbench_ct::ico_monitor_thread() {

  while(1) {

    ico_buf_out.write(ico.read());
    wait();
  }
}


void testbench_ct::clock_gen_thread() {

  while(1) {

    signal_clk.write(SC_LOGIC_0);
    wait();
    signal_clk.write(SC_LOGIC_1);
    wait();

  }
}

void testbench_ct::reset_gen_thread() {

  icache_in_type ival;
  dcache_in_type dval;

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

  ici.write(ival);
  dci.write(dval);

  rst.write(0);
 

  rst_scl.write(SC_LOGIC_0);
  wait(95, sc_core::SC_NS);
  rst.write(1);
  rst_scl.write(SC_LOGIC_1);

}

void testbench_ct::iread(unsigned int address, unsigned int * data) {

  icache_in_type val;

  val.rpc = address;
  val.fpc = address;
  val.dpc = address;
  val.rbranch = SC_LOGIC_0;
  val.fbranch = SC_LOGIC_0;
  val.inull = SC_LOGIC_0;
  val.su = SC_LOGIC_0;
  val.flush = SC_LOGIC_0;
  val.flushl = SC_LOGIC_0;
  val.fline = 0;
  val.pnull = SC_LOGIC_0;

  ici.write(val);

}

void testbench_ct::dread(unsigned int address, unsigned int * data, unsigned int length, unsigned int asi, unsigned int lock) {

  dcache_in_type val;

  val.asi = asi;
  val.maddress = address;
  val.eaddress = address;
  val.edata = 0;
  val.size = length - 1;
  val.enaddr = SC_LOGIC_1;
  val.eenaddr = SC_LOGIC_1;
  val.nullify = SC_LOGIC_0;
  val.lock = SC_LOGIC_0;
  val.read = SC_LOGIC_1;
  val.write = SC_LOGIC_0;
  val.flush = SC_LOGIC_0;
  val.flushl = SC_LOGIC_0;
  val.dsuen = SC_LOGIC_0;
  val.msu = SC_LOGIC_0;
  val.esu = SC_LOGIC_0;
  val.intack = SC_LOGIC_0;

  dci.write(val);
    
}

void testbench_ct::dwrite(unsigned int address, unsigned int data, unsigned int length, unsigned int asi, unsigned int lock) {

  dcache_in_type val;

  val.asi = asi;
  val.maddress = address;
  val.eaddress = address;
  val.edata = data;
  val.size = length - 1;
  val.enaddr = SC_LOGIC_1;
  val.eenaddr = SC_LOGIC_1;
  val.nullify = SC_LOGIC_0;
  val.lock = SC_LOGIC_0;
  val.read = SC_LOGIC_0;
  val.write = SC_LOGIC_1;
  val.flush = SC_LOGIC_0;
  val.flushl = SC_LOGIC_0;
  val.dsuen = SC_LOGIC_0;
  val.msu = SC_LOGIC_0;
  val.esu = SC_LOGIC_0;
  val.intack = SC_LOGIC_0;

  dci.write(val);
      
}
