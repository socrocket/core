/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       ahb_rtl_ct.h                                            */
/*             header file defining a generic AHB RTL to CT adapter    */
/*             it enables the user to connect an grlib AHB model       */
/*             to an AMBAKit CT bus                                    */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TU Braunschweig              */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef APB_CT_RTL_ADAPTER
#define APB_CT_RTL_ADAPTER

#include "adapters/AHB_Master_RTL_CT_Adapter.h"

/// @addtogroup utils Model Utils
/// @{

/// This class is a specific adapter to connect an 
/// GRLIB AHH Master to an AMBAKit AHB CT Bus.
/// 
///>  Interrupt channels to the TLM components.
///>  |              clk  rst - needed by the adapter
///>  |                |  |
///>  | The TLM Bus  +------+
///>  |  ct.socket --| AHB  |-- ahbo 
///>  |              | RTL  |        connected to the RTL Model
///>  hirqi, hirqo --|  CT  |-- ahbi
///>                 +------+
///>                  | |  |
///>          pconfig_0/1  pindex
///>          Device mapper output
///

/*
template<unsigned int BUSWIDTH, typename MODULE >
class AHB_Master_RTL_CT_Adapter:public sc_core::sc_module
{
public:

  //Socket to connect with AHBCT slave/bus
  amba::amba_master_socket<BUSWIDTH> master_sock;

  sc_core::sc_in_clk m_clk;
  sc_core::sc_in<bool> m_Reset;
  ///incoming signals from AHB RTL Master
  sc_core::sc_in<bool> m_HBUSREQ;
  sc_core::sc_in<bool> m_HLOCK;
  sc_core::sc_in<sc_dt::sc_uint<2> > m_HTRANS;
  sc_core::sc_in<sc_dt::sc_uint<32> >m_HADDR;
  sc_core::sc_in<sc_dt::sc_uint<3> > m_HSIZE;
  sc_core::sc_in<sc_dt::sc_uint<3> > m_HBURST;
  sc_core::sc_in<sc_dt::sc_uint<4> > m_HPROT;
  sc_core::sc_in<bool> m_HWRITE;
  sc_core::sc_in<sc_dt::sc_uint<BUSWIDTH> > m_HWDATA;

  ///Outgoing signal to AHB RTL Master
  sc_core::sc_out<sc_dt::sc_uint<BUSWIDTH> > m_HRDATA;
  sc_core::sc_out<sc_dt::sc_uint<2> >m_HRESP;
  sc_core::sc_out<bool> m_HGRANT;
  sc_core::sc_out<bool> m_HREADYIN;

-- AHB master outputs
  type ahb_mst_out_type is record
    hbusreq   : std_ulogic;                           -- bus request
    hlock     : std_ulogic;                           -- lock request
    htrans    : std_logic_vector(1 downto 0);         -- transfer type
    haddr     : std_logic_vector(31 downto 0);        -- address bus (byte)
    hwrite    : std_ulogic;                           -- read/write
    hsize     : std_logic_vector(2 downto 0);         -- transfer size
    hburst    : std_logic_vector(2 downto 0);         -- burst type
    hprot     : std_logic_vector(3 downto 0);         -- protection control
    hwdata    : std_logic_vector(31 downto 0);        -- write data bus
    hirq      : std_logic_vector(NAHBIRQ-1 downto 0); -- interrupt bus
    hconfig   : ahb_config_type;                      -- memory access reg.
    hindex    : integer range 0 to NAHBMST-1;         -- diagnostic use only
  end record;
  
-- AHB master inputs
  type ahb_mst_in_type is record
    hgrant    : std_logic_vector(0 to NAHBMST-1);     -- bus grant
    hready    : std_ulogic;                           -- transfer done
    hresp     : std_logic_vector(1 downto 0);         -- response type
    hrdata    : std_logic_vector(31 downto 0);        -- read data bus
    hcache    : std_ulogic;                           -- cacheable
    hirq      : std_logic_vector(NAHBIRQ-1 downto 0); -- interrupt result bus
    testen    : std_ulogic;                           -- scan test enable
    testrst   : std_ulogic;                           -- scan test reset
    scanen    : std_ulogic;                           -- scan enable
    testoen   : std_ulogic;                           -- test output enable
  end record;
*/

class AHB_RTL_CT : public sc_module {
  public:
    /// A small subclass wich wraps the core functionality inhireted by amba::AHB_Master_RTL_CT_Adapter
    /// It has knowledge about addressdecoding and translates between the TLM Port and RTL Signals.
    /// But we need another class to map the RTL Signals to GRLIB Signals.
    class ct : public AHB_Master_RTL_CT_Adapter<32, ct>, public amba_slave_base {
      public:
        ct(sc_core::sc_module_name nm, sc_dt::uint64 base, sc_dt::uint64 size)
          : APB_CT_RTL_Slave_Adapter<32, ct>(nm), m_base(base), m_size(size) {}

        sc_dt::uint64 get_base_addr()     { return m_base; }
        sc_dt::uint64 get_size()          { return m_size; }
      private:
        sc_dt::uint64                       m_base;
        sc_dt::uint64                       m_size;
    };

    SC_HAS_PROCESS(AHB_RTL_CT);
    sc_core::sc_in_clk               clk;
    sc_core::sc_out<bool>            reset;

    sc_core::sc_in<ahb_mst_out_type> ahbo;
    sc_core::sc_out<ahb_mst_in_type> ahbi;

    sc_core::sc_in<sc_uint32>     hirqi;
    sc_core::sc_out<sc_uint32>    hirqo;
    sc_core::sc_out<sc_uint32>    hconfig_0;
    sc_core::sc_out<sc_uint32>    hconfig_1;
    sc_core::sc_out<sc_uint16>    hindex;

  private:
    sc_core::sc_signal<bool> m_hbusreq;
    sc_core::sc_signal<bool> m_hlock;
    sc_core::sc_signal<sc_dt::sc_uint<2> > m_htrans;
    sc_core::sc_signal<sc_dt::sc_uint<32> >m_haddr;
    sc_core::sc_signal<sc_dt::sc_uint<3> > m_hsize;
    sc_core::sc_signal<sc_dt::sc_uint<3> > m_hburst;
    sc_core::sc_signal<sc_dt::sc_uint<4> > m_hprot;
    sc_core::sc_signal<bool> m_hwrite;
    sc_core::sc_signal<sc_dt::sc_uint<BUSWIDTH> > m_hwdata;

    ///Outgoing signal to AHB RTL Master
    sc_core::sc_signal<sc_dt::sc_uint<BUSWIDTH> > m_hrdata;
    sc_core::sc_signal<sc_dt::sc_uint<2> >m_hresp;
    sc_core::sc_signal<bool> m_hgrant;
    sc_core::sc_signal<bool> m_hreadyin;
  public:
    ct ct;

  public:
    /// Constructor: Simply give name, baseaddress and size as an argument.
    /// After construction ensure that interrupt ports ahbi ahbo and the TLM Port
    /// are connected before starting the simulation.
    APB_CT_RTL(sc_core::sc_module_name nm, sc_dt::uint64 base, sc_dt::uint64 size)
      : sc_module(nm), clk("CLOCK"), reset("RESET"), ahbo("ahbo"), ahbi("ahbi"),
        hirqi("GR_IRQ_IN"), hirqo("GR_IRQ_OUT"), hconfig_0("GR_CONFIG_0"),
        hconfig_1("GR_CONFIG_1"), hindex("GR_INDEX"), m_hbusreq("AHB_BUSREQ"),
        m_hlock("AHB_LOCK"), m_htrans("AHB_TRANS"), m_haddr("AHB_ADDRESS"),
        m_hsize("AHB_SIZE"), m_hburst("AHB_BURST"), m_prot("AHB_PROT"),
        m_hwrite("AHB_WRITE"), m_hwdata("AHB_WRITE_DATA"), m_hrdata("AHB_READ_DATA"), 
        m_hresp("AHB_RESPONSE"), m_hgrant("AHB_GRANT"), m_hreadyin("AHB_READY_IN"), 
        ct("CT", base, size) {
      ct.m_clk(clk);
      ct.m_Reset(reset);
      ct.m_HBUSREQ(m_hbusreq);
      ct.m_HLOCK(m_hlock);
      ct.m_HTRANS(m_htrans);
      ct.m_HADDR(m_haddr);
      ct.m_HSIZE(m_hsize);
      ct.m_HBURST(m_hburst);
      ct.m_HPROT(m_hprot);
      ct.m_HWRITE(m_hwrite);
      ct.m_HWDATA(m_hwdata);
      ct.m_HRDATA(m_hrdata);
      ct.m_HRESP(m_hresp);
      ct.m_HGRANT(m_hgrant);
      ct.m_HREADYIN(m_hreadyin);

      SC_THREAD(ahbo_ctrl);
      sensitive << ahbo;
      SC_THREAD(ahbi_ctrl);
      sensitive << m_hrdata << m_hresp << m_hgrand << m_hreadyin << hirqi;
    }

    /// Takes ahbo inputs and converts them into TLM communication and irq signals.
    void ahbo_ctrl() {
      while(1) {
        ahb_mst_out_type val = ahbo.read();
        m_hbusreq.write(val.hbusreq);
        m_hlock.write(val.hlock);
        m_htrans.write(val.htrans);
        m_haddr.write(val.haddr);
        m_hwrite.write(val.hwrite);
        m_hsize.write(val.hsize);
        m_hburst.write(val.hburst);
        m_hprot.write(val.hprot);
        m_hwdata.write(val.hwdata);
        
        hirqo.write(val.hwirq);
        hconfig_0.write(val.hconfig[0]);
        hconfig_1.write(val.hconfig[1]);
        hindex.write(val.hindex);
        wait();
      }
    }

    /// Collectes all data from the input ports and writes them into the ahbi record for the GRLIB Model.
    void ahbi_ctrl() {
      while(1) {
        ahb_mst_in_type val;
        val.hgrant = (m_hgrant.read())? 0xFFFFFFFF : 0x0;
        val.hready = m_hreadyin.read();
        val.hresp = m_hresp.read();
        val.hrdata = m_hrdata.read();
        val.hcache = 0;
        val.hirq = hirqi.read();
        val.testen = false;
        val.testrst = false;
        val.scanen = false;
        val.testoen = false;
        apbi.write(val);
        wait();
      }
    }
};

/// @}

#endif
