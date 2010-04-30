
//#define DEBUG_AMBA
//#define AMBA_DEBUG
#ifdef DEBUG_AMBA
# define DUMP(name, msg) std::cout<<"@"<<sc_core::sc_time_stamp()<<" /"<<(unsigned)sc_core::sc_delta_count()<<" ("<<name  <<"): "<<msg<<std::endl
# define AMBA_DUMP(name, msg) std::cout<<"@"<<sc_core::sc_time_stamp()<<" /"<<(unsigned)sc_core::sc_delta_count()<<" ("<<name  <<"): "<<msg<<std::endl
#else
# define DUMP(name, msg) 
//# define AMBA_DUMP(name, msg) 
#endif

#include "amba.h"
#include "adapters/AMBA_LT_CT_Adapter.h"
#include "modelsim/gptimer.h"
#include "ct_rtl.h"
#include "timreg.h"
#include "testbench.h"

int sc_main(int argc, char** argv) {
  sc_core::sc_signal<bool >               penable("APB_ENABLE")
                                        , pwrite("APB_WRITE")
                                        , reset("RESET")
                                        , testen("TEST_ENABLE")
                                        , testrst("TEST_RESTART")
                                        , testoen("TEST_OUT_ENABLE")
                                        , scanen("SCAN_ENABLE")
                                        , dhalt("DHALT")
                                        , wdog("WATCH_DOG")
                                        , wdogn("NOT_WATCH_DOG");
  sc_core::sc_signal<sc_dt::sc_uint< 8> > tick("TICK");
  sc_core::sc_signal<sc_dt::sc_uint<16> > psel("GR_SELECT")
                                        , pindex("GR_INDEX");
  sc_core::sc_signal<sc_dt::sc_uint<32> > paddr("APB_ADDRESS")
                                        , pwdata("APB_WRITE_DATA")
                                        , prdata("APB_READ_DATA")
                                        , pirqi("GR_APB_IRQ_IN")
                                        , pirqo("GR_APB_IRQ_OUT")
                                        , timer1("TIMER1")
                                        , pconfig_0("PCONFIG_0")
                                        , pconfig_1("PCONFIG_1");
  sc_core::sc_signal<apb_slv_in_type>     apbi("apbi");
  sc_core::sc_signal<apb_slv_out_type>    apbo("apbo");
  sc_core::sc_signal<gptimer_in_type>     gpti("gpti");
  sc_core::sc_signal<gptimer_out_type>    gpto("gpto");

  testbench<32> testbench("testbench");
  amba::AMBA_LT_CT_Adapter<32> lt_ct("LT_CT", amba::amba_APB);
  ct_rtl ct_rtl("CT_RTL", 0x0, 0x40);
  gptimer<0, 0, 4095, 0, 0, 16, 4, 32, 0> timer("timer", "gaisler.gptimer");
  sc_core::sc_clock clk("clock",sc_core::sc_time(10,sc_core::SC_NS));

  testbench.master_sock(lt_ct.slave_sock);
  testbench.reset(reset);
  testbench.gpti(gpti);
  testbench.gpto(gpto);
  testbench.pirqi(pirqi);
  testbench.pirqo(pirqo);
  testbench.pconfig_0(pconfig_0);
  testbench.pconfig_1(pconfig_1);
  testbench.pindex(pindex);

  lt_ct.master_sock(ct_rtl.m_rtl.slave_sock);

  ct_rtl.apbi(apbi);
  ct_rtl.apbo(apbo);
  ct_rtl.pirqi(pirqi);
  ct_rtl.pirqo(pirqo);
  ct_rtl.pindex(pindex);
  ct_rtl.reset(reset);
  ct_rtl.pconfig_0(pconfig_0);
  ct_rtl.pconfig_1(pconfig_1);
  timer.apbi(apbi);
  timer.apbo(apbo);
  timer.gpti(gpti);
  timer.gpto(gpto);
  pindex = 0;

  lt_ct.clk(clk);
  ct_rtl.clk(clk);
  timer.clk(clk);
  timer.rst(reset);

  sc_core::sc_start();
  return 0;
}

