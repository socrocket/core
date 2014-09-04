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


#ifndef IRQPORTS_HPP
#define IRQPORTS_HPP

#include <tlm.h>
#include "core/common/trapgen/utils/trap_utils.hpp"
#include <tlm_utils/multi_passthrough_target_socket.h>
#include "core/common/systemc.h"

#include "signalkit/signalkit.h"

#define FUNC_MODEL
#define AT_IF
namespace leon3_funcat_trap{

    class IntrTLMPort_32 : public signalkit::signal_module<IntrTLMPort_32>, public sc_module{

        public:
        IntrTLMPort_32( sc_module_name portName, unsigned int & irqSignal );

        void callbackMethod( const std::pair<unsigned int, bool>& value, const sc_time &delay );

        unsigned int & irqSignal;

        signal< std::pair<unsigned int, bool> >::in irq_signal;
    };

};

#endif
