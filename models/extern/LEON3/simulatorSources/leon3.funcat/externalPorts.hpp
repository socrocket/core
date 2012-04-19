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


#ifndef EXTERNALPORTS_HPP
#define EXTERNALPORTS_HPP

#include <leon3.funcat/memory.hpp>
#include <systemc.h>
#include <ToolsIf.hpp>
#include <tlm.h>
#include <trap_utils.hpp>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/peq_with_cb_and_phase.h>
#include "icio_payload_extension.h"
#include "dcio_payload_extension.h"
#include "verbose.h"

#define FUNC_MODEL
#define AT_IF
namespace leon3_funcat_trap{

    class TLMMemory : public MemoryInterface, public sc_module{
        private:
        MemoryToolsIf< unsigned int > * debugger;
        tlm::tlm_generic_payload * request_in_progress;
        sc_event end_request_event;
        sc_event end_response_event;
        tlm_utils::peq_with_cb_and_phase< TLMMemory > m_peq;

        public:
        TLMMemory( sc_module_name portName );
        void setDebugger( MemoryToolsIf< unsigned int > * debugger );
        inline tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload & trans, tlm::tlm_phase \
            & phase, sc_time & delay ) throw(){
            // TLM-2 backward non-blocking transport method
            // The timing annotation must be honored
            m_peq.notify(trans, phase, delay);
            return tlm::TLM_ACCEPTED;
        }
        void peq_cb( tlm::tlm_generic_payload & trans, const tlm::tlm_phase & phase );

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

            tlm::tlm_generic_payload trans;

	    // Create & init data payload extension
            dcio_payload_extension* dcioExt = new dcio_payload_extension();
            dcioExt->asi    = asi;
	    dcioExt->flush  = flush;
	    dcioExt->lock   = lock;

            unsigned int* debug = new unsigned int;
            dcioExt->debug = debug;
	    
            trans.set_address(address);
            trans.set_read();
            trans.set_data_ptr(reinterpret_cast<unsigned char*>(&datum));
            trans.set_data_length(sizeof(datum));
            trans.set_streaming_width(sizeof(datum));
            trans.set_byte_enable_ptr(0);
            trans.set_dmi_allowed(false);
            trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	    // Hook extension onto payload
            trans.set_extension(dcioExt);

            if(this->request_in_progress != NULL){
                wait(this->end_request_event);
            }
            request_in_progress = &trans;

            // Non-blocking transport call on the forward path
            sc_time delay = SC_ZERO_TIME;
            tlm::tlm_phase phase = tlm::BEGIN_REQ;
            tlm::tlm_sync_enum status;
            status = initSocket->nb_transport_fw(trans, phase, delay);

            // Check value returned from nb_transport_fw
            if(status == tlm::TLM_UPDATED){
                // The timing annotation must be honored
                m_peq.notify(trans, phase, delay);
                wait(this->end_request_event);
            }
            else if(status == tlm::TLM_COMPLETED){
                // The completion of the transaction necessarily ends the BEGIN_REQ phase
                this->request_in_progress = NULL;
                // The target has terminated the transaction, I check the correctness
                if(trans.is_response_error()){
                    SC_REPORT_ERROR("TLM-2", ("Transaction returned with error, response status = " + \
                        trans.get_response_string()).c_str());
                }
            }
            wait(this->end_response_event);
            //Now the code for endianess conversion: the processor is always modeled
            //with the host endianess; in case they are different, the endianess
            //is turned

            if(trans.is_response_error()){
                std::string errorStr("Error from nb_transport_fw, response status = " + trans.get_response_string());
                SC_REPORT_ERROR("TLM-2", errorStr.c_str());
            }

            #ifdef LITTLE_ENDIAN_BO
            this->swapEndianess(datum);
            #endif

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

            tlm::tlm_generic_payload trans;

	    // Create & init instruction payload extension
            icio_payload_extension* icioExt = new icio_payload_extension();
	    icioExt->flush  = flush;

            unsigned int* debug = new unsigned int;
            icioExt->debug = debug;	    
	    
            trans.set_address(address);
            trans.set_read();
            trans.set_data_ptr(reinterpret_cast<unsigned char*>(&datum));
            trans.set_data_length(sizeof(datum));
            trans.set_streaming_width(sizeof(datum));
            trans.set_byte_enable_ptr(0);
            trans.set_dmi_allowed(false);
            trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	    // Hook extension onto payload
            trans.set_extension(icioExt);

            if(this->request_in_progress != NULL){
                wait(this->end_request_event);
            }
            request_in_progress = &trans;

            // Non-blocking transport call on the forward path
            sc_time delay = SC_ZERO_TIME;
            tlm::tlm_phase phase = tlm::BEGIN_REQ;
            tlm::tlm_sync_enum status;
            status = initSocket->nb_transport_fw(trans, phase, delay);

            // Check value returned from nb_transport_fw
            if(status == tlm::TLM_UPDATED){
                // The timing annotation must be honored
                m_peq.notify(trans, phase, delay);
                wait(this->end_request_event);
            }
            else if(status == tlm::TLM_COMPLETED){
                // The completion of the transaction necessarily ends the BEGIN_REQ phase
                this->request_in_progress = NULL;
                // The target has terminated the transaction, I check the correctness
                if(trans.is_response_error()){
                    SC_REPORT_ERROR("TLM-2", ("Transaction returned with error, response status = " + \
                        trans.get_response_string()).c_str());
                }
            }

	    wait(this->end_response_event);

            if(trans.is_response_error()){
                std::string errorStr("Error from nb_transport_fw, response status = " + trans.get_response_string());
                SC_REPORT_ERROR("TLM-2", errorStr.c_str());
            }

            //Now the code for endianess conversion: the processor is always modeled
            //with the host endianess; in case they are different, the endianess
            //is turned
            #ifdef LITTLE_ENDIAN_BO
            this->swapEndianess(datum);
            #endif

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
                this->debugger->notifyAddress(address, sizeof(datum));
            }
            tlm::tlm_generic_payload trans;

	    // Create & init data payload extension
	    dcio_payload_extension* dcioExt = new dcio_payload_extension();
	    dcioExt->asi   = asi;
	    dcioExt->flush = flush;
	    dcioExt->lock  = lock;

	    unsigned int* debug = new unsigned int;
	    dcioExt->debug = debug;	    
	    
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

            if(this->request_in_progress != NULL){
                wait(this->end_request_event);
            }
            request_in_progress = &trans;

            // Non-blocking transport call on the forward path
            sc_time delay = SC_ZERO_TIME;
            tlm::tlm_phase phase = tlm::BEGIN_REQ;
            tlm::tlm_sync_enum status;
            status = initSocket->nb_transport_fw(trans, phase, delay);

            // Check value returned from nb_transport_fw
            if(status == tlm::TLM_UPDATED){
                // The timing annotation must be honored
                m_peq.notify(trans, phase, delay);
                wait(this->end_request_event);
            }
            else if(status == tlm::TLM_COMPLETED){
                // The completion of the transaction necessarily ends the BEGIN_REQ phase
                this->request_in_progress = NULL;
                // The target has terminated the transaction, I check the correctness
                if(trans.is_response_error()){
                    SC_REPORT_ERROR("TLM-2", ("Transaction returned with error, response status = " + \
                        trans.get_response_string()).c_str());
                }
            }
            wait(this->end_response_event);

            if(trans.is_response_error()){
                std::string errorStr("Error from nb_transport_fw, response status = " + trans.get_response_string());
                SC_REPORT_ERROR("TLM-2", errorStr.c_str());
            }

        }
      
        // Write half-word
        void write_half( const unsigned int & address, 
			 unsigned short int datum,
			 unsigned int asi,
			 unsigned int flush,
			 unsigned int lock) throw();

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
