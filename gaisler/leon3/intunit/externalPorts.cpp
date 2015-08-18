/***************************************************************************\
 *
 *   
 *         _/        _/_/_/_/    _/_/    _/      _/   _/_/_/
 *        _/        _/        _/    _/  _/_/    _/         _/
 *       _/        _/_/_/    _/    _/  _/  _/  _/     _/_/
 *      _/        _/        _/    _/  _/    _/_/         _/
 *     _/_/_/_/  _/_/_/_/    _/_/    _/      _/   _/_/_/
 *   
 *
 *
 *   
 *   This file is part of LEON3.
 *   
 *   LEON3 is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *   
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *   or see <http://www.gnu.org/licenses/>.
 *   
 *
 *
 *   (c) Luca Fossati, fossati.l@gmail.com
 *
\***************************************************************************/



#include "gaisler/leon3/intunit/externalPorts.hpp"
#include "gaisler/leon3/intunit/memory.hpp"
#include "core/common/trapgen/ToolsIf.hpp"
#include "core/common/trapgen/utils/trap_utils.hpp"
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/tlm_quantumkeeper.h>
#include "core/common/systemc.h"

using namespace leon3_funclt_trap;
void leon3_funclt_trap::TLMMemory::setDebugger( MemoryToolsIf< unsigned int > * debugger \
    ){
    this->debugger = debugger;
}

// Read dword
sc_dt::uint64 leon3_funclt_trap::TLMMemory::read_dword( const unsigned int & address, \
							const unsigned int asi, \
							const unsigned int flush,  \
							const unsigned int lock) throw(){

    sc_dt::uint64 datum = 0;
    if (this->dmi_ptr_valid){
        if(address + this->dmi_data.get_start_address() > this->dmi_data.get_end_address()){
            SC_REPORT_ERROR("TLM-2", "Error in reading memory data through DMI: address out of \
                bounds");
        }
        memcpy(&datum, this->dmi_data.get_dmi_ptr() - this->dmi_data.get_start_address() \
            + address, sizeof(datum));
        this->quantKeeper.inc(this->dmi_data.get_read_latency());
        if(this->quantKeeper.need_sync()){
            this->quantKeeper.sync();
        }

    } else{
        sc_time delay = this->quantKeeper.get_local_time();
        unsigned int debug = 0;
 
        dcio_payload_extension *dcioExt = new dcio_payload_extension();
        tlm::tlm_generic_payload trans;

        // Create & init data payload extension
        dcioExt->asi   = asi;
        dcioExt->flush = flush;
        dcioExt->lock  = lock;
        dcioExt->debug = &debug;

        trans.set_address(address);
        trans.set_read();
        trans.set_data_ptr(reinterpret_cast<unsigned char*>(&datum));
        trans.set_data_length(sizeof(datum));
        trans.set_byte_enable_ptr(0);
        trans.set_dmi_allowed(false);
        trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

        // Hook extension onto payload
        trans.set_extension(dcioExt);

        this->initSocket->b_transport(trans, delay);

        if(trans.is_response_error()){
            std::string errorStr("Error from b_transport, response status = " + trans.get_response_string());
            SC_REPORT_ERROR("TLM-2", errorStr.c_str());
        }
        if(trans.is_dmi_allowed()){
            this->dmi_data.init();
            this->dmi_ptr_valid = this->initSocket->get_direct_mem_ptr(trans, this->dmi_data);
        }

        //Now lets keep track of time
        this->quantKeeper.set(delay);
        if(this->quantKeeper.need_sync()){
          //std::cout << "Quantum (external) sync" << std::endl;
            this->quantKeeper.sync();
        }

        trans.free_all_extensions();
    }
    #ifdef LITTLE_ENDIAN_BO
    unsigned int datum1 = (unsigned int)(datum);
    this->swapEndianess(datum1);
    unsigned int datum2 = (unsigned int)(datum >> 32);
    this->swapEndianess(datum2);
    datum = datum1 | (((sc_dt::uint64)datum2) << 32);
    #endif

    return datum;
}

// read half word
unsigned short int leon3_funclt_trap::TLMMemory::read_half( const unsigned int & address,
							    const unsigned int asi,
							    const unsigned int flush,
							    const unsigned int lock) throw(){

    unsigned short int datum = 0;
    if (this->dmi_ptr_valid){
        if(address + this->dmi_data.get_start_address() > this->dmi_data.get_end_address()){
            SC_REPORT_ERROR("TLM-2", "Error in reading memory data through DMI: address out of \
                bounds");
        }
        memcpy(&datum, this->dmi_data.get_dmi_ptr() - this->dmi_data.get_start_address() \
            + address, sizeof(datum));
        this->quantKeeper.inc(this->dmi_data.get_read_latency());
        if(this->quantKeeper.need_sync()){
            this->quantKeeper.sync();
        }

    } else{
        sc_time delay = this->quantKeeper.get_local_time();
        unsigned int debug = 0;
        
        dcio_payload_extension *dcioExt = new dcio_payload_extension();
        tlm::tlm_generic_payload trans;

        // Create & init data payload extension
        dcioExt->asi   = asi;
        dcioExt->flush = flush;
        dcioExt->lock  = lock;
        dcioExt->debug = &debug;

        trans.set_address(address);
        trans.set_read();
        trans.set_data_ptr(reinterpret_cast<unsigned char*>(&datum));
        trans.set_data_length(sizeof(datum));
        trans.set_streaming_width(sizeof(datum));
        trans.set_byte_enable_ptr(0);
        trans.set_dmi_allowed(false);
        trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

        // Hook extension onto payload
        trans.set_extension(dcioExt);

        this->initSocket->b_transport(trans, delay);

        if(trans.is_response_error()){
            std::string errorStr("Error from b_transport, response status = " + trans.get_response_string());
            SC_REPORT_ERROR("TLM-2", errorStr.c_str());
        }
        if(trans.is_dmi_allowed()){
            this->dmi_data.init();
            this->dmi_ptr_valid = this->initSocket->get_direct_mem_ptr(trans, this->dmi_data);
        }

        //Now lets keep track of time
        this->quantKeeper.set(delay);
        if(this->quantKeeper.need_sync()){
	  //std::cout << "Quantum (external) sync" << std::endl;
            this->quantKeeper.sync();
        }

        trans.free_all_extensions();

    }
    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    #ifdef LITTLE_ENDIAN_BO
    this->swapEndianess(datum);
    #endif

    return datum;
}

// read byte
unsigned char leon3_funclt_trap::TLMMemory::read_byte( const unsigned int & address, \
						       const unsigned int asi,
						       const unsigned int flush,
						       const unsigned int lock) throw(){

    unsigned char datum = 0;
    if (this->dmi_ptr_valid){
        if(address + this->dmi_data.get_start_address() > this->dmi_data.get_end_address()){
            SC_REPORT_ERROR("TLM-2", "Error in reading memory data through DMI: address out of \
                bounds");
        }
        memcpy(&datum, this->dmi_data.get_dmi_ptr() - this->dmi_data.get_start_address() \
            + address, sizeof(datum));
        this->quantKeeper.inc(this->dmi_data.get_read_latency());
        if(this->quantKeeper.need_sync()){
            this->quantKeeper.sync();
        }

    } else {
        sc_time delay = this->quantKeeper.get_local_time();
        unsigned int debug = 0;

        dcio_payload_extension *dcioExt = new dcio_payload_extension();
        tlm::tlm_generic_payload trans;

        // Create & init data payload extension
        dcioExt->asi    = asi;
        dcioExt->flush  = flush;
        dcioExt->lock   = lock;
        dcioExt->debug  = &debug;

        trans.set_address(address);
        trans.set_read();
        trans.set_data_ptr(reinterpret_cast<unsigned char*>(&datum));
        trans.set_data_length(sizeof(datum));
        trans.set_streaming_width(sizeof(datum));
        trans.set_byte_enable_ptr(0);
        trans.set_dmi_allowed(false);
        trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
        
        // Hook extension onto payload
        trans.set_extension(dcioExt);

        this->initSocket->b_transport(trans, delay);

        if(trans.is_response_error()){
            std::string errorStr("Error from b_transport, response status = " + trans.get_response_string());
            SC_REPORT_ERROR("TLM-2", errorStr.c_str());
        }
        if(trans.is_dmi_allowed()){
            this->dmi_data.init();
            this->dmi_ptr_valid = this->initSocket->get_direct_mem_ptr(trans, this->dmi_data);
        }

        // Now lets keep track of time
        this->quantKeeper.set(delay);
        if(this->quantKeeper.need_sync()){
	  //std::cout << "Quantum (external) sync" << std::endl;
            this->quantKeeper.sync();
        }

        trans.free_all_extensions();
    }

    return datum;
}

// Write dword
void leon3_funclt_trap::TLMMemory::write_dword( const unsigned int & address, 
						sc_dt::uint64 datum,
						const unsigned int asi,
						const unsigned int flush,
						const unsigned int lock) throw(){

    #ifdef LITTLE_ENDIAN_BO
    unsigned int datum1 = (unsigned int)(datum);
    this->swapEndianess(datum1);
    unsigned int datum2 = (unsigned int)(datum >> 32);
    this->swapEndianess(datum2);
    datum = datum1 | (((sc_dt::uint64)datum2) << 32);
    #endif
    if(this->debugger != NULL){
        this->debugger->notifyAddress(address, sizeof(datum));
    }
    if(this->dmi_ptr_valid){
        if(address + this->dmi_data.get_start_address() > this->dmi_data.get_end_address()){
            SC_REPORT_ERROR("TLM-2", "Error in writing memory data through DMI: address out of \
                bounds");
        }
        memcpy(this->dmi_data.get_dmi_ptr() - this->dmi_data.get_start_address() + address, \
            &datum, sizeof(datum));
        this->quantKeeper.inc(this->dmi_data.get_write_latency());
        if(this->quantKeeper.need_sync()){
            this->quantKeeper.sync();
        }
    } else {
        sc_time delay = this->quantKeeper.get_local_time();
        unsigned int debug = 0;
        
        dcio_payload_extension *dcioExt = new dcio_payload_extension();
        tlm::tlm_generic_payload trans;

        // Create & init data payload extension
        dcioExt->asi    = asi;
        dcioExt->flush  = flush;
        dcioExt->lock   = lock;
        dcioExt->debug  = &debug;

        trans.set_address(address);
        trans.set_write();
        trans.set_data_ptr((unsigned char*)&datum);
        trans.set_data_length(sizeof(datum));
        trans.set_streaming_width(sizeof(datum));
        trans.set_byte_enable_ptr(0);
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        
        // Hook extension onto payload
        trans.set_extension(dcioExt);	

        this->initSocket->b_transport(trans, delay);

        if(trans.is_response_error()){
            std::string errorStr("Error from b_transport, response status = " + trans.get_response_string());
            SC_REPORT_ERROR("TLM-2", errorStr.c_str());
        }
        if(trans.is_dmi_allowed()){
            this->dmi_data.init();
            this->dmi_ptr_valid = this->initSocket->get_direct_mem_ptr(trans, this->dmi_data);
        }

        //Now lets keep track of time
        this->quantKeeper.set(delay);
        if(this->quantKeeper.need_sync()){
	  //std::cout << "Quantum (external) sync" << std::endl;
            this->quantKeeper.sync();
        }

        trans.free_all_extensions();
    }
}

// write half word
void leon3_funclt_trap::TLMMemory::write_half( const unsigned int & address, 
					       unsigned short int datum,
					       unsigned int asi,
					       unsigned int flush,
					       unsigned int lock) throw(){

    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    #ifdef LITTLE_ENDIAN_BO
    this->swapEndianess(datum);
    #endif
    #ifdef LITTLE_ENDIAN_BO
    #else
    #endif
    if(this->debugger != NULL){
        this->debugger->notifyAddress(address, sizeof(datum));
    }
    if(this->dmi_ptr_valid){
        if(address + this->dmi_data.get_start_address() > this->dmi_data.get_end_address()){
            SC_REPORT_ERROR("TLM-2", "Error in writing memory data through DMI: address out of \
                bounds");
        }
        memcpy(this->dmi_data.get_dmi_ptr() - this->dmi_data.get_start_address() + address, \
            &datum, sizeof(datum));
        this->quantKeeper.inc(this->dmi_data.get_write_latency());
        if(this->quantKeeper.need_sync()){
            this->quantKeeper.sync();
        }
    } else{
        sc_time delay = this->quantKeeper.get_local_time();
        unsigned int debug = 0;
        
        dcio_payload_extension *dcioExt = new dcio_payload_extension();
        tlm::tlm_generic_payload trans;
		
        // Create & init data payload extension
        dcioExt->asi    = asi;
        dcioExt->flush  = flush;
        dcioExt->lock   = lock;
        dcioExt->debug = &debug;

        trans.set_address(address);
        trans.set_write();
        trans.set_data_ptr((unsigned char*)&datum);
        trans.set_data_length(sizeof(datum));
        trans.set_streaming_width(sizeof(datum));
        trans.set_byte_enable_ptr(0);
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

        // Hook extension onto payload
        trans.set_extension(dcioExt);

        this->initSocket->b_transport(trans, delay);

        if(trans.is_response_error()){
            std::string errorStr("Error from b_transport, response status = " + trans.get_response_string());
            SC_REPORT_ERROR("TLM-2", errorStr.c_str());
        }
        if(trans.is_dmi_allowed()){
            this->dmi_data.init();
            this->dmi_ptr_valid = this->initSocket->get_direct_mem_ptr(trans, this->dmi_data);
        }

        // Now lets keep track of time
        this->quantKeeper.set(delay);
        if(this->quantKeeper.need_sync()){
	  //std::cout << "Quantum (external) sync" << std::endl;
            this->quantKeeper.sync();
        }

        trans.free_all_extensions();

    }
}

// write byte
void leon3_funclt_trap::TLMMemory::write_byte( const unsigned int & address, 
					       unsigned char datum,
					       unsigned int asi,
					       unsigned int flush,
					       unsigned int lock) throw(){

    #ifdef LITTLE_ENDIAN_BO
    #else
    #endif
    if(this->debugger != NULL){
        this->debugger->notifyAddress(address, sizeof(datum));
    }
    if(this->dmi_ptr_valid){
        if(address + this->dmi_data.get_start_address() > this->dmi_data.get_end_address()){
            SC_REPORT_ERROR("TLM-2", "Error in writing memory data through DMI: address out of \
                bounds");
        }
        memcpy(this->dmi_data.get_dmi_ptr() - this->dmi_data.get_start_address() + address, \
            &datum, sizeof(datum));
        this->quantKeeper.inc(this->dmi_data.get_write_latency());
        if(this->quantKeeper.need_sync()){
            this->quantKeeper.sync();
        }
    } else {
        sc_time delay = this->quantKeeper.get_local_time();
        unsigned int debug = 0;

        dcio_payload_extension *dcioExt = new dcio_payload_extension();
        tlm::tlm_generic_payload trans;

        // Create & init data payload extension
        dcioExt->asi    = asi;
        dcioExt->flush  = flush;
        dcioExt->lock   = lock;
        dcioExt->debug  = &debug;	

        trans.set_address(address);
        trans.set_write();
        trans.set_data_ptr((unsigned char*)&datum);
        trans.set_data_length(sizeof(datum));
        trans.set_streaming_width(sizeof(datum));
        trans.set_byte_enable_ptr(0);
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
 
        // Hook extension onto payload
        trans.set_extension(dcioExt);	

        this->initSocket->b_transport(trans, delay);

        if(trans.is_response_error()){
            std::string errorStr("Error from b_transport, response status = " + trans.get_response_string());
            SC_REPORT_ERROR("TLM-2", errorStr.c_str());
        }
        if(trans.is_dmi_allowed()){
            this->dmi_data.init();
            this->dmi_ptr_valid = this->initSocket->get_direct_mem_ptr(trans, this->dmi_data);
        }

        wait(delay);

        //Now lets keep track of time
        this->quantKeeper.set(delay);
        if(this->quantKeeper.need_sync()){
	  //std::cout << "Quantum (external) sync" << std::endl;
            this->quantKeeper.sync();
        }

        trans.free_all_extensions();
    }
}

sc_dt::uint64 leon3_funclt_trap::TLMMemory::read_dword_dbg( const unsigned int & \
    address ) throw(){

    unsigned int debug = 0;
    dcio_payload_extension *dcioExt = new dcio_payload_extension();
    tlm::tlm_generic_payload trans;
    
    trans.set_address(address);
    trans.set_read();
    trans.set_data_length(8);
    trans.set_streaming_width(8);
    sc_dt::uint64 datum = 0;
    trans.set_data_ptr(reinterpret_cast<unsigned char *>(&datum));
    
    // Create & init data payload extension
    dcioExt->asi   = 8;
    dcioExt->flush = 0;
    dcioExt->lock  = 0;
    dcioExt->debug = &debug;

    // Hook extension onto payload
    trans.set_extension(dcioExt);

    this->initSocket->transport_dbg(trans);
    #ifdef LITTLE_ENDIAN_BO
    unsigned int datum1 = (unsigned int)(datum);
    this->swapEndianess(datum1);
    unsigned int datum2 = (unsigned int)(datum >> 32);
    this->swapEndianess(datum2);
    datum = datum1 | (((sc_dt::uint64)datum2) << 32);
    #endif
    
    trans.free_all_extensions();

    return datum;
}

unsigned int leon3_funclt_trap::TLMMemory::read_word_dbg( const unsigned int & address \
    ) throw(){
    unsigned int debug = 0;
    unsigned int datum = 0;
    dcio_payload_extension *dcioExt = new dcio_payload_extension();
    tlm::tlm_generic_payload trans;

    trans.set_address(address);
    trans.set_read();
    trans.set_data_length(4);
    trans.set_streaming_width(4);
    trans.set_data_ptr(reinterpret_cast<unsigned char *>(&datum));

    // Create & init data payload extension
    dcioExt->asi   = 8;
    dcioExt->flush = 0;
    dcioExt->lock  = 0;
    dcioExt->debug = &debug;

    // Hook extension onto payload
    trans.set_extension(dcioExt);

    this->initSocket->transport_dbg(trans);
    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    #ifdef LITTLE_ENDIAN_BO
    this->swapEndianess(datum);
    #endif

    trans.free_all_extensions();

    return datum;
}

unsigned short int leon3_funclt_trap::TLMMemory::read_half_dbg( const unsigned int \
    & address ) throw(){
    unsigned int debug = 0;
    unsigned short int datum = 0;
    dcio_payload_extension *dcioExt = new dcio_payload_extension();
    tlm::tlm_generic_payload trans;

    trans.set_address(address);
    trans.set_read();
    trans.set_data_length(2);
    trans.set_streaming_width(2);
    trans.set_data_ptr(reinterpret_cast<unsigned char *>(&datum));

    // Create & init data payload extension
    dcioExt->asi   = 8;
    dcioExt->flush = 0;
    dcioExt->lock  = 0;
    dcioExt->debug = &debug;

    // Hook extension onto payload
    trans.set_extension(dcioExt);

    this->initSocket->transport_dbg(trans);
    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    #ifdef LITTLE_ENDIAN_BO
    this->swapEndianess(datum);
    #endif
    return datum;
}

unsigned char leon3_funclt_trap::TLMMemory::read_byte_dbg( const unsigned int & address \
    ) throw(){
    unsigned int debug = 0;
    unsigned char datum = 0;
    dcio_payload_extension *dcioExt = new dcio_payload_extension();
    tlm::tlm_generic_payload trans;

    trans.set_address(address);
    trans.set_read();
    trans.set_data_length(1);
    trans.set_streaming_width(1);
    trans.set_data_ptr(reinterpret_cast<unsigned char *>(&datum));

    // Create & init data payload extension
    dcioExt->asi   = 8;
    dcioExt->flush = 0;
    dcioExt->lock  = 0;
    dcioExt->debug = &debug;

    // Hook extension onto payload
    trans.set_extension(dcioExt);

    this->initSocket->transport_dbg(trans);

    trans.free_all_extensions();

    return datum;
}

void leon3_funclt_trap::TLMMemory::write_dword_dbg( const unsigned int & address, \
    sc_dt::uint64 datum ) throw(){
    #ifdef LITTLE_ENDIAN_BO
    unsigned int datum1 = (unsigned int)(datum);
    this->swapEndianess(datum1);
    unsigned int datum2 = (unsigned int)(datum >> 32);
    this->swapEndianess(datum2);
    datum = datum1 | (((sc_dt::uint64)datum2) << 32);
    #endif

    unsigned int debug = 0;
    dcio_payload_extension *dcioExt = new dcio_payload_extension();
    tlm::tlm_generic_payload trans;
    trans.set_address(address);
    trans.set_write();
    trans.set_data_length(8);
    trans.set_streaming_width(8);
    trans.set_data_ptr((unsigned char *)&datum);

    // Create & init data payload extension
    dcioExt->asi   = 8;
    dcioExt->flush = 0;
    dcioExt->lock  = 0;
    dcioExt->debug = &debug;

    // Hook extension onto payload
    trans.set_extension(dcioExt);

    this->initSocket->transport_dbg(trans);

    trans.free_all_extensions();
}

void leon3_funclt_trap::TLMMemory::write_word_dbg( const unsigned int & address, \
    unsigned int datum ) throw(){
    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    #ifdef LITTLE_ENDIAN_BO
    this->swapEndianess(datum);
    #endif
    unsigned int debug = 0;
    dcio_payload_extension *dcioExt = new dcio_payload_extension();
    tlm::tlm_generic_payload trans;

    trans.set_address(address);
    trans.set_write();
    trans.set_data_length(4);
    trans.set_streaming_width(4);
    trans.set_data_ptr((unsigned char *)&datum);

    // Create & init data payload extension
    dcioExt->asi   = 8;
    dcioExt->flush = 0;
    dcioExt->lock  = 0;
    dcioExt->debug = &debug;

    // Hook extension onto payload
    trans.set_extension(dcioExt);

    this->initSocket->transport_dbg(trans);

    trans.free_all_extensions();
}

void leon3_funclt_trap::TLMMemory::write_half_dbg( const unsigned int & address, \
    unsigned short int datum ) throw(){
    //Now the code for endianess conversion: the processor is always modeled
    //with the host endianess; in case they are different, the endianess
    //is turned
    #ifdef LITTLE_ENDIAN_BO
    this->swapEndianess(datum);
    #endif
    #ifdef LITTLE_ENDIAN_BO
    #else
    #endif
    unsigned int debug = 0;
    dcio_payload_extension *dcioExt = new dcio_payload_extension();
    tlm::tlm_generic_payload trans;
    trans.set_address(address);
    trans.set_write();
    trans.set_data_length(2);
    trans.set_streaming_width(2);
    trans.set_data_ptr((unsigned char *)&datum);

    // Create & init data payload extension
    dcioExt->asi   = 8;
    dcioExt->flush = 0;
    dcioExt->lock  = 0;
    dcioExt->debug = &debug;

    // Hook extension onto payload
    trans.set_extension(dcioExt);

    this->initSocket->transport_dbg(trans);

    trans.free_all_extensions();
}

void leon3_funclt_trap::TLMMemory::write_byte_dbg( const unsigned int & address, \
    unsigned char datum ) throw(){
    #ifdef LITTLE_ENDIAN_BO
    #else
    #endif
    unsigned int debug = 0;
    dcio_payload_extension *dcioExt = new dcio_payload_extension();
    tlm::tlm_generic_payload trans;

    trans.set_address(address);
    trans.set_write();
    trans.set_data_length(1);
    trans.set_streaming_width(1);
    trans.set_data_ptr((unsigned char *)&datum);

    // Create & init data payload extension
    dcioExt->asi   = 8;
    dcioExt->flush = 0;
    dcioExt->lock  = 0;
    dcioExt->debug = &debug;

    // Hook extension onto payload
    trans.set_extension(dcioExt);

    this->initSocket->transport_dbg(trans);

    trans.free_all_extensions();
}

void leon3_funclt_trap::TLMMemory::lock(){

}

void leon3_funclt_trap::TLMMemory::unlock(){

}

leon3_funclt_trap::TLMMemory::TLMMemory( sc_module_name portName, tlm_utils::tlm_quantumkeeper \
    & quantKeeper ) : sc_module(portName), quantKeeper(quantKeeper){
    this->debugger = NULL;
    this->dmi_ptr_valid = false;
    end_module();
}


