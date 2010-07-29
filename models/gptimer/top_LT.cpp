/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       top_LT.cpp                                              */
/*             source file containing the top-level instantiation      */
/*             for all tlm testbenches to test the tlm gptimer model.  */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

// Precompiler switches:
// GETREGISTERS - Enables the logging of all GreenReg Registers into register.vcd
// GETSIGNALS   - Enables the logging of all SystemC Signals into signals.vcd
// DEBUG_AMBA   - Enables AMBAKit Debuging output
// CTBUS        - Implements the Bus with CT Adappters. Default is LT
// 
// More switches are in testbench.h

//#define DEBUG_AMBA
//#define AMBA_DEBUG
#ifdef DEBUG_AMBA
# define DUMP(name, msg) std::cout<<"@"<<sc_core::sc_time_stamp()<<" /"<<(unsigned)sc_core::sc_delta_count()<<" ("<<name  <<"): "<<msg<<std::endl
//# define AMBA_DUMP(name, msg) std::cout<<"@"<<sc_core::sc_time_stamp()<<" /"<<(unsigned)sc_core::sc_delta_count()<<" ("<<name  <<"): "<<msg<<std::endl
#else
# define DUMP(name, msg) 
//# define AMBA_DUMP(name, msg) 
#endif

#include <sys/time.h>
#include <time.h>
#include <boost/tokenizer.hpp>
#include <systemc>
#include "amba.h"
#include "gptimerregisters.h"
#include "gptimer.h"
#include "tests/speedtest.h"

#include "adapters/AMBA_LT_CT_Adapter.h"
#include "adapters/AMBA_CT_LT_Adapter.h"
#include "greencontrol/config.h" 
#include "greencontrol/analysis_vcd_outputplugin.h"

using namespace std;

int sc_main(int argc, char** argv) {
  timeval tstart, tend;
  clock_t cstart, cend;

#ifdef GETREGISTERS
  // Prepare register logging.
  // GreenControl Core instance
  gs::ctr::GC_Core       core("ControlCore");

  // GreenConfig Plugin
  gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin("ConfigPlugin", &cnfdatabase);

  // GreenAV Plugin
  gs::av::GAV_Plugin vcd_plugin("AnalysisPlugin", gs::av::VCD_FILE_OUT);
  gs::cnf::cnf_api* GCF = gs::cnf::GCnf_Api::getApiInstance(NULL);
  boost::shared_ptr<gs::av::GAV_Api>  GAV = gs::av::GAV_Api::getApiInstance(NULL); 
  gs::av::OutputPlugin_if* vcd = GAV->create_OutputPlugin(gs::av::VCD_FILE_OUT, "register.vcd");
#endif

  sc_core::sc_signal<bool>                reset("reset");

  testbench                               tb("testbench");
#ifdef CTBUS
  // Prepare the usage of the CT Bus
  sc_core::sc_clock                       clk("clock",sc_core::sc_time(10,sc_core::SC_NS));
  amba::AMBA_LT_CT_Adapter<32>            ltct("LT_CT", amba::amba_APB);
  amba::AMBA_CT_LT_Adapter<32>            ctlt("CT_LT", amba::amba_APB);
#endif
  Timer<>                                 dut("timer", 4);
  
  dut.out(tb.out);
  tb.out.set_source<set_irq>(dut.out.name());
  tb.out.set_source<clr_irq>(dut.out.name());
  tb.out.set_source<tick>(dut.out.name());
  
  tb.in(dut.in);
  dut.in.set_source<rst>(tb.in.name());
  dut.in.set_source<dhalt>(tb.in.name());

#ifdef CTBUS
  // Use CT Bus implementation to connect testbench and timer
  tb.master_sock(ltct.slave_sock);
  ltct.master_sock(ctlt.slave_sock);
  ltct.clk(clk);

  ctlt.master_sock(dut.bus);
  ctlt.clk(clk);

  dut.clk(clk);
#else
  // Connect timer and testbench directly. LT connection
  tb.master_sock(dut.bus);
  dut.clk(10.0, sc_core::SC_NS);
#endif
    
#ifdef GETREGISTERS
  // Register all GPTimer registers for logging
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.scaler"));
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.screload"));
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.conf"));
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.value_0"));
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.reload_0"));
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.ctrl_0"));
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.value_1"));
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.reload_1"));
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.ctrl_1"));
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.value_2"));
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.reload_2"));
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.ctrl_2"));
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.value_3"));
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.reload_3"));
  GAV->add_to_output(vcd, GCF->getPar("timer.default_registers.ctrl_3"));
#endif

  // Get times for execution speed tests.
  gettimeofday(&tstart, 0);
  cstart = clock();

  // Start simulation until the testbench exits.
  sc_core::sc_start();

  // Get times for comparison
  cend = clock();
  gettimeofday(&tend, 0);

  // Print execution speedtest results.
  cout << "Start time: " << dec << tstart.tv_sec << ", " << tstart.tv_usec << "s" << endl;
  cout << "End time: " << dec << tend.tv_sec << ", " << tend.tv_usec << "s" << endl;
  cout << "Start clocks: " << dec << cstart << endl;
  cout << "End clocks: " << dec << cend << endl;
  cout << "Clocks per sec: " << dec << CLOCKS_PER_SEC << endl;
  cout << "Clocks: " << dec << setprecision(5) << ((double)(cend - cstart) / (double)CLOCKS_PER_SEC) << "s" << endl << endl;
  return 0;
}

