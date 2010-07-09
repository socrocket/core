
//#define DEBUG_AMBA
//#define AMBA_DEBUG
#ifdef DEBUG_AMBA
# define DUMP(name, msg) std::cout<<"@"<<sc_core::sc_time_stamp()<<" /"<<(unsigned)sc_core::sc_delta_count()<<" ("<<name  <<"): "<<msg<<std::endl
# define AMBA_DUMP(name, msg) std::cout<<"@"<<sc_core::sc_time_stamp()<<" /"<<(unsigned)sc_core::sc_delta_count()<<" ("<<name  <<"): "<<msg<<std::endl
#else
# define DUMP(name, msg) 
//# define AMBA_DUMP(name, msg) 
#endif

#include <sys/time.h>
#include <time.h>
#include "amba.h"
#include "adapters/AMBA_LT_CT_Adapter.h"
#include "modelsim/gptimer.h"
#include "ct_rtl.h"
#include "timreg.h"
#include "testbench.h"

using namespace std;

int sc_main(int argc, char** argv) {
  timeval tstart, tend;
  clock_t cstart, cend;

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

  sc_core::sc_clock                       clk("clock",sc_core::sc_time(10,sc_core::SC_NS));
  testbench<32>                           tb("testbench");
  amba::AMBA_LT_CT_Adapter<32>            ltct("LT_CT", amba::amba_APB);
  APB_CT_RTL                              ctrtl("CT_RTL", 0x0, 0x40);
  gptimer<0, 0, 4095, 0, 0, 16, 4, 32, 0> dut("timer", "gaisler.gptimer");

  tb.master_sock(ltct.slave_sock);
  tb.reset(reset);
  tb.gpti(gpti);
  tb.gpto(gpto);
  tb.pirqi(pirqi);
  tb.pirqo(pirqo);
  tb.pconfig_0(pconfig_0);
  tb.pconfig_1(pconfig_1);
  tb.pindex(pindex);

  ltct.master_sock(ctrtl.ct.slave_sock);
  ltct.clk(clk);

  ctrtl.clk(clk);
  ctrtl.reset(reset);
  ctrtl.apbi(apbi);
  ctrtl.apbo(apbo);
  ctrtl.pirqi(pirqi);
  ctrtl.pirqo(pirqo);
  ctrtl.pindex(pindex);
  ctrtl.pconfig_0(pconfig_0);
  ctrtl.pconfig_1(pconfig_1);

  dut.clk(clk);
  dut.rst(reset);
  dut.apbi(apbi);
  dut.apbo(apbo);
  dut.gpti(gpti);
  dut.gpto(gpto);

  pindex = 0;

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
  return 0;
}

