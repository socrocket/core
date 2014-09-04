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

#ifndef BREAKPOINTMANAGER_HPP
#define BREAKPOINTMANAGER_HPP

#include <iostream>
#include <string>
#include <vector>

#ifdef __GNUC__
#ifdef __GNUC_MINOR__
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 3)
#include <tr1/unordered_map>
#define template_map std::tr1::unordered_map
#else
#include <ext/hash_map>
#define  template_map __gnu_cxx::hash_map
#endif
#else
#include <ext/hash_map>
#define  template_map __gnu_cxx::hash_map
#endif
#else // ifdef __GNUC__
#ifdef _WIN32
#include <hash_map>
#define  template_map stdext::hash_map
#else
#include <map>
#define  template_map std::map
#endif
#endif

namespace trap {
template<class AddressType>
struct Breakpoint {
  enum Type {MEM_break = 0, HW_break};
  AddressType address;
  unsigned int length;
  Type type;
};

template<class AddressType>
class BreakpointManager {
  private:
    template_map<AddressType, Breakpoint<AddressType> > breakpoints;
    typename template_map<AddressType, Breakpoint<AddressType> >::iterator lastBreak;
  public:
    BreakpointManager() {
      this->lastBreak = this->breakpoints.end();
    }

    // Eliminates all the breakpoints
    void clearAllBreaks() {
      this->breakpoints.clear();
      this->lastBreak = this->breakpoints.end();
    }

    bool addBreakpoint(typename Breakpoint<AddressType>::Type type, AddressType address, unsigned int length) {
      if (this->breakpoints.find(address) != this->lastBreak) {
        return false;
      }
      this->breakpoints[address].address = address;
      this->breakpoints[address].length = length;
      this->breakpoints[address].type = type;
      this->lastBreak = this->breakpoints.end();
      return true;
    }

    bool removeBreakpoint(AddressType address) {
      if (this->breakpoints.find(address) == this->lastBreak) {
        return false;
      }
      this->breakpoints.erase(address);
      this->lastBreak = this->breakpoints.end();
      return true;
    }

    inline bool hasBreakpoint(AddressType address) const throw() {
      return this->breakpoints.find(address) != this->lastBreak;
    }

    Breakpoint<AddressType>*getBreakPoint(AddressType address) throw() {
      if (this->breakpoints.find(address) == this->lastBreak) {
        return NULL;
      } else {
        return &(this->breakpoints[address]);
      }
    }

    template_map<AddressType, Breakpoint<AddressType> >&getBreakpoints() throw() {
      return this->breakpoints;
    }
};
}

#endif
