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



#include "core/common/systemc.h"
#include "gaisler/leon3/intunit/irqPorts.hpp"
#include "core/common/trapgen/utils/trap_utils.hpp"

#include "core/common/sr_signal.h"
#include "core/common/verbose.h"


using namespace leon3_funclt_trap;

void leon3_funclt_trap::IntrTLMPort_32::callbackMethod( const std::pair<unsigned int, bool>& value, const sc_time & delay ){
    if(!value.second){
        //Lower the interrupt
        this->irqSignal = -1;
    }
    else{
        //Raise the interrupt
        this->irqSignal = value.first;
        v::debug << name() << "InterruptIN " << value.first << v::endl;
    }
}

leon3_funclt_trap::IntrTLMPort_32::IntrTLMPort_32( sc_module_name portName, unsigned \
    int & irqSignal ) : sc_module(portName), irqSignal(irqSignal),
    irq_signal(&IntrTLMPort_32::callbackMethod, "irq"){
    end_module();
}
