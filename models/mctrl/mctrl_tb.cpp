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
// Title:      mctrl_tb.tpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    stimulus file for the MCTRL module
//             can be used for systemc or vhdl simulation w/ modelsim
//             included by testbench header file
//
// Method:
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Dennis Bode
// Reviewed:
//*********************************************************************

#include "amba.h"
#include "mctrl.h"
#include "mctrl_tb.h"

#define APB true
#define AHB false

#define REG(name, length, apb) \
{ std::cout << ": 0x" << std::hex << std::setfill('0') << std::setw(8) << (unsigned int) read(name, length, apb) << std::endl << std::endl; }
#define SET(name, val, length, apb) \
{ write(name, val, length, apb); }

//constructor
Mctrl_tb::Mctrl_tb(sc_core::sc_module_name name, int _romasel, int _sdrasel,
                   int _romaddr, int _rommask, int _ioaddr, int _iomask,
                   int _ramaddr, int _rammask, int _paddr, int _pmask,
                   int _wprot, int _srbanks, int _ram8, int _ram16,
                   int _sepbus, int _sdbits, int _mobile, int _sden) :
    apb_master_sock("apb_master_sock", amba::amba_APB, amba::amba_LT, false),
            ahb_master_sock("ahb_master_sock", amba::amba_AHB, amba::amba_LT,
                    false), romasel(_romasel), sdrasel(_sdrasel), romaddr(
                    _romaddr), rommask(_rommask), ioaddr(_ioaddr), iomask(
                    _iomask), ramaddr(_ramaddr), rammask(_rammask), paddr(
                    _paddr), pmask(_pmask), wprot(_wprot), srbanks(_srbanks),
            ram8(_ram8), ram16(_ram16), sepbus(_sepbus), sdbits(_sdbits),
            mobile(_mobile), sden(_sden) {

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
        apb_master_sock->b_transport(gp, t);
    } else {
        ahb_master_sock->b_transport(gp, t);
    }
//    SHOW;
    v::info << "Mctrl_tb" << " WRITE " << gp.get_response_string() << ": 0x" << std::hex 
            << std::setfill('0') << std::setw(8) << gp.get_address() << std::endl;
    wait(t);
}

//TLM read transaction
uint32_t Mctrl_tb::read(uint32_t addr, uint32_t width, bool apb) {
    sc_core::sc_time t;
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
        apb_master_sock->b_transport(gp, t);
    } else {
        ahb_master_sock->b_transport(gp, t);
    }
//    SHOW;
    v::info << "Mctrl_tb" << " READ " << gp.get_response_string() << ": 0x" << std::hex
            << std::setfill('0') << std::setw(8) << gp.get_address() << ": 0x";
    wait(t);

    //check for cacheable_access extension
//    if (ahb_master_sock.get_extension<amba::amba_cacheable>(gp) ) {
//      std::cout << std::endl << "CACHE EXT TRANSMITTED" << std::endl;
//    }

    return data;
}


void Mctrl_tb::check64 (uint32_t address) {
    uint64_t input = ~address;
    input <<= 32;
    input += ~address;
    write (address, input, 8, AHB);
    v::info << "Mctrl_tb" << "   Expected at address 0x" << std::hex << std::setfill('0') << std::setw(8) << (unsigned int) address
            << ": 0x" << (uint64_t) input << std::endl;
    uint64_t data = read (address, 8, AHB);
    std::cout << std::hex << std::setfill('0') << std::setw(8) << (uint64_t) data << std::endl << std::endl;
}

void Mctrl_tb::check32 (uint32_t address) {
    uint32_t input = ~address;
    write (address, input, 4, AHB);
    v::info << "Mctrl_tb" << "   Expected at address 0x" << std::hex << std::setfill('0') << std::setw(8) << (unsigned int) address
            << ": 0x" << (unsigned int) input << std::endl;
    uint32_t data = read (address, 4, AHB);
    std::cout << std::hex << std::setfill('0') << std::setw(8) << (uint32_t) data << std::endl;
    if (data + address == 0xFFFFFFFF) {
        v::info << "Mctrl_tb" << "Test passed" << std::endl << std::endl;
    }
    else {
        v::warn << "Mctrl_tb" << "Test failed" << std::endl << std::endl;
    }
}

void Mctrl_tb::check16 (uint32_t address) {
    uint32_t input = ~address;
    uint16_t input_l, input_h;
    input_l = input & 0xFF;
    input_h = (input >> 16) & 0xFF;
    write (address, input_l, 2, AHB);
    write (address+2, input_h, 2, AHB);
    v::info << "Mctrl_tb" << "   Expected at address 0x" << std::hex << std::setfill('0') << std::setw(8) << (unsigned int) address
            << ": 0x" << (unsigned int) input << std::endl;
    uint32_t data = read (address, 4, AHB);
    std::cout << std::hex << std::setfill('0') << std::setw(8) << (uint32_t) data << std::endl;
    if (data + address == 0xFFFFFFFF) {
        v::info << "Mctrl_tb" << "Test passed" << std::endl << std::endl;
    }
    else {
        v::warn << "Mctrl_tb" << "Test failed" << std::endl << std::endl;
    }
}

void Mctrl_tb::check8 (uint32_t address) {
    uint32_t input = ~address;
    uint8_t input_b4, input_b3, input_b2, input_b1;
    input_b1 = input & 0xF;
    input_b2 = (input >> 8) & 0xF;
    input_b3 = (input >> 16) & 0xF;
    input_b4 = (input >> 24) & 0xF;
    write (address, input_b1, 1, AHB);
    write (address+1, input_b2, 1, AHB);
    write (address+2, input_b3, 1, AHB);
    write (address+3, input_b4, 1, AHB);
    v::info << "Mctrl_tb" << "   Expected at address 0x" << std::hex << std::setfill('0') << std::setw(8) << (unsigned int) address
            << ": 0x" << (unsigned int) input << std::endl;
    uint32_t data = read (address, 4, AHB);
    std::cout << std::hex << std::setfill('0') << std::setw(8) << (uint32_t) data << std::endl;
    if (data + address == 0xFFFFFFFF) {
        v::info << "Mctrl_tb" << "Test passed" << std::endl << std::endl;
    }
    else {
        v::warn << "Mctrl_tb" << "Test failed" << std::endl << std::endl;
    }
}

/*
void Mctrl_tb::check (uint32_t start, uint32_t end, uint8_t inc) {
  for (uint32_t i=start; i<=end; i+=inc) {
    uint64_t input;
    switch (inc) {
      case 4:
        input = 0x44434241;
        write (i, (uint32_t) input, inc, AHB);
        break;
      case 2:
        input = 0x2221;
        write (i, (uint16_t) input, inc, AHB);
        break;
      case 1:
        input = 0x11;
        write (i, (uint8_t) input, inc, AHB);
        break;
      case 8:
        input = 0x88878685;
        input <<= 32;
        input += 0x84838281;
        write (i, input, inc, AHB);
        break;
      default:
        v::warn << "Mctrl_tb" << "invalid streaming width given to 'check' function" << std::endl;;
    }
  }
  std::cout << std::endl;
  for (uint32_t i=start; i<=end; i+=4) {
    switch (inc) {
      case 4:
        v::info << "Mctrl_tb" << "Expected at address 0x" << std::hex << std::setfill('0') << std::setw(8) << i << ": 0x44434241" << std::endl;
        break;
      case 2:
        v::info << "Mctrl_tb" << "Expected at address 0x" << std::hex << std::setfill('0') << std::setw(8) << i << ": 0x22212221" << std::endl;
        break;
      case 1:
        v::info << "Mctrl_tb" << "Expected at address 0x" << std::hex << std::setfill('0') << std::setw(8) << i << ": 0x11111111" << std::endl;
        break;
      case 8:
        v::info << "Mctrl_tb" << "Expected at address 0x" << std::hex << std::setfill('0') << std::setw(8) 
                                                           << i << ": 0x8887868584838281" << std::endl;
    }
    REG (i, 4, AHB);
  }
  std::cout << std::endl;

}
*/

//stimuli
void Mctrl_tb::run() {
    sc_core::sc_time t;

  v::info << "Mctrl_tb" << std::endl << "--------------------------------------------------"
          << std::endl << "---------- write and read all memories -----------"
          << std::endl << "--------------------------------------------------" << std::endl;

  v::info << "Mctrl_tb" << "Check 4-byte-access to PROM area in 4-byte-mode" << std::endl;
  for (uint32_t i=0x00000000; i<=0x00000008; i+=4) {
    check32 (i);
  }
  v::info << "Mctrl_tb" << "Check 4-byte-access to I/O area in 4-byte-mode" << std::endl;

  for (uint32_t i=0x20000000; i<=0x20000008; i+=4) {
    check32 (i);
  }
  v::info << "Mctrl_tb" << "Check 4-byte-access to SRAM area in 4-byte-mode" << std::endl;

  for (uint32_t i=0x40000000; i<=0x40000008; i+=4) {
    check32 (i);
  }
  v::info << "Mctrl_tb" << "Check 4-byte-access to SDRAM area in 4-byte-mode" << std::endl;

  for (uint32_t i=0x60000000; i<=0x60000008; i+=4) {
    check32 (i);
  }


/*
  v::info << "Mctrl_tb" << "Check 2-byte-access to PROM area in 4-byte-mode" << std::endl;
  check (0x0000000C, 0x00000010, 2);
  v::info << "Mctrl_tb" << "Check 2-byte-access to I/O area in 4-byte-mode" << std::endl;
  check (0x2000000C, 0x20000010, 2);
  v::info << "Mctrl_tb" << "Check 2-byte-access to SRAM area in 4-byte-mode" << std::endl;
  check (0x4000000C, 0x40000010, 2);
  v::info << "Mctrl_tb" << "Check 2-byte-access to SDRAM area in 4-byte-mode" << std::endl;
  check (0x6000000C, 0x60000010, 2);

  v::info << "Mctrl_tb" << "Check 1-byte-access to PROM area in 4-byte-mode" << std::endl;
  check (0x00000014, 0x00000017, 1);
  v::info << "Mctrl_tb" << "Check 1-byte-access to I/O area in 4-byte-mode" << std::endl;
  check (0x20000014, 0x20000017, 1);
  v::info << "Mctrl_tb" << "Check 1-byte-access to SRAM area in 4-byte-mode" << std::endl;
  check (0x40000014, 0x40000017, 1);
  v::info << "Mctrl_tb" << "Check 1-byte-access to SDRAM area in 4-byte-mode" << std::endl;
  check (0x60000014, 0x60000017, 1);

  v::info << "Mctrl_tb" << std::endl << "--------------------------------------------------"
                     << std::endl << "--switching widths to: rom-16, sram-16, sdram-64--"
                     << std::endl << "--------------------------------------------------" << std::endl;

  //switch ROM to 16 bit access
  uint32_t temp = read (Mctrl::MCTRL_MCFG1, 4, APB);
  std::cout << std::endl << "old MCFG1 contents: " << std::hex << (unsigned int) temp << std::endl;
  temp &= ~Mctrl::MCTRL_MCFG1_PROM_WIDTH;
  std::cout << "intermediate MCFG1 contents: " << std::hex << (unsigned int) temp << std::endl;
  temp |= (Mctrl::MCTRL_MCFG1_PROM_WIDTH & 0x00000100);
  std::cout << "new MCFG1 contents: " << std::hex << (unsigned int) temp << std::endl;
  SET (Mctrl::MCTRL_MCFG1, temp, 4, APB);

  //switch SRAM to 16 bit access
  temp = read (Mctrl::MCTRL_MCFG2, 4, APB);
  std::cout << std::endl << "old MCFG2 contents: " << std::hex << (unsigned int) temp << std::endl;
  temp &= ~Mctrl::MCTRL_MCFG2_RAM_WIDTH;
  std::cout << "intermediate MCFG2 contents: " << std::hex << (unsigned int) temp << std::endl;
  temp |= (Mctrl::MCTRL_MCFG2_RAM_WIDTH & 0x00000010);

  //switch SDRAM to 64 bit access
  temp |= Mctrl::MCTRL_MCFG2_D64;
  std::cout << "new MCFG2 contents: " << std::hex << (unsigned int) temp << std::endl;
  SET (Mctrl::MCTRL_MCFG2, temp, 4, APB);
  std::cout << std::endl << "MCFG2 contents after writing:" << std::endl;
  REG (Mctrl::MCTRL_MCFG2, 4, APB);

    v::info << "Mctrl_tb" << std::endl
            << "--------------------------------------------------"
            << std::endl
            << "-------- write / read ROM / SRAM / SDRAM ---------"
            << std::endl
            << "--------------------------------------------------"
            << std::endl;

  v::info << "Mctrl_tb" << "Check 4-byte-access to PROM area in 4-byte-mode" << std::endl;
  check (0x00000100, 0x00000108, 4);
  v::info << "Mctrl_tb" << "Check 4-byte-access to SRAM area in 4-byte-mode" << std::endl;
  check (0x40000100, 0x40000108, 4);
  v::info << "Mctrl_tb" << "Check 4-byte-access to SDRAM area in 4-byte-mode" << std::endl;
  check (0x60000100, 0x60000108, 4);

  v::info << "Mctrl_tb" << "Check 2-byte-access to PROM area in 4-byte-mode" << std::endl;
  check (0x0000010C, 0x00000110, 2);
  v::info << "Mctrl_tb" << "Check 2-byte-access to SRAM area in 4-byte-mode" << std::endl;
  check (0x4000010C, 0x40000110, 2);
  v::info << "Mctrl_tb" << "Check 2-byte-access to SDRAM area in 4-byte-mode" << std::endl;
  check (0x6000010C, 0x60000110, 2);

  v::info << "Mctrl_tb" << "Check 1-byte-access to PROM area in 4-byte-mode" << std::endl;
  check (0x00000114, 0x00000117, 1);
  v::info << "Mctrl_tb" << "Check 1-byte-access to SRAM area in 4-byte-mode" << std::endl;
  check (0x40000114, 0x40000117, 1);
  v::info << "Mctrl_tb" << "Check 1-byte-access to SDRAM area in 4-byte-mode" << std::endl;
  check (0x60000114, 0x60000117, 1);

  v::info << "Mctrl_tb" << std::endl << std::endl << "--------------------------------------------------"
                       << std::endl << "------- switching widths to: rom-8, sram-8 -------"
                       << std::endl << "--------------------------------------------------" << std::endl;

  //switch ROM to 8 bit access
  temp = read (Mctrl::MCTRL_MCFG1, 4, APB);
  std::cout << std::endl << "old MCFG1 contents: " << std::hex << (unsigned int) temp << std::endl;
  temp &= ~Mctrl::MCTRL_MCFG1_PROM_WIDTH;
  std::cout << "new MCFG1 contents: " << std::hex << (unsigned int) temp << std::endl;
  SET (Mctrl::MCTRL_MCFG1, temp, 4, APB);

  //switch SRAM to 8 bit access
  temp = read (Mctrl::MCTRL_MCFG2, 4, APB);
  std::cout << std::endl << "old MCFG2 contents: " << std::hex << (unsigned int) temp << std::endl;
  temp &= ~Mctrl::MCTRL_MCFG2_RAM_WIDTH;
  std::cout << "new MCFG2 contents: " << std::hex << (unsigned int) temp << std::endl;
  SET (Mctrl::MCTRL_MCFG2, temp, 4, APB);

    v::info << "Mctrl_tb" << std::endl << std::endl
            << "--------------------------------------------------"
            << std::endl
            << "---------- write and read ROM and SRAM -----------"
            << std::endl
            << "--------------------------------------------------"
            << std::endl;

  v::info << "Mctrl_tb" << "Check 4-byte-access to PROM area in 4-byte-mode" << std::endl;
  check (0x00010000, 0x00010008, 4);
  v::info << "Mctrl_tb" << "Check 4-byte-access to SRAM area in 4-byte-mode" << std::endl;
  check (0x40010000, 0x40010008, 4);

  v::info << "Mctrl_tb" << "Check 2-byte-access to SRAM area in 4-byte-mode" << std::endl;
  check (0x0001000C, 0x00010010, 2);
  v::info << "Mctrl_tb" << "Check 2-byte-access to SRAM area in 4-byte-mode" << std::endl;
  check (0x4001000C, 0x40010010, 2);

  v::info << "Mctrl_tb" << "Check 1-byte-access to SRAM area in 4-byte-mode" << std::endl;
  check (0x00010014, 0x00010017, 1);
  v::info << "Mctrl_tb" << "Check 1-byte-access to SRAM area in 4-byte-mode" << std::endl;
  check (0x40010014, 0x40010017, 1);

  v::info << "Mctrl_tb" << std::endl << std::endl << "--------------------------------------------------"
                     << std::endl << "---------- switch off PROM write enable ----------"
                     << std::endl << "--------------------------------------------------" << std::endl;

  //switch off ROM write capability
  temp = read (Mctrl::MCTRL_MCFG1, 4, APB);
  temp &= ~Mctrl::MCTRL_MCFG1_PWEN;
  std::cout << std::endl;
  SET (Mctrl::MCTRL_MCFG1, temp, 4, APB);

    v::info << "Mctrl_tb" << std::endl << std::endl
            << "--------------------------------------------------"
            << std::endl
            << "---------------- write into PROM -----------------"
            << std::endl
            << "--------------------------------------------------"
            << std::endl;

  //write ROM
  uint8_t i8=0;
  for (uint32_t i=0x00000010; i<0x00000012; i++) {
    SET (i, i8, 4, AHB);
    i8++;
  }

    v::info << "Mctrl_tb" << std::endl << std::endl
            << "--------------------------------------------------"
            << std::endl
            << "------- send SDRAM to deep power down mode -------"
            << std::endl
            << "--------------------------------------------------"
            << std::endl;

  //send SDRAM to deep power down mode
  temp = read (Mctrl::MCTRL_MCFG4, 4, APB);
  std::cout << std::endl << "old MCFG4 contents: " << std::hex << (unsigned int) temp << std::endl;
  temp |= Mctrl::MCTRL_MCFG4_PMODE & 0x00050000;
  std::cout << "modified MCFG4 contents: " << std::hex << (unsigned int) temp << std::endl;
  SET (Mctrl::MCTRL_MCFG4, temp, 4, APB);

    v::info << "Mctrl_tb" << std::endl << std::endl
            << "--------------------------------------------------"
            << std::endl
            << "---------------- read from SDRAM -----------------"
            << std::endl
            << "--------------------------------------------------"
            << std::endl;

  //read SDRAM
  for (uint32_t i=0x60000010; i<=0x60000020; i+=8) {
    REG (i, 8, AHB);
  }
*/
    std::cout << std::endl;
}


