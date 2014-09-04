/***************************************************************************\
*
*   This file is part of TRAP.
*
*   TRAP is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 3 of the License, or
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
*   (c) Luca Fossati, fossati@elet.polimi.it, fossati.l@gmail.com
*
\ ***************************************************************************/

#ifndef INSTRUCTIONBASE_HPP
#define INSTRUCTIONBASE_HPP

#include <ostream>
#include <sstream>
#include <string>

#include <boost/lexical_cast.hpp>

namespace trap {
/// Base class for all instructions; it enables access to the instruction
/// fields from the tools
class InstructionBase {
  public:
    ///Returns the instruction name
    virtual std::string getInstructionName() const throw() = 0;
    ///Returns the instruction mnemonic, so how the current
    ///instruction translated to assebmly code
    virtual std::string getMnemonic() const throw() = 0;
    ///Gets the ID of the instruction as returned by the decoder
    virtual unsigned int getId() const throw() = 0;
};

///Type representing a single entry in the instruction history queue
struct HistoryInstrType {
  unsigned int address;
  std::string name;
  std::string mnemonic;
  unsigned int cycle;

  HistoryInstrType() : name("--"), mnemonic("--") {
    this->address = 0;
    this->cycle = 0;
  }

  ///Creates a string representation of the current history element
  std::string toStr() const {
    std::stringstream outStr;
    outStr << std::hex << std::showbase << this->address << "\t\t" << this->name;
    if (this->name.size() > 7) {
      outStr << "\t\t";
    } else {
      outStr << "\t\t\t";
    }
    outStr << this->mnemonic;
    if (this->mnemonic.size() < 8) {
      outStr << "\t\t\t";
    } else if (this->mnemonic.size() > 15) {
      outStr << "\t";
    } else {
      outStr << "\t\t";
    }
    outStr << std::dec << this->cycle;
    return outStr.str();
  }

  ///Prints the string representation of the current instruction on a stream
  std::ostream &operator<<(std::ostream &other) const {
    other << this->toStr();
    return other;
  }
};
}

#endif
