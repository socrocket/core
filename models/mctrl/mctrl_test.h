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
// The program is provided "as is", ther is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      mctrl_test.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Top-Level class for all mctrl tests.
//             All tests of this module inherit from this class.
//
// Method:
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#ifndef __MCTRL_TEST_H__
#define __MCTRL_TEST_H__

#include <tlm.h>
#include "amba.h"
#include "socrocket.h"
#include "vendian.h"
#include "signalkit.h"
#include <cstdlib>
#include <ctime>

#if defined(MTI_SYSTEMC) || defined(NO_INCLUDE_PATHS) 
#include "simple_initiator_socket.h"
#else
#include "tlm_utils/simple_initiator_socket.h"
#endif

#include "verbose.h"

// All mctrl tests inherit from this class
class mctrl_test : public sc_module, 
                   public signalkit::signal_module<mctrl_test> {

  public:

  // AMBA TLM master sockets for connection to MCTRL
  amba::amba_master_socket<32> apb;
  amba::amba_master_socket<32> ahb;

  // Reset generator
  signal<bool>::out rst;

  // Gaisler specific APB signals must be driven from testbench 
  signal<uint16_t>::out            pindex;
  signal<uint32_t>::out            pirqi;
  signal<bool>::infield            pirqo;

  // Gaisler specific APB signals must be driven from testbench
  signal<uint8_t>::out             hmbsel;
  signal<bool>::out                hcachei;
  signal<uint32_t>::out            hirqi;
  signal<bool>::out                hcacheo;
  signal<uint32_t>::in             hirqo;

  // Testbench interface functions
  bool writeAHB_LT(uint32_t addr, uint8_t *data, uint32_t width, uint32_t length, bool fail);
  void writeAHB_DBG(uint32_t addr, uint8_t *data, uint32_t width, uint32_t length);
  bool readAHB_LT(uint32_t addr, uint8_t *data, uint32_t width, uint32_t length, bool fail);
  void readAHB_DBG(uint32_t addr, uint8_t *data, uint32_t width, uint32_t length);
  void writeAPB(uint32_t addr, uint32_t data);
  uint32_t readAPB(uint32_t addr);
  bool writeCheck(const uint32_t start, uint32_t end, uint32_t width, const uint32_t length, bool fail);
  bool readCheck(const uint32_t start, uint32_t end, uint32_t width, const uint32_t length, bool fail);

  // Constructor
  mctrl_test(sc_core::sc_module_name nm, int _romasel = 28, 
	     int _sdrasel = 29, int _romaddr = 0, int _rommask = 3584, int _ioaddr = 512,
	     int _iomask = 3584, int _ramaddr = 1024, int _rammask = 3072,
             int _paddr = 0, int _pmask = 4095, int _wprot = 0, int _srbanks = 4, 
	     int _ram8 = 0, int _ram16 = 0, int _sepbus = 0, int _sdbits = 32, 
	     int _mobile = 0, int _sden = 0, amba::amba_layer_ids abstractionLayer = amba::amba_LT);

  protected:
 
  // Constructor parameters (modeling VHDL generics)
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

  public:
   
  // --- Constant bit masks for APB register access

  // Register address offset
  static const uint32_t MCFG1                         = 0x00000000;
  static const uint32_t MCFG2                         = 0x00000004;
  static const uint32_t MCFG3                         = 0x00000008;
  static const uint32_t MCFG4                         = 0x0000000C;

  // Memory configuration register 1
  static const uint32_t MCFG1_WRITE_MASK              = 0x1FE80BFF;
  static const uint32_t MCFG1_IOBUSW                  = 0x18000000;
  static const uint32_t MCFG1_IBRDY                   = 0x04000000;
  static const uint32_t MCFG1_BEXCN                   = 0x02000000;
  static const uint32_t MCFG1_IO_WAITSTATES           = 0x01E00000;
  static const uint32_t MCFG1_IOEN                    = 0x00080000;
  static const uint32_t MCFG1_PWEN                    = 0x00000800;
  static const uint32_t MCFG1_IO_WIDTH                = 0x18000000;
  static const uint32_t MCFG1_PROM_WIDTH              = 0x00000300;
  static const uint32_t MCFG1_PROM_WRITE_WS           = 0x000000F0;
  static const uint32_t MCFG1_PROM_READ_WS            = 0x0000000F;

  //memory configuration register 2
  static const uint32_t MCFG2_WRITE_MASK              = 0xFFD07EFF;
  static const uint32_t MCFG2_SDRF                    = 0x80000000;
  static const uint32_t MCFG2_TRP                     = 0x40000000;
  static const uint32_t MCFG2_SDRAM_TRFC              = 0x38000000;
  static const uint32_t MCFG2_TCAS                    = 0x04000000;
  static const uint32_t MCFG2_SDRAM_BANKSZ            = 0x03800000;
  static const uint32_t MCFG2_SDRAM_COSZ              = 0x00600000;
  static const uint32_t MCFG2_SDRAM_CMD               = 0x00180000;
  static const uint32_t MCFG2_D64                     = 0x00040000;
  static const uint32_t MCFG2_MS                      = 0x00010000;
  static const uint32_t MCFG2_SE                      = 0x00004000;
  static const uint32_t MCFG2_SI                      = 0x00002000;
  static const uint32_t MCFG2_RAM_BANK_SIZE           = 0x00001E00;
  static const uint32_t MCFG2_RBRDY                   = 0x00000080;
  static const uint32_t MCFG2_RMW                     = 0x00000040;
  static const uint32_t MCFG2_RAM_WIDTH               = 0x00000030;
  static const uint32_t MCFG2_RAM_WRITE_WS            = 0x0000000C;
  static const uint32_t MCFG2_RAM_READ_WS             = 0x00000003;

  // Memory configuration register 3
  static const uint32_t MCFG3_WRITE_MASK              = 0x07FFF000;
  static const uint32_t MCFG3_SDRAM_RLD_VAL           = 0x07FFF000;

  // Memory configuration register 4
  static const uint32_t MCFG4_WRITE_MASK              = 0xE0F7007F;
  static const uint32_t MCFG4_ME                      = 0x80000000;
  static const uint32_t MCFG4_CE                      = 0x40000000;
  static const uint32_t MCFG4_EM                      = 0x20000000;
  static const uint32_t MCFG4_TXSR                    = 0x00F00000;
  static const uint32_t MCFG4_PMODE                   = 0x00070000;
  static const uint32_t MCFG4_DC                      = 0x00000060;
  static const uint32_t MCFG4_TCSR                    = 0x00000018;
  static const uint32_t MCFG4_PASR                    = 0x00000007;

  // --- Register default values

  // Memory configuration register 1
  static const uint32_t MCFG1_IOBUSW_DEFAULT          = 0x00000000;
  static const uint32_t MCFG1_IBRDY_DEFAULT           = 0x00000000;
  static const uint32_t MCFG1_BEXCN_DEFAULT           = 0x00000000;
  static const uint32_t MCFG1_IO_WAITSTATES_DEFAULT   = 0x00000000;
  static const uint32_t MCFG1_IOEN_DEFAULT            = 0x00000000;
  static const uint32_t MCFG1_PWEN_DEFAULT            = 0x00000000;
  static const uint32_t MCFG1_PROM_WIDTH_DEFAULT      = 0x00000000;
  static const uint32_t MCFG1_PROM_WRITE_WS_DEFAULT   = 0x000000F0;
  static const uint32_t MCFG1_PROM_READ_WS_DEFAULT    = 0x0000000F;
  //                                                  + __________;
  static const uint32_t MCFG1_DEFAULT =                 0x000000FF;

  // Memory configuration register 2
  static const uint32_t MCFG2_SDRF_DEFAULT            = 0x80000000;
  static const uint32_t MCFG2_TRP_DEFAULT             = 0x40000000;
  static const uint32_t MCFG2_SDRAM_TRFC_DEFAULT      = 0x38000000;
  static const uint32_t MCFG2_TCAS_DEFAULT            = 0x04000000;
  static const uint32_t MCFG2_SDRAM_BANKSZ_DEFAULT    = 0x03000000;
  static const uint32_t MCFG2_SDRAM_COSZ_DEFAULT      = 0x00600000;
  static const uint32_t MCFG2_SDRAM_CMD_DEFAULT       = 0x00000000;
  static const uint32_t MCFG2_D64_DEFAULT             = 0x00000000;
  static const uint32_t MCFG2_MS_DEFAULT              = 0x00000000;
  static const uint32_t MCFG2_SE_DEFAULT              = 0x00000000;
  static const uint32_t MCFG2_SI_DEFAULT              = 0x00000000;
  static const uint32_t MCFG2_RAM_BANK_SIZE_DEFAULT   = 0x00001C00;
  static const uint32_t MCFG2_RBRDY_DEFAULT           = 0x00000000;
  static const uint32_t MCFG2_RMW_DEFAULT             = 0x00000000;
  static const uint32_t MCFG2_RAM_WIDTH_DEFAULT       = 0x00000030;
  static const uint32_t MCFG2_RAM_WRITE_WS_DEFAULT    = 0x0000000C;
  static const uint32_t MCFG2_RAM_READ_WS_DEFAULT     = 0x00000003;
  //                                                  + __________;
  static const uint32_t MCFG2_DEFAULT                 = 0xFF601C3F;

  // Memory configuration register 3
  static const uint32_t MCFG3_DEFAULT                 = 0x07FFF000;

  // Memory configuration register 4
  static const uint32_t MCFG4_ME_DEFAULT              = 0x00000000;
  static const uint32_t MCFG4_CE_DEFAULT              = 0x00000000;
  static const uint32_t MCFG4_EM_DEFAULT              = 0x00000000;
  static const uint32_t MCFG4_TXSR_DEFAULT            = 0x00F00000;
  static const uint32_t MCFG4_PMODE_DEFAULT           = 0x00000000;
  static const uint32_t MCFG4_DS_DEFAULT              = 0x00000000;
  static const uint32_t MCFG4_TCSR_DEFAULT            = 0x00000000;
  static const uint32_t MCFG4_PASR_DEFAULT            = 0x00000000;
  //                                                  + __________;
  static const uint32_t MCFG4_DEFAULT                 = 0x00F00000;

};

#endif // __MCTRL_TEST_H__

  
