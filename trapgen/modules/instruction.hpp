/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     instruction.hpp
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

#ifndef TRAP_INSTRUCTIONBASE_HPP
#define TRAP_INSTRUCTIONBASE_HPP

#include <ostream>
#include <sstream>
#include <string>

namespace trap {

/**
 * @brief InstructionBase
 *
 * Base class for all instructions. Enables access to the instruction fields
 * from the tools.
 */
class InstructionBase {
  public:
  virtual std::string get_name() const throw() = 0;

  /// Returns the assembly code of the instruction.
  virtual std::string get_mnemonic() const throw() = 0;

  /// Gets the ID of the instruction assigned by the decoder.
  virtual unsigned get_id() const throw() = 0;
}; // class InstructionBase

/// ****************************************************************************

/**
 * @brief HistoryInstrType
 *
 * Type representing a single entry in the instruction history queue.
 */
struct HistoryInstrType {

  HistoryInstrType() : name("--"), mnemonic("--") {
    this->address = 0;
    this->cycle = 0;
  } // HistoryInstrType()

  /// ..........................................................................

  /// Creates a string representation of the current history element.
  std::string get_mnemonic() const {
    std::stringstream ss;
    ss << std::hex << std::showbase << this->address << "\t\t" << this->name;
    if (this->name.size() > 7) {
      ss << "\t\t";
    } else {
      ss << "\t\t\t";
    }
    ss << this->mnemonic;
    if (this->mnemonic.size() < 8) {
      ss << "\t\t\t";
    } else if (this->mnemonic.size() > 15) {
      ss << "\t";
    } else {
      ss << "\t\t";
    }
    ss << std::dec << this->cycle;
    return ss.str();
  } // get_mnemonic()

  /// ..........................................................................

  /// Prints the string representation of the current instruction.
  std::ostream& operator<<(std::ostream& os) const {
    os << this->get_mnemonic();
    return os;
  } // operator<<()

  std::string name;
  std::string mnemonic;
  unsigned address;
  unsigned cycle;
}; // struct HistoryInstrType

} // namespace trap

/// ****************************************************************************
#endif
