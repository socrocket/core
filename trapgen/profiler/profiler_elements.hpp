/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     profiler_elements.hpp
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

#ifndef TRAP_PROFILER_ELEMENTS_H
#define TRAP_PROFILER_ELEMENTS_H

#include <systemc.h>

#include <string>

namespace trap {

/**
 * @brief ProfInstruction
 *
 * Represents all the profiling data which can be associated with a single
 * assembly instruction.
 */
struct ProfInstruction {
  /// Empty constructor. Performs the initialization of the statistics.
  ProfInstruction();

  /// Name of the assembly instruction (MOV, ADD ...).
  std::string name;
  /// Number of times this instruction is called.
  unsigned long long num_calls;
  /// Total number of instructions executed.
  static unsigned long long num_total_calls;
  /// Total time spent in executing the instruction.
  sc_time time;
  /// Dumps this information to a string in a CSV format.
  std::string print_csv();
  /// Prints the information describing an instruction in a CSV format.
  static std::string print_csv_header();
  /// Prints the summary of all executed instructions in a CSV format.
  static std::string print_csv_summary();
}; // struct ProfInstruction

/// ****************************************************************************

/**
 * @brief ProfFunction
 *
 * Represents all the profiling data which can be associated with a single
 * function.
 */
struct ProfFunction {
  /// Empty constructor. Performs the initialization of the statistics.
  ProfFunction();

  /// Name of the function.
  std::string name;
  /// Address of the function.
  unsigned address;
  /// Number of times this function is called.
  unsigned long long num_calls;
  /// Total number of function calls.
  static unsigned long long num_total_calls;
  /// Total number of assembly instructions executed inside the function.
  unsigned long long num_total_instr;
  /// Number of assembly instructions executed exclusively inside the function.
  unsigned long long num_excl_instr;
  /// Total time spent in the function.
  sc_time total_time;
  /// Time spent exclusively in the function.
  sc_time excl_time;
  /// Used for keeping track of the increment of time, instruction count, etc.
  /// in recursive functions.
  bool already_examined;
  /// Dumps this information to a string in a CSV format.
  std::string print_csv();
  /// Prints the information describing a function in a CSV format.
  static std::string print_csv_header();
}; // struct ProfFunction

} // namespace trap

/// ****************************************************************************
#endif // TRAP_PROFILER_ELEMENTS_H
