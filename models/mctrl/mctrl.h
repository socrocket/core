/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mctrl.h                                                 */
/*             header file defining the mctrl module template          */
/*             includes implementation file mctrl.tpp at the bottom    */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

#ifndef MCTRL_H
#define MCTRL_H

#define DEBUG

//#include <cmath.h>
#include <algorithm>
#include <iostream>
#include <boost/config.hpp>
#include <systemc.h>
#include <tlm.h>
#include <greenreg.h>
#include <greenreg_ambasocket.h>
#include "greencontrol/all.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "mctrlreg.h"
#include "generic_memory.h"
#include "grlibdevice.h"

class Mctrl : public gs::reg::gr_device, public amba_slave_base
{
public:
    //plug and play devices for AHB and APB
    CGrlibDevice pnpahb, pnpapb;

    //APB slave socket: connects mctrl config registers to apb
    gs::reg::greenreg_socket< gs::amba::amba_slave<32> > apb;

    //AHB slave socket: receives instructions (mem access) from CPU
    ::amba::amba_slave_socket<32> ahb;

    //Master sockets: Initiate communication with memory modules
    tlm_utils::simple_initiator_socket<Mctrl> mctrl_rom;
    tlm_utils::simple_initiator_socket<Mctrl> mctrl_io;
    tlm_utils::simple_initiator_socket<Mctrl> mctrl_sram;
    tlm_utils::simple_initiator_socket<Mctrl> mctrl_sdram;

    //constructor / destructor
    Mctrl(sc_module_name name,  int _romasel = 28,   int _sdrasel = 29,    int _romaddr = 0x0,   int _rommask = 0xE00,
           int _ioaddr = 0x200, int _iomask = 0xE00, int _ramaddr = 0x400, int _rammask = 0xC00,
           int _paddr = 0x0,    int _pmask = 0xFFF,  int _wprot = 0,       int _srbanks = 4,
           int _ram8 = 0,       int _ram16 = 0,      int _sepbus = 0,      int _sdbits = 32,
           int _mobile = 0,     int _sden = 0);
    ~Mctrl();

    //device identification on AHB bus
  	inline sc_dt::uint64 get_size() {
      //get start address of memory area
      uint64_t base = std::min(romaddr, ioaddr);
      base = std::min(static_cast<uint64_t>(ramaddr), base);

      //get end address of memory area
      uint64_t end = std::max(romaddr, ioaddr);
      end = std::max(static_cast<uint64_t>(ramaddr), end);
      //base of highest mem area + size of that area
      if ( end == static_cast<uint64_t>(ramaddr) ) {
        end += rammask;
      }
      else if ( end == static_cast<uint64_t>(ioaddr) ) {
        end += iomask;
      }
      else if ( end == static_cast<uint64_t>(romaddr) ) {
        end += rommask;
      }

      //size is given in MB, so << 20
		  return ((end - base) << 20);
  	}
	  inline sc_dt::uint64 get_base_addr() {
      //get start address of memory area
      uint64_t base = std::min(romaddr, ioaddr);
      base = std::min(static_cast<uint64_t>(ramaddr), base) << 20;

	  	return base;
  	}

    //proclamation of processes and callbacks
    SC_HAS_PROCESS(Mctrl);
    GC_HAS_CALLBACKS();

    //function prototypes
    void end_of_elaboration();
    void sram_calculate_bank_addresses(uint32_t sram_bank_size);

    //thread process to initialize MCTRL (set registers, define address spaces, etc.)
    void initialize_mctrl();

    //callbacks reacting on register access
    void launch_sdram_command();
    void configure_sdram();
    void erase_sdram();
    void sram_disable();
    void sdram_enable();
    void sram_change_bank_size();
    void sdram_change_bank_size();
    void sdram_change_refresh_cycle();

    //define TLM transport functions
    virtual void b_transport(tlm::tlm_generic_payload& gp, sc_time& delay);

    //simple payload extension for erasing memory
    struct ext_erase : public tlm::tlm_extension<ext_erase> {
    public:
      ext_erase() {erase_flag = 0;}
      bool erase_flag;
      //must_override pure virtual clone method
      virtual tlm::tlm_extension_base* clone() const {
        ext_erase* t = new ext_erase;
        t->erase_flag = this->erase_flag;
        return t;
      }
      //must override pure virtual copy_from method
      virtual void copy_from (tlm::tlm_extension_base const &ext) {
        erase_flag = static_cast<ext_erase const &>(ext).erase_flag;
      }

    };

  private:
    //address space variables
    uint32_t rom_bk1_s, rom_bk1_e, rom_bk2_s, rom_bk2_e,
             io_s, io_e,
             sram_bk1_s, sram_bk1_e, sram_bk2_s, sram_bk2_e, sram_bk3_s, sram_bk3_e,
                                     sram_bk4_s, sram_bk4_e, sram_bk5_s, sram_bk5_e,
             sdram_bk1_s, sdram_bk1_e, sdram_bk2_s, sdram_bk2_e;

    //control / timing variables
    sc_core::sc_time callback_delay; //count time elapsing in callbacks (to be added in next transaction)
    sc_core::sc_time start_idle;     //capture end time of last transaction to calculate sdram idle time
    sc_core::sc_time next_refresh;   //time to perform next refresh
    sc_core::sc_time refresh_stall;  //refresh can only be started in idle state, so it might be necessary to stall
    uint8_t trfc;                    //length of refresh cycle
    uint8_t pmode;                   //capture current state of power mode

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

#include "mctrl.tpp"

#endif

