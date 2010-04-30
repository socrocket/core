#include "amba.h"
#include "timreg.h"

#define SHOW \
{ std::printf("\n@%-7s /%-4d:", sc_core::sc_time_stamp().to_string().c_str(), (unsigned)sc_core::sc_delta_count());}
#define REG(name) \
{ std::cout << " "#name << ": 0x" << std::hex << std::setfill('0') << std::setw(2) << read(name, 4); }
#define SET(name, val) \
{ write(name, val, 4); }

template<unsigned int BUSWIDTH>
class testbench : public sc_core::sc_module {
  public:
    amba::amba_master_socket<BUSWIDTH>   master_sock;
    sc_core::sc_out<bool>                reset;
    sc_core::sc_out<gptimer_in_type>     gpti;
    sc_core::sc_in<gptimer_out_type>     gpto;

    sc_core::sc_out<sc_dt::sc_uint<32> > pirqi;
    sc_core::sc_in<sc_dt::sc_uint<32> >  pirqo;
    sc_core::sc_in<sc_dt::sc_uint<32> >  pconfig_0;
    sc_core::sc_in<sc_dt::sc_uint<32> >  pconfig_1;
    sc_core::sc_in<sc_dt::sc_uint<16> >  pindex;

    SC_HAS_PROCESS(testbench);

    testbench(sc_core::sc_module_name nm)
    : master_sock("socket", amba::amba_APB, amba::amba_LT, false)
    , reset("RESET"), gpti("GPTIMER_INPUT"), gpto("GPTIMER_OUTPUT"), pirqi("GP_IRQ_TO_TIMER"), pirqo("GP_IRQ_FROM_TIMER"), pconfig_0("PCONFIG_0"), pconfig_1("PCONFIG_1"), pindex("PINDEX")  {
      SC_THREAD(run);

      SC_METHOD(ticks);
      sensitive << gpto;
    }
    
    void write(uint32_t addr, uint32_t data, uint32_t width) {
      sc_core::sc_time t;
      tlm::tlm_generic_payload *gp = master_sock.get_transaction();
      gp->set_command(tlm::TLM_WRITE_COMMAND);
      gp->set_address(addr);
      gp->set_data_length(width);
      gp->set_streaming_width(4);
      gp->set_byte_enable_ptr(NULL);
      gp->set_data_ptr((unsigned char*)&data);
      master_sock->b_transport(*gp,t);
      //SHOW;
      //std::cout << " WRITE " << gp->get_response_string() << ": 0x" << std::hex << std::setfill('0') << std::setw(2) << gp->get_address();
      wait(t); 
      master_sock.release_transaction(gp);
    }

    uint32_t read(uint32_t addr, uint32_t width) {
      sc_core::sc_time t;
      uint32_t data;
      tlm::tlm_generic_payload *gp = master_sock.get_transaction();
      gp->set_command(tlm::TLM_READ_COMMAND);
      gp->set_address(addr);
      gp->set_data_length(width);
      gp->set_streaming_width(4);
      gp->set_byte_enable_ptr(NULL);
      gp->set_data_ptr((unsigned char*)&data);
      gp->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
      master_sock->b_transport(*gp,t);
      //SHOW;
      //std::cout << " READ " << gp->get_response_string() << ": 0x" << std::hex << std::setfill('0') << std::setw(2) << gp->get_address();
      wait(t); 
      master_sock.release_transaction(gp);
      return data;
    }

    void run() {
      sc_core::sc_time t;
      //trivial one word write
      reset = 0;
      wait(30, sc_core::SC_NS);
      reset = 1;
      wait(20, sc_core::SC_NS);

      SET(TIM_SCRELOAD,  0x04);
      SET(TIM_SCALER  ,  0x04);

      SET(TIM_RELOAD(0), 0x03);
      SET(TIM_VALUE(0),  0x01);
      SET(TIM_CTRL(0) ,  0x4F);

      SET(TIM_RELOAD(1), 0x02);
      SET(TIM_VALUE(1),  0x01);
      SET(TIM_CTRL(1) ,  0x6F);

      SET(TIM_RELOAD(2), 0x05);
      SET(TIM_VALUE(2),  0x01);
      SET(TIM_CTRL(2) ,  0x4F);

      //SHOW REG(TIM_SCALER) REG(TIM_VALUE(0)) REG(TIM_VALUE(1));
      wait(10, sc_core::SC_NS);

      /*for(int i=0;i<100;i++) {
        //SHOW;
        //REG(TIM_SCALER);
        //REG(TIM_VALUE(0));
        //REG(TIM_VALUE(1));
        wait(10, sc_core::SC_NS);
      }*/
      wait(400, sc_core::SC_NS);
      gptimer_in_type in;
      in.dhalt = 1;
      gpti.write(in);
      SHOW; std::cout <<" DHALT";
      wait(100, sc_core::SC_NS);
      in.dhalt = 0;
      gpti.write(in);
      SHOW; std::cout <<" !DHALT";
      wait(400, sc_core::SC_NS);

      sc_core::sc_stop();
    }

    void ticks() {
      SHOW;
      gptimer_out_type val = gpto.read();
      std::cout << "Tick: " << std::hex << val.tick;// << std::endl;
    }
};

