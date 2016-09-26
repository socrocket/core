/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     osemu_base.hpp
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

#ifndef TRAP_OSEMU_BASE_H
#define TRAP_OSEMU_BASE_H

#include <map>
#include <set>
#include <vector>
#include <string>

namespace trap {

/**
 * @brief OSEmulatorBase
 */
class OSEmulatorBase {
  public:
  void set_env(const std::string name, const std::string value);
  void set_sysconf(const std::string name, int value);
  void set_program_args(const std::vector<std::string> args);
  void correct_flags(int& val);
  void reset();

  public:
  std::map<std::string, std::string> env;
  std::map<std::string, int> sysconf;
  std::vector<std::string> program_args;
  static unsigned num_programs;
  static std::vector<unsigned> group_ids;
  unsigned heap_ptr;
}; // class OSEmulatorBase

} // namespace trap

/// ****************************************************************************
#endif // TRAP_OSEMU_BASE_H
