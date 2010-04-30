
//#define DEBUG_AMBA
//#define AMBA_DEBUG
#ifdef DEBUG_AMBA
# define DUMP(name, msg) std::cout<<"@"<<sc_core::sc_time_stamp()<<" /"<<(unsigned)sc_core::sc_delta_count()<<" ("<<name  <<"): "<<msg<<std::endl
//# define AMBA_DUMP(name, msg) std::cout<<"@"<<sc_core::sc_time_stamp()<<" /"<<(unsigned)sc_core::sc_delta_count()<<" ("<<name  <<"): "<<msg<<std::endl
#else
# define DUMP(name, msg) 
//# define AMBA_DUMP(name, msg) 
#endif

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

const char *generics[] = {
  "gpindex=0",
  "gpaddr=0",
  "gpmask=16#fff#",
  "gpirq=0",
  "sepirq=0",
  "sbits=16",
  "ntimers=4",
  "nbits=32",
  "gwdog=0"
};

typedef sc_core::sc_signal<sc_dt::sc_uint<32> > signal_uint32_t;

int sc_main(int argc, char** argv) {
//  sc_core::sc_report_handler::set_actions(sc_core::SC_ERROR, sc_core::SC_ABORT);    // make a breakpoint in SystemC file sc_stop_here.cpp
//  sc_core::sc_report_handler::set_actions(sc_core::SC_WARNING, sc_core::SC_ABORT);  // make a breakpoint in SystemC file sc_stop_here.cpp
//  sc_core::sc_report_handler::set_actions(sc_core::SC_INFO, sc_core::SC_DO_NOTHING);

 
  // GreenControl Core instance
  gs::ctr::GC_Core       core("ControlCore");

  // GreenConfig Plugin
  gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
  gs::cnf::ConfigPlugin configPlugin("ConfigPlugin", &cnfdatabase);

  // GreenAV Plugin
  //gs::av::GAV_Plugin vcd("AnalysisPlugin", gs::av::STDOUT_OUT);
  gs::av::GAV_Plugin vcd_plugin("AnalysisPlugin", gs::av::VCD_FILE_OUT);

  gs::cnf::cnf_api* GCF = gs::cnf::GCnf_Api::getApiInstance(NULL);
  boost::shared_ptr<gs::av::GAV_Api>  GAV = gs::av::GAV_Api::getApiInstance(NULL); 
  gs::av::OutputPlugin_if* vcd = GAV->create_OutputPlugin(gs::av::VCD_FILE_OUT, "gav.vcd");

  sc_core::sc_trace_file *wave = sc_core::sc_create_vcd_trace_file("top_LT.vcd");
  sc_core::sc_signal<bool>                reset("reset");
  sc_core::sc_signal<gptimer_in_type>     gpti("GPTIMER_IN");
  sc_core::sc_signal<gptimer_out_type>    gpto("GPTIMER_OUT");
  sc_core::sc_signal<sc_dt::sc_uint<32> > pirqi("GPTIMER_IRQ_IN");
  sc_core::sc_signal<sc_dt::sc_uint<32> > pirqo("GPTIMER_IRQ_OUT");
  sc_core::sc_signal<sc_dt::sc_uint<32> > pconfig_0("PCONFIG(0)");
  sc_core::sc_signal<sc_dt::sc_uint<32> > pconfig_1("PCONFIG(1)");
  sc_core::sc_signal<sc_dt::sc_uint<16> > pindex("PINDEX");

  sc_core::sc_clock clk("clock",sc_core::sc_time(10,sc_core::SC_NS));
  testbench<32> testbench("testbench");
  amba::AMBA_LT_CT_Adapter<32> lt_ct("LT_CT", amba::amba_APB);
  amba::AMBA_CT_LT_Adapter<32> ct_lt("CT_LT", amba::amba_APB);
  Timer timer("timer", 0x0, true, 4);

  testbench.master_sock(lt_ct.slave_sock);
  testbench.reset(reset);
  testbench.gpti(gpti);
  testbench.gpto(gpto);
  testbench.pirqi(pirqi);
  testbench.pirqo(pirqo);
  testbench.pconfig_0(pconfig_0);
  testbench.pconfig_1(pconfig_1);
  testbench.pindex(pindex);

  lt_ct.master_sock(ct_lt.slave_sock);
  lt_ct.clk(clk);

  ct_lt.master_sock(timer.bus);
  ct_lt.clk(clk);

  timer.wave = wave;
  timer.clk(clk);
  timer.reset(reset);
  timer.gpti(gpti);
  timer.gpto(gpto);
  timer.pirqi(pirqi);
  timer.pirqo(pirqo);
  timer.pconfig_0(pconfig_0);
  timer.pconfig_1(pconfig_1);
  timer.pindex(pindex);

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

/*
  sc_core::sc_trace(wave, clk, "clk");
  sc_core::sc_trace(wave, reset, "reset");
  sc_core::sc_trace(wave, pirqi, "pirqi");
  sc_core::sc_trace(wave, pirqo, "pirqo");
  sc_core::sc_trace(wave, pconfig_0, "pconfig.0");
  sc_core::sc_trace(wave, pconfig_1, "pconfig.1");
  sc_core::sc_trace(wave, pindex, "pindex");
    sc_trace(wave, timer.r[0x00], "gptimer.scale");
    sc_trace(wave, timer.r[0x04], "gptimer.screload");
    sc_trace(wave, timer.r[0x08], "gptimer.config");
    sc_trace(wave, timer.r[0x10], "gptimer.value0");
    sc_trace(wave, timer.r[0x14], "gptimer.reload0");
    sc_trace(wave, timer.r[0x18], "gptimer.ctrl0");
    sc_trace(wave, timer.r[0x20], "gptimer.value1");
    sc_trace(wave, timer.r[0x24], "gptimer.reload1");
    sc_trace(wave, timer.r[0x28], "gptimer.ctrl1");
    sc_trace(wave, timer.r[0x30], "gptimer.value2");
    sc_trace(wave, timer.r[0x34], "gptimer.reload2");
    sc_trace(wave, timer.r[0x38], "gptimer.ctrl2");
*/
  sc_core::sc_start();

//  sc_core::ShowSCObjects::showSCObjects();
  // ** List of all parameters
  std::cout << "--------------------------------------------------------" << std::endl;
  std::cout << "Parameter list: "<< std::endl << std::endl;
  std::vector<std::string> parlist = GCF->getParamList();
  for (unsigned int i = 0; i < parlist.size(); i++)
    std::cout << "  " << parlist[i] << std::endl;
  std::cout << std::endl << std::endl;

  sc_core::sc_close_vcd_trace_file(wave);
  return 0;
}

