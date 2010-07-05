/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mctrl.tpp                                               */
/*             implementation of the mctrl module                      */
/*             is included by mctrl.h template header file             */
/*                                                                     */
/* Modified on $Date: 2010-06-09 10:30:16 +0200 (Wed, 09 Jun 2010) $   */
/*          at $Revision: 10 $                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef MCTRL_TPP
#define MCTRL_TPP

//constructor
template <int hindex,    int pindex,   int romaddr, int rommask,
          int ioaddr,    int iomask,   int ramaddr, int rammask,
          int paddr,     int pmask,    int wprot,   int invclk,
          int fast,      int romasel,  int sdrasel, int srbanks,
          int ram8,      int ram16,    int sden,    int sepbus,
          int sdbits,    int sdlsb,    int oepol,   int syncrst,
          int pageburst, int scantest, int mobile>
         Mctrl<hindex,    pindex,   romaddr,
               rommask,   ioaddr,   iomask,
               ramaddr,   rammask,  paddr,
               pmask,     wprot,    invclk,
               fast,      romasel,  sdrasel,
               srbanks,   ram8,     ram16,
               sden,      sepbus,   sdbits,
               sdlsb,     oepol,    syncrst,
               pageburst, scantest, mobile  >::Mctrl(sc_core::sc_module_name name)
  :
  gr_device(
            name,                      //sc_module name
            gs::reg::ALIGNED_ADDRESS,  //address mode (options: aligned / indexed)
            16,                        //dword size (of register file)
            NULL                       //parent module
           ),
  bus( //greenreg_socket
      "bus",            //name
      r,                //register container
      0x0,              // ?
      0xFFFFFFFF,       // ?
      ::amba::amba_APB, // ?bus type?
      ::amba::amba_LT,  // ?communication type / abstraction level?
      false             // ?
     ),
  mctrl_rom("mctrl_rom"),
  mctrl_stdio("mctrl_stdio"),
  mctrl_sram("mctrl_sram"),
  mctrl_sdram("mctrl_sdram")
  {

#ifdef MONOLITHIC_MODULE
  //connect sockets to memory module
  mem.mem_rom(mctrl_rom);
  mem.mem_stdio(mctrl_stdio);
  mem.mem_sram(mctrl_sram);
  mem.mem_sdram(mctrl_sdram);
#endif

  // create register | name + description
  r.create_register( "MCFG1", "Memory Configuration Register 1",
                   // offset
                      0x00,
                   // config
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                   // init value (to be calculated from the generics for all 4 registers)
                      0x00000000,
                   // write mask
                      MCTRL_MCFG1_WRITE_MASK,
                   // reg width (maximum 32 bit)
                      32,
                   // lock mask: Not implementet, has to be zero.
                      0x00
                   );
  r.create_register( "MCFG2", "Memory Configuration Register 2",
                      0x04,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      0x00000000,
                      MCTRL_MCFG2_WRITE_MASK,
                      32,
                      0x00
                   );
  r.create_register( "MCFG3", "Memory Configuration Register 3", 
                      0x08,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      0x00000000, 
                      MCTRL_MCFG3_WRITE_MASK,
                      32, 
                      0x00
                   );
  r.create_register( "MCFG4", "Power-Saving Configuration Register", 
                      0x0C,
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      0x00000000, 
                      MCTRL_MCFG4_WRITE_MASK,
                      32,
                      0x00
                   );

//------- still from Irqmp---------//
/*
if (0) {
  //process registration
  SC_METHOD(reset_registers);
  sensitive << rst.neg();

  SC_METHOD(register_irq);
  dont_initialize();
  sensitive << apbi_pirq;
}
*/
}

//standard destructor
template <int hindex,    int pindex,   int romaddr, int rommask,
          int ioaddr,    int iomask,   int ramaddr, int rammask,
          int paddr,     int pmask,    int wprot,   int invclk,
          int fast,      int romasel,  int sdrasel, int srbanks,
          int ram8,      int ram16,    int sden,    int sepbus,
          int sdbits,    int sdlsb,    int oepol,   int syncrst,
          int pageburst, int scantest, int mobile>
         Mctrl<hindex,    pindex,   romaddr,
               rommask,   ioaddr,   iomask,
               ramaddr,   rammask,  paddr,
               pmask,     wprot,    invclk,
               fast,      romasel,  sdrasel,
               srbanks,   ram8,     ram16,
               sden,      sepbus,   sdbits,
               sdlsb,     oepol,    syncrst,
               pageburst, scantest, mobile  >::~Mctrl() {

}



#endif
