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



#include <irqPorts.hpp>
#include <systemc.h>
#include <tlm.h>
#include <trap_utils.hpp>
#include <tlm_utils/multi_passthrough_target_socket.h>

using namespace leon3_funclt_trap;
void leon3_funclt_trap::IntrTLMPort_32::b_transport( int tag, tlm::tlm_generic_payload \
    & trans, sc_time & delay ){
    unsigned char* ptr = trans.get_data_ptr();
    sc_dt::uint64 adr = trans.get_address();
    if(*ptr == 0){
        //Lower the interrupt
        this->irqSignal = -1;
    }
    else{
        //Raise the interrupt
        this->irqSignal = adr;
    }
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
}

unsigned int leon3_funclt_trap::IntrTLMPort_32::transport_dbg( int tag, tlm::tlm_generic_payload \
    & trans ){
    unsigned char* ptr = trans.get_data_ptr();
    sc_dt::uint64 adr = trans.get_address();
    if(*ptr == 0){
        //Lower the interrupt
        this->irqSignal = -1;
    }
    else{
        //Raise the interrupt
        this->irqSignal = adr;
    }
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
    return trans.get_data_length();
}

tlm::tlm_sync_enum leon3_funclt_trap::IntrTLMPort_32::nb_transport_fw( int tag, tlm::tlm_generic_payload \
    & trans, tlm::tlm_phase & phase, sc_time & delay ){
    THROW_EXCEPTION("Method not yet implemented");
}

leon3_funclt_trap::IntrTLMPort_32::IntrTLMPort_32( sc_module_name portName, unsigned \
    int & irqSignal ) : sc_module(portName), irqSignal(irqSignal), socket(portName){
    this->socket.register_b_transport(this, &IntrTLMPort_32::b_transport);
    this->socket.register_transport_dbg(this, &IntrTLMPort_32::transport_dbg);
    this->socket.register_nb_transport_fw(this, &IntrTLMPort_32::nb_transport_fw);
    end_module();
}


