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


#ifndef LT_EXTERNALPINS_HPP
#define LT_EXTERNALPINS_HPP

#include "core/common/trapgen/utils/trap_utils.hpp"
#include "core/common/systemc.h"

#include "core/common/sr_signal.h"

#define FUNC_MODEL
#define LT_IF
namespace leon3_funclt_trap{
    class PinTLM_out_32 : public sc_module {
      public:
        SR_HAS_SIGNALS(PinTLM_out_32);
        PinTLM_out_32( sc_module_name portName );
        void send_pin_req( const unsigned int & value ) throw();
        void on_run(const bool &value, const sc_time &delay) throw();

        signal< unsigned int >::out initSignal;

        /// If the processor is stoped status will be true
        /// If it is running it will be false.
        signal<bool>::out           status;

        /// Will prepare the start of the processor.
        /// If set to 1 stoped will be false and the start notification will be sent.
        signal<bool>::in            run;

        /// Needed to halt the main processor loop
        bool stopped;

        /// Needed to start the main processor loop
        sc_event start;
    };
};



#endif
