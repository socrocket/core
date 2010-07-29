/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       irqmp.h                                                 */
/*             header file defining the irqmp module template          */
/*             includes implementation file irqmp.tpp at the bottom    */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef IRQMP_H
#define IRQMP_H

#define CLOCK_PERIOD 2

#include <boost/config.hpp>
#include <systemc.h>
#include <greenreg.h>
#include <greenreg_ambasocket.h>
//#include <greenreg.h>
//#include <greenreg_socket/greenreg_ambasocket.h>

#include "greencontrol/all.h"

#include "multisignalhandler.h"
#include "irqmpsignals.h"


template <int pindex = 0, int paddr = 0, int pmask = 0xFFF, int ncpu = 2, int eirq = 1>
class Irqmp : public gs::reg::gr_device, public MultiSignalSender, public MultiSignalTarget<Irqmp> {
  public:
    //Slave socket with delayed switch; responsible for all bus communication
    gs::reg::greenreg_socket< gs::amba::amba_slave<32> > bus;
    gs_generic_signal::target_signal_multi_socket<
              Irqmp<pindex, paddr, pmask, ncpu, eirq> >  in;
    gs_generic_signal::initiator_signal_multi_socket     out;

    //Non-AMBA-Signals

    GC_HAS_CALLBACKS();
    SC_HAS_PROCESS(Irqmp);

    //constructor takes vhdl generics as parameters
    Irqmp(sc_core::sc_module_name name); // interrupt cascade for extended interrupts
    ~Irqmp();

    //function prototypes
    void end_of_elaboration();
    void reset_registers(unsigned int &id, gs_generic_signal_payload& trans, sc_core::sc_time& delay);

    //bus communication
    void clear_write();               //write to IR clear register
    void clear_forced_ir();           //write to IFC bits of IR force register
    void mpstat_write();              //write to MP status register
    void register_read();             //one read function for all registers

    //processor communication
    void register_irq(unsigned int &id, gs_generic_signal_payload& trans, sc_core::sc_time& delay);              //bus and processor communication
    void launch_irq();                //processor communication
    void clear_acknowledged_irq(unsigned int &id, gs_generic_signal_payload& trans, sc_core::sc_time& delay);    //processor communication

//    void clk(sc_core::sc_clock &clk);
//    void clk(sc_core::sc_time &period);
//    void clk(double &period, sc_core::sc_time_unit &base);

};


#include "irqmp.tpp"

#endif
