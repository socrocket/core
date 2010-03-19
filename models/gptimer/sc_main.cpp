#include <boost/config.hpp>
#include <systemc>
#include <greenreg.h>
#include <greenreg_socket.h>
#include <iostream>
#include <iomanip>

#include "greencontrol/all.h"

#include "Timer.h"
#include "timreg.h"

#define SHOW \
{ std::printf("\n@%-7s /%-4d:", sc_core::sc_time_stamp().to_string().c_str(), (unsigned)sc_core::sc_delta_count());}
#define REG(name) \
{ std::cout << " "#name << ": 0x" << std::hex << std::setfill('0') << std::setw(2) << bus->read(name, 4); }
#define SET(name, val) \
{ bus->write(name, val, 4); }

/// Device sending register accesses over the bus
class Testbench : public gs::reg::gr_device {
  public:
    /* IO */
    gs::reg::greenreg_socket< gs::gp::generic_master> bus;	// TLM Master Port declaration

    sc_core::sc_out<bool>     reset;
    sc_core::sc_out<sc_dt::sc_logic> dhalt;
    
    sc_core::sc_in<sc_dt::sc_logic>  wdog, wdogn;
    sc_core::sc_in<sc_dt::sc_lv<8> > tick;

    SC_HAS_PROCESS( Testbench );
	
    Testbench(sc_core::sc_module_name name)
      : gr_device(name, gs::reg::INDEXED_ADDRESS, 2, NULL), bus( "dut" ) { // TLM bus master socket
      SC_THREAD( test_thread );
    }
  
    void test_thread() {
      unsigned int value;

      std::cout << std::endl << "Simulation begin" << std::endl << std::endl;
      /* Read configuration defaults */
      reset = 1;
      SHOW REG(TIM_SCALER) REG(TIM_VALUE(0)) REG(TIM_VALUE(1));
      wait(1, sc_core::SC_NS);
      reset = 0;
      SHOW REG(TIM_SCALER) REG(TIM_VALUE(0)) REG(TIM_VALUE(1));
      wait(1, sc_core::SC_NS);

      value = bus->read(TIM_CONF, 4);
      std::cout << "Configure: 0x" << std::hex << value << std::endl;
      
      SET(TIM_SCRELOAD,  0x01);
      SET(TIM_SCALER  ,  0x01);
      
      SET(TIM_RELOAD(0), 0x03);
      SET(TIM_VALUE(0),  0x01);
      SET(TIM_CTRL(0) ,  0x4F);
        
      SET(TIM_RELOAD(1), 0x02);
      SET(TIM_VALUE(1),  0x01);
      SET(TIM_CTRL(1) ,  0x6F);
        
      SHOW REG(TIM_SCALER) REG(TIM_VALUE(0)) REG(TIM_VALUE(1));
      wait(1, sc_core::SC_NS);
      
      for(int i=0;i<200;i++) {
        SHOW;
        REG(TIM_SCALER);
        REG(TIM_VALUE(0));
        REG(TIM_VALUE(1));
        wait(1, sc_core::SC_NS);
      }
      
      std::cout << std::endl << "Simulation end" << std::endl << std::endl;
      sc_core::sc_stop();
    }
  
};

int sc_main(int argc, char** argv) {
  /* Signals */
  sc_core::sc_signal<bool> reset;
  sc_core::sc_signal<sc_dt::sc_logic> wdog, wdogn, dhalt;
  sc_core::sc_signal<sc_dt::sc_lv<8> > tick;
  
  sc_core::sc_report_handler::set_actions(sc_core::SC_INFO, sc_core::SC_DISPLAY);
  
  /* Configure the Register behaviour
   * Configure warnings about accesses to write protected register bits:
   */
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_current", sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/unequal_zero", sc_core::SC_DO_NOTHING);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/bit_range_access", sc_core::SC_DISPLAY);
  sc_core::sc_report_handler::set_actions("/GreenSocs/GreenReg/write_protected/bit_access", sc_core::SC_DISPLAY);
  
  GS_INIT_STANDARD_GREENCONTROL;
  
  /* Create devices */
  Timer dut("Timer", 0x4, true, 4);
  Testbench tb("Testbench");
  
  /* Connect socets */
  //test_send_dev.m_master_socket.get_bus_port()(  slave_dev.m_slave_socket.get_bus_port()  );
  tb.bus(dut.bus);
  
  dut.reset(reset);
  dut.dhalt(dhalt);
  dut.wdog(wdog);
  dut.wdogn(wdogn);
  dut.tick(tick);

  tb.reset(reset);
  tb.dhalt(dhalt);
  tb.wdog(wdog);
  tb.wdogn(wdogn);
  tb.tick(tick);
  
  /* Start simulation */
  sc_core::sc_start();
  
  /* End simulation */
  return 0;
}
