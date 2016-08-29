/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     register_alias.hpp
* @brief    This file is part of the TRAP runtime library.
* @details  @see RegisterAlias
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
#ifndef TRAP_REGISTER_ALIAS_H_
#define TRAP_REGISTER_ALIAS_H_

#include "register_if.hpp"
#include "register_register.hpp"

#include <systemc>

#include <vector>
#include <typeinfo>
#include <assert.h>

namespace trap {

/**
 * @brief RegisterAlias
 *
 * Contains a pointer to a Register object. Provides (almost) the same interface
 * as Register and simply forwards all requests to the register.
 * The reason we need a class at all (instead of a plain old Register pointer)
 * is a possible fixed offset to the register value. For aliases with an offset,
 * all operations are applied to the raw register value. The offset is added for
 * reads and when we return a value. This causes interesting mathematical
 * properties, such as (alias = x) != x.
 * Since aliases with offsets are not very common, I considered different
 * solutions that do without offsets altogether:
 * - Register&: Assignable only with ugly pointer casts; Arrays of references
 *   for alias banks unallowed.
 * - Register*: Requires that the user remembers to use a different syntax for
 *   registers and aliases.
 * - std::reference_wrapper<Register>: No pointer syntax required, but
 *   Register::methods are only accessible via alias.get().method().
 * Added to this is the fact that offset/non-offset aliases will still require
 * a separate class.
 * In the end, I kept RegisterAlias as the only implementation for aliases.
 * I added comments for possible bug tracing whenever the offset is interpreted
 * differently than in Luca's previous implementation.
 */
template<typename DATATYPE>
class RegisterAlias
: public RegisterInterface<DATATYPE, RegisterField<DATATYPE> > {

  public:
  typedef typename RegisterInterface<DATATYPE, RegisterField<DATATYPE> >::child_iterator field_iterator;
  typedef unsigned index_type;

  /// @name Constructors and Destructors
  /// @{

  public:
  RegisterAlias(std::string name = "")
    : m_name(name),
      m_offset(0),
      m_offset_to_predecessor(0),
      m_predecessor(NULL)
  {}

  RegisterAlias(std::string name, Register<DATATYPE>* reg, unsigned offset = 0)
    : m_name(name),
      m_reg(reg),
      m_offset(offset),
      m_offset_to_predecessor(offset),
      m_predecessor(NULL)
  {}

  RegisterAlias(std::string name, RegisterAlias* alias, unsigned offset = 0)
    : m_name(name),
      m_reg(alias->m_reg),
      m_offset(alias->m_offset + offset),
      m_offset_to_predecessor(offset),
      m_predecessor(alias) {
    alias->m_successors.push_back(this);
  }

  ~RegisterAlias() {
    // Aliases that point to this one.
    typename std::vector<RegisterAlias*>::iterator successor_it, successor_end;
    for (successor_it = this->m_successors.begin(), successor_end = this->m_successors.end();
    successor_it != successor_end; ++successor_it) {
      if ((*successor_it)->m_predecessor == this) {
        (*successor_it)->m_predecessor = NULL;
      }
    }

    // Aliases this one points to.
    if (this->m_predecessor != NULL) {
      successor_it = std::find(this->m_predecessor->m_successors.begin(), this->m_predecessor->m_successors.end(), this);
      if (successor_it != this->m_predecessor->m_successors.end())
        this->m_predecessor->m_successors.erase(successor_it);
    }
    this->m_predecessor = NULL;
  }

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Traversal Methods
  /// @{

  public:
  /*/// Get parent SystemC modules associated with this region.
  scireg_ns::scireg_response scireg_get_parent_modules(std::vector<sc_core::sc_module*>& modules) const {
    return this->m_reg->scireg_get_parent_modules(modules);
  }

  /// Get parent regions of this region.
  scireg_ns::scireg_response scireg_get_parent_regions(std::vector<scireg_ns::scireg_region_if*>& regions) const {
    return this->m_reg->scireg_get_parent_regions(regions);
  }

  /// Get child regions mapped into this region, by returning a mapped region
  /// object representing each mapping. The size and offset parameters can be
  /// used to constrain the range of the search.
  scireg_ns::scireg_response scireg_get_child_regions(std::vector<scireg_ns::scireg_mapped_region>& regions, sc_dt::uint64 size, sc_dt::uint64 offset) const {
    return this->m_reg->scireg_get_child_regions(regions, size, offset);
  }*/

  field_iterator begin() {
    assert(!this->m_offset);
    return this->m_reg->begin();
  }

  field_iterator end() {
    assert(!this->m_offset);
    return this->m_reg->end();
  }

  RegisterField<DATATYPE>& operator[](index_type index) {
    assert(!this->m_offset);
    return (*this->m_reg)[index];
  }

  Register<DATATYPE>* get_reg() const { return this->m_reg; }

  /// Updates register pointer but keeps successors pointing to old register.
  void set_alias(Register<DATATYPE>& reg, unsigned offset = 0) {
    // Detach successors.
    // NOTE: Luca does not detach successors. Since each alias has its own
    // reference to the register, as well as a linked list of So if I had PC -> REGS[15] ->
    // RB[15], then I called REGS[15].set_alias(RB[31]), PC will still operate
    // on RB[15] but claim it is linked to REGS[15]. update_alias()
    // consistently updates the children, so I assume this one is the case where
    // we do not want to update them, but then we should at least detach PC from
    // REGS[15]. I chose to then set it to point to REGS[15]'s predecessor alias
    // (NULL in our example), which is not strictly necessary but intuitive.
    typename std::vector<RegisterAlias*>::iterator successor_it, successor_end;
    for (successor_it = this->m_successors.begin(), successor_end = this->m_successors.end();
    successor_it != successor_end; ++successor_it) {
      if (this->m_predecessor != NULL)
        this->m_predecessor->m_successors.push_back(*successor_it);
      (*successor_it)->m_predecessor = this->m_predecessor;
      (*successor_it)->m_offset_to_predecessor += this->m_offset_to_predecessor;
    }

    // Detach self from predecessor.
    if (this->m_predecessor != NULL) {
      successor_it = std::find(this->m_predecessor->m_successors.begin(), this->m_predecessor->m_successors.end(), this);
      if (successor_it != this->m_predecessor->m_successors.end())
        this->m_predecessor->m_successors.erase(successor_it);
    }
    this->m_predecessor = NULL;

    // Update register and offset.
    this->m_reg = &reg;
    // NOTE: Not reset by Luca.
    this->m_offset = offset;
    // NOTE: Not reset by Luca.
    this->m_offset_to_predecessor = offset;
  }

  void set_alias(RegisterAlias& alias, unsigned offset = 0) {
    // Detach successors.
    // NOTE: Not detached by Luca.
    typename std::vector<RegisterAlias*>::iterator successor_it, successor_end;
    for (successor_it = this->m_successors.begin(), successor_end = this->m_successors.end();
    successor_it != successor_end; ++successor_it) {
      if (this->m_predecessor != NULL)
        this->m_predecessor->m_successors.push_back(*successor_it);
      (*successor_it)->m_predecessor = this->m_predecessor;
      (*successor_it)->m_offset_to_predecessor += this->m_offset_to_predecessor;
    }

    // Detach self from predecessor.
    if (this->m_predecessor != NULL) {
      successor_it = std::find(this->m_predecessor->m_successors.begin(), this->m_predecessor->m_successors.end(), this);
      if (successor_it != this->m_predecessor->m_successors.end())
        this->m_predecessor->m_successors.erase(successor_it);
    }
    alias.m_successors.push_back(this);
    this->m_predecessor = &alias;

    // Update register and offset.
    this->m_reg = alias.m_reg;
    this->m_offset = alias.m_offset + offset;
    // NOTE: Not reset by Luca.
    this->m_offset_to_predecessor = offset;
  }

  /// Updates register pointer and successors' pointers as well.
  void update_alias(Register<DATATYPE>& reg, unsigned offset = 0) {
    // Update register and offset.
    this->m_reg = &reg;
    this->m_offset = this->m_offset_to_predecessor = offset;

    // Update successors.
    typename std::vector<RegisterAlias*>::iterator successor_it, successor_end;
    for (successor_it = this->m_successors.begin(), successor_end = this->m_successors.end();
    successor_it != successor_end; ++successor_it) {
      (*successor_it)->update_successors(&reg, this->m_offset);
    }

    // Detach self from predecessor.
    if (this->m_predecessor != NULL) {
      successor_it = std::find(this->m_predecessor->m_successors.begin(), this->m_predecessor->m_successors.end(), this);
      if (successor_it != this->m_predecessor->m_successors.end())
        this->m_predecessor->m_successors.erase(successor_it);
    }
    this->m_predecessor = NULL;
  }

  void update_alias(RegisterAlias& alias, unsigned offset = 0) {
    // Update register and offset.
    this->m_reg = alias.m_reg;
    this->m_offset = alias.m_offset + offset;
    this->m_offset_to_predecessor = offset;

    // Update successors.
    typename std::vector<RegisterAlias*>::iterator successor_it, successor_end;
    for (successor_it = this->m_successors.begin(), successor_end = this->m_successors.end();
    successor_it != successor_end; ++successor_it) {
      (*successor_it)->update_successors(alias.m_reg, this->m_offset);
    }

    // Detach self from predecessor.
    if (this->m_predecessor != NULL) {
      successor_it = std::find(this->m_predecessor->m_successors.begin(), this->m_predecessor->m_successors.end(), this);
      if (successor_it != this->m_predecessor->m_successors.end())
        this->m_predecessor->m_successors.erase(successor_it);
    }
    this->m_predecessor = &alias;
    alias.m_successors.push_back(this);
  }

  void update_successors(Register<DATATYPE>* reg, unsigned offset) {
    this->m_reg = reg;
    this->m_offset = this->m_offset_to_predecessor + offset;

    typename std::vector<RegisterAlias*>::iterator successor_it, successor_end;
    for (successor_it = this->m_successors.begin(), successor_end = this->m_successors.end();
    successor_it != successor_end; ++successor_it) {
      // NOTE: Luca passed offset (relative) instead of m_offset (absolute).
      (*successor_it)->update_successors(reg, this->m_offset);
    }
  }

  unsigned size() const { return this->m_reg->size(); }

  /// @} Traversal Methods
  /// --------------------------------------------------------------------------
  /// @name Access and Modification Methods
  /// @{

  public:
  // Call write() so that any value change callbacks are triggered.
  // NOTE: Callbacks will be triggered as often as there are aliases to m_reg!
  // TRAP will take care of not resetting aliases in the generated Registers::
  // reset() methods, but it is the responsibility of the programmer not to loop
  // over all register and aliases calling reset.
  void reset() { this->m_reg->reset(); }

  /// There are three read methods:
  /// 1.scireg_read() calls Register::read(). It permits specifying size [byte]
  ///   and offset [bit].
  /// 2.read() executes callbacks for this register, then calls Register::
  ///   read_dbg().
  /// 3.read_dbg() returns m_value.
  /*scireg_ns::scireg_response scireg_read(scireg_ns::vector_byte& bytes, sc_dt::uint64 size, sc_dt::uint64 offset) const {
    assert(!this->m_offset);
    return this->m_reg->scireg_read(bytes, size, offset);
  }*/

  const DATATYPE read() {
    return this->m_reg->read() + this->m_offset;
  }

  // NOTE: Luca: read_new_value() { m_reg->read_dbg(); } disregards own offset.
  const DATATYPE read_dbg() const {
    return this->m_reg->read_dbg() + this->m_offset;
  }

  // Reads last written value.
  const DATATYPE read_force() const {
    return this->m_reg->read_force() + this->m_offset;
  }

  /// There are three write methods:
  /// 1.scireg_write() calls Register::write(). It permits specifying size
  ///   [byte] and offset [bit].
  /// 2.write() executes callbacks for this register, then calls Register::
  ///   write_dbg().
  /// 3.write_dbg() writes m_value.
  /*scireg_ns::scireg_response scireg_write(const scireg_ns::vector_byte& bytes, sc_dt::uint64 size, sc_dt::uint64 offset) {
    assert(!this->m_offset);
    return this->m_reg->scireg_write(bytes, size, offset);
  }*/

  bool write(const DATATYPE& data) {
    return this->m_reg->write(data);
  }

  bool write_dbg(const DATATYPE& data) {
    return this->m_reg->write_dbg(data);
  }

  // Writes value immediately discarding delay.
  bool write_force(const DATATYPE& data) {
    return this->m_reg->write_force(data);
  }

  /*bool bit(unsigned index) const {
    assert(!this->m_offset);
    return this->m_reg->bit(index);
  }*/

  virtual operator DATATYPE() const {
    return this->read_dbg();
  }

  // NOTE: Luca: { *this->m_reg = *alias.m_reg; } disregards alias' offset.
  virtual RegisterAlias& operator=(const RegisterAlias& alias) {
    *this->m_reg = alias.read_dbg();
    return *this;
  }

  virtual RegisterAlias& operator=(const Register<DATATYPE>& reg) {
    *this->m_reg = reg;
    return *this;
  }

  virtual RegisterAlias& operator=(const DATATYPE& data) {
    *this->m_reg = data;
    return *this;
  }

  // NOTE: Luca: { read_dbg() + *alias.m_reg; } disregards alias' offset.
  virtual DATATYPE operator+(const RegisterAlias& alias) const {
    return (this->read_dbg() + alias.read_dbg());
  }

  virtual DATATYPE operator+(const Register<DATATYPE>& reg) const {
    return (this->read_dbg() + reg.read_dbg());
  }

  // NOTE: Luca: { read_dbg() - *alias.m_reg; } disregards alias' offset.
  virtual DATATYPE operator-(const RegisterAlias& alias) const {
    return (this->read_dbg() - alias.read_dbg());
  }

  virtual DATATYPE operator-(const Register<DATATYPE>& reg) const {
    return (this->read_dbg() - reg.read_dbg());
  }

  // NOTE: Luca: { read_dbg() * *alias.m_reg; } disregards alias' offset.
  virtual DATATYPE operator*(const RegisterAlias& alias) const {
    return (this->read_dbg() * alias.read_dbg());
  }

  virtual DATATYPE operator*(const Register<DATATYPE>& reg) const {
    return (this->read_dbg() * reg.read_dbg());
  }

  // NOTE: Luca: { read_dbg() / *alias.m_reg; } disregards alias' offset.
  virtual DATATYPE operator/(const RegisterAlias& alias) const {
    return (this->read_dbg() / alias.read_dbg());
  }

  virtual DATATYPE operator/(const Register<DATATYPE>& reg) const {
    return (this->read_dbg() / reg.read_dbg());
  }

  // NOTE: Luca: { *m_reg += *alias.m_reg; } disregards alias' offset.
  virtual RegisterAlias& operator+=(const RegisterAlias& alias) {
    *this->m_reg += alias.read_dbg();
    return *this;
  }

  virtual RegisterAlias& operator+=(const Register<DATATYPE>& reg) {
    *this->m_reg += reg;
    return *this;
  }

  virtual RegisterAlias& operator+=(const DATATYPE& data) {
    *this->m_reg += data;
    return *this;
  }

  // NOTE: Luca: { *m_reg -= *alias.m_reg; } disregards alias' offset.
  virtual RegisterAlias& operator-=(const RegisterAlias& alias) {
    *this->m_reg -= alias.read_dbg();
    return *this;
  }

  virtual RegisterAlias& operator-=(const Register<DATATYPE>& reg) {
    *this->m_reg -= reg;
    return *this;
  }

  virtual RegisterAlias& operator-=(const DATATYPE& data) {
    *this->m_reg -= data;
    return *this;
  }

  // NOTE: Luca: { *m_reg *= *alias.m_reg; } disregards alias' offset.
  virtual RegisterAlias& operator*=(const RegisterAlias& alias) {
    *this->m_reg *= alias.read_dbg();
    return *this;
  }

  virtual RegisterAlias& operator*=(const Register<DATATYPE>& reg) {
    *this->m_reg *= reg;
    return *this;
  }

  virtual RegisterAlias& operator*=(const DATATYPE& data) {
    *this->m_reg *= data;
    return *this;
  }

  // NOTE: Luca: { *m_reg /= *alias.m_reg; } disregards alias' offset.
  virtual RegisterAlias& operator/=(const RegisterAlias& alias) {
    *this->m_reg /= alias.read_dbg();
    return *this;
  }

  virtual RegisterAlias& operator/=(const Register<DATATYPE>& reg) {
    *this->m_reg /= reg;
    return *this;
  }

  virtual RegisterAlias& operator/=(const DATATYPE& data) {
    *this->m_reg /= data;
    return *this;
  }

  // NOTE: Luca: { read_dbg() << *alias.m_reg; } disregards alias' offset.
  virtual DATATYPE operator<<(const RegisterAlias& alias) const {
    assert(!(this->m_offset || alias.m_offset));
    return (this->read_dbg() << alias.read_dbg());
  }

  virtual DATATYPE operator<<(const Register<DATATYPE>& reg) const {
    assert(!this->m_offset);
    return (this->read_dbg() << reg.read_dbg());
  }

  // NOTE: Luca: { read_dbg() >> *alias.m_reg; } disregards alias' offset.
  virtual DATATYPE operator>>(const RegisterAlias& alias) const {
    assert(!(this->m_offset || alias.m_offset));
    return (this->read_dbg() >> alias.read_dbg());
  }

  virtual DATATYPE operator>>(const Register<DATATYPE>& reg) const {
    assert(!this->m_offset);
    return (this->read_dbg() >> reg.read_dbg());
  }

  virtual RegisterAlias& operator<<=(const RegisterAlias& alias) {
    assert(!(this->m_offset || alias.m_offset));
    *this->m_reg <<= alias.read_dbg();
    return *this;
  }

  virtual RegisterAlias& operator<<=(const Register<DATATYPE>& reg) {
    assert(!this->m_offset);
    *this->m_reg <<= reg;
    return *this;
  }

  virtual RegisterAlias& operator<<=(const DATATYPE& data) {
    assert(!this->m_offset);
    *this->m_reg <<= data;
    return *this;
  }

  virtual RegisterAlias& operator>>=(const RegisterAlias& alias) {
    assert(!(this->m_offset || alias.m_offset));
    *this->m_reg >>= alias.read_dbg();
    return *this;
  }

  virtual RegisterAlias& operator>>=(const Register<DATATYPE>& reg) {
    assert(!this->m_offset);
    *this->m_reg >>= reg;
    return *this;
  }

  virtual RegisterAlias& operator>>=(const DATATYPE& data) {
    assert(!this->m_offset);
    *this->m_reg >>= data;
    return *this;
  }

  virtual bool operator==(const RegisterAlias& alias) const {
    return (this->read_dbg() == alias.read_dbg());
  }

  virtual bool operator==(const Register<DATATYPE>& reg) const {
    return (this->read_dbg() == reg.read_dbg());
  }

  virtual bool operator!=(const RegisterAlias& alias) const {
    return (this->read_dbg() != alias.read_dbg());
  }

  virtual bool operator!=(const Register<DATATYPE>& reg) const {
    return (this->read_dbg() != reg.read_dbg());
  }

  virtual bool operator<(const RegisterAlias& alias) const {
    return (this->read_dbg() < alias.read_dbg());
  }

  virtual bool operator<(const Register<DATATYPE>& reg) const {
    return (this->read_dbg() < reg.read_dbg());
  }

  virtual bool operator<=(const RegisterAlias& alias) const {
    return (this->read_dbg() <= alias.read_dbg());
  }

  virtual bool operator<=(const Register<DATATYPE>& reg) const {
    return (this->read_dbg() <= reg.read_dbg());
  }

  virtual bool operator>(const RegisterAlias& alias) const {
    return (this->read_dbg() > alias.read_dbg());
  }

  virtual bool operator>(const Register<DATATYPE>& reg) const {
    return (this->read_dbg() > reg.read_dbg());
  }

  virtual bool operator>=(const RegisterAlias& alias) const {
    return (this->read_dbg() >= alias.read_dbg());
  }

  virtual bool operator>=(const Register<DATATYPE>& reg) const {
    return (this->read_dbg() >= reg.read_dbg());
  }

  virtual DATATYPE operator~() {
    assert(!this->m_offset);
    return ~(this->read_dbg());
  }

  // NOTE: Luca: { read_dbg() & *alias.m_reg; } disregards alias' offset.
  virtual DATATYPE operator&(const RegisterAlias& alias) const {
    assert(!(this->m_offset || alias.m_offset));
    return (this->read_dbg() & alias.read_dbg());
  }

  virtual DATATYPE operator&(const Register<DATATYPE>& reg) const {
    assert(!this->m_offset);
    return (this->read_dbg() & reg.read_dbg());
  }

  // NOTE: Luca: { read_dbg() | *alias.m_reg; } disregards alias' offset.
  virtual DATATYPE operator|(const RegisterAlias& alias) const {
    assert(!(this->m_offset || alias.m_offset));
    return (this->read_dbg() | alias.read_dbg());
  }

  virtual DATATYPE operator|(const Register<DATATYPE>& reg) const {
    assert(!this->m_offset);
    return (this->read_dbg() | reg.read_dbg());
  }

  // NOTE: Luca: { read_dbg() ^ *alias.m_reg; } disregards alias' offset.
  virtual DATATYPE operator^(const RegisterAlias& alias) const {
    assert(!(this->m_offset || alias.m_offset));
    return (this->read_dbg() ^ alias.read_dbg());
  }

  virtual DATATYPE operator^(const Register<DATATYPE>& reg) const {
    assert(!this->m_offset);
    return (this->read_dbg() ^ reg.read_dbg());
  }

  // NOTE: Luca: { *m_reg &= *alias.m_reg; } disregards alias' offset.
  virtual RegisterAlias& operator&=(const RegisterAlias& alias) {
    assert(!(this->m_offset || alias.m_offset));
    *this->m_reg &= alias.read_dbg();
    return *this;
  }

  virtual RegisterAlias& operator&=(const Register<DATATYPE>& reg) {
    assert(!this->m_offset);
    *this->m_reg &= reg;
    return *this;
  }

  virtual RegisterAlias& operator&=(const DATATYPE& data) {
    assert(!this->m_offset);
    *this->m_reg &= data;
    return *this;
  }

  // NOTE: Luca: { *m_reg |= *alias.m_reg; } disregards alias' offset.
  virtual RegisterAlias& operator|=(const RegisterAlias& alias) {
    assert(!(this->m_offset || alias.m_offset));
    *this->m_reg |= alias.read_dbg();
    return *this;
  }

  virtual RegisterAlias& operator|=(const Register<DATATYPE>& reg) {
    assert(!this->m_offset);
    *this->m_reg |= reg;
    return *this;
  }

  virtual RegisterAlias& operator|=(const DATATYPE& data) {
    assert(!this->m_offset);
    *this->m_reg |= data;
    return *this;
  }

  // NOTE: Luca: { *m_reg ^= *alias.m_reg; } disregards alias' offset.
  virtual RegisterAlias& operator^=(const RegisterAlias& alias) {
    assert(!(this->m_offset || alias.m_offset));
    *this->m_reg ^= alias.read_dbg();
    return *this;
  }

  virtual RegisterAlias& operator^=(const Register<DATATYPE>& reg) {
    assert(!this->m_offset);
    *this->m_reg ^= reg;
    return *this;
  }

  virtual RegisterAlias& operator^=(const DATATYPE& data) {
    assert(!this->m_offset);
    *this->m_reg ^= data;
    return *this;
  }

  /// @} Access and Modification Methods
  /// --------------------------------------------------------------------------
  /// @name Observer Methods
  /// @{

  /*/// Add/Delete Callback objects associated with this region.
  scireg_ns::scireg_response scireg_add_callback(scireg_ns::scireg_callback& cb) {
    this->m_callbacks.push_back(&cb);
    return scireg_ns::SCIREG_SUCCESS;
  }

  scireg_ns::scireg_response scireg_remove_callback(scireg_ns::scireg_callback& cb) {
    CallbackVec::iterator cb_it;
    cb_it = std::find(this->m_callbacks.begin(), this->m_callbacks.end(), &cb);
    if (cb_it != this->m_callbacks.end())
      this->m_callbacks.erase(cb_it);
    return scireg_ns::SCIREG_SUCCESS;
  }*/

  void execute_callbacks(const scireg_ns::scireg_callback_type& type, const uint32_t& offset, const uint32_t& size) {
    this->m_reg->execute_callbacks(type, offset, size);
  }

  /// @} Observer Methods
  /// --------------------------------------------------------------------------
  /// @name Information and Helper Methods
  /// @{

  public:
  const char* name() const {
    if (!this->m_name) return NULL;
    return this->m_name.str();
  }

  const char* kind() const { return "RegisterAlias"; }

  /*/// Get string attributes of type "type" associated with this region. The
  /// returned string is assigned to "attr".
  scireg_ns::scireg_response scireg_get_string_attribute(const char*& attr, scireg_ns::scireg_string_attribute_type type) const {
    return this->m_reg->scireg_get_string_attribute(attr, type);
  }

  /// Get the region_type of this region.
  scireg_ns::scireg_response scireg_get_region_type(scireg_ns::scireg_region_type& type) const {
    return this->m_reg->scireg_get_region_type(type);
  }

  sc_dt::uint64 scireg_get_bit_width() const { return this->m_reg->scireg_get_bit_width(); }

  virtual const DATATYPE& get_used_mask() const { return this->m_reg->get_used_mask(); }

  virtual const DATATYPE& get_read_mask() const { return this->m_reg->get_read_mask(); }

  virtual const DATATYPE& get_write_mask() const { return this->m_reg->get_write_mask(); }*/

  /// sc_object style print() of register value.
  void print(std::ostream& os) const {
    os << std::hex << std::showbase << this->read_dbg() << std::dec;
    return;
  }

  std::ostream& operator<<(std::ostream& os) const {
    os << std::hex << std::showbase << this->read_dbg() << std::dec;
    return os;
  }

  /// @} Information and Helper Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  protected:
  std::string m_name;
  Register<DATATYPE>* m_reg;
  // Absolute offset to the register.
  unsigned m_offset;
  // Offset to the preceding alias in the linked list, if any. Set to m_offset
  // if it points to the register directly.
  unsigned m_offset_to_predecessor;
  RegisterAlias* m_predecessor;
  std::vector<RegisterAlias*> m_successors;

  /// @} Data
}; // class RegisterAlias

} // namespace trap

/// ****************************************************************************
#endif
