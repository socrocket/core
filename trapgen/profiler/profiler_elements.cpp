/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     profiler_elements.cpp
* @brief    This file is part of the TRAP runtime library.
* @details
* @author   Luca Fossati
* @author   Lillian Tadros (Technische Universitaet Dortmund)
* @date     2008-2013 Luca Fossati
*           2015-2016 Technische Universitaet Dortmund
* @copyright
*
* This file is part of TRAP.
*
* TRAP is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
* or see <http://www.gnu.org/licenses/>.
*
* (c) Luca Fossati, fossati@elet.polimi.it, fossati.l@gmail.com
*
*******************************************************************************/
#include "profiler_elements.hpp"

#include <systemc.h>

#include <boost/lexical_cast.hpp>

#include <string>

/// Total number of instructions executed.
unsigned long long trap::ProfInstruction::num_total_calls = 0;

/// Empty constructor. Performs the initialization of the statistics.
trap::ProfInstruction::ProfInstruction() {
  this->num_calls = 1;
  this->time = SC_ZERO_TIME;
}

/// Dumps this information to a string in a CSV format.
std::string trap::ProfInstruction::print_csv() {
  double instr_time = (this->time.to_default_time_units())/(sc_time(1, SC_NS).to_default_time_units());
  std::string csv_line(this->name + ";");
  csv_line += boost::lexical_cast<std::string>(this->num_calls) + ";";
  double perc_calls = ((double)this->num_calls*100)/ProfInstruction::num_total_calls;
  if (perc_calls < 10e-3)
    perc_calls = 0;
  csv_line += boost::lexical_cast<std::string>(perc_calls) + ";";
  csv_line += boost::lexical_cast<std::string>(instr_time) + ";";
  csv_line += boost::lexical_cast<std::string>(instr_time/this->num_calls);
  return csv_line;
} // ProfInstruction::print_csv()

/// --------------------------------------------------------------------------

/// Prints the information describing an instruction in a CSV format.
std::string trap::ProfInstruction::print_csv_header() {
  return "name;num_calls;num_calls %;time;Time per call";
} // ProfInstruction::print_csv_header()

/// --------------------------------------------------------------------------

/// Prints the summary of all executed instructions in a CSV format.
std::string trap::ProfInstruction::print_csv_summary() {
  return "Total calls;;" + boost::lexical_cast<std::string>(ProfInstruction::num_total_calls);
} // ProfInstruction::print_csv_summary()

/// ****************************************************************************

/// Total number of function calls.
unsigned long long trap::ProfFunction::num_total_calls = 0;

/// Empty constructor. Performs the initialization of the statistics.
trap::ProfFunction::ProfFunction() {
  this->num_calls = 1;
  this->total_time = SC_ZERO_TIME;
  this->excl_time = SC_ZERO_TIME;
  this->num_total_instr = 0;
  this->num_excl_instr = 0;
  this->already_examined = false;
} // ProfFunction::ProfFunction()

/// --------------------------------------------------------------------------

/// Dumps this information to a string in a CSV format.
std::string trap::ProfFunction::print_csv() {
  double func_total_time = (this->total_time.to_default_time_units())/(sc_time(1, SC_NS).to_default_time_units());
  double func_excl_time = (this->excl_time.to_default_time_units())/(sc_time(1, SC_NS).to_default_time_units());
  std::string csv_line(this->name + ";");
  csv_line += boost::lexical_cast<std::string>(this->num_calls) + ";";
  double perc_calls = ((double)this->num_calls*100)/ProfFunction::num_total_calls;
  if (perc_calls < 10e-3)
    perc_calls = 0;
  csv_line += boost::lexical_cast<std::string>(perc_calls) + ";";
  csv_line += boost::lexical_cast<std::string>(this->num_total_instr) + ";";
  csv_line += boost::lexical_cast<std::string>(this->num_excl_instr) + ";";
  csv_line += boost::lexical_cast<std::string>(((double)this->num_excl_instr)/this->num_calls) + ";";
  csv_line += boost::lexical_cast<std::string>(func_total_time) + ";";
  csv_line += boost::lexical_cast<std::string>(func_excl_time) + ";";
  csv_line += boost::lexical_cast<std::string>(func_excl_time/this->num_calls);
  return csv_line;
} // ProfFunction::print_csv()

/// --------------------------------------------------------------------------

/// Prints the information describing a function in a CSV format.
std::string trap::ProfFunction::print_csv_header() {
  return "Name;Num Calls;Num Calls %;Num Total Instr;Num Excl Instr;Num Instr per Call;Total Time;Excl Time;Time per call";
} // ProfFunction::print_csv_header()

/// ****************************************************************************
