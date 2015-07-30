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
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *   
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *   
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *   or see <http://www.gnu.org/licenses/>.
 *   
 *
 *
 *   (c) Luca Fossati, fossati.luca@fastwebnet.it
 *
\***************************************************************************/


#ifndef LT_EXTERNALPORTS_HPP
#define LT_EXTERNALPORTS_HPP

#include "gaisler/leon3/intunit/memory.hpp"
#include "core/common/systemc.h"
#include "core/common/trapgen/ToolsIf.hpp"
#include "core/common/trapgen/utils/trap_utils.hpp"
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/tlm_quantumkeeper.h>
#include "gaisler/leon3/mmucache/icio_payload_extension.h"
#include "gaisler/leon3/mmucache/dcio_payload_extension.h"
#include "core/common/verbose.h"

#define FUNC_MODEL
#define LT_IF
namespace leon3_funclt_trap{

    class TLMMemory : public MemoryInterface, public sc_module{
        private:
        MemoryToolsIf< unsigned int > * debugger;
        tlm_utils::tlm_quantumkeeper & quantKeeper;
        bool dmi_ptr_valid;
        tlm::tlm_dmi dmi_data;

        public:
        TLMMemory( sc_module_name portName, tlm_utils::tlm_quantumkeeper & quantKeeper );
        void setDebugger( MemoryToolsIf< unsigned int > * debugger );

        // Read dword
        sc_dt::uint64 read_dword( const unsigned int & address,
				  const unsigned int asi,
				  const unsigned int flush,
				  const unsigned int lock) throw();


        // Read data word 
        inline unsigned int read_word( const unsigned int & address,
				       const unsigned int asi,
				       const unsigned int flush,
				       const unsigned int lock) throw(){

            unsigned int datum = 0;
            if(this->dmi_ptr_valid) {
                v::debug << name() << "DMI Access" << endl;
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

                // Release extension
                trans.free_all_extensions();
                
                //Now lets keep track of time
                this->quantKeeper.set(delay);
                if(this->quantKeeper.need_sync()){
		  // std::cout << "Quantum (external) sync" << std::endl;
                  this->quantKeeper.sync();
                }
            }
            //Now the code for endianess conversion: the processor is always modeled
            //with the host endianess; in case they are different, the endianess
            //is turned
            #ifdef LITTLE_ENDIAN_BO
            this->swapEndianess(datum);
            #endif
            v::debug << name() << "Read word:0x" << hex << v::setw(8) << v::setfill('0')
                     << datum << ", from:0x" << hex << v::setw(8) << v::setfill('0')
                     << address << endl;

            return datum;
        }

        // Read half-word
        unsigned short int read_half( const unsigned int & address,
				      const unsigned int asi,
				      const unsigned int flush,
				      const unsigned int lock) throw();

        // Read byte
        unsigned char read_byte( const unsigned int & address,
				 const unsigned int asi,
				 const unsigned int flush,
				 const unsigned int lock) throw();

        // Read instruction
        inline unsigned int read_instr( const unsigned int & address,
				        const unsigned int flush) throw() {

            unsigned int datum = 0;
            if (this->dmi_ptr_valid){
                v::debug << name() << "DMI Access" << endl;
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
                icio_payload_extension *icioExt = new icio_payload_extension();
                tlm::tlm_generic_payload trans;
		
                // Create & init instruction payload extension
                icioExt->flush  = flush;
                icioExt->debug = &debug;

                trans.set_address(address);
                trans.set_read();
                trans.set_data_ptr(reinterpret_cast<unsigned char*>(&datum));
                trans.set_data_length(sizeof(datum));
                trans.set_byte_enable_ptr(0);
                trans.set_dmi_allowed(false);
                trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

                // Hook extension onto payload
                trans.set_extension(icioExt);

                this->initSocket->b_transport(trans, delay);

                if(trans.is_response_error()){
                    std::string errorStr("Error from b_transport, response status = " + trans.get_response_string());
                    SC_REPORT_ERROR("TLM-2", errorStr.c_str());
                }
                if(trans.is_dmi_allowed()){
                    this->dmi_data.init();
                    this->dmi_ptr_valid = this->initSocket->get_direct_mem_ptr(trans, this->dmi_data);
                }

                // Remove payload extension
                trans.free_all_extensions();

                //Now lets keep track of time
                this->quantKeeper.set(delay);
                if(this->quantKeeper.need_sync()){
		  //std::cout << "Quantum (external) sync" << std::endl;
                  this->quantKeeper.sync();
                }
            }
            //Now the code for endianess conversion: the processor is always modeled
            //with the host endianess; in case they are different, the endianess
            //is turned
            this->swapEndianess(datum);
            #ifdef __LITTLE_ENDIAN_BO
            //this->swapEndianess(datum);
            #endif
            v::debug << name() << "Read word:0x" << hex << v::setw(8) << v::setfill('0')
                     << datum << ", from:0x" << hex << v::setw(8) << v::setfill('0')
                     << address << endl;

            return datum;
        }

        // Write dword
        void write_dword( const unsigned int & address, 
			  sc_dt::uint64 datum,
			  const unsigned int asi,
			  const unsigned int flush,
			  const unsigned int lock) throw();

        // Write data word
        inline void write_word( const unsigned int & address, 
				unsigned int datum,
				const unsigned int asi,
				const unsigned int flush,
				const unsigned int lock) throw(){

            //Now the code for endianess conversion: the processor is always modeled
            //with the host endianess; in case they are different, the endianess
            //is turned
            #ifdef LITTLE_ENDIAN_BO
            this->swapEndianess(datum);
            #endif
            if(this->debugger != NULL){
                v::debug << name() << "Debugger" << endl;
                this->debugger->notifyAddress(address, sizeof(datum));
            }
            if(this->dmi_ptr_valid){
                v::debug << name() << "DMI Access" << endl;
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
                dcioExt->asi = asi;
                dcioExt->flush = flush;
                dcioExt->lock = lock;
                dcioExt->debug = &debug;

                trans.set_address(address);
                trans.set_write();
                trans.set_data_ptr((unsigned char*)&datum);
                trans.set_data_length(sizeof(datum));
                trans.set_byte_enable_ptr(0);
                trans.set_dmi_allowed(false);
                trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
                trans.set_extension(dcioExt);

                this->initSocket->b_transport(trans, delay);
                v::debug << name() << "Wrote word:0x" << hex << v::setw(8) << v::setfill('0')
                         << datum << ", at:0x" << hex << v::setw(8) << v::setfill('0')
                         << address << endl;

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
            }
        }

        // Write half-word
        void write_half( const unsigned int & address, 
			 unsigned short int datum,
			 unsigned int asi,
			 unsigned int flush,
			 unsigned int lock) throw();
        // Write byte
        void write_byte( const unsigned int & address, 
			 unsigned char datum,
			 unsigned int asi,
			 unsigned int flush,
			 unsigned int lock) throw();

        // Debug read/write prototypes 
        sc_dt::uint64 read_dword_dbg( const unsigned int & address ) throw();
        unsigned int read_word_dbg( const unsigned int & address ) throw();
        unsigned short int read_half_dbg( const unsigned int & address ) throw();
        unsigned char read_byte_dbg( const unsigned int & address ) throw();
        void write_dword_dbg( const unsigned int & address, sc_dt::uint64 datum ) throw();
        void write_word_dbg( const unsigned int & address, unsigned int datum ) throw();
        void write_half_dbg( const unsigned int & address, unsigned short int datum ) throw();
        void write_byte_dbg( const unsigned int & address, unsigned char datum ) throw();
        void lock();
        void unlock();
        tlm_utils::simple_initiator_socket< TLMMemory, 32 > initSocket;
    };
};



#endif
