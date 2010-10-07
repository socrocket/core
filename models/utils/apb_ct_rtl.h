//*********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is 
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      apb_ct_rtl.h.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    header file defining a generic APB CT to RTL adapter
//             it enables the user to connect an grlib APB model
//             to an AMBAKit CT bus
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TU Braunschweig
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#ifndef APB_CT_RTL_ADAPTER
#define APB_CT_RTL_ADAPTER

#include <systemc.h>
#include <amba.h>
#include <adapters/APB_CT_RTL_Slave_Adapter.h>
#include "signalkit.h"

/// @addtogroup utils
/// @{

/// This class is a specific adapter to connect an
/// GRLIB APH Model to an AMBAKit APB CT Bus.
///
///>  Interrupt channels to the TLM components.
///>  |              clk  rst - needed by the adapter
///>  |                |  |
///>  | The TLM Bus  +------+
///>  |  ct.socket --| APB  |-- apbo
///>  |              |  CT  |        connected to the RTL Model
///>  pirqi, pirqo --|  RTL |-- apbi
///>                 +------+
///>                  | |  |
///>          pconfig_0/1  pindex
///>          Device mapper output
///
class CAPB_CT_RTL : public sc_module, public signalkit::signal_module<CAPB_CT_RTL> {
  public:
    /// A small subclass wich wraps the core functionality inhireted by amba::APB_CT_RTL_Slave_Adapter
    /// It has knowledge about addressdecoding and translates between the TLM Port and RTL Signals.
    /// But we need another class to map the RTL Signals to GRLIB Signals.
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

    SC_HAS_PROCESS(CAPB_CT_RTL);
    sc_core::sc_in_clk                      clk;
    signal<bool>::in                        rst;

    sc_core::sc_in<apb_slv_out_type>        apbo;
    sc_core::sc_out<apb_slv_in_type>        apbi;

    signal<uint32_t>::in                    pirqi;
    signal<uint32_t>::out                   pirqo;
    signal<uint32_t>::out                   pconfig_0;
    signal<uint32_t>::out                   pconfig_1;
    signal<uint16_t>::out                   pindex;

  private:
    // Internal Signals to rerout the RTL Signals form the AMBA Adapter to the GRLIB format.
    sc_core::sc_signal<bool>                m_reset;
    sc_core::sc_signal<bool>                m_psel;
    sc_core::sc_signal<bool>                m_penable;
    sc_core::sc_signal<bool>                m_pwrite;
    sc_core::sc_signal<sc_dt::sc_uint<32> > m_paddr;
    sc_core::sc_signal<sc_dt::sc_uint<32> > m_pwdata;
    sc_core::sc_signal<bool>                m_pready;
    sc_core::sc_signal<sc_dt::sc_uint<32> > m_prdata;
    sc_core::sc_signal<bool>                m_pslverr;
    sc_core::sc_signal<sc_dt::sc_uint<32> > m_irqi;

  public:
    ct ct;

  public:
    /// Constructor: Simply give name, baseaddress and size as an argument.
    /// After construction ensure that interrupt ports apbi apbo and the TLM Port
    /// are connected before starting the simulation.
    CAPB_CT_RTL(sc_core::sc_module_name nm, sc_dt::uint64 base, sc_dt::uint64 size);

    void onreset(const bool &value, signalkit::signal_in_if<bool> *signal, signalkit::signal_out_if<bool> *sender, const sc_core::sc_time &time);

    void onirq(const uint32_t &value, signalkit::signal_in_if<uint32_t> *signal, signalkit::signal_out_if<uint32_t> *sender, const sc_core::sc_time &time);

    /// Takes apbo inputs and converts them into TLM communication and irq signals.
    void apbo_ctrl();

    /// Collectes all data from the input ports and writes them into the apbi record for the GRLIB Model.
    void apbi_ctrl();
};

/// @}

#endif
