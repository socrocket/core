/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     breakpoint_manager.hpp
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
#ifndef TRAP_BREAKPOINTMANAGER_HPP
#define TRAP_BREAKPOINTMANAGER_HPP

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
 * @brief Breakpoint
 */
template<class AddressType>
struct Breakpoint {
  enum Type {MEM_BREAK = 0, HW_BREAK};
  AddressType address;
  unsigned length;
  Type type;
}; // struct Breakpoint

/// ****************************************************************************

/**
 * @brief BreakpointManager
 */
template<class AddressType>
class BreakpointManager {
  public:
  BreakpointManager() {
    this->last_break = this->breakpoints.end();
  }

  bool add_breakpoint(typename Breakpoint<AddressType>::Type type, AddressType address, unsigned length) {
    if (this->breakpoints.find(address) != this->last_break) {
      return false;
    }
    this->breakpoints[address].address = address;
    this->breakpoints[address].length = length;
    this->breakpoints[address].type = type;
    this->last_break = this->breakpoints.end();
    return true;
  }

  bool remove_breakpoint(AddressType address) {
    if (this->breakpoints.find(address) == this->last_break) {
      return false;
    }
    this->breakpoints.erase(address);
    this->last_break = this->breakpoints.end();
    return true;
  }

  void clear_all_breaks() {
    this->breakpoints.clear();
    this->last_break = this->breakpoints.end();
  }

  inline bool has_breakpoint(AddressType address) const throw() {
    return this->breakpoints.find(address) != this->last_break;
  }

  Breakpoint<AddressType>* get_breakpoint(AddressType address) throw() {
    if (this->breakpoints.find(address) == this->last_break) {
      return NULL;
    } else {
      return &(this->breakpoints[address]);
    }
  }

  template_map<AddressType, Breakpoint<AddressType> >& get_breakpoints() throw() {
    return this->breakpoints;
  }

  private:
  template_map<AddressType, Breakpoint<AddressType> > breakpoints;
  typename template_map<AddressType, Breakpoint<AddressType> >::iterator last_break;
}; // class BreakpointManager

} // namespace trap

/// ****************************************************************************
#endif
