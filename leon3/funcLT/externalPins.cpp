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



#include <externalPins.hpp>
#include <systemc.h>
#include <trap_utils.hpp>
#include <tlm.h>
#include <tlm_utils/multi_passthrough_initiator_socket.h>

using namespace leon3_funclt_trap;
void leon3_funclt_trap::PinTLM_out_32::send_pin_req( const unsigned int & address, \
    unsigned int datum ) throw(){
    tlm::tlm_generic_payload trans;
    sc_time delay;
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
}

leon3_funclt_trap::PinTLM_out_32::PinTLM_out_32( sc_module_name portName ) : sc_module(portName), \
    initSocket(sc_gen_unique_name(portName)){
    end_module();
}


