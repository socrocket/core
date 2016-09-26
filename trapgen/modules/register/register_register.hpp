/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     register_register.hpp
* @brief    This file is part of the TRAP runtime library.
* @details  @see Register
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

#ifndef TRAP_REGISTER_REGISTER_H
#define TRAP_REGISTER_REGISTER_H

#include "register_if.hpp"
#include "register_abstraction.hpp"
#include "register_field.hpp"

#include <systemc>
//#include <amba_parameters.h>

#include <vector>
#include <typeinfo>
#include <assert.h>

namespace trap {

/**
 * @brief Register
 *
 * Implements the read/write, callback and traversal interface. Register fields
 * can be accessed in array notation using a custom int-based custom enum.
 * Both RegisterField and RegisterBank delegate operations here. This simplifies
 * e.g. implementing things like const registers, since only this class needs to
 * be changed (@see RegisterAbstraction).
 * The implementation of the scireg methods is inspired by sc_register.h from
 * Cadence.
 */
template<typename DATATYPE>
class Register
: public sc_core::sc_object,
  public scireg_ns::scireg_region_if,
  public RegisterInterface<DATATYPE, RegisterField<DATATYPE> > {

  public:
  typedef std::vector<RegisterField<DATATYPE>*> field_container_type;
  typedef typename RegisterInterface<DATATYPE, RegisterField<DATATYPE> >::child_iterator field_iterator;
  typedef unsigned index_type;
  typedef std::vector<scireg_ns::scireg_callback*> callback_container_type;
  typedef bool (*clock_cycle_func_t)(DATATYPE*, DATATYPE*, unsigned long long*, unsigned long long*);

  /// @name Constructors and Destructors
  /// @{

  public:
  // Initially assumes the register does not define fields. Masks are set for
  // the register as a whole. If/when fields are added later on with insert(),
  // the masks are overwritten by the OR of the field masks.
  // TODO: Bundle abstraction-specific parameters in a struct. This makes the
  // interface more consistent and eases maintenance.
  Register(std::string name,
      amba_layer_ids abstraction = amba_LT,
      bool is_const = false,
      unsigned offset = 0,
      unsigned delay = 0,
      const DATATYPE& reset_val = (DATATYPE)0,
      unsigned num_pipe_stages = 1,
      clock_cycle_func_t clock_cycle_func = NULL,
      bool is_global = false,
      const DATATYPE& used_mask = (DATATYPE)~0,
      const DATATYPE& read_mask = (DATATYPE)~0,
      const DATATYPE& write_mask = (DATATYPE)~0)
    : sc_core::sc_object(name.c_str()),
      m_value(reset_val),
      m_reset_value(reset_val),
      m_used_mask(used_mask),
      m_read_mask(read_mask),
      m_write_mask(write_mask),
      m_is_const(is_const),
      m_offset(offset),
      m_delay(delay) {

    assert(!((is_const && offset) || (is_const && delay)));

    if (abstraction > amba_CT) {
      if (is_const) {
        assert(!(delay || offset));
        this->m_strategy = new RegisterTLMConst<DATATYPE>(this->m_value, this->m_used_mask, this->m_read_mask, this->m_write_mask, this->m_reset_value);
      } else if (offset && delay) {
        this->m_strategy = new RegisterTLMDelayOffset<DATATYPE>(this->m_value, this->m_used_mask, this->m_read_mask, this->m_write_mask, this->m_delay, this->m_offset);
      } else if (offset) {
        this->m_strategy = new RegisterTLMOffset<DATATYPE>(this->m_value, this->m_used_mask, this->m_read_mask, this->m_write_mask, this->m_offset);
      } else if (delay) {
        this->m_strategy = new RegisterTLMDelay<DATATYPE>(this->m_value, this->m_used_mask, this->m_read_mask, this->m_write_mask, this->m_delay);
      } else {
        this->m_strategy = new RegisterTLM<DATATYPE>(this->m_value, this->m_used_mask, this->m_read_mask, this->m_write_mask);
      }
    } else {
      if (is_const) {
        assert(!(delay || offset));
        this->m_strategy = new RegisterTLMConst<DATATYPE>(this->m_value, this->m_used_mask, this->m_read_mask, this->m_write_mask, this->m_reset_value);
      } else if (is_global) {
        this->m_strategy = new RegisterCAGlobal<DATATYPE>(this->m_value, this->m_used_mask, this->m_read_mask, this->m_write_mask, num_pipe_stages);
      } else {
        this->m_strategy = new RegisterCA<DATATYPE>(this->m_value, this->m_used_mask, this->m_read_mask, this->m_write_mask, num_pipe_stages, clock_cycle_func);
      }
    }
  }

  virtual ~Register() {
    delete this->m_strategy;

    for(typename field_container_type::iterator field_it = this->m_fields.begin(); field_it != this->m_fields.end(); ++field_it) {
      delete *field_it;
    }
    this->m_fields.clear();

    typename callback_container_type::iterator cb_it;
    for (unsigned i = 0; i < 3; ++i) {
      for(cb_it = this->m_callbacks[i].begin(); cb_it != this->m_callbacks[i].end(); ++cb_it) {
        delete *cb_it;
      }
      this->m_callbacks[i].clear();
    }
  }

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

  /// Get child regions mapped into this region, by returning a mapped region
  /// object representing each mapping. The size and offset parameters can be
  /// used to constrain the range of the search.
  scireg_ns::scireg_response scireg_get_child_regions(std::vector<scireg_ns::scireg_mapped_region>& regions, sc_dt::uint64 size, sc_dt::uint64 offset) const {

    if ((size != sc_dt::uint64(-1)) || (offset != 0))
      return scireg_ns::SCIREG_FAILURE;

    typename field_container_type::const_iterator field_it;
    const scireg_region_if* region;

    for (field_it = this->m_fields.begin(); field_it != this->m_fields.end(); ++field_it) {
      if ((region = dynamic_cast<scireg_ns::scireg_region_if*>(*field_it)) != NULL) {
        scireg_ns::scireg_mapped_region mapped_region;
        mapped_region.region = const_cast<scireg_region_if*>(region);
        static const char* name = "";
        region->scireg_get_string_attribute(name, scireg_ns::SCIREG_NAME);
        mapped_region.name = name;
        mapped_region.offset = 0;
        regions.push_back(mapped_region);
      }
    }

    return scireg_ns::SCIREG_SUCCESS;
  }

  field_iterator begin() {
    return field_iterator(this, 0);
  }

  field_iterator end() {
    return field_iterator(this, this->size());
  }

  RegisterField<DATATYPE>& operator[](index_type index) {
    assert(index < this->m_fields.size());
    return *this->m_fields[index];
  }

  unsigned size() const { return this->m_fields.size(); }

  virtual bool add_field(std::string name, unsigned highpos, unsigned lowpos, RegisterAccessMode access_mode = REGISTER_RW_ACCESS) {

    // Create field object.
    RegisterField<DATATYPE>* field = new RegisterField<DATATYPE>((this->basename() + std::string("_") + name), *this, highpos, lowpos, access_mode);

    if (!field) return false;

    execute_callbacks(scireg_ns::SCIREG_STATE_CHANGE);
    return add_field(field);
  }

  virtual bool add_field(RegisterField<DATATYPE>* field) {

    // Check for overlap.
    typename field_container_type::iterator field_it = this->m_fields.begin();
    while (field_it != this->m_fields.end()) {
      if (field->scireg_get_high_pos() > (*field_it)->scireg_get_high_pos()) {
        // Error: Overlapping fields.
        if (field->scireg_get_low_pos() <= (*field_it)->scireg_get_high_pos())
          return false;
        // Found the idx to insert.
        break;
      }
      ++field_it;
    }

    // Insert field object.
    this->m_fields.insert(field_it, field);

    // If this is the first field, overwrite the masks.
    if (this->m_fields.size() == 1) {
      this->m_used_mask = this->m_read_mask = this->m_write_mask = 0;
    }

    // Generate register used, read and write masks from fields.
    DATATYPE field_mask = field->get_mask();
    this->m_used_mask |= field_mask;
    if (field->get_access_mode() & REGISTER_READ_ACCESS)
      this->m_read_mask |= field_mask;
    else
      this->m_read_mask &= ~field_mask;
    if (field->get_access_mode() & REGISTER_WRITE_ACCESS)
      this->m_write_mask |= field_mask;
    else
      this->m_write_mask &= ~field_mask;

    execute_callbacks(scireg_ns::SCIREG_STATE_CHANGE);
    return true;
  }

  //virtual const field_container_type& get_fields() const { return this->m_fields; }

  RegisterAbstraction<DATATYPE>* get_strategy() const { return this->m_strategy; }

  /// @} Traversal Methods
  /// --------------------------------------------------------------------------
  /// @name Access and Modification Methods
  /// @{

  public:
  //void before_end_of_elaboration() { this->init(); }

  // Call write() so that any value change callbacks are triggered.
  void reset() { this->write_force(this->m_reset_value); }

  /// There are three read methods:
  /// 1.scireg_read() calls Register::read_dbg(). It permits specifying size
  ///   [byte] and offset [bit].
  /// 2.read() executes callbacks for this register, then calls Register::
  ///   read_dbg().
  /// 3.read_dbg() returns m_value.
  scireg_ns::scireg_response scireg_read(scireg_ns::vector_byte& bytes, sc_dt::uint64 size, sc_dt::uint64 offset) const {
    if ((offset + (size << 3)) > (sizeof(DATATYPE) << 3))
      return scireg_ns::SCIREG_FAILURE;

    bytes.resize(size);
    DATATYPE data = (this->read_dbg() >> offset) & ((1 << (size << 3)) - 1);
    *(DATATYPE*)&bytes[0] = data;
    return scireg_ns::SCIREG_SUCCESS;
  }

  const DATATYPE read() {
    execute_callbacks(scireg_ns::SCIREG_READ_ACCESS);
    return this->read_dbg();
  }

  const DATATYPE read_dbg() const {
    return this->m_strategy->read_dbg();
  }

  // Reads last written value.
  const DATATYPE read_force() const {
    return this->m_strategy->read_force();
  }

  /// There are three write methods:
  /// 1.scireg_write() calls Register::write(). It permits specifying size
  ///   [byte] and offset [bit].
  /// 2.write() executes callbacks for this register, then calls Register::
  ///   write_dbg().
  /// 3.write_dbg() writes m_value.
  scireg_ns::scireg_response scireg_write(const scireg_ns::vector_byte& bytes, sc_dt::uint64 size, sc_dt::uint64 offset) {
    if ((offset + (size << 3)) > (sizeof(DATATYPE) << 3))
      return scireg_ns::SCIREG_FAILURE;

    DATATYPE data = *(DATATYPE*)&bytes[0];
    this->write((data & ((1 << (size << 3)) - 1)) << offset);
    return scireg_ns::SCIREG_SUCCESS;
  }

  bool write(const DATATYPE& data) {
    bool ret = this->write_dbg(data);
    if (!ret) return false;
    execute_callbacks(scireg_ns::SCIREG_WRITE_ACCESS);
    return true;
  }

  bool write_dbg(const DATATYPE& data) {
    return (this->m_strategy->write_dbg(data));
  }

  // Writes value immediately discarding delay.
  bool write_force(const DATATYPE& data) {
    return this->m_strategy->write_force(data);
  }

  bool bit(unsigned index) const {
    assert(index < sizeof(DATATYPE)*8);
    return this->read_dbg() & (unsigned long long)(1 << index);
  }

  virtual operator DATATYPE() const {
    return this->read_dbg();
  }

  virtual Register& operator=(const Register& reg) {
    this->write(reg.read_dbg());
    return *this;
  }

  virtual Register& operator=(const DATATYPE& data) {
    this->write(data);
    return *this;
  }

  virtual DATATYPE operator+(const Register& reg) const {
    return (this->read_dbg() + reg.read_dbg());
  }

  virtual DATATYPE operator-(const Register& reg) const {
    return (this->read_dbg() - reg.read_dbg());
  }

  virtual DATATYPE operator*(const Register& reg) const {
    return (this->read_dbg() * reg.read_dbg());
  }

  virtual DATATYPE operator/(const Register& reg) const {
    return (this->read_dbg() / reg.read_dbg());
  }

  virtual Register& operator+=(const Register& reg) {
    this->write_dbg(this->read_dbg() + reg.read_dbg());
    return *this;
  }

  virtual Register& operator+=(const DATATYPE& data) {
    this->write_dbg(this->read_dbg() + data);
    return *this;
  }

  virtual Register& operator-=(const Register& reg) {
    this->write_dbg(this->read_dbg() - reg.read_dbg());
    return *this;
  }

  virtual Register& operator-=(const DATATYPE& data) {
    this->write_dbg(this->read_dbg() - data);
    return *this;
  }

  virtual Register& operator*=(const Register& reg) {
    this->write_dbg(this->read_dbg() * reg.read_dbg());
    return *this;
  }

  virtual Register& operator*=(const DATATYPE& data) {
    this->write_dbg(this->read_dbg() * data);
    return *this;
  }

  virtual Register& operator/=(const Register& reg) {
    this->write_dbg(this->read_dbg() / reg.read_dbg());
    return *this;
  }

  virtual Register& operator/=(const DATATYPE& data) {
    this->write_dbg(this->read_dbg() / data);
    return *this;
  }

  virtual DATATYPE operator<<(const Register& reg) const {
    return (this->read_dbg() << reg.read_dbg());
  }

  virtual DATATYPE operator>>(const Register& reg) const {
    return (this->read_dbg() >> reg.read_dbg());
  }

  virtual Register& operator<<=(const Register& reg) {
    this->write_dbg(this->read_dbg() << reg.read_dbg());
    return *this;
  }

  virtual Register& operator<<=(const DATATYPE& data) {
    this->write_dbg(this->read_dbg() << data);
    return *this;
  }

  virtual Register& operator>>=(const Register& reg) {
    this->write_dbg(this->read_dbg() >> reg.read_dbg());
    return *this;
  }

  virtual Register& operator>>=(const DATATYPE& data) {
    this->write_dbg(this->read_dbg() >> data);
    return *this;
  }

  virtual bool operator==(const Register& reg) const {
    return (this->read_dbg() == reg.read_dbg());
  }

  virtual bool operator!=(const Register& reg) const {
    return (this->read_dbg() != reg.read_dbg());
  }

  virtual bool operator<(const Register& reg) const {
    return (this->read_dbg() < reg.read_dbg());
  }

  virtual bool operator<=(const Register& reg) const {
    return (this->read_dbg() <= reg.read_dbg());
  }

  virtual bool operator>(const Register& reg) const {
    return (this->read_dbg() > reg.read_dbg());
  }

  virtual bool operator>=(const Register& reg) const {
    return (this->read_dbg() >= reg.read_dbg());
  }

  virtual DATATYPE operator~() {
    return ~(this->read_dbg());
  }

  virtual DATATYPE operator&(const Register& reg) const {
    return (this->read_dbg() & reg.read_dbg());
  }

  virtual DATATYPE operator|(const Register& reg) const {
    return (this->read_dbg() | reg.read_dbg());
  }

  virtual DATATYPE operator^(const Register& reg) const {
    return (this->read_dbg() ^ reg.read_dbg());
  }

  virtual Register& operator&=(const Register& reg) {
    this->write_dbg(this->read_dbg() & reg.read_dbg());
    return *this;
  }

  virtual Register& operator&=(const DATATYPE& data) {
    this->write_dbg(this->read_dbg() & data);
    return *this;
  }

  virtual Register& operator|=(const Register& reg) {
    this->write_dbg(this->read_dbg() | reg.read_dbg());
    return *this;
  }

  virtual Register& operator|=(const DATATYPE& data) {
    this->write_dbg(this->read_dbg() | data);
    return *this;
  }

  virtual Register& operator^=(const Register& reg) {
    this->write_dbg(this->read_dbg() ^ reg.read_dbg());
    return *this;
  }

  virtual Register& operator^=(const DATATYPE& data) {
    this->write_dbg(this->read_dbg() ^ data);
    return *this;
  }

  void clock_cycle() {
    this->m_strategy->clock_cycle();
  }

  /// @} Access and Modification Methods
  /// --------------------------------------------------------------------------
  /// @name Observer Methods
  /// @{

  /// Add/Delete Callback objects associated with this region.
  scireg_ns::scireg_response scireg_add_callback(scireg_ns::scireg_callback& cb) {
    this->m_callbacks[cb.type].push_back(&cb);
    return scireg_ns::SCIREG_SUCCESS;
  }

  scireg_ns::scireg_response scireg_remove_callback(scireg_ns::scireg_callback& cb) {
    callback_container_type::iterator cb_it;
    cb_it = std::find(this->m_callbacks[cb.type].begin(), this->m_callbacks[cb.type].end(), &cb);
    if (cb_it != this->m_callbacks[cb.type].end())
      this->m_callbacks[cb.type].erase(cb_it);
    return scireg_ns::SCIREG_SUCCESS;
  }

  void execute_callbacks(const scireg_ns::scireg_callback_type& type, const uint32_t& offset = 0, const uint32_t& size = 0) {
    assert(offset + size <= scireg_get_bit_width());

    scireg_ns::scireg_callback* callback;
    callback_container_type::iterator cb_it;
    for (cb_it = this->m_callbacks[type].begin(); cb_it != this->m_callbacks[type].end(); ++cb_it) {
      callback = *cb_it;
      if (((callback->offset >= offset) && (callback->offset <= offset + size))
        || ((offset >= callback->offset) && (offset <= callback->offset + callback->size)))
        callback->do_callback(*this);
    }
  }

  // TODO: This delegation needs rethinking. @see note in RegisterInterface.hpp.
  void set_stage(unsigned stage) {
    this->m_strategy->set_stage(stage);
  }

  void unset_stage() {
    this->m_strategy->unset_stage();
  }

  virtual void stall(unsigned stage) {
    return this->m_strategy->stall(stage);
  }

  virtual void advance() {
    return this->m_strategy->advance();
  }

  virtual void flush(unsigned stage) {
    return this->m_strategy->flush(stage);
  }

  // Hazard Detection Functions
  virtual unsigned is_locked(unsigned stage, unsigned latency) {
    return this->m_strategy->is_locked(stage, latency);
  }

  virtual bool lock(void* instr, unsigned stage, unsigned latency) {
    return this->m_strategy->lock(instr, stage, latency);
  }

  virtual bool unlock(void* instr) {
    return this->m_strategy->unlock(instr);
  }

  /// @} Observer Methods
  /// --------------------------------------------------------------------------
  /// @name Information and Helper Methods
  /// @{

  public:
  const char* kind() const { return "Register"; }

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
    type = scireg_ns::SCIREG_REGISTER;
    return scireg_ns::SCIREG_SUCCESS;
  }

  sc_dt::uint64 scireg_get_bit_width() const { return sizeof(DATATYPE) * 8; }

  virtual const DATATYPE& get_used_mask() const { return this->m_used_mask; }

  virtual const DATATYPE& get_read_mask() const { return this->m_read_mask; }

  virtual const DATATYPE& get_write_mask() const { return this->m_write_mask; }

  /// sc_object style print() of register value.
  void print(std::ostream& os) const {
    this->m_strategy->print(os);
  }

  /// @} Information and Helper Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  protected:
  DATATYPE m_value;
  DATATYPE m_reset_value;
  DATATYPE m_used_mask;
  DATATYPE m_read_mask;
  DATATYPE m_write_mask;
  const bool m_is_const;
  const unsigned m_offset;
  const unsigned m_delay;
  RegisterAbstraction<DATATYPE>* m_strategy;
  field_container_type m_fields;
  // Creating a container for each callback type saves us the search.
  callback_container_type m_callbacks[3];

  /// @} Data
}; // class Register

} // namespace trap

/// ****************************************************************************
#endif // TRAP_REGISTER_REGISTER_H
