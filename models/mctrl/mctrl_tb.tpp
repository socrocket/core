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

#define APB true
#define AHB false

#define SHOW \
{ std::printf("\n@%-7s /%-4d:", sc_core::sc_time_stamp().to_string().c_str(), (unsigned)sc_core::sc_delta_count());}
#define REG(name, apb) \
{ std::cout << " "#name << ": 0x" << std::hex << std::setfill('0') << std::setw(2) << read(name, 4, apb); }
#define SET(name, val, apb) \
{ write(name, val, 4, apb); }

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
  : apb_master_sock ("apb_master_sock", amba::amba_APB, amba::amba_LT, false),
    ahb_master_sock ("ahb_master_sock") {

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
                  pageburst, scantest, mobile, BUSWIDTH >::write(uint32_t addr, uint32_t data, uint32_t width, bool apb) {
    sc_core::sc_time t;
    tlm::tlm_generic_payload gp;
      gp.set_command(tlm::TLM_WRITE_COMMAND);
      gp.set_address(addr);
      gp.set_data_length(width);
      gp.set_streaming_width(4);
      gp.set_byte_enable_ptr(NULL);
      gp.set_data_ptr((unsigned char*)&data);
    if (apb) {
      apb_master_sock->b_transport(gp,t);
    }
    else {
      ahb_master_sock->b_transport(gp,t);
    }
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
                      pageburst, scantest, mobile, BUSWIDTH >::read(uint32_t addr, uint32_t width, bool apb) {
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
    if (apb) {
      apb_master_sock->b_transport(gp,t);
    }
    else {
      ahb_master_sock->b_transport(gp,t);
    }
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

  cout << endl << endl << "--------------------------------------------------"
               << endl << "---------- write and read all memories -----------"
               << endl << "--------------------------------------------------" << endl;

  //write ROM
  for (uint32_t i=0x00000000; i<0x0000000A; i+=4) {
    SET (i, i+0x200, AHB);
  }
  //write IO
  for (uint32_t i=0x20000000; i<0x2000000A; i+=4) {
    SET (i, i, AHB);
  }
  //write SRAM
  for (uint32_t i=0x40000000; i<0x4000000A; i+=4) {
    SET (i, i, AHB);
  }
  //write SDRAM
  for (uint32_t i=0x80000000; i<0x8000000A; i+=4) {
    SET (i, i, AHB);
  }
  //read ROM
  for (uint32_t i=0x00000000; i<0x0000000F; i+=4) {
    REG (i, AHB);
  }
  //read IO
  for (uint32_t i=0x20000000; i<0x2000000F; i+=4) {
    REG (i, AHB);
  }
  //read SRAM
  for (uint32_t i=0x40000000; i<0x4000000F; i+=4) {
    REG (i, AHB);
  }
  //read SDRAM
  for (uint32_t i=0x80000000; i<0x8000000F; i+=4) {
    REG (i, AHB);
  }

  cout << endl << endl << "--------------------------------------------------"
               << endl << "--switching widths to: rom-16, sram-16, sdram-64--"
               << endl << "--------------------------------------------------" << endl;

  //switch ROM to 16 bit access
  unsigned int temp = read (MCTRL_MCFG1, 4, APB);
  cout << endl << "old MCFG1 contents: " << hex << (unsigned int) temp;
  temp &= ~MCTRL_MCFG1_PROM_WIDTH;
  cout << endl << "intermediate MCFG1 contents: " << hex << (unsigned int) temp;
  temp |= (MCTRL_MCFG1_PROM_WIDTH & 0x00000100);
  cout << endl << "new MCFG1 contents: " << hex << (unsigned int) temp;
  SET (MCTRL_MCFG1, temp, APB);

  //switch SRAM to 16 bit access
  temp = read (MCTRL_MCFG2, 4, APB);
  cout << endl << "old MCFG2 contents: " << hex << (unsigned int) temp;
  temp &= ~MCTRL_MCFG2_RAM_WIDTH;
  cout << endl << "intermediate MCFG2 contents: " << hex << (unsigned int) temp;
  temp |= (MCTRL_MCFG2_RAM_WIDTH & 0x00000010);

  //switch SDRAM to 64 bit access
  temp |= MCTRL_MCFG2_D64;
  cout << endl << "new MCFG2 contents: " << hex << (unsigned int) temp;
  SET (MCTRL_MCFG2, temp, APB);

  cout << endl << endl << "--------------------------------------------------"
               << endl << "-------- write / read ROM / SRAM / SDRAM ---------"
               << endl << "--------------------------------------------------" << endl;

  //write ROM
  uint16_t i16=0;
  uint64_t i64=0;
  for (uint32_t i=0x0000000A; i<0x00000010; i+=2) {
    SET (i, i16, AHB);
    i16 += 2;
  }
  //write SRAM
  i16 = 0;
  for (uint32_t i=0x4000000A; i<0x40000010; i+=2) {
    SET (i, i16, AHB);
    i16 += 2;
  }
  //write SDRAM
  for (uint32_t i=0x80000010; i<0x80000028; i+=8) {
    SET (i, i64, AHB);
    i64 += 8;
  }
  //read ROM
  for (uint32_t i=0x00000008; i<0x00000014; i+=4) {
    REG (i, AHB);
  }
  //read SRAM
  for (uint32_t i=0x40000008; i<0x40000014; i+=4) {
    REG (i, AHB);
  }
  //read SDRAM
  for (uint32_t i=0x80000010; i<0x80000030; i+=8) {
    REG (i, AHB);
  }

  cout << endl << endl << "--------------------------------------------------"
               << endl << "------- switching widths to: rom-8, sram-8 -------"
               << endl << "--------------------------------------------------" << endl;

  //switch ROM to 8 bit access
  temp = read (MCTRL_MCFG1, 4, APB);
  cout << endl << "old MCFG1 contents: " << hex << (unsigned int) temp;
  temp &= ~MCTRL_MCFG1_PROM_WIDTH;
  cout << endl << "new MCFG1 contents: " << hex << (unsigned int) temp;
  SET (MCTRL_MCFG1, temp, APB);

  //switch SRAM to 8 bit access
  temp = read (MCTRL_MCFG2, 4, APB);
  cout << endl << "old MCFG2 contents: " << hex << (unsigned int) temp;
  temp &= ~MCTRL_MCFG2_RAM_WIDTH;
  cout << endl << "new MCFG1 contents: " << hex << (unsigned int) temp;
  SET (MCTRL_MCFG2, temp, APB);

  cout << endl << endl << "--------------------------------------------------"
               << endl << "---------- write and read ROM and SRAM -----------"
               << endl << "--------------------------------------------------" << endl;

  //write ROM
  uint8_t i8=0;
  for (uint32_t i=0x00000010; i<0x00000012; i++) {
    SET (i, i8, AHB);
    i8++;
  }
  //write SRAM
  i8 = 0;
  for (uint32_t i=0x40000010; i<0x40000012; i++) {
    SET (i, i8, AHB);
    i8++;
  }
  //read ROM
  for (uint32_t i=0x00000008; i<0x00000014; i += 4) {
    REG (i, AHB);
  }
  //read SRAM
  for (uint32_t i=0x40000008; i<0x40000014; i += 4) {
    REG (i, AHB);
  }

  cout << endl << endl << "--------------------------------------------------"
               << endl << "---------- switch off PROM write enable ----------"
               << endl << "--------------------------------------------------" << endl;

  //switch off ROM write capability
  temp = read (MCTRL_MCFG1, 4, APB);
  temp &= ~MCTRL_MCFG1_PWEN;
  SET (MCTRL_MCFG1, temp, APB);

  cout << endl << endl << "--------------------------------------------------"
               << endl << "---------------- write into PROM -----------------"
               << endl << "--------------------------------------------------" << endl;

  //write ROM
  i8=0;
  for (uint32_t i=0x00000010; i<0x00000012; i++) {
    SET (i, i8, AHB);
    i8++;
  }
  cout << endl;
}

#endif
