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

#include "signalkit.h"

class Irqmp : public gs::reg::gr_device, public signalkit::signal_module<Irqmp> {
  public:
    //Slave socket with delayed switch; responsible for all bus communication
    gs::reg::greenreg_socket< gs::amba::amba_slave<32> > bus;

    signal<bool>::in           rst;
    signal<bool>::selector     cpu_rst;
    signal<uint32_t>::selector irq_req;
    signal<uint32_t>::infield  irq_ack;
    signal<uint32_t>::infield  irq_in;

    //Non-AMBA-Signals

    GC_HAS_CALLBACKS();
    SC_HAS_PROCESS(Irqmp);

    //constructor takes vhdl generics as parameters
    Irqmp(sc_core::sc_module_name name, int _pindex = 0, int _paddr = 0, int _pmask = 0xFFF, int _ncpu = 2, int _eirq = 1); // interrupt cascade for extended interrupts
    ~Irqmp();

    //function prototypes
    void end_of_elaboration();
    void reset_registers(const bool &value, signalkit::signal_in_if<bool> *signal, signalkit::signal_out_if<bool> *sender, const sc_core::sc_time &time);

    //bus communication
    void clear_write();               //write to IR clear register
    void clear_forced_ir();           //write to IFC bits of IR force register
    void mpstat_write();              //write to MP status register
    void register_read();             //one read function for all registers

    //processor communication
    void register_irq(const uint32_t &cleared_irq, const unsigned int &i_cpu, signalkit::signal_in_if<uint32_t> *signal, signalkit::signal_out_if<uint32_t> *sender, const sc_core::sc_time &time);
                                      //bus and processor communication
    void launch_irq();                //processor communication
    void clear_acknowledged_irq(const uint32_t &cleared_irq, const unsigned int &i_cpu, signalkit::signal_in_if<uint32_t> *signal, signalkit::signal_out_if<uint32_t> *sender, const sc_core::sc_time &time);
                                      //processor communication

//    void clk(sc_core::sc_clock &clk);
//    void clk(sc_core::sc_time &period);
//    void clk(double &period, sc_core::sc_time_unit &base);
  private:
    const int ncpu;
    const int eirq;

};


#include "irqmp.tpp"

#endif
