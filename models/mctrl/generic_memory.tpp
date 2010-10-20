/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       generic_memory.tpp                                      */
/*             implementation of the generic_memory module             */
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Dennis Bode                                             */
/***********************************************************************/

//scope
template<typename T> Generic_memory<T>::
//constructor
Generic_memory(sc_core::sc_module_name name) :
    // construct and name socket
            slave_socket("slave_socket") {
    // register transport functions to sockets
    slave_socket.register_b_transport(this, &Generic_memory::b_transport);
}

//explicit declaration of standard destructor required for linking
template<typename T> Generic_memory<T>::~Generic_memory() {
}

//-----------TLM--TRANSPORT--FUNCTIONS-----------//

//scope
template<typename T> void Generic_memory<T>::
//blocking transport
b_transport(tlm::tlm_generic_payload& gp, sc_time& delay) {

  //check erase extension first
  typename ext_erase::ext_erase* e;
  gp.get_extension(e);
  if ( e ) {
    uint32_t data = *reinterpret_cast<uint32_t *>( gp.get_data_ptr() );
    erase_memory( gp.get_address(), data, gp.get_streaming_width() );
    gp.set_response_status(tlm::TLM_OK_RESPONSE);
  }
  //process regular read / write transactions
  else {
    //check TLM COMMAND field
    tlm::tlm_command cmd = gp.get_command();
    if (cmd == tlm::TLM_READ_COMMAND) {
      //map of uint32_t type --> 32 bit read function, no matter what type of memory
      if (sizeof( memory[gp.get_address()] ) == 4) {
        uint8_t length = gp.get_data_length();
        uint32_t* data_ptr32 = reinterpret_cast<uint32_t *>( gp.get_data_ptr() );
        read_32( gp.get_address(), data_ptr32, length );
      }
      //map of uint8_t type --> 8 bit read function, no matter what type of memory
      else if (sizeof( memory[gp.get_address()] ) == 1) {
        uint8_t length = gp.get_data_length();
        unsigned char* data_ptr32 = reinterpret_cast<unsigned char *>( gp.get_data_ptr() );
        read_8( gp.get_address(), data_ptr32, length );
      }

      //update response status
      gp.set_response_status(tlm::TLM_OK_RESPONSE);
    }
    else if (cmd == tlm::TLM_WRITE_COMMAND) {
      //map of uint32_t type --> 32 bit read function, no matter what type of memory
      if (sizeof( memory[gp.get_address()] ) == 4) {
        uint8_t length = gp.get_data_length();
        uint32_t* data32 = reinterpret_cast<uint32_t *>( gp.get_data_ptr() );
        write_32( gp.get_address(), data32, length );
      }
      //map of uint8_t type --> 8 bit read function, no matter what type of memory
      else if (sizeof( memory[gp.get_address()] ) == 1) {
        uint8_t length = gp.get_data_length();
        unsigned char* data8 = reinterpret_cast<unsigned char *>(gp.get_data_ptr());
        write_8( gp.get_address(), data8, length );
      }

      //update response status
      gp.set_response_status(tlm::TLM_OK_RESPONSE);
    }
  }
}

//-----------------WRITE--FUNCTIONS----------------//

//scope
template<typename T> void Generic_memory<T>::
//write into ROM / SRAM: 8 bit access
write_8(uint32_t address, unsigned char* data, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        memory[address + i] = data[i];
    }
}

//scope
template<typename T> void Generic_memory<T>::
//write into IO / SDRAM: 32 bit access
write_32(uint32_t address, uint32_t* data, uint8_t length) {
    for (uint8_t i = 0; i < length / 4; i++) {
        memory[address + 4 * i] = data[i];
    }
}

//-----------------READ--FUNCTIONS-----------------//

//scope
template<typename T> void Generic_memory<T>::
//read from ROM / SRAM: 32 bit access only
read_8(uint32_t address, unsigned char* data_ptr, uint8_t length) {

    //Independent from 8, 16, or 32 bit access, 4 adjacent 8 bit words
    //will be read. Address and length have to be aligned!
    assert(!(address % 4));
    assert(!(length % 4));

    //processor must take care of consistent memory allocation and length parameter in gp
    for (int i = 0; i < length; i++) {
        data_ptr[i] = memory[address + i];
    }
}

//scope
template<typename T> void Generic_memory<T>::
//read from IO / SDRAM: 32 bit access
read_32(uint32_t address, uint32_t* data_ptr, uint8_t length) {

    //processor must take care of consistent memory allocation and length parameter in gp
    for (int i = 0; i < length / 4; i++) {
        data_ptr[i] = memory[address + 4 * i];
    }
}

//-------------SDRAM--ERASE--FUNCTION--------------//

//scope
template<typename T> void Generic_memory<T>::
//erase sdram
erase_memory(uint32_t start_address, uint32_t end_address, unsigned int length) {
    for (unsigned int i = start_address; i <= end_address; i += length) {
        memory.erase(i);
    }
}

//for (i=0, i<8, i++) {
//  c2 |= (c1 & (1 << i) << (7-i));
//}
