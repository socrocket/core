/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     register_field.hpp
* @brief    This file is part of the TRAP runtime library.
* @details  @see RegisterField
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

#ifndef TRAP_REGISTER_FIELD_H
#define TRAP_REGISTER_FIELD_H

#include "register_if.hpp"

#include <systemc>

#include <assert.h>

namespace trap {

/// Types

// NOTE: REGISTER_RW_ACCESS == REGISTER_READ_ACCESS | REGISTER_WRITE_ACCESS
enum RegisterAccessMode {
  REGISTER_READ_ACCESS  = 1,
  REGISTER_WRITE_ACCESS = 2,
  REGISTER_RW_ACCESS    = 3
};

/// Forward declarations

template <typename DATATYPE>
class Register;

/// ****************************************************************************

/**
 * @brief RegisterField
 *
 * Saves a pointer to a Register parent. Most operations are masked and shifted
 * then delegated to Register.
 * The implementation of the scireg methods is inspired by sc_register.h from
 * Cadence.
 */
template<typename DATATYPE>
class RegisterField
: public sc_core::sc_object,
  public scireg_ns::scireg_region_if,
  public RegisterInterface<DATATYPE, bool> {

  public:
  typedef typename RegisterInterface<DATATYPE, bool>::child_iterator bit_iterator;

  /// @name Constructors and Destructors
  /// @{

  public:
  RegisterField(std::string name, Register<DATATYPE>& parent, unsigned highpos, unsigned lowpos, RegisterAccessMode access_mode = REGISTER_RW_ACCESS)
    : sc_core::sc_object(name.c_str()),
      m_highpos(highpos),
      m_lowpos(lowpos),
      m_access_mode(access_mode),
      m_reg(parent) {

    assert(highpos >= lowpos && highpos < sizeof(DATATYPE)*8);
    gen_mask();
  }

  virtual ~RegisterField() {}

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Traversal Methods
  /// @{

  public:
  /// Get parent SystemC modules associated with this region.
  scireg_ns::scireg_response scireg_get_parent_modules(std::vector<sc_core::sc_module*>& modules) const {
    sc_core::sc_module* module = dynamic_cast<sc_core::sc_module*>(this->get_parent_object());
    if (module) {
      modules.push_back(module);
      return scireg_ns::SCIREG_SUCCESS;
    }
    return scireg_ns::SCIREG_FAILURE;
  }

  /// Get parent regions of this region.
  scireg_ns::scireg_response scireg_get_parent_regions(std::vector<scireg_ns::scireg_region_if*>& regions) const {
    scireg_ns::scireg_region_if* region = dynamic_cast<scireg_ns::scireg_region_if*>(this->get_parent_object());
    if (region) {
      regions.push_back(region);
      return scireg_ns::SCIREG_SUCCESS;
    }
    return scireg_ns::SCIREG_FAILURE;
  }

  bit_iterator begin() {
    return bit_iterator(this, 0);
  }

  bit_iterator end() {
    return bit_iterator(this, this->size());
  }

  bool& operator[](unsigned index) {
    assert(index < this->size());
    this->m_value = ((this->m_reg.read_dbg() & this->m_mask) >> (this->m_lowpos + index)) & 0x1;
    return this->m_value;
  }

  unsigned size() const { return this->m_highpos - this->m_lowpos + 1; }

  virtual Register<DATATYPE>& parent() const { return this->m_reg; }

  /// @} Traversal Methods
  /// --------------------------------------------------------------------------
  /// @name Access and Modification Methods
  /// @{

  public:
  //void before_end_of_elaboration() { this->m_reg.add_field(this); }

  /// There are three read methods:
  /// 1.scireg_read() calls RegisterField::read_dbg(). It theoretically permits
  ///   specifying size/offset. This makes little sense for fields, so it is
  ///   unsupported.
  /// 2.read() executes callbacks for this field, then calls
  ///   RegisterField::read_dbg().
  /// 3.read_dbg() calls Register::read_dbg() and masks/shifts the result.
  scireg_ns::scireg_response scireg_read(scireg_ns::vector_byte& bytes, sc_dt::uint64 size, sc_dt::uint64 offset = 0) const {
    //if (offset || (size != ((scireg_get_bit_width() + 7) >> 3)))
    if (offset || (size != sizeof(DATATYPE)))
      return scireg_ns::SCIREG_FAILURE;

    bytes.resize(sizeof(DATATYPE));
    *(DATATYPE*)&bytes[0] = this->read_dbg();
    return scireg_ns::SCIREG_SUCCESS;
  }

  const DATATYPE read() {
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS, this->m_lowpos, this->size());
    return this->read_dbg();
  }

  const DATATYPE read_dbg() const {
    return (this->m_reg.read_dbg() & this->m_mask) >> this->m_lowpos;
  }

  // Reads last written value.
  const DATATYPE read_force() const {
    return (this->m_reg.read_force() & this->m_mask) >> this->m_lowpos;
  }

  /// There are three write methods:
  /// 1.scireg_write() calls RegisterField::write(). It theoretically permits
  ///   specifying size/offset. This makes little sense for fields, so it is
  ///   unsupported.
  /// 2.write() executes callbacks for this field, then calls
  ///   RegisterField::write_dbg().
  /// 3.write_dbg() calls Register::write_dbg() and masks/shifts the result.
  scireg_ns::scireg_response scireg_write(const scireg_ns::vector_byte& bytes, sc_dt::uint64 size, sc_dt::uint64 offset = 0) {
    //if (offset || (size != ((scireg_get_bit_width() + 7) >> 3)))
    if (offset || (size != sizeof(DATATYPE)))
      return scireg_ns::SCIREG_FAILURE;

    this->write(*(DATATYPE*)&(bytes[0]));
    return scireg_ns::SCIREG_SUCCESS;
  }

  bool write(const DATATYPE& data) {
    bool ret = this->write_dbg(data);
    if (!ret) return false;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS, this->m_lowpos, this->size());
    return true;
  }

  bool write_dbg(const DATATYPE& data) {
    return (this->m_reg.write_dbg((this->m_reg.read_dbg() & ~this->m_mask) | ((data << this->m_lowpos) & this->m_mask)));
  }

  // Writes value immediately discarding delay.
  bool write_force(const DATATYPE& data) {
    return (this->m_reg.write_force((this->m_reg.read_dbg() & ~this->m_mask) | ((data << this->m_lowpos) & this->m_mask)));
  }

  virtual operator DATATYPE() const {
    return (this->m_reg.read_dbg() & this->m_mask) >> this->m_lowpos;
  }

  virtual RegisterField& operator=(const RegisterField& field) {
    this->write(field.read_dbg());
    return *this;
  }

  virtual RegisterField& operator=(const DATATYPE& data) {
    this->write(data);
    return *this;
  }

  /// @} Access and Modification Methods
  /// --------------------------------------------------------------------------
  /// @name Observer Methods
  /// @{

  public:
  void execute_callbacks(const scireg_ns::scireg_callback_type& type, const uint32_t& offset = 0, const uint32_t& size = 0) {
    // TODO: I could implement callback hooks for fields, to be called here in
    // addition to the register callbacks.
    if (offset == this->m_lowpos && size == this->size())
      this->m_reg.execute_callbacks(type, offset, size);
  }

  /// @} Observer Methods
  /// --------------------------------------------------------------------------
  /// @name Information and Helper Methods
  /// @{

  public:
  const char* kind() const { return "RegisterField"; }

  /// Get string attributes of type "type" associated with this region. The
  /// returned string is assigned to "attr".
  scireg_ns::scireg_response scireg_get_string_attribute(const char*& attr, scireg_ns::scireg_string_attribute_type type) const {
    switch (type) {
      case scireg_ns::SCIREG_NAME:
        attr = this->name();
        return scireg_ns::SCIREG_SUCCESS;

      case scireg_ns::SCIREG_DESCRIPTION:
      case scireg_ns::SCIREG_STRING_VALUE:
        return scireg_ns::SCIREG_UNSUPPORTED;
    }

    return scireg_ns::SCIREG_FAILURE;
  }

  /// Get the region_type of this region.
  scireg_ns::scireg_response scireg_get_region_type(scireg_ns::scireg_region_type& type) const {
    type = scireg_ns::SCIREG_FIELD;
    return scireg_ns::SCIREG_SUCCESS;
  }

  sc_dt::uint64 scireg_get_low_pos() const { return this->m_lowpos; }

  sc_dt::uint64 scireg_get_high_pos() const { return this->m_highpos; }

  sc_dt::uint64 scireg_get_bit_width() const { return (this->m_highpos - this->m_lowpos + 1); }

  virtual const DATATYPE& get_mask() const { return this->m_mask; }

  virtual const RegisterAccessMode get_access_mode() const { return this->m_access_mode; }

  /// sc_object style print() of field value.
  void print(std::ostream& os) const {
    os << std::hex << std::showbase << this->read_dbg() << std::dec;
  }

  protected:
  // Based on lowpos, highpos, generate mask of type DATATYPE.
  void gen_mask() {
    unsigned width = this->m_highpos - this->m_lowpos + 1;
    this->m_mask = ((unsigned long long)(1 << width) - 1) << this->m_lowpos;
  }

  /// @} Information and Helper Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  protected:
  const unsigned m_highpos;
  const unsigned m_lowpos;
  const RegisterAccessMode m_access_mode;
  Register<DATATYPE>& m_reg;
  /// Mock variable used by the iterator interface.
  /// @see operator[]
  bool m_value;
  DATATYPE m_mask;

  /// @} Data
}; // class RegisterField

} // namespace trap

/// ****************************************************************************
#endif // TRAP_REGISTER_FIELD_H
