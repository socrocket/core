/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       apb_ct_rtl.h.h                                          */
/*             header file defining a generic APB CT to RTL adapter    */
/*             it enables the user to connect an grlib APB model       */
/*             to an AMBAKit CT bus                                    */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TU Braunschweig              */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef APB_CT_RTL_ADAPTER
#define APB_CT_RTL_ADAPTER

#include "adapters/APB_CT_RTL_Slave_Adapter.h"

// This class is a specific adapter to connect an 
// GRLIB APH Model to an AMBAKit APB CT Bus.
// 
//  Interrupt channels to the TLM components.
//  |              clk  rst - needed by the adapter
//  |                |  |
//  | The TLM Bus  +------+
//  |  ct.socket --| APB  |-- apbo 
//  |              |  CT  |        connected to the RTL Model
//  pirqi, pirqo --|  RTL |-- apbi
//                 +------+
//                  | |  |
//          pconfig_0/1  pindex
//          Device mapper output
//
class APB_CT_RTL : public sc_module {
  public:
    // A small subclass wich wraps the core functionality inhireted by amba::APB_CT_RTL_Slave_Adapter
    // It has knowledge about addressdecoding and translates between the TLM Port and RTL Signals.
    // But we need another class to map the RTL Signals to GRLIB Signals.
    class ct : public APB_CT_RTL_Slave_Adapter<ct>, public amba_slave_base {
      public:
        ct(sc_core::sc_module_name nm, sc_dt::uint64 base, sc_dt::uint64 size)
          : APB_CT_RTL_Slave_Adapter<ct, 32>(nm), m_base(base), m_size(size) {}

        sc_dt::uint64 get_base_addr()     { return m_base; }
        sc_dt::uint64 get_size()          { return m_size; }
      private:
        sc_dt::uint64                       m_base;
        sc_dt::uint64                       m_size;
    };

    SC_HAS_PROCESS(APB_CT_RTL);
    sc_core::sc_in_clk               clk;
    sc_core::sc_out<bool>            reset;

    sc_core::sc_in<apb_slv_out_type> apbo;
    sc_core::sc_out<apb_slv_in_type> apbi;

    sc_core::sc_in<uint32_t>     pirqi;
    sc_core::sc_out<uint32_t>    pirqo;
    sc_core::sc_out<uint32_t>    pconfig_0;
    sc_core::sc_out<uint32_t>    pconfig_1;
    sc_core::sc_out<uint16_t>    pindex;

  private:
    // Internal Signals to rerout the RTL Signals form the AMBA Adapter to the GRLIB format.
    sc_core::sc_signal<bool>     m_psel;
    sc_core::sc_signal<bool>     m_penable;
    sc_core::sc_signal<bool>     m_pwrite;
    sc_core::sc_signal<uint32_t> m_paddr;
    sc_core::sc_signal<uint32_t> m_pwdata;
    sc_core::sc_signal<bool>     m_pready;
    sc_core::sc_signal<uint32_t> m_prdata;
    sc_core::sc_signal<bool>     m_pslverr;
  public:
    ct ct;

  public:
    // Constructor: Simply give name, baseaddress and size as an argument.
    // After construction ensure that interrupt ports apbi apbo and the TLM Port
    // are connected before starting the simulation.
    APB_CT_RTL(sc_core::sc_module_name nm, sc_dt::uint64 base, sc_dt::uint64 size)
      : sc_module(nm), clk("CLOCK"), reset("RESET"), apbo("apbo"), apbi("apbi"),
        pirqi("GR_IRQ_IN"), pirqo("GR_IRQ_OUT"), pconfig_0("GR_CONFIG_0"),
        pconfig_1("GR_CONFIG_1"), pindex("GR_INDEX"), m_psel("APB_SELECT"),
        m_penable("APB_ENABLE"), m_pwrite("APB_WRITE"), m_paddr("APB_ADDRESS"),
        m_pwdata("APB_WRITE_DATA"), m_pready("APB_READY"), m_prdata("APB_READ_DATA"),
        m_pslverr("APB_SLAVE_ERROR"), ct("CT", base, size) {
      ct.m_clk(clk);
      ct.m_Reset(reset);
      ct.psel(m_psel);
      ct.penable(m_penable);
      ct.pwrite(m_pwrite);
      ct.paddr(m_paddr);
      ct.pwdata(m_pwdata);
      ct.pready(m_pready);   m_pready = 1;
      ct.prdata(m_prdata);
      ct.pslverr(m_pslverr); m_pslverr = 0;

      SC_THREAD(apbo_ctrl);
      sensitive << apbo;
      SC_THREAD(apbi_ctrl);
      sensitive << m_psel << m_penable << m_paddr << m_pwrite << m_pwdata << pirqi;
    }

    // Takes apbo inputs and converts them into TLM communication and irq signals.
    void apbo_ctrl() {
      while(1) {
        apb_slv_out_type val = apbo.read();
        m_prdata.write(val.prdata);
        pirqo.write(val.pirq);
        pconfig_0.write(val.pconfig[0]);
        pconfig_1.write(val.pconfig[1]);
        pindex.write(val.pindex);
        wait();
      }
    }

    // Collectes all data from the input ports and writes them into the apbi record for the GRLIB Model.
    void apbi_ctrl() {
      while(1) {
        apb_slv_in_type val;
        val.psel = ((bool)m_psel.read())? 0xFFFF : 0x0;
        val.penable = m_penable.read();
        val.paddr = m_paddr.read();
        val.pwrite = m_pwrite.read();
        val.pwdata = m_pwdata.read();
        val.pirq = pirqi.read();
        val.testen = false;
        val.scanen = false;
        val.testoen = false;
        apbi.write(val);
        wait();
      }
    }
};

#endif
