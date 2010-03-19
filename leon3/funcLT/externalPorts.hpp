/***************************************************************************\
 *
 *   
 *            ___        ___           ___           ___
 *           /  /\      /  /\         /  /\         /  /\
 *          /  /:/     /  /::\       /  /::\       /  /::\
 *         /  /:/     /  /:/\:\     /  /:/\:\     /  /:/\:\
 *        /  /:/     /  /:/~/:/    /  /:/~/::\   /  /:/~/:/
 *       /  /::\    /__/:/ /:/___ /__/:/ /:/\:\ /__/:/ /:/
 *      /__/:/\:\   \  \:\/:::::/ \  \:\/:/__\/ \  \:\/:/
 *      \__\/  \:\   \  \::/~~~~   \  \::/       \  \::/
 *           \  \:\   \  \:\        \  \:\        \  \:\
 *            \  \ \   \  \:\        \  \:\        \  \:\
 *             \__\/    \__\/         \__\/         \__\/
 *   
 *
 *
 *   
 *   This file is part of TRAP.
 *   
 *   TRAP is free software; you can redistribute it and/or modify
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
 *   (c) Luca Fossati, fossati@elet.polimi.it
 *
\***************************************************************************/


#ifndef EXTERNALPORTS_HPP
#define EXTERNALPORTS_HPP

#include <memory.hpp>
#include <systemc.h>
#include <ToolsIf.hpp>
#include <trap_utils.hpp>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/tlm_quantumkeeper.h>

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
        sc_dt::uint64 read_dword( const unsigned int & address ) throw();
        inline unsigned int read_word( const unsigned int & address ) throw(){
            unsigned int datum = 0;
            if (this->dmi_ptr_valid){
                if(address + this->dmi_data.get_start_address() > this->dmi_data.get_end_address()){
                    SC_REPORT_ERROR("TLM-2", "Error in reading memory data through DMI: address out of \
                        bounds");
                }
                memcpy(&datum, this->dmi_data.get_dmi_ptr() - this->dmi_data.get_start_address() \
                    + address, sizeof(datum));
                this->quantKeeper.inc(this->dmi_data.get_read_latency());
            }
            else{
                sc_time delay = this->quantKeeper.get_local_time();
                tlm::tlm_generic_payload trans;
                trans.set_address(address);
                trans.set_read();
                trans.set_data_ptr(reinterpret_cast<unsigned char*>(&datum));
                trans.set_data_length(sizeof(datum));
                trans.set_byte_enable_ptr(0);
                trans.set_dmi_allowed(false);
                trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
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
            }
            //Now the code for endianess conversion: the processor is always modeled
            //with the host endianess; in case they are different, the endianess
            //is turned
            #ifdef LITTLE_ENDIAN_BO
            this->swapEndianess(datum);
            #endif

            return datum;
        }
        unsigned short int read_half( const unsigned int & address ) throw();
        unsigned char read_byte( const unsigned int & address ) throw();
        void write_dword( const unsigned int & address, sc_dt::uint64 datum ) throw();
        inline void write_word( const unsigned int & address, unsigned int datum ) throw(){
            //Now the code for endianess conversion: the processor is always modeled
            //with the host endianess; in case they are different, the endianess
            //is turned
            #ifdef LITTLE_ENDIAN_BO
            this->swapEndianess(datum);
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
            }
            else{
                sc_time delay = this->quantKeeper.get_local_time();
                tlm::tlm_generic_payload trans;
                trans.set_address(address);
                trans.set_write();
                trans.set_data_ptr((unsigned char*)&datum);
                trans.set_data_length(sizeof(datum));
                trans.set_byte_enable_ptr(0);
                trans.set_dmi_allowed(false);
                trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
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
            }
        }
        void write_half( const unsigned int & address, unsigned short int datum ) throw();
        void write_byte( const unsigned int & address, unsigned char datum ) throw();
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
