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

#ifndef WATCHPOINTMANAGER_HPP
#define WATCHPOINTMANAGER_HPP

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
struct Watchpoint {
  enum Type {WRITE_watch, READ_watch, ACCESS_watch};
  AddressType address;
  unsigned int length;
  Type type;
};

template<class AddressType>
class WatchpointManager {
  private:
    template_map<AddressType, Watchpoint<AddressType> > watchpoints;
    typename template_map<AddressType, Watchpoint<AddressType> >::iterator lastWatch;
  public:
    WatchpointManager() {
      this->lastWatch = this->watchpoints.end();
    }
    // Eliminates all the breakpoints
    void clearAllWatchs() {
      this->watchpoints.clear();
      this->lastWatch = this->watchpoints.end();
    }
    bool addWatchpoint(typename Watchpoint<AddressType>::Type type, AddressType address, unsigned int length) {
      for (unsigned int i = 0; i < length; i++) {
        if (this->watchpoints.find(address + i) != this->lastWatch) {
          return false;
        }
      }
      for (unsigned int i = 0; i < length; i++) {
        this->watchpoints[address + i].address = address;
        this->watchpoints[address + i].length = length;
        this->watchpoints[address + i].type = type;
      }
      this->lastWatch = this->watchpoints.end();
      return true;
    }

    bool removeWatchpoint(AddressType address, unsigned int length) {
      for (unsigned int i = 0; i < length; i++) {
        if (this->watchpoints.find(address + i) == this->lastWatch) {
          return false;
        }
      }
      for (unsigned int i = 0; i < length; i++) {
        this->watchpoints.erase(address + i);
      }
      this->lastWatch = this->watchpoints.end();
      return true;
    }

    inline bool hasWatchpoint(AddressType address, unsigned int size) const throw() {
      for (unsigned int i = 0; i < size; i++) {
        if (this->watchpoints.find(address + i) != this->lastWatch) {
          return true;
        }
      }
      return false;
    }

    Watchpoint<AddressType>*getWatchPoint(AddressType address, unsigned int size) throw() {
      for (unsigned int i = 0; i < size; i++) {
        typename template_map<AddressType, Watchpoint<AddressType> >::iterator foundWatchPoint = this->watchpoints.find(
          address + i);
        if (foundWatchPoint != this->lastWatch) {
          return &(foundWatchPoint->second);
        }
      }
      return NULL;
    }

    template_map<AddressType, Watchpoint<AddressType> >&getWatchpoints() throw() {
      return this->watchpoints;
    }
};
}

#endif
