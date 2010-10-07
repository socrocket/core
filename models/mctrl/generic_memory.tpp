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
//blocking transport for ROM
b_transport(tlm::tlm_generic_payload& gp, sc_time& delay) {

    //check erase extension first
    typename ext_erase::ext_erase* e;
    gp.get_extension(e);
    if (e) {
        uint32_t data = *reinterpret_cast<uint32_t *> (gp.get_data_ptr());
        erase_memory(gp.get_address(), data, gp.get_streaming_width());
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
    }
    //process regular read / write transactions
    else {
        unsigned int streaming_width = gp.get_streaming_width();
        //check TLM COMMAND field
        tlm::tlm_command cmd = gp.get_command();
        if (cmd == tlm::TLM_READ_COMMAND) {
            switch (streaming_width) {
                //64 bit --> SDRAM
                case 8: {
                    uint8_t length = gp.get_data_length();
                    uint32_t *data_ptr64 =
                            reinterpret_cast<uint32_t *> (gp.get_data_ptr());
                    read_32(gp.get_address(), data_ptr64, length);
                    break;
                }
                    //32 bit --> SRAM / ROM or SDRAM / IO --> 4x8 or 1x32
                case 4: {
                    //SDRAM or IO: map of uint32_t type
                    if (sizeof(memory[gp.get_address()]) == 4) {
                        uint8_t length = gp.get_data_length();
                        uint32_t
                                * data_ptr32 =
                                        reinterpret_cast<uint32_t *> (gp.get_data_ptr());
                        read_32(gp.get_address(), data_ptr32, length);
                    }
                    //SRAM or ROM: map of uint8_t type
                    else if (sizeof(memory[gp.get_address()]) == 1) {
                        uint8_t length = gp.get_data_length();
                        unsigned char
                                * data_ptr32 =
                                        reinterpret_cast<unsigned char *> (gp.get_data_ptr());
                        read_8(gp.get_address(), data_ptr32, length);
                    }
                    break;
                }
                    //16 bit --> SRAM or ROM --> 4x8
                case 2: {
                    uint8_t length = gp.get_data_length();
                    unsigned char
                            * data_ptr16 =
                                    reinterpret_cast<unsigned char *> (gp.get_data_ptr());
                    gp.set_streaming_width(4);
                    read_8(gp.get_address(), data_ptr16, length);
                    break;
                }
                    //8 bit --> SRAM or ROM --> 4x8
                case 1: {
                    uint8_t length = gp.get_data_length();
                    unsigned char
                            * data_ptr8 =
                                    reinterpret_cast<unsigned char *> (gp.get_data_ptr());
                    gp.set_streaming_width(4);
                    read_8(gp.get_address(), data_ptr8, length);
                }
            }

            //update response status
            gp.set_response_status(tlm::TLM_OK_RESPONSE);
        } else if (cmd == tlm::TLM_WRITE_COMMAND) {
            switch (streaming_width) {
                case 8: {
                    uint8_t length = gp.get_data_length();
                    uint32_t* data64 =
                            reinterpret_cast<uint32_t *> (gp.get_data_ptr());
                    write_32(gp.get_address(), data64, length);
                    break;
                }
                case 4: {
                    //SDRAM or IO: map of uint32_t type
                    if (sizeof(memory[gp.get_address()]) == 4) {
                        uint8_t length = gp.get_data_length();
                        uint32_t
                                * data32 =
                                        reinterpret_cast<uint32_t *> (gp.get_data_ptr());
                        write_32(gp.get_address(), data32, length);
                    }
                    //SRAM or ROM: map of uint8_t type
                    else if (sizeof(memory[gp.get_address()]) == 1) {
                        uint8_t length = gp.get_data_length();
                        unsigned char
                                * data8 =
                                        reinterpret_cast<unsigned char *> (gp.get_data_ptr());
                        write_8(gp.get_address(), data8, length);
                    }
                    break;
                }
                case 2: {
                    uint8_t length = gp.get_data_length();
                    unsigned char
                            * data8 =
                                    reinterpret_cast<unsigned char *> (gp.get_data_ptr());
                    write_8(gp.get_address(), data8, length);
                    break;
                }
                case 1: {
                    uint8_t length = gp.get_data_length();
                    unsigned char
                            * data8 =
                                    reinterpret_cast<unsigned char *> (gp.get_data_ptr());
                    write_8(gp.get_address(), data8, length);
                }
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
