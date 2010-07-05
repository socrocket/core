/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mctrl_tb_tlm.h                                          */
/*             header file defining the mctrl_tb template              */
/*             can be used for systemc or vhdl simulation w/ modelsim  */
/*             includes mctrl_tb.tpp at the bottom                     */
/*                                                                     */
/* Modified on $Date: 2010-06-02 13:16:10 +0200 (Wed, 02 Jun 2010) $   */
/*          at $Revision: 9 $                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TU Braunschweig              */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef MCTRL_TB_H
#define MCTRL_TB_H

#include "amba.h"

template <int hindex = 0,    int pindex = 0,    int romaddr = 0,    int rommask = 3584,
          int ioaddr = 512,  int iomask = 3584, int ramaddr = 1024, int rammask = 3072,
          int paddr = 0,     int pmask = 4095,  int wprot = 0,      int invclk = 0,
          int fast = 0,      int romasel = 28,  int sdrasel = 29,   int srbanks = 4,
          int ram8 = 0,      int ram16 = 0,     int sden = 0,       int sepbus = 0,
          int sdbits = 32,   int sdlsb = 2,     int oepol = 0,      int syncrst = 0,
          int pageburst = 0, int scantest = 0,  int mobile = 0,     int BUSWIDTH = 32>
class Mctrl_tb : public sc_core::sc_module {
  public:
    //bus communication via socket
    amba::amba_master_socket<BUSWIDTH> master_sock;

    SC_HAS_PROCESS(Mctrl_tb);

    //constructor
    Mctrl_tb(sc_core::sc_module_name nm);

    //define TLM write and read transactions
    void write(uint32_t addr, uint32_t data, uint32_t width);
    uint32_t read(uint32_t addr, uint32_t width);

    //stimuli (make use of TLM write / read transaction functions defined above)
    void run();
};

#include "mctrl_tb.tpp"

#endif
