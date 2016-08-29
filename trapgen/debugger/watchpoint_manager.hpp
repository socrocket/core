/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     watchpoint_manager.hpp
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
#ifndef TRAP_WATCHPOINTMANAGER_HPP
#define TRAP_WATCHPOINTMANAGER_HPP

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
#else
#ifdef _WIN32
#include <hash_map>
#define  template_map stdext::hash_map
#else
#include <map>
#define  template_map std::map
#endif
#endif

#include <iostream>
#include <string>
#include <vector>

namespace trap {

/**
 * @brief Watchpoint
 */
template<class AddressType>
struct Watchpoint {
  enum Type {WRITE_WATCH, READ_WATCH, ACCESS_WATCH};
  AddressType address;
  unsigned length;
  Type type;
}; // struct Watchpoint

/// ****************************************************************************

/**
 * @brief WatchpointManager
 */
template<class AddressType>
class WatchpointManager {
  public:
  WatchpointManager() {
    this->last_watch = this->watchpoints.end();
  }

  bool add_watchpoint(typename Watchpoint<AddressType>::Type type, AddressType address, unsigned length) {
    for (unsigned i = 0; i < length; i++) {
      if (this->watchpoints.find(address + i) != this->last_watch) {
        return false;
      }
    }
    for (unsigned i = 0; i < length; i++) {
      this->watchpoints[address + i].address = address;
      this->watchpoints[address + i].length = length;
      this->watchpoints[address + i].type = type;
    }
    this->last_watch = this->watchpoints.end();
    return true;
  }

  bool remove_watchpoint(AddressType address, unsigned length) {
    for (unsigned i = 0; i < length; i++) {
      if (this->watchpoints.find(address + i) == this->last_watch) {
        return false;
      }
    }
    for (unsigned i = 0; i < length; i++) {
      this->watchpoints.erase(address + i);
    }
    this->last_watch = this->watchpoints.end();
    return true;
  }

  void clear_all_watchpoints() {
    this->watchpoints.clear();
    this->last_watch = this->watchpoints.end();
  }

  inline bool has_watchpoint(AddressType address, unsigned size) const throw() {
    for (unsigned i = 0; i < size; i++) {
      if (this->watchpoints.find(address + i) != this->last_watch) {
        return true;
      }
    }
    return false;
  }

  Watchpoint<AddressType>* get_watchpoint(AddressType address, unsigned size) throw() {
    for (unsigned i = 0; i < size; i++) {
      typename template_map<AddressType, Watchpoint<AddressType> >::iterator watchpoint_it = this->watchpoints.find(address + i);
      if (watchpoint_it != this->last_watch)
          return &(watchpoint_it->second);
    }
    return NULL;
  }

  template_map<AddressType, Watchpoint<AddressType> >& get_watchpoints() throw() {
    return this->watchpoints;
  }

  private:
  template_map<AddressType, Watchpoint<AddressType> > watchpoints;
  typename template_map<AddressType, Watchpoint<AddressType> >::iterator last_watch;
}; // class WatchpointManager

} // namespace trap

/// ****************************************************************************
#endif
