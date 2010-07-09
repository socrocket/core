
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
#include "timreg.h"
#include "gptimer.h"
#include "testbench.h"
#include "adapters/AMBA_LT_CT_Adapter.h"
#include "adapters/AMBA_CT_LT_Adapter.h"
#include "greencontrol/config.h" 
#include "greencontrol/analysis_vcd_outputplugin.h"

using namespace std;

int sc_main(int argc, char** argv) {
  timeval tstart, tend;
  clock_t cstart, cend;

#ifdef GETREGISTERS
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

#ifdef GETSIGNALS
  // sc_trace
  sc_core::sc_trace_file *wave = sc_core::sc_create_vcd_trace_file("signals");
#endif

  sc_core::sc_signal<bool>                reset("reset");
  sc_core::sc_signal<gptimer_in_type>     gpti("GPTIMER_IN");
  sc_core::sc_signal<gptimer_out_type>    gpto("GPTIMER_OUT");
  sc_core::sc_signal<sc_dt::sc_uint<32> > pirqi("GPTIMER_IRQ_IN");
  sc_core::sc_signal<sc_dt::sc_uint<32> > pirqo("GPTIMER_IRQ_OUT");
  sc_core::sc_signal<sc_dt::sc_uint<32> > pconfig_0("PCONFIG(0)");
  sc_core::sc_signal<sc_dt::sc_uint<32> > pconfig_1("PCONFIG(1)");
  sc_core::sc_signal<sc_dt::sc_uint<16> > pindex("PINDEX");

  testbench<32>                           tb("testbench");
#ifdef CTBUS
  sc_core::sc_clock                       clk("clock",sc_core::sc_time(10,sc_core::SC_NS));
  amba::AMBA_LT_CT_Adapter<32>            ltct("LT_CT", amba::amba_APB);
  amba::AMBA_CT_LT_Adapter<32>            ctlt("CT_LT", amba::amba_APB);
#endif
  Timer<>                                 dut("timer", 4);


  
  tb.reset(reset);
  tb.gpti(gpti);
  tb.gpto(gpto);
  tb.pirqi(pirqi);
  tb.pirqo(pirqo);
  tb.pconfig_0(pconfig_0);
  tb.pconfig_1(pconfig_1);
  tb.pindex(pindex);

#ifdef CTBUS
  tb.master_sock(ltct.slave_sock);
  ltct.master_sock(ctlt.slave_sock);
  ltct.clk(clk);

  ctlt.master_sock(dut.bus);
  ctlt.clk(clk);

  dut.clk(clk);
#else
  tb.master_sock(dut.bus);
  dut.clk(10.0, sc_core::SC_NS);
#endif
    
  dut.reset(reset);
  dut.gpti(gpti);
  dut.gpto(gpto);
  dut.pirqi(pirqi);
  dut.pirqo(pirqo);
  dut.pconfig_0(pconfig_0);
  dut.pconfig_1(pconfig_1);
  dut.pindex(pindex);

#ifdef GETREGISTERS
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

#ifdef GETSIGNALS
  sc_core::sc_trace(wave, clk, "clk");
  sc_core::sc_trace(wave, reset, "reset");
  sc_core::sc_trace(wave, pirqi, "pirqi");
  sc_core::sc_trace(wave, pirqo, "pirqo");
  sc_core::sc_trace(wave, gpto, "gpto");
  sc_core::sc_trace(wave, gpti, "gpti");
#endif

  gettimeofday(&tstart, 0);
  cstart = clock();
  sc_core::sc_start();
  cend = clock();
  gettimeofday(&tend, 0);
  cout << "Start time: " << dec << tstart.tv_sec << ", " << tstart.tv_usec << "s" << endl;
  cout << "Clocks: " << dec << setprecision(5) << ((double)(cend - cstart) / (double)CLOCKS_PER_SEC) << "s" << endl << endl;
  cout << "End time: " << dec << tend.tv_sec << ", " << tend.tv_usec << "s" << endl;
  cout << "Start clocks: " << dec << cstart << endl;
  cout << "End clocks: " << dec << cend << endl;
  cout << "Clocks per sec: " << dec << CLOCKS_PER_SEC << endl;
  return 0;
}

