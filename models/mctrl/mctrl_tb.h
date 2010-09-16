/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mctrl_tb_tlm.h                                          */
/*             header file defining the mctrl_tb template              */
/*             can be used for systemc or vhdl simulation w/ modelsim  */
/*             includes mctrl_tb.tpp at the bottom                     */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TU Braunschweig              */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef MCTRL_TB_H
#define MCTRL_TB_H

#include "amba.h"

class Mctrl_tb : public sc_core::sc_module {
  public:
    //constructor and destructor
    Mctrl_tb(sc_core::sc_module_name nm, int _romasel = 28,  int _sdrasel = 29,  int _romaddr = 0,    int _rommask = 3584,
                                         int _ioaddr = 512,  int _iomask = 3584, int _ramaddr = 1024, int _rammask = 3072,
                                         int _paddr = 0,     int _pmask = 4095,  int _wprot = 0,      int _srbanks = 4,
                                         int _ram8 = 0,      int _ram16 = 0,     int _sepbus = 0,     int _sdbits = 32,
                                         int _mobile = 0,    int _sden = 0);
    ~Mctrl_tb();

  public:
    //bus communication via socket
    amba::amba_master_socket<32> apb_master_sock;
    amba::amba_master_socket<32> ahb_master_sock;

    SC_HAS_PROCESS(Mctrl_tb);

    //define TLM write and read transactions
    void write(uint32_t addr, uint64_t data, uint32_t width, bool apb);
    uint32_t read(uint32_t addr, uint32_t width, bool apb);

    //stimuli (make use of TLM write / read transaction functions defined above)
    void run();

  private:
  //constructor parameters (modeling VHDL generics)
    const int romasel;
    const int sdrasel;
    const int romaddr;
    const int rommask;
    const int ioaddr;
    const int iomask;
    const int ramaddr;
    const int rammask;
    const int paddr;
    const int pmask;
    const int wprot;
    const int srbanks;
    const int ram8;
    const int ram16;
    const int sepbus;
    const int sdbits;
    const int mobile;
    const int sden;
};

#endif
