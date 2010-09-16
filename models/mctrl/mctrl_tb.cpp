/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mctrl_tb.tpp                                            */
/*             stimulus file for the MCTRL module                      */
/*             can be used for systemc or vhdl simulation w/ modelsim  */
/*             included by testbench header file                       */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                          */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#include "amba.h"
#include "mctrlreg.h"
#include "mctrl_tb.h"

#define APB true
#define AHB false

#define SHOW \
{ std::printf("\n@%-7s /%-4d:", sc_core::sc_time_stamp().to_string().c_str(), (unsigned)sc_core::sc_delta_count());}
#define REG(name, length, apb) \
{ std::cout << " "#name << ": 0x" << std::hex << std::setfill('0') << std::setw(8) << read(name, length, apb); }
#define SET(name, val, length, apb) \
{ write(name, val, length, apb); }

//constructor
Mctrl_tb::Mctrl_tb(sc_core::sc_module_name name, int _romasel, int _sdrasel, int _romaddr,
                                   int _rommask, int _ioaddr,  int _iomask,  int _ramaddr,
                                   int _rammask, int _paddr,   int _pmask,   int _wprot,
                                   int _srbanks, int _ram8,    int _ram16,   int _sepbus,
                                   int _sdbits,  int _mobile,  int _sden)
  : apb_master_sock ("apb_master_sock", amba::amba_APB, amba::amba_LT, false),
    ahb_master_sock ("ahb_master_sock", amba::amba_AHB, amba::amba_LT, false),
  romasel  (_romasel),
  sdrasel  (_sdrasel),
  romaddr  (_romaddr),
  rommask  (_rommask),
  ioaddr   (_ioaddr),
  iomask   (_iomask),
  ramaddr  (_ramaddr),
  rammask  (_rammask),
  paddr    (_paddr),
  pmask    (_pmask),
  wprot    (_wprot),
  srbanks  (_srbanks),
  ram8     (_ram8),
  ram16    (_ram16),
  sepbus   (_sepbus),
  sdbits   (_sdbits),
  mobile   (_mobile),
  sden     (_sden) {

    SC_THREAD(run);
}

//explicit declaration of standard destructor required for linking
Mctrl_tb::~Mctrl_tb() {
}

//TLM write transaction
void Mctrl_tb::write(uint32_t addr, uint64_t data, uint32_t width, bool apb) {
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
    std::cout << " WRITE " << gp.get_response_string() << ": 0x" << std::hex << std::setfill('0') << std::setw(8) << gp.get_address();
    wait(t);
}

//TLM read transaction
uint32_t Mctrl_tb::read(uint32_t addr, uint32_t width, bool apb) {
    sc_core::sc_time t;
    //data needs to be a struct containing all information required for the transaction
    //uint32_t is not sufficient for the 64 bit transaction of sdram.
    //using the testbench like this will overwrite the timing information with
    //memory contents in the 64 bit read function.
    uint64_t data;
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
    std::cout << " READ " << gp.get_response_string() << ": 0x" << std::hex << std::setfill('0') << std::setw(8) << gp.get_address();
    wait(t);
    return data;
}

//stimuli
void Mctrl_tb::run() {
  sc_core::sc_time t;

  std::cout << std::endl << std::endl << "--------------------------------------------------"
                         << std::endl << "---------- write and read all memories -----------"
                         << std::endl << "--------------------------------------------------" << std::endl;

  //write ROM
  for (uint32_t i=0x00000000; i<0x0000000A; i+=4) {
    SET (i, i+0x200, 4, AHB);
  }
  //write IO
  for (uint32_t i=0x20000000; i<0x2000000A; i+=4) {
    SET (i, i, 4, AHB);
  }
  //write SRAM
  for (uint32_t i=0x40000000; i<0x4000000A; i+=4) {
    SET (i, i, 4, AHB);
  }
  //write SDRAM
  for (uint32_t i=0x80000000; i<0x8000000A; i+=4) {
    SET (i, i, 4, AHB);
  }
  //read ROM
  for (uint32_t i=0x00000000; i<0x0000000F; i+=4) {
    REG (i, 4, AHB);
  }
  //read IO
  for (uint32_t i=0x20000000; i<0x2000000F; i+=4) {
    REG (i, 4, AHB);
  }
  //read SRAM
  for (uint32_t i=0x40000000; i<0x4000000F; i+=4) {
    REG (i, 4, AHB);
  }
  //read SDRAM
  for (uint32_t i=0x80000000; i<0x8000000F; i+=4) {
    REG (i, 4, AHB);
  }

  std::cout << std::endl << std::endl << "--------------------------------------------------"
                         << std::endl << "--switching widths to: rom-16, sram-16, sdram-64--"
                         << std::endl << "--------------------------------------------------" << std::endl;

  //switch ROM to 16 bit access
  unsigned int temp = read (MCTRL_MCFG1, 4, APB);
  std::cout << std::endl << "old MCFG1 contents: " << std::hex << (unsigned int) temp;
  temp &= ~MCTRL_MCFG1_PROM_WIDTH;
  std::cout << std::endl << "intermediate MCFG1 contents: " << std::hex << (unsigned int) temp;
  temp |= (MCTRL_MCFG1_PROM_WIDTH & 0x00000100);
  std::cout << std::endl << "new MCFG1 contents: " << std::hex << (unsigned int) temp;
  SET (MCTRL_MCFG1, temp, 4, APB);

  //switch SRAM to 16 bit access
  temp = read (MCTRL_MCFG2, 4, APB);
  std::cout << std::endl << "old MCFG2 contents: " << std::hex << (unsigned int) temp;
  temp &= ~MCTRL_MCFG2_RAM_WIDTH;
  std::cout << std::endl << "intermediate MCFG2 contents: " << std::hex << (unsigned int) temp;
  temp |= (MCTRL_MCFG2_RAM_WIDTH & 0x00000010);

  //switch SDRAM to 64 bit access
  temp |= MCTRL_MCFG2_D64;
  std::cout << std::endl << "new MCFG2 contents: " << std::hex << (unsigned int) temp;
  SET (MCTRL_MCFG2, temp, 4, APB);

  std::cout << std::endl << std::endl << "--------------------------------------------------"
                         << std::endl << "-------- write / read ROM / SRAM / SDRAM ---------"
                         << std::endl << "--------------------------------------------------" << std::endl;

  //write ROM
//  uint16_t i16=0;
  uint64_t i64=0;
  for (uint32_t i=0x0000000A; i<0x00000010; i+=4) {
    SET (i, i, 4, AHB);
//    SET (i, i16, AHB);
//    i16 += 2;
  }
  //write SRAM
//  i16 = 0;
  for (uint32_t i=0x4000000A; i<0x40000010; i+=4) {
    SET (i, i, 4, AHB);
//    SET (i, i16, AHB);
//    i16 += 2;
  }
  //write SDRAM
  for (uint32_t i=0x80000010; i<0x80000028; i+=8) {
    SET (i, i64, 8, AHB);
    i64 += 8;
  }
  //read ROM
  for (uint32_t i=0x00000008; i<0x00000014; i+=4) {
    REG (i, 4, AHB);
  }
  //read SRAM
  for (uint32_t i=0x40000008; i<0x40000014; i+=4) {
    REG (i, 4, AHB);
  }
  //read SDRAM
  for (uint32_t i=0x80000010; i<0x80000030; i+=8) {
    REG (i, 8, AHB);
  }

  std::cout << std::endl << std::endl << "--------------------------------------------------"
                         << std::endl << "------- switching widths to: rom-8, sram-8 -------"
                         << std::endl << "--------------------------------------------------" << std::endl;

  //switch ROM to 8 bit access
  temp = read (MCTRL_MCFG1, 4, APB);
  std::cout << std::endl << "old MCFG1 contents: " << std::hex << (unsigned int) temp;
  temp &= ~MCTRL_MCFG1_PROM_WIDTH;
  std::cout << std::endl << "new MCFG1 contents: " << std::hex << (unsigned int) temp;
  SET (MCTRL_MCFG1, temp, 4, APB);

  //switch SRAM to 8 bit access
  temp = read (MCTRL_MCFG2, 4, APB);
  std::cout << std::endl << "old MCFG2 contents: " << std::hex << (unsigned int) temp;
  temp &= ~MCTRL_MCFG2_RAM_WIDTH;
  std::cout << std::endl << "new MCFG1 contents: " << std::hex << (unsigned int) temp;
  SET (MCTRL_MCFG2, temp, 4, APB);

  std::cout << std::endl << std::endl << "--------------------------------------------------"
                         << std::endl << "---------- write and read ROM and SRAM -----------"
                         << std::endl << "--------------------------------------------------" << std::endl;

  //write ROM
  uint8_t i8=0;
  for (uint32_t i=0x00000010; i<0x00000012; i++) {
    SET (i, i8, 4, AHB);
    i8++;
  }
  //write SRAM
  i8 = 0;
  for (uint32_t i=0x40000010; i<0x40000012; i++) {
    SET (i, i8, 4, AHB);
    i8++;
  }
  //read ROM
  for (uint32_t i=0x00000008; i<0x00000014; i += 4) {
    REG (i, 4, AHB);
  }
  //read SRAM
  for (uint32_t i=0x40000008; i<0x40000014; i += 4) {
    REG (i, 4, AHB);
  }

  std::cout << std::endl << std::endl << "--------------------------------------------------"
                         << std::endl << "---------- switch off PROM write enable ----------"
                         << std::endl << "--------------------------------------------------" << std::endl;

  //switch off ROM write capability
  temp = read (MCTRL_MCFG1, 4, APB);
  temp &= ~MCTRL_MCFG1_PWEN;
  SET (MCTRL_MCFG1, temp, 4, APB);

  std::cout << std::endl << std::endl << "--------------------------------------------------"
                         << std::endl << "---------------- write into PROM -----------------"
                         << std::endl << "--------------------------------------------------" << std::endl;

  //write ROM
  i8=0;
  for (uint32_t i=0x00000010; i<0x00000012; i++) {
    SET (i, i8, 4, AHB);
    i8++;
  }

  std::cout << std::endl << std::endl << "--------------------------------------------------"
                         << std::endl << "------- send SDRAM to deep power down mode -------"
                         << std::endl << "--------------------------------------------------" << std::endl;

  //send SDRAM to deep power down mode
  temp = read (MCTRL_MCFG4, 4, APB);
  std::cout << std::endl << "old MCFG4 contents: " << std::hex << (unsigned int) temp;
  temp |= MCTRL_MCFG4_PMODE & 0x00050000;
  std::cout << std::endl << "modified MCFG4 contents: " << std::hex << (unsigned int) temp;
  SET (MCTRL_MCFG4, temp, 4, APB);

  std::cout << std::endl << std::endl << "--------------------------------------------------"
                         << std::endl << "---------------- read from SDRAM -----------------"
                         << std::endl << "--------------------------------------------------" << std::endl;

  //read SDRAM
  for (uint32_t i=0x80000010; i<=0x80000020; i+=8) {
    REG (i, 8, AHB);
  }

  std::cout << std::endl;
}

