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


#ifndef IRQPORTS_HPP
#define IRQPORTS_HPP

#include <systemc.h>
#include <tlm.h>
#include <trap_utils.hpp>
#include <tlm_utils/multi_passthrough_target_socket.h>

#define FUNC_MODEL
#define LT_IF
namespace leon3_funclt_trap{

    class IntrTLMPort_32 : public sc_module{

        public:
        IntrTLMPort_32( sc_module_name portName, unsigned int & irqSignal );
        void b_transport( int tag, tlm::tlm_generic_payload & trans, sc_time & delay );
        unsigned int transport_dbg( int tag, tlm::tlm_generic_payload & trans );
        tlm::tlm_sync_enum nb_transport_fw( int tag, tlm::tlm_generic_payload & trans, tlm::tlm_phase \
            & phase, sc_time & delay );
        tlm_utils::multi_passthrough_target_socket< IntrTLMPort_32, 32, tlm::tlm_base_protocol_types, \
            1, sc_core::SC_ZERO_OR_MORE_BOUND > socket;
        unsigned int & irqSignal;
    };

};



#endif
