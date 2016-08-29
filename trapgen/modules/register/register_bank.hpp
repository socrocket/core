/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     register_bank.hpp
* @brief    This file is part of the TRAP runtime library.
* @details  @see RegisterBank
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
#ifndef TRAP_REGISTER_BANK_H_
#define TRAP_REGISTER_BANK_H_

#include "register_if.hpp"
#include "register_register.hpp"

#include <systemc>

#include <map>
#include <typeinfo>
#include <assert.h>

namespace trap {

/**
 * @brief RegisterBank
 *
 * Encapsulating registers in one place provides two advantages: First, it
 * greatly simplifies passing registers around (from processor to instructions,
 * ABI, tests, etc). Second, it could provide an iterable interface, so that the
 * processor or some test function can conventiently call RegisterBank::reset()
 * or RegisterBank::write() with no further knowledge of the stored registers.
 * If we only had registers to store, we could accomplish both goals with a
 * simple Register[] or std::vector<Register> and looping. However, we would
 * like to abstract away aliases and register banks as well, which makes four
 * different types that need to be stored, i.e. Register, RegisterAlias,
 * Register[] and RegisterAlias[]. There are two solutions:
 * - This class provides a dynamic solution, where a map of <RegisterInterface*,
 *   unsigned> is defined for storing either registers/aliases (unsigned = 1),
 *   or register/alias banks (unsigned = size of bank). This class then
 *   implements RegisterInterface by iterating over all elements.
 *   This solution has two disadvantages: First, there is the dynamic overhead
 *   of accessing map elements. Second, accessing a register by name, which is
 *   by far the most common operation, requires expensive, roundabout ways. This
 *   brings us to the second solution.
 * - This class is not used directly. A similar class to this one is auto-
 *   generated, which does without a container altogether and simply hardcodes
 *   the register names and sizes as class members. The generated implementation
 *   is otherwise very similar to this one. Thus, the dynamic overhead is
 *   minimal and we can conveniently access registers using a Bank.Register
 *   notation. For simplicity, the generated container does not implement the
 *   scireg interface.
 *   Note that hardcoding elements is similar to Luca's initial solution for
 *   the analogous situation of registers containing fields. In the latter case,
 *   however, he had to create a separate register class for each set of fields,
 *   which is almost one class per processor register. In the former case (banks
 *   containing registers), we only create one container class, which is well
 *   worth it.
 * The implementation of the scireg methods is inspired by sc_register.h from
 * Cadence.
 */
template<typename DATATYPE>
class RegisterBank
: public scireg_ns::scireg_region_if,
  public RegisterInterface<DATATYPE, std::pair<RegisterInterface<DATATYPE, RegisterField<DATATYPE> >*, unsigned> > {

  public:
  typedef RegisterInterface<DATATYPE, RegisterField<DATATYPE> > register_interface_type;
  typedef std::pair<register_interface_type*, unsigned> register_node_type;
  typedef std::map<register_interface_type*, unsigned> register_container_type;
  typedef typename RegisterInterface<DATATYPE, register_node_type>::child_iterator register_iterator;
  typedef unsigned index_type;

  /// @name Constructors and Destructors
  /// @{

  public:
  RegisterBank(std::string name = "", unsigned long long size = 0)
  : m_name(name)
  {}

  virtual ~RegisterBank() {
    for(typename register_container_type::iterator reg_it = this->m_regs.begin(); reg_it != this->m_regs.end(); ++reg_it) {
      if (reg_it->second == 1)
          delete reg_it->first;
      else for (unsigned i = 0; i < reg_it->second; ++i)
        delete [] reg_it->first;
    }
    this->m_regs.clear();
  }

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Traversal Methods
  /// @{

  public:
  /// Get child regions mapped into this region, by returning a mapped region
  /// object representing each mapping. The size and offset parameters can be
  /// used to constrain the range of the search.
  scireg_ns::scireg_response scireg_get_child_regions(std::vector<scireg_ns::scireg_mapped_region>& regions, sc_dt::uint64 size, sc_dt::uint64 offset) const {

    if ((size != sc_dt::uint64(-1)) || (offset != 0))
      return scireg_ns::SCIREG_FAILURE;

    typename register_container_type::const_iterator reg_it;
    unsigned i_reg;
    const scireg_region_if* region;

    for (i_reg = 0, reg_it = this->m_regs.begin(); reg_it != this->m_regs.end(); ++i_reg, ++reg_it) {
      if (reg_it->second == 1) {
        if ((region = dynamic_cast<scireg_ns::scireg_region_if*>(reg_it->first)) != NULL) {
          scireg_ns::scireg_mapped_region mapped_region;
          mapped_region.region = const_cast<scireg_region_if*>(region);
          static const char* name = "";
          region->scireg_get_string_attribute(name, scireg_ns::SCIREG_NAME);
          mapped_region.name = name;
          mapped_region.offset = i_reg;
          regions.push_back(mapped_region);
        }
      } else {
        for (unsigned i = 0; i < reg_it->second; ++i) {
          if ((region = dynamic_cast<scireg_ns::scireg_region_if*>(reg_it->first+i)) != NULL) {
            scireg_ns::scireg_mapped_region mapped_region;
            mapped_region.region = const_cast<scireg_region_if*>(region);
            static const char* name = "";
            region->scireg_get_string_attribute(name, scireg_ns::SCIREG_NAME);
            mapped_region.name = name;
            mapped_region.offset = i_reg;
            regions.push_back(mapped_region);
          }
        }
      }
    }

    return scireg_ns::SCIREG_SUCCESS;
  }

  register_iterator begin() {
    return register_iterator(this, 0);
  }

  register_iterator end() {
    return register_iterator(this, this->size());
  }

  register_node_type& operator[](index_type index) {
    assert(this->m_regs.size());
    return this->m_regs[index];
  }

  unsigned size() const { return this->m_regs.size(); }

  virtual bool add_register(register_interface_type* reg, unsigned size = 0) {
    typename register_container_type::iterator reg_it = this->m_regs.begin();
    /*register_node_type found_reg;
    reg_it = std::find(this->m_regs.begin(), this->m_regs.end(), found_reg);
    if (reg_it != this->m_regs.end())
      return false;
   */
    for(; reg_it != this->m_regs.end(); ++reg_it) {
      if (reg_it->second == 1) {
        if (reg_it->first == reg)
          return false;
      } else {
        for (unsigned i = 0; i < reg_it->second; ++i) {
          if (&(reg_it->first[i]) == reg)
            return false;
        }
      }
    }

    this->m_regs.insert(register_node_type(reg, size));
    return true;
  }

  //virtual const register_container_type& get_registers() const { return this->m_regs; };

  virtual bool add_port(sc_core::sc_object* busport) {
    if (!busport) return false;
    if (dynamic_cast<sc_core::sc_export_base*>(busport) == NULL) return false;
    this->m_ports.push_back(busport);
    return true;
  }

  /// Get SC TLM2 target socket associated with this region.
  scireg_ns::scireg_response scireg_get_target_ports(std::vector<sc_core::sc_object*>& ports) const {
    ports = m_ports;
    return scireg_ns::SCIREG_SUCCESS;
  }

  /// @} Traversal Methods
  /// --------------------------------------------------------------------------
  /// @name Access and Modification Methods
  /// @{

  public:
  // Call write() so that any value change callbacks are triggered.
  void reset() {
    Register<DATATYPE>* reg;
    for(typename register_container_type::iterator reg_it = this->m_regs.begin(); reg_it != this->m_regs.end(); ++reg_it) {
      if (reg_it->second == 1) {
        if ((reg = dynamic_cast<Register<DATATYPE>*>(reg_it->first)) != NULL)
          reg->reset();
      } else {
        for (unsigned i = 0; i < reg_it->second; ++i) {
          if ((reg = dynamic_cast<Register<DATATYPE>*>(reg_it->first+i)) != NULL)
          reg->reset();
        }
      }
    }
  }

  /// There are three read methods:
  /// 1.scireg_read() calls Register::read_dbg(). It theoretically permits
  ///   specifying size/offset.
  /// 2.read() calls Register::read().
  /// 3.read_dbg() calls Register::read_dbg().
  /// There are different possible interpretations of a register bank read.
  /// There is, however, none consistent with our interpretation of write, which
  /// functions as write_all (Would read then return an array of all read
  /// values? When will this be used?) I therefore left out the read interface
  /// altogether.
  /*scireg_ns::scireg_response scireg_read(scireg_ns::vector_byte& bytes, sc_dt::uint64 size, sc_dt::uint64 offset) const {
    if ((offset >= this->m_regs.size()) || (size != sizeof(DATATYPE)))
      return scireg_ns::SCIREG_FAILURE;

    bytes.resize(size);
    *(DATATYPE*)&bytes[0] = this->m_regs[offset]->read_dbg();
    return scireg_ns::SCIREG_SUCCESS;
  }*/

  const DATATYPE read() {
    return (DATATYPE)0;
  }

  const DATATYPE read_dbg() const {
    return (DATATYPE)0;
  }

  // Reads last written value.
  const DATATYPE read_force() const {
    return (DATATYPE)0;
  }

  /// There are three write methods:
  /// 1.scireg_write() calls Register::write(). It permits specifying size
  ///   [byte] and offset [bit].
  /// 2.write() calls Register::write().
  /// 3.write_dbg() calls Register::write_dbg().
  scireg_ns::scireg_response scireg_write(const scireg_ns::vector_byte& bytes, sc_dt::uint64 size, sc_dt::uint64 offset) {
    scireg_ns::scireg_region_if* region;
    for(typename register_container_type::iterator reg_it = this->m_regs.begin(); reg_it != this->m_regs.end(); ++reg_it) {
      if (reg_it->second == 1) {
        if ((region = dynamic_cast<scireg_ns::scireg_region_if*>(reg_it->first)) != NULL)
          region->scireg_write(bytes, size, offset);
      } else {
        for (unsigned i = 0; i < reg_it->second; ++i) {
          if ((region = dynamic_cast<scireg_ns::scireg_region_if*>(reg_it->first+i)) != NULL)
            region->scireg_write(bytes, size, offset);
        }
      }
    }
    return scireg_ns::SCIREG_SUCCESS;
  }

  bool write(const DATATYPE& data) {
    bool ret = true;
    for(typename register_container_type::iterator reg_it = this->m_regs.begin(); reg_it != this->m_regs.end(); ++reg_it) {
      if (reg_it->second == 1)
          ret = ret && reg_it->first->write(data);
      else for (unsigned i = 0; i < reg_it->second; ++i)
        ret = ret && reg_it->first[i].write(data);
    }
    return ret;
  }

  bool write_dbg(const DATATYPE& data) {
    bool ret = true;
    for(typename register_container_type::iterator reg_it = this->m_regs.begin(); reg_it != this->m_regs.end(); ++reg_it) {
      if (reg_it->second == 1)
          ret = ret && reg_it->first->write_dbg(data);
      else for (unsigned i = 0; i < reg_it->second; ++i)
        ret = ret && reg_it->first[i].write_dbg(data);
    }
    return ret;
  }

  // Writes value immediately discarding delay.
  bool write_force(const DATATYPE& data) {
    bool ret = true;
    for(typename register_container_type::iterator reg_it = this->m_regs.begin(); reg_it != this->m_regs.end(); ++reg_it) {
      if (reg_it->second == 1)
          ret = ret && reg_it->first->write_force(data);
      else for (unsigned i = 0; i < reg_it->second; ++i)
        ret = ret && reg_it->first[i].write_force(data);
    }
    return ret;
  }

  /// @} Access and Modification Methods
  /// --------------------------------------------------------------------------
  /// @name Observer Methods
  /// @{

  void execute_callbacks(const scireg_ns::scireg_callback_type& type, const uint32_t& offset = 0, const uint32_t& size = 0) {
    // TODO: I could implement callback hooks for banks, to be called here in
    // addition to the register callbacks.
    for(typename register_container_type::iterator reg_it = this->m_regs.begin(); reg_it != this->m_regs.end(); ++reg_it) {
      if (reg_it->second == 1)
          reg_it->first->execute_callbacks(type, 0, sizeof(DATATYPE));
      else for (unsigned i = 0; i < reg_it->second; ++i)
        reg_it->first[i].execute_callbacks(type, 0, sizeof(DATATYPE));
    }
  }

  /// @} Observer Methods
  /// --------------------------------------------------------------------------
  /// @name Information and Helper Methods
  /// @{
  public:
  const char* kind() const { return "RegisterBank"; }

  /// Get string attributes of type "type" associated with this region. The
  /// returned string is assigned to "attr".
  scireg_ns::scireg_response scireg_get_string_attribute(const char*& attr, scireg_ns::scireg_string_attribute_type type) const {
    switch (type) {
      case scireg_ns::SCIREG_NAME:
        attr = this->m_name.c_str();
        return scireg_ns::SCIREG_SUCCESS;

      case scireg_ns::SCIREG_DESCRIPTION:
      case scireg_ns::SCIREG_STRING_VALUE:
        return scireg_ns::SCIREG_UNSUPPORTED;
    }
    return scireg_ns::SCIREG_FAILURE;
  }

  /// Get the region_type of this region.
  scireg_ns::scireg_response scireg_get_region_type(scireg_ns::scireg_region_type& type) const {
    type = scireg_ns::SCIREG_BANK;
    return scireg_ns::SCIREG_SUCCESS;
  }

  sc_dt::uint64 scireg_get_bit_width() const { return sizeof(DATATYPE) * 8; }

  sc_dt::uint64 scireg_get_byte_width() const {
    unsigned bitwidth = sizeof(DATATYPE) * 8;
    unsigned num_regs = 0;
    for(typename register_container_type::const_iterator reg_it = this->m_regs.begin(); reg_it != this->m_regs.end(); ++reg_it) {
      num_regs += reg_it->second;
    }
    return num_regs * bitwidth;
  }

  void print(std::ostream& os) const {
    sc_core::sc_object* object;
    os << std::hex << std::showbase;
    for(typename register_container_type::const_iterator reg_it = this->m_regs.begin(); reg_it != this->m_regs.end(); ++reg_it) {
      if (reg_it->second == 1) {
        if ((object = dynamic_cast<sc_core::sc_object*>(reg_it->first)) != NULL)
          os << object->name() << ": " << reg_it->first->read_dbg() << '\n';
      } else {
        for (unsigned i = 0; i < reg_it->second; ++i) {
          if ((object = dynamic_cast<sc_core::sc_object*>(reg_it->first+i)) != NULL)
            os << object->name() << ": " << reg_it->first[i].read_dbg() << '\n';
        }
      }
    }
    os << std::dec;
  }

  std::ostream& operator<<(std::ostream& os) const {
    sc_core::sc_object* object;
    os << std::hex << std::showbase;
    for(typename register_container_type::const_iterator reg_it = this->m_regs.begin(); reg_it != this->m_regs.end(); ++reg_it) {
      if (reg_it->second == 1) {
        if ((object = dynamic_cast<sc_core::sc_object*>(reg_it->first)) != NULL)
          os << object->name() << ": " << reg_it->first->read_dbg() << '\n';
      } else {
        for (unsigned i = 0; i < reg_it->second; ++i) {
          if ((object = dynamic_cast<sc_core::sc_object*>(reg_it->first+i)) != NULL)
            os << object->name() << ": " << reg_it->first[i].read_dbg() << '\n';
        }
      }
    }
    os << std::dec;
    return os;
  }

  /// @} Information and Helper Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  protected:
  std::string m_name;
  register_container_type m_regs;
  std::vector<sc_core::sc_object*> m_ports;

  /// @} Data
}; // class RegisterBank

} // namespace trap

/// ****************************************************************************
#endif
