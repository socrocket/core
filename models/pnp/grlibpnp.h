/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       grlibpnp.h                                              */
/*             header file containing the definition of the grlib      */
/*             ahbmaster plug and play functionality. Due to the use   */
/*             of the AMBAKit the plug and play functionality is       */
/*             implemented in its own model                            */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Rolf Meyer                                              */
/***********************************************************************/

#ifndef GRLIB_PLUG_AND_PLAY_H
#define GRLIB_PLUG_AND_PLAY_H

#include "grlibdevice.h"

#include "amba.h"

/// This model contains the plug and play functionality of the GRLIB AHB Master. 
/// It provides the registers to the AHB Bus. And gathers its content from the instantiated model in the simulation environment.
///
/// See the GRLIB user’s manual for more information. 
/// These reegister are combined into an array which is connected to the AHB controller unit.
/// The plug&play information is mapped on a read-only address area, defined by the cfgaddr and cfgmask
/// VHDL generics, in combination with the ioaddr and iomask VHDL generics. By default, the
/// area is mapped on address 0xFFFFF000 - 0xFFFFFFFF. The master information is placed on the first
/// 2 kbyte of the block (0xFFFFF000 - 0xFFFFF800), while the slave information is placed on the second
/// 2 kbyte block. Each unit occupies 32 bytes, which means that the area has place for 64 masters
/// and 64 slaves. The address of the plug&play information for a certain unit is defined by its bus index.
/// The address for masters is thus 0xFFFFF000 + n*32, and 0xFFFFF800 + n*32 for slaves.
///
/// In oder to use this implementation you have to know that ther is no mechanism to unregister devices
/// , once a device is registered it connected. Furthermore there is no logic to handle the deletion of
/// a device.
///
/// @param ioaddr   The MSB address of the I/O area. Sets the 12 most significant bits in the 
///                 32-bit AHB address (i.e. 31 downto 20). Allowed values are between 0 and 16#FFF#. 
///                 Default value is 16#FFF#.
/// @param iomask   The I/O area address mask. Sets the size of the I/O area and the start 
///                 address together with ioaddr. Allowed values are between 0 and 16#FFF#. 
///                 Default value is 16#FFF#.
/// @param cfgaddr  The MSB address of the configuration area. Sets 12 bits in the 32-bit AHB 
///                 address (i.e. 19 downto 8). Allowed are values from 0 to 16#FFF#. 
///                 Default value is 16#FF0#.
/// @param cfgmask  The address mask of the configuration area. Sets the size of the configuration 
///                 area and the start address together with cfgaddr. If set to 0, the configuration 
///                 will be disabled. Values between 0 and 16#FFF# are allowed, default is 16#FF0#.
template <uint16_t ioaddr, uint16_t iomask, uint16_t cfgaddr, uint16_t cfgmask, uint32_t BUSWIDTH = 32>
class GrlibPnP : public sc_core::sc_module {
  public:

    /// AHB Slave Socket
	  amba::amba_slave_socket<BUSWIDTH> ahb_slave;
    
	  SC_HAS_PROCESS(GrlibPnP);

    // Constructor
  	GrlibPnP(sc_core::sc_module_name nm)
      : ahb_slave("ahb_slave", amba::amba_AHB, amba::amba_LT, false)
      , master_count(0), slave_count(0) {
      ahb_slave.register_b_transport(this, &GrlibPnP::b_transport);
    }
    
    /// Blocking Transport function for the AHB Slave Socket.
    /// Incomming requests will be processed here. Write requests are ignored.
    /// Read requests will seperated in register access and processt in 
    /// get_registers(uint32_t nr).
    ///
    /// @param gp The payload object form the socket.
    /// @param t  A time object for delaying.
    void b_transport(tlm::tlm_generic_payload& gp, sc_core::sc_time &t) {
      uint32_t *data    = reinterpret_cast<unsigned int *>(gp.get_data_ptr());
      if(gp.is_write()){
        gp.set_response_status(tlm::TLM_OK_RESPONSE);  
      }
      if(gp.is_read()){
        std::cout<<"  Return Data is :"<<std::endl;
        unsigned int address = (gp.get_address() & 0xFFF) >> 2;
        unsigned int length = gp.get_data_length() >> 2;
        for (unsigned int i=0; i<length; i++){
          data[i] = get_register(address + i);
        }
      }
      gp.set_response_status(tlm::TLM_OK_RESPONSE); 
    }

    /// Return the content of a specific register.
    /// The register nr is seperated into slave or maste area,
    /// device number and registernumber to access the register.
    inline uint32_t get_register(uint32_t nr) const {
      bool      slave   = (nr >> 9) & 0x1;
      uint8_t   dev     = (nr >> 3) & 0x3F;
      uint8_t   reg     = (nr)      & 0x7;
      if(slave) {
        if(dev < slave_count) {
          return slaves[dev][reg];
        }
      } else {
        if(dev < master_count) {
          return masters[dev][reg];
        }
      }
      return 0;
    }
    
    /// This function is used to register a master device.
    /// In oder to know all mambers on the bus they need to be registerd.
    ///
    /// @param dev The master device which will be registerd.
    void register_master(GrlibDevice *dev) {
      masters[master_count++] = dev->get_device_pointer();
    }
    
    /// This function is used to register a slave device.
    /// In oder to know all mambers on the bus they need to be registerd.
    ///
    /// @param dev The slave device which will be registerd.
    void register_slave(GrlibDevice *dev) {
      slaves[slave_count++] = dev->get_device_pointer();
    }
    
  private:
    /// An array of master device registers files.
    const uint32_t *masters[64];

    /// An array of slave device registers files.
    const uint32_t *slaves[64];

    /// The number of registerd masters.
    uint8_t   master_count;

    /// The number of registerd slaves.
    uint8_t   slave_count;
};

#endif
