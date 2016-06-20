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

#include "gaisler/leon3/intunit/externalPins.hpp"
#include "core/common/trapgen/utils/trap_utils.hpp"
#include "core/common/sr_report.h"
#include "core/common/verbose.h"

using namespace leon3_funclt_trap;

void leon3_funclt_trap::PinTLM_out_32::on_run(const bool &run, const sc_time &delay) throw() {
  if(!run) {
      stopped = true;
      status =  false;
  } else {
      stopped = false;
      start.notify();
      status = true;
  }
  srInfo()
    ("run", run)
    ("stopped", stopped)
    ("status", status)
    ("Receiving run event");
}



void leon3_funclt_trap::PinTLM_out_32::send_pin_req(const unsigned int &value) throw() {
  initSignal = value;
  v::debug << name() << "InterruptACK " << value << v::endl;
}

leon3_funclt_trap::PinTLM_out_32::PinTLM_out_32(sc_module_name portName) : sc_module(portName),
  // In stand-alone mode do not wait for run-bit to be set
  #ifdef LEON3_STANDALONE
    initSignal("ack"), status("status"), run(&leon3_funclt_trap::PinTLM_out_32::on_run, "run"), stopped(false) {
    status.write(true);
  #else
    initSignal("ack"), status("status"), run(&leon3_funclt_trap::PinTLM_out_32::on_run, "run"), stopped(true) {
    status.write(false);
  #endif
  end_module();
}
