#include "mctrl_test.h"

// Constructor
mctrl_test::mctrl_test(sc_core::sc_module_name name, int _romasel, int _sdrasel,
                   int _romaddr, int _rommask, int _ioaddr, int _iomask,
                   int _ramaddr, int _rammask, int _paddr, int _pmask,
                   int _wprot, int _srbanks, int _ram8, int _ram16,
                   int _sepbus, int _sdbits, int _mobile, int _sden, amba::amba_layer_ids abstractionLayer) :
                       apb("apb_master_sock", amba::amba_APB, amba::amba_LT, true),
                       ahb("ahb_master_sock", amba::amba_AHB, amba::amba_LT, true),
                       pindex("pindex"), pirqi("pirqi"), pirqo("pirqo"), 
                       hmbsel("hmbsel"), hcachei("hcachei"), hirqi("hirqi"), 
                       hcacheo("hcacheo"), hirqo("hirqo"),
                       romasel(_romasel), sdrasel(_sdrasel), 
                       romaddr(_romaddr), rommask(_rommask), 
                       ioaddr(_ioaddr), iomask(_iomask), 
                       ramaddr(_ramaddr), rammask(_rammask), 
                       paddr(_paddr), pmask(_pmask), wprot(_wprot),
                       srbanks(_srbanks), ram8(_ram8), ram16(_ram16), sepbus(_sepbus),
                       sdbits(_sdbits), mobile(_mobile), sden(_sden) {

    std::srand(std::time(NULL));
}

// TLM write transaction
bool mctrl_test::writeAHB_LT(uint32_t addr, uint8_t *data, uint32_t width, uint32_t length, bool fail) {
    sc_core::sc_time t;
    tlm::tlm_generic_payload gp;

    // For sub-word access over the AHB, the burst-size extension must be set
    amba::amba_burst_size *amba_burst_size;
    ahb.validate_extension<amba::amba_burst_size> (gp);
    ahb.get_extension<amba::amba_burst_size>(amba_burst_size, gp);
    amba_burst_size->value = (width<4)? width : 4;

    gp.set_command(tlm::TLM_WRITE_COMMAND);
    gp.set_address(addr);
    gp.set_data_length(length);
    gp.set_streaming_width(4);
    gp.set_byte_enable_ptr(NULL);
    gp.set_data_ptr(data);
    this->ahb->b_transport(gp, t);
    
    if(gp.get_response_status()!=tlm::TLM_OK_RESPONSE&&!fail) {
      v::warn << name() << "Write to Address " << v::uint32 << addr
              << " was " << gp.get_response_string() << v::endl;
    } else {
      v::debug << name() << "Write to Address " << v::uint32 << addr
               << " was " << gp.get_response_string() << v::endl;
    }
    wait(t);
    return !(gp.get_response_status()!=tlm::TLM_OK_RESPONSE&&!fail);
}

// TLM write transaction
void mctrl_test::writeAHB_DBG(uint32_t addr, uint8_t *data, uint32_t width, uint32_t length) {
    tlm::tlm_generic_payload gp;

    // For sub-word access over the AHB, the burst-size extension must be set
    amba::amba_burst_size *amba_burst_size;
    ahb.validate_extension<amba::amba_burst_size> (gp);
    ahb.get_extension<amba::amba_burst_size>(amba_burst_size, gp);
    amba_burst_size->value = (width<4)? width : 4;

    gp.set_command(tlm::TLM_WRITE_COMMAND);
    gp.set_address(addr);
    gp.set_data_length(length);
    gp.set_streaming_width(4);
    gp.set_byte_enable_ptr(NULL);
    gp.set_data_ptr(data);
    this->ahb->transport_dbg(gp);
    
    if(gp.get_response_status()!=tlm::TLM_OK_RESPONSE) {
      v::warn << name() << "Write to Address " << v::uint32 << addr
              << " was " << gp.get_response_string() << v::endl;
    }
}

//TLM read transaction
bool mctrl_test::readAHB_LT(uint32_t addr, uint8_t *data, uint32_t width, uint32_t length, bool fail) {
    sc_core::sc_time t;
    tlm::tlm_generic_payload gp;

    gp.set_command(tlm::TLM_READ_COMMAND);
    gp.set_address(addr);
    gp.set_data_length(length);
    gp.set_streaming_width(4);
    gp.set_byte_enable_ptr(NULL);
    gp.set_data_ptr(data);
    gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    this->ahb->b_transport(gp, t);
    
    if(gp.get_response_status()!=tlm::TLM_OK_RESPONSE&&!fail) {
      v::warn << name() << "Read from Address " << v::uint32 << addr 
              << " was " << gp.get_response_string() << v::endl;
    } else {
      v::debug << name() << "Read from Address " << v::uint32 << addr 
              << " was " << gp.get_response_string() << v::endl;
    }
    wait(t);
    return !(gp.get_response_status()!=tlm::TLM_OK_RESPONSE&&!fail);
}

//TLM read transaction
void mctrl_test::readAHB_DBG(uint32_t addr, uint8_t *data, uint32_t width, uint32_t length) {
    tlm::tlm_generic_payload gp;

    gp.set_command(tlm::TLM_READ_COMMAND);
    gp.set_address(addr);
    gp.set_data_length(length);
    gp.set_streaming_width(4);
    gp.set_byte_enable_ptr(NULL);
    gp.set_data_ptr(data);
    gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    this->ahb->transport_dbg(gp);
    
    if(gp.get_response_status()!=tlm::TLM_OK_RESPONSE) {
      v::warn << name() << "Read from Address " << v::uint32 << addr 
              << " was " << gp.get_response_string() << v::endl;
    }
}



//TLM write transaction
void mctrl_test::writeAPB(uint32_t addr, uint32_t data) {
    sc_core::sc_time t;
    tlm::tlm_generic_payload gp;
    #ifdef LITTLE_ENDIAN_BO
    swap_Endianess(data);
    #endif
    //for sub-word access over the AHB, the burst-size extension must be set
    amba::amba_burst_size *amba_burst_size;
    ahb.validate_extension<amba::amba_burst_size> (gp);
    ahb.get_extension<amba::amba_burst_size>(amba_burst_size, gp);
    amba_burst_size->value = 4;

    gp.set_command(tlm::TLM_WRITE_COMMAND);
    gp.set_address(addr);
    gp.set_data_length(4);
    gp.set_streaming_width(4);
    gp.set_byte_enable_ptr(NULL);
    gp.set_data_ptr((unsigned char*)&data);
    this->apb->b_transport(gp, t);
    
    if(gp.get_response_status()!=tlm::TLM_OK_RESPONSE) {
      v::warn << name() << "Write to Address " << v::uint32 << addr
              << " was " << gp.get_response_string() << v::endl;
    }
    wait(t);
}

//TLM read transaction
uint32_t mctrl_test::readAPB(uint32_t addr) {
    sc_core::sc_time t;
    //uint32_t is not sufficient for the 64 bit transaction of sdram.
    //using the testbench like this will overwrite the timing information with
    //memory contents in the 64 bit read function.
    uint32_t data;
    tlm::tlm_generic_payload gp;

    gp.set_command(tlm::TLM_READ_COMMAND);
    gp.set_address(addr);
    gp.set_data_length(4);
    gp.set_streaming_width(4);
    gp.set_byte_enable_ptr(NULL);
    gp.set_data_ptr((unsigned char*)&data);
    gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    this->apb->b_transport(gp, t);
    
    if(gp.get_response_status()!=tlm::TLM_OK_RESPONSE) {
      v::warn << name() << "Read from Address " << v::uint32 << addr 
              << " was " << gp.get_response_string() << v::endl;
    }
    wait(t);
    #ifdef LITTLE_ENDIAN_BO
    swap_Endianess(data);
    #endif

    return data;
}

bool mctrl_test::writeCheck(const uint32_t start, uint32_t end, uint32_t width, const uint32_t length, bool fail) {
    end += end % width;
    uint8_t data[end-start];
    uint8_t *data8 = reinterpret_cast<uint8_t *>(data);
    uint16_t *data16 = reinterpret_cast<uint16_t *>(data);
    uint32_t *data32 = reinterpret_cast<uint32_t *>(data);
    uint64_t *data64 = reinterpret_cast<uint64_t *>(data);
    bool error = false;
    //write into memory
    v::debug << name() << "Write Check " << v::endl;
    for(uint32_t i = 0; i < end - start; i+=width) {
        uint32_t index = i >> ((int)log2(width));
        //prepare data word
        uint32_t address = start + i;
        uint32_t rdata = std::rand();
        v::debug << name() << "loop counter=" << i << ", index=" << index << ", length=" << length << ", address=" << address << v::endl;
        switch(width) {
            case 1:
                  data8[index] = static_cast<uint8_t>((rdata) & 0xFF);
                  break;
            case 2:
                  data16[index] = static_cast<uint16_t>((rdata) & 0xFFFF);
                  break;
            case 4:
                  data32[index] = static_cast<uint32_t>((rdata) & 0xFFFFFFFF);
                  break;
            case 8:
                  data64[index] = static_cast<uint64_t>(((rdata) & 0xFFFFFFFFll) | (((rdata) & 0xFFFFFFFFll) << 32));
                  break;
        }
        v::debug << name() << "Write at counter=" << i << "(" << i-length+width << ") and from address=" << v::uint32 << address << "(" << v::uint32 << address-length+width << ") with length=" << std::dec << length << "(" << i-length+width+length-1 << ")" << v::endl;
        if((i+width)%length==0) {
            //send data word to write function
            if(writeAHB_LT(address-length+width, &data[i-length+width], width, length, fail)&&fail) {
                // Return if the transaction fails as predicted
                v::debug << name() << "Test failed as planed" << v::endl;
                return error;
            }
        }
    }
    //read from memory
    for(uint32_t i = 0; i < end - start; i++) {
        uint32_t address = start + i;
        uint8_t c = 0;
        readAHB_DBG(address, &c, 1, 1);
        if(c!=data[i]&&!fail) {
            v::info << name() << "Expecting " << v::uint8 << (uint32_t)data[i]
                << " at address " << v::uint32 << address << " but got " 
                << v::uint8 << (uint32_t)c << v::endl;
            v::warn << name() << "Test failed" << v::endl << v::endl;
            error |= true;
        } else if(c==data[i]&&fail) {
            v::warn << name() << "Test failed: Result is matching, but this test should not match!" << v::endl << v::endl; 
            error |= true;
        }
    }
    return error;
}

bool mctrl_test::readCheck(const uint32_t start, uint32_t end, uint32_t width, const uint32_t length, bool fail) {
    end += end % width;
    uint8_t data[end-start];
    uint8_t *data8 = reinterpret_cast<uint8_t *>(data);
    uint16_t *data16 = reinterpret_cast<uint16_t *>(data);
    uint32_t *data32 = reinterpret_cast<uint32_t *>(data);
    uint64_t *data64 = reinterpret_cast<uint64_t *>(data);
    bool error = false;
    //write into memory
    v::debug << name() << "Read Check " << v::endl;
    for(uint32_t i = 0; i < end - start; i+=width) {
        uint32_t index = i / width;
        //prepare data word
        uint32_t address = start + i;
        uint32_t rdata = std::rand();
        v::debug << name() << "loop counter=" << i << ", index=" << index << ", length=" << length << ", address=" << address << v::endl;
        switch(width) {
            case 1:
                  data8[index] = static_cast<uint8_t>((rdata) & 0xFF);
                  break;
            case 2:
                  data16[index] = static_cast<uint16_t>((rdata) & 0xFFFF);
                  break;
            case 4:
                  data32[index] = static_cast<uint32_t>((rdata) & 0xFFFFFFFF);
                  break;
            case 8:
                  data64[index] = static_cast<uint64_t>(((rdata) & 0xFFFFFFFFll) | (((rdata) & 0xFFFFFFFFll) << 32));
                  break;
        }
        if((i+width)%length==0) {
            //send data word to write function
            v::debug << name() << "Write at counter=" << i << "(" << i-length+width << ") and from address=" << v::uint32 << address << "(" << v::uint32 << address-length+width << ") with length=" << std::dec << length << "(" << i-length+width+length-1 << ")" << v::endl;
            writeAHB_DBG(address-length+width, &data[i-length+width], width, length);
        }
    }
    //read from memory
    for(uint32_t i = 0, j = 0; i < end - start; i+=width, j++) {
        uint32_t address = start + i;
        uint8_t d[width];
        uint8_t *d8 = reinterpret_cast<uint8_t *>(d);
        uint16_t *d16 = reinterpret_cast<uint16_t *>(d);
        uint32_t *d32 = reinterpret_cast<uint32_t *>(d);
        uint64_t *d64 = reinterpret_cast<uint64_t *>(d);
        
        if(readAHB_LT(address, d, width, width, fail)&&fail) {
            // if transaction fails return as predicted
            v::debug << name() << "Test failed as planed" << v::endl;
            return error;
        }
        switch(width) {
            case 1:
                if(*d8!=data8[j]&&!fail) {
                    v::info << name() << "Expecting " << v::uint8 << (uint32_t)data8[j]
                            << " at address " << v::uint32 << address << " but got " 
                            << v::uint8 << (uint32_t)*d8 << v::endl;
                    v::warn << name() << "Test failed" << v::endl << v::endl;
                    error |= true;
                } else if(*d16==data8[j]&&fail) {
                    v::warn << name() << "Test failed: Result is matching, but this test should not match!" << v::endl << v::endl; 
                    error |= true;
                }
                break;
            case 2:
                if(*d16!=data16[j]&&!fail) {
                    v::info << name() << "Expecting " << v::uint16 << data16[j]
                            << " at address " << v::uint32 << address << " but got " 
                            << v::uint16 << *d16 << v::endl;
                    v::warn << name() << "Test failed" << v::endl << v::endl;
                    error |= true;
                } else if(*d16==data16[j]&&fail) {
                    v::warn << name() << "Test failed: Result is matching, but this test should not match!" << v::endl << v::endl; 
                    error |= true;
                }
                break;
            case 4:
                if(*d32!=data32[j]&&!fail) {
                    v::info << name() << "Expecting " << v::uint32 << data32[j]
                            << " at address " << v::uint32 << address << " but got " 
                            << v::uint32 << *d32 << v::endl;
                    v::warn << name() << "Test failed" << v::endl << v::endl;
                    error |= true;
                } else if(*d32==data32[j]&&fail) {
                    v::warn << name() << "Test failed: Result is matching, but this test should not match!" << v::endl << v::endl; 
                    error |= true;
                }
                break;
            case 8:
                if(*d64!=data64[j]&&!fail) {
                    v::info << name() << "Expecting " << v::uint64 << data64[j]
                            << " at address " << v::uint32 << address << " but got " 
                            << v::uint64 << *d64 << v::endl;
                    v::warn << name() << "Test failed" << v::endl << v::endl;
                    error |= true;
                } else if(*d64==data64[i]&&fail) {
                    v::warn << name() << "Test failed: Result is matching, but this test should not match!" << v::endl << v::endl; 
                    error |= true;
                }
                break;
        }
    }
    return error;
}
