/***********************************************************************/
/* Project:    Hardware-Software Co-Simulation SoC Validation Platform */
/*                                                                     */
/* File:       irqmp.h                                                 */
/*             header file defining the irqmp module template          */
/*             includes implementation file irqmp.tpp at the bottom    */
/*                                                                     */
/* Date:       18.05.2010                                              */
/* Revision:   1                                                       */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef IRQMP_H
#define IRQMP_H

#include <boost/config.hpp>
#include <systemc.h>
#include <greenreg.h>
#include <greenreg_ambasocket.h>

#include "greencontrol/all.h"

#include <string>
#include <ostream>
#include <vector>

/*GRLIB records translated to SC structs by modelsim scgenmod command*/

//apb input type
struct apb_slv_in_type {
    sc_uint<16> psel;
    bool penable;
    sc_uint<32> paddr;
    bool pwrite;
    sc_uint<32> pwdata;
    sc_uint<32> pirq;
    bool testen;
    bool testrst;
    bool scanen;
    bool testoen;
};

//operators overloaded for the struct
inline ostream& operator<<(ostream& os, const apb_slv_in_type& a) {return os;}
inline void sc_trace(sc_trace_file *, const apb_slv_in_type&, const std::string &) {}
inline int operator== (const apb_slv_in_type& left, const apb_slv_in_type& right) {return 0;}

//apb output type
struct apb_slv_out_type {
    sc_uint<32> prdata;
    sc_uint<32> pirq;
    sc_uint<32> pconfig[2];
    int pindex;
};

inline ostream& operator<<(ostream& os, const apb_slv_out_type& a) {return os;}
inline void sc_trace(sc_trace_file *, const apb_slv_out_type&, const std::string &) {}
inline int operator== (const apb_slv_out_type& left, const apb_slv_out_type& right) {return 0;}

//IRQ output type, direct communication with processor
//   (output from processor's point of view)
struct l3_irq_out_type {
    bool intack;
    sc_uint<4> irl;
    bool pwd;
    bool fpen;
};

inline ostream& operator<<(ostream& os, const l3_irq_out_type& a) {return os;}
inline void sc_trace(sc_trace_file *, const l3_irq_out_type&, const std::string &) {}
inline int operator== (const l3_irq_out_type& left, const l3_irq_out_type& right) {return 0;}

//IRQ input type, direct communication with processor
struct l3_irq_in_type {
    sc_uint<4> irl;
    bool rst;
    bool run;
    sc_uint<20> rstvec;
};

inline ostream& operator<<(ostream& os, const l3_irq_in_type& a) {return os;}
inline void sc_trace(sc_trace_file *, const l3_irq_in_type&, const std::string &) {}
inline int operator== (const l3_irq_in_type& left, const l3_irq_in_type& right) {return 0;}


template <int pindex = 0, int paddr = 0, int pmask = 0xFFF, int ncpu = 2, int eirq = 1>
class Irqmp : public gs::reg::gr_device {
  public:
    /*Slave socket with delayed switch; responsible for all bus communication*/
    gs::reg::greenreg_socket< gs::amba::amba_slave<32> > bus;

    /***Non-AMBA-Signals***/

    /*reset*/
    sc_core::sc_in<bool>                 rst;

    /*PnP bus signals*/
    sc_core::sc_in<sc_uint<32> >         apbi_pirq;
    sc_core::sc_out<sc_dt::sc_uint<32> > apbo_pconfig_0;
    sc_core::sc_out<sc_dt::sc_uint<32> > apbo_pconfig_1;
    sc_core::sc_out<sc_dt::sc_uint<16> > apbo_pindex;


    /*direct processor communication*/
    sc_core::sc_in<l3_irq_out_type>      irqi[ncpu];
    sc_core::sc_out<l3_irq_in_type>      irqo[ncpu];

    GC_HAS_CALLBACKS();
    SC_HAS_PROCESS(Irqmp);

    /*constructor takes vhdl generics as parameters*/
    Irqmp(sc_core::sc_module_name name); // interrupt cascade for extended interrupts
    ~Irqmp();

    /*function prototypes*/    
    void end_of_elaboration();
    void reset_registers();

    //bus communication
    void clear_write();               //write to IR clear register
    void mpstat_write();              //write to MP status register
    void register_read();             //one read function for all registers

    //processor communicatio
    void register_irq();              //bus and processor communication
    void launch_irq();                //processor communication
    void clear_acknowledged_irq();    //processor communication

//    void clk(sc_core::sc_clock &clk);
//    void clk(sc_core::sc_time &period);
//    void clk(double &period, sc_core::sc_time_unit &base);

};



#include "irqmp.tpp"

#endif
