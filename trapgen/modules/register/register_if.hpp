/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     register_if.hpp
* @brief    This file is part of the TRAP runtime library.
* @details  Provides a unified interface for register fields, registers,
*           register banks and register aliases. Also provides an iterator
*           class, interpreted as a bit iterator by fields, a field iterator by
*           registers and aliases, and a register iterator by register banks.
* @author   Lillian Tadros (Technische Universitaet Dortmund)
* @date     2016 Technische Universitaet Dortmund
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
*******************************************************************************/

#ifndef TRAP_REGISTER_IF_H
#define TRAP_REGISTER_IF_H

#include "register_abstraction.hpp"
#include "scireg.h"

#include <assert.h>

namespace trap {

/// Forward declarations

template <typename DATATYPE, typename CHILDTYPE>
class RegisterInterface;

/// ****************************************************************************

/**
* @brief RegisterIterator
*
* Provides basic sequential access with bounds-checking.
* Relies on RegisterInterface::operator[] for accessing children.
*
* @param CHILDTYPE Enables specifying both RegisterInterfaces as well as bits
* as children: RegisterField -> bool; Register/Alias -> RegisterField;
* RegisterBank -> Register.
*/
template <typename DATATYPE, typename CHILDTYPE>
class RegisterIterator {
  /// @name Types
  /// @{

  public:
  typedef RegisterInterface<DATATYPE, CHILDTYPE> container_type;
  typedef CHILDTYPE value_type;
  typedef value_type* pointer;
  typedef value_type& reference;
  typedef unsigned size_type;
  typedef std::size_t difference_type;
  typedef std::random_access_iterator_tag iterator_category;

  /// @} Types
  /// --------------------------------------------------------------------------
  /// @name Constructors and Destructors
  /// @{

  public:
  //RegisterIterator() : m_container (NULL), m_index(0) {}

  //RegisterIterator(const RegisterIterator& it) : m_container(it.m_container), m_index(it.m_index) {}

  RegisterIterator(container_type* container, size_type index) : m_container (container), m_index(index) {}

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Access and Traversal Methods
  /// @{

  public:
  value_type& operator*() const {
    assert(this->m_container && this->m_index < this->m_container.size());
    return *this->m_container[this->m_index];
  }

  value_type* operator->() const {
    assert(this->m_container && this->m_index < this->m_container.size());
    return &this->m_container[this->m_index];
  }

  RegisterIterator& operator++() {
    // NOTE: An index equal to m_container.size() is allowed and interpreted as
    // the end() iterator. It is neither dereferenceable nor incrementable.
    assert(this->m_container && this->m_index < this->m_container.size());
    this->m_index++;
    return *this;
  }

  RegisterIterator operator++(int) {
    RegisterIterator ret = *this;
    ++(*this);
    return ret;
  }

  RegisterIterator& operator--() {
    assert(this->m_container && this->m_index);
    this->m_index--;
    return *this;
  }

  RegisterIterator operator--(int) {
    RegisterIterator ret = *this;
    --(*this);
    return ret;
  }

  /*RegisterIterator& operator+=(difference_type rhs) throw() {
    assert(this->m_container);
    if (this->m_index + rhs > this->m_container.size()) throw std::out_of_range;
    this->m_index += rhs;
    return *this;
  }

  RegisterIterator& operator-=(difference_type rhs) throw() {
    assert(this->m_container);
    if ((int)((int)this->m_index - rhs) < 0) throw std::out_of_range;
    this->m_index -= rhs;
    return *this;
  }*/

  bool operator==(const RegisterIterator& rhs) const {
    assert(this->m_container);
    return this->m_container == rhs.m_container && this->m_index == rhs.m_index;
  }

  bool operator!=(const RegisterIterator& rhs) const {
    assert(this->m_container);
    return this->m_container != rhs.m_container || this->m_index != rhs.m_index;
  }

  bool operator<(const RegisterIterator& rhs) const {
    assert(this->m_container && this->m_container == rhs.m_container);
    return this->m_index < rhs.m_index;
  }

  bool operator>(const RegisterIterator& rhs) const {
    assert(this->m_container && this->m_container == rhs.m_container);
    return this->m_index > rhs.m_index;
  }

  bool operator<=(const RegisterIterator& rhs) const {
    assert(this->m_container && this->m_container == rhs.m_container);
    return this->m_index <= rhs.m_index;
  }

  bool operator>=(const RegisterIterator& rhs) const {
    assert(this->m_container && this->m_container == rhs.m_container);
    return this->m_index >= rhs.m_index;
  }

  /// @} Access and Traversal Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  protected:
  container_type* m_container;
  size_type m_index;

  /// @} Data
}; // class RegisterIterator

/// ****************************************************************************

/**
 * @brief RegisterInterface
 *
 * Interface class for a register field, register, register bank. Together
 * with scireg_region_if and RegisterIterator, it serves three purposes:
 *
 * 1.Declaring a composite interface: The region hierarchy is RegisterBank ->
 *   Register -> RegisterField -> bit. The interface includes common operations
 *   that can be performed by all types of regions. It also provides for child/
 *   parent access and delegation e.g. in read/write. Some common operations
 *   that are not applicable for all regions (operator=, casts, etc) are kept
 *   out of RegisterInterface for efficiency and for avoiding dynamic casts.
 *   As a side-note, a further beauty of composites is that it automatically
 *   takes care of the RTL/CA implementation, where the hierarchy becomes
 *   RegisterBank -> Register -> RegisterPipelineStage -> RegisterField -> bit.
 *   Since child types are template parameters, this becomes trivial.
 *
 * 2.Declaring an iterator interface: Instantiating RegisterInterface::iterator
 *   provides for child traversal.
 *   @see RegisterIterator
 *
 * 3.Declaring an observer interface for attaching/detaching callbacks. This is
 *   mostly already done in scireg_region_if.
 */
template <typename DATATYPE, typename CHILDTYPE>
class RegisterInterface
// TODO: This inheritance leads to two RegisterAbstraction objects relating to a
// Register, one via inheritance, the other via delegation. The inheritance is
// obviously only artificial for saving on typing. I could either:
// - Break it, put the access functions in RegisterInterface and the abstraction-
//   specific ones in Register.
// - Keep it, and come up with brand new pattern to automate delegation from
//   Register -> Abstraction.
//   One possibility is defining a delegate(func_ptr) function as part of
//   RegisterInterface. Register implements it by doing return this->m_strategy
//   ->func_ptr(), while Alias implements it as return this->m_reg->func_ptr().
//   It's elegant, but 1) requires the caller to wrap all abstraction-related
//   calls inside a delegate() and 2) is difficult to implement with parameters
//   (but not impossible - boost has a whole lib of higher-order functions).
: public RegisterAbstraction<DATATYPE> {
  /// @name Types
  /// @{

  public:
  typedef RegisterIterator<DATATYPE, CHILDTYPE> child_iterator;
  typedef unsigned index_type;

  /// @} Types
  /// --------------------------------------------------------------------------
  /// @name Traversal Methods
  /// @{

  public:
  /// Return iterator to the first child.
  virtual child_iterator begin() = 0;

  /// Return (null) end iterator.
  virtual child_iterator end() = 0;

  virtual CHILDTYPE& operator[](index_type index) = 0;

  /// Return the number of children.
  virtual unsigned size() const = 0;

  /// @} Traversal Methods
  /// --------------------------------------------------------------------------
  /// @name Observer Methods
  /// @{

  public:
  virtual void execute_callbacks(const scireg_ns::scireg_callback_type& type, const uint32_t& offset = 0, const uint32_t& size = 0) = 0;

  /// @} Observer Methods
  /// --------------------------------------------------------------------------
  /// @name Information and Helper Methods
  /// @{

  public:
  /// sc_object style print() of field value.
  virtual void print(std::ostream& os) const = 0;

  friend inline std::ostream& operator<<(std::ostream& os, const RegisterInterface& obj) {
    obj.print(os);
    return os;
  }

  /// @} Information and Helper Methods
  /// --------------------------------------------------------------------------
}; // class RegisterInterface

} // namespace trap

/// ****************************************************************************
#endif // TRAP_REGISTER_IF_H
