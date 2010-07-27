/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mctrl_tb.tpp                                            */
/*             stimulus file for the MCTRL module                      */
/*             can be used for systemc or vhdl simulation w/ modelsim  */
/*             included by testbench header file                       */
/*                                                                     */
/* Modified on $Date: 2010-06-02 13:16:10 +0200 (Wed, 02 Jun 2010) $   */
/*          at $Revision: 9 $                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef MCTRL_TB_TPP
#define MCTRL_TB_TPP

#include "amba.h"
#include "mctrlreg.h"

#define SHOW \
{ std::printf("\n@%-7s /%-4d:", sc_core::sc_time_stamp().to_string().c_str(), (unsigned)sc_core::sc_delta_count());}
#define REG(name) \
{ std::cout << " "#name << ": 0x" << std::hex << std::setfill('0') << std::setw(2) << read(name, 4); }
#define SET(name, val) \
{ write(name, val, 4); }

//constructor
template <int hindex,    int pindex,   int romaddr, int rommask,
          int ioaddr,    int iomask,   int ramaddr, int rammask,
          int paddr,     int pmask,    int wprot,   int invclk,
          int fast,      int romasel,  int sdrasel, int srbanks,
          int ram8,      int ram16,    int sden,    int sepbus,
          int sdbits,    int sdlsb,    int oepol,   int syncrst,
          int pageburst, int scantest, int mobile,  int BUSWIDTH>
         Mctrl_tb<hindex,    pindex,   romaddr,
                  rommask,   ioaddr,   iomask,
                  ramaddr,   rammask,  paddr,
                  pmask,     wprot,    invclk,
                  fast,      romasel,  sdrasel,
                  srbanks,   ram8,     ram16,
                  sden,      sepbus,   sdbits,
                  sdlsb,     oepol,    syncrst,
                  pageburst, scantest, mobile, BUSWIDTH >::Mctrl_tb(sc_core::sc_module_name name)
  : master_sock ("msock", amba::amba_APB, amba::amba_LT, false) {

  SC_THREAD(run);
}

//explicit declaration of standard destructor required for linking
template <int hindex,    int pindex,   int romaddr, int rommask,
          int ioaddr,    int iomask,   int ramaddr, int rammask,
          int paddr,     int pmask,    int wprot,   int invclk,
          int fast,      int romasel,  int sdrasel, int srbanks,
          int ram8,      int ram16,    int sden,    int sepbus,
          int sdbits,    int sdlsb,    int oepol,   int syncrst,
          int pageburst, int scantest, int mobile,  int BUSWIDTH>
         Mctrl_tb<hindex,    pindex,   romaddr,
                  rommask,   ioaddr,   iomask,
                  ramaddr,   rammask,  paddr,
                  pmask,     wprot,    invclk,
                  fast,      romasel,  sdrasel,
                  srbanks,   ram8,     ram16,
                  sden,      sepbus,   sdbits,
                  sdlsb,     oepol,    syncrst,
                  pageburst, scantest, mobile, BUSWIDTH >::~Mctrl_tb() {
}

//TLM write transaction
template <int hindex,    int pindex,   int romaddr, int rommask,
          int ioaddr,    int iomask,   int ramaddr, int rammask,
          int paddr,     int pmask,    int wprot,   int invclk,
          int fast,      int romasel,  int sdrasel, int srbanks,
          int ram8,      int ram16,    int sden,    int sepbus,
          int sdbits,    int sdlsb,    int oepol,   int syncrst,
          int pageburst, int scantest, int mobile,  int BUSWIDTH>
    void Mctrl_tb<hindex,    pindex,   romaddr,
                  rommask,   ioaddr,   iomask,
                  ramaddr,   rammask,  paddr,
                  pmask,     wprot,    invclk,
                  fast,      romasel,  sdrasel,
                  srbanks,   ram8,     ram16,
                  sden,      sepbus,   sdbits,
                  sdlsb,     oepol,    syncrst,
                  pageburst, scantest, mobile, BUSWIDTH >::write(uint32_t addr, uint32_t data, uint32_t width) {
    sc_core::sc_time t;
    tlm::tlm_generic_payload gp;
      gp.set_command(tlm::TLM_WRITE_COMMAND);
      gp.set_address(addr);
      gp.set_data_length(width);
      gp.set_streaming_width(4);
      gp.set_byte_enable_ptr(NULL);
      gp.set_data_ptr((unsigned char*)&data);
    master_sock->b_transport(gp,t);
    SHOW;
    std::cout << " WRITE " << gp.get_response_string() << ": 0x" << std::hex << std::setfill('0') << std::setw(2) << gp.get_address();
    wait(t);
}

//TLM read transaction
template <int hindex,    int pindex,   int romaddr, int rommask,
          int ioaddr,    int iomask,   int ramaddr, int rammask,
          int paddr,     int pmask,    int wprot,   int invclk,
          int fast,      int romasel,  int sdrasel, int srbanks,
          int ram8,      int ram16,    int sden,    int sepbus,
          int sdbits,    int sdlsb,    int oepol,   int syncrst,
          int pageburst, int scantest, int mobile,  int BUSWIDTH>
    uint32_t Mctrl_tb<hindex,    pindex,   romaddr,
                      rommask,   ioaddr,   iomask,
                      ramaddr,   rammask,  paddr,
                      pmask,     wprot,    invclk,
                      fast,      romasel,  sdrasel,
                      srbanks,   ram8,     ram16,
                      sden,      sepbus,   sdbits,
                      sdlsb,     oepol,    syncrst,
                      pageburst, scantest, mobile, BUSWIDTH >::read(uint32_t addr, uint32_t width) {
    sc_core::sc_time t;
    //data needs to be a struct containing all information required for the transaction
    uint32_t data;
    tlm::tlm_generic_payload gp;
      gp.set_command(tlm::TLM_READ_COMMAND);
      gp.set_address(addr);
      gp.set_data_length(width);
      gp.set_streaming_width(4);
      gp.set_byte_enable_ptr(NULL);
      gp.set_data_ptr((unsigned char*)&data);
      gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    master_sock->b_transport(gp,t);
    SHOW;
    std::cout << " READ " << gp.get_response_string() << ": 0x" << std::hex << std::setfill('0') << std::setw(2) << gp.get_address();
    wait(t);
    return data;
}

//stimuli
template <int hindex,    int pindex,   int romaddr, int rommask,
          int ioaddr,    int iomask,   int ramaddr, int rammask,
          int paddr,     int pmask,    int wprot,   int invclk,
          int fast,      int romasel,  int sdrasel, int srbanks,
          int ram8,      int ram16,    int sden,    int sepbus,
          int sdbits,    int sdlsb,    int oepol,   int syncrst,
          int pageburst, int scantest, int mobile,  int BUSWIDTH>
    void Mctrl_tb<hindex,    pindex,   romaddr,
                  rommask,   ioaddr,   iomask,
                  ramaddr,   rammask,  paddr,
                  pmask,     wprot,    invclk,
                  fast,      romasel,  sdrasel,
                  srbanks,   ram8,     ram16,
                  sden,      sepbus,   sdbits,
                  sdlsb,     oepol,    syncrst,
                  pageburst, scantest, mobile, BUSWIDTH >::run() {
    sc_core::sc_time t;

//put stimuli here

}

#endif
