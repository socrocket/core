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
#include <boost/config.hpp>
#include <systemc.h>
#include <tlm.h>
#include <greenreg.h>
#include <greenreg_ambasocket.h>
#include "greencontrol/all.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "mctrlreg.h"
#include "generic_memory.h"

class Mctrl : public gs::reg::gr_device
{
public:
    //APB slave socket: connects mctrl config registers to apb
    gs::reg::greenreg_socket< gs::amba::amba_slave<32> > apb;

    //AHB slave socket: receives instructions (mem access) from CPU
    tlm_utils::simple_target_socket<Mctrl> ahb;

    //Master sockets: Initiate communication with memory modules
    tlm_utils::simple_initiator_socket<Mctrl> mctrl_rom;
    tlm_utils::simple_initiator_socket<Mctrl> mctrl_io;
    tlm_utils::simple_initiator_socket<Mctrl> mctrl_sram;
    tlm_utils::simple_initiator_socket<Mctrl> mctrl_sdram;

    //constructor / destructor
    Mctrl(sc_module_name name, int _hindex = 0,    int _pindex = 0,     int _romaddr = 0,    int _rommask = 3584,
           int _ioaddr = 512,  int _iomask = 3584, int _ramaddr = 1024, int _rammask = 3072,
           int _paddr = 0,     int _pmask = 4095,  int _wprot = 0,      int _invclk = 0,
           int _fast = 0,      int _srbanks = 4,
           int _ram8 = 0,      int _ram16 = 0,     int _sden = 0,       int _sepbus = 0,
           int _sdbits = 32,   int _sdlsb = 2,     int _oepol = 0,      int _syncrst = 0,
           int _pageburst = 0, int _scantest = 0,  int _mobile = 0);
    ~Mctrl();

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

    //define TLM transport functions
    virtual void b_transport(tlm::tlm_generic_payload& gp, sc_time& delay);

    //address space variables
    uint32_t rom_bk1_s, rom_bk1_e, rom_bk2_s, rom_bk2_e,
             io_s, io_e,
             sram_bk1_s, sram_bk1_e, sram_bk2_s, sram_bk2_e, sram_bk3_s, sram_bk3_e,
                                     sram_bk4_s, sram_bk4_e, sram_bk5_s, sram_bk5_e,
             sdram_bk1_s, sdram_bk1_e, sdram_bk2_s, sdram_bk2_e;

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
    sc_core::sc_time callback_delay; //count time elapsing in callbacks (to be added in next transaction)
    sc_core::sc_time start_idle;     //capture end time of last transaction to calculate sdram idle time
    uint8_t pmode;                   //capture current state of power mode

    //constructor parameters (modeling VHDL generics)
    const int hindex;
    const int pindex;
    const int romaddr;
    const int rommask;
    const int ioaddr;
    const int iomask;
    const int ramaddr;
    const int rammask;
    const int paddr;
    const int pmask;
    const int wprot;
    const int invclk;
    const int fast;
    const int srbanks;
    const int ram8;
    const int ram16;
    const int sden;
    const int sepbus;
    const int sdbits;
    const int sdlsb;
    const int oepol;
    const int syncrst;
    const int pageburst;
    const int scantest;
    const int mobile;
};

#include "mctrl.tpp"

#endif

