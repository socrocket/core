// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_register
/// @{
/// @file sr_register.h
/// @date 2010-2015
/// @author Rolf Meyer
/// @copyright
///   Licensed under the Apache License, Version 2.0 (the "License");
///   you may not use this file except in compliance with the License.
///   You may obtain a copy of the License at
///
///       http://www.apache.org/licenses/LICENSE-2.0
///
///   Unless required by applicable law or agreed to in writing, software
///   distributed under the License is distributed on an "AS IS" BASIS,
///   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
///   See the License for the specific language governing permissions and
///   limitations under the License.
///
/// This file contains the implementation of the sr_register base classes,
/// which can be used to model registers and register banks in a TLM modules.
/// The sr_register classes are not part of the SCIREG API standard.
/// Instead, it extends it as a possible register implementation using the
/// SCIREG API standard.
#ifndef CORE_COMMON_SR_REGISTER_SR_REGISTER_H_
#define CORE_COMMON_SR_REGISTER_SR_REGISTER_H_

#include <stdexcept>
#include "systemc.h"
#include "sc_register.h"

/// lsb0 mode is used in field position specification.
/// (lsb0 defines the 0 as the least significant bit. It is the default
/// position specification mode in IP-XACT systemRDL.
template<typename T>
class sr_register_field : public sc_register_field_b<T> {
  public:
    sr_register_field(const char* name, sc_register_b<T> *c, unsigned int highpos, unsigned int lowpos)
      : sc_register_field_b<T>(name, c), m_highpos(highpos), m_lowpos(lowpos) {
      // Generate mask.
      gen_mask();
    }

    sr_register_field( const char* name, sc_register_b<T>* c, const char* desc, unsigned int highpos, unsigned int lowpos)
      : sc_register_field_b<T>(name, c, desc), m_highpos(highpos), m_lowpos(lowpos) {
      // Generate mask.
      gen_mask();
    }

    ~sr_register_field() {}

    //  const char *name(); // in sc_register_field_base.
    virtual unsigned int low_pos() const { return m_lowpos; }
    virtual unsigned int high_pos() const { return m_highpos; }
    virtual unsigned int bit_width() const { return m_highpos-m_lowpos + 1; }

  protected:
    virtual const T& get_mask() const { return m_mask; }
    virtual unsigned int get_shift_bits() const { return low_pos(); }

  private:

    // Based on lowpos, highpos, generate mask of type T.
    void gen_mask() {
        assert( (m_lowpos <= m_highpos) && (m_highpos < sizeof(T)*8) );
        unsigned int width = m_highpos - m_lowpos + 1;
        unsigned int m0 = 0x1;
        m_mask = 0;
        for( unsigned int i = 0; i < width; i++) {
          m_mask = m_mask << 1 | m0;
        }
        m_mask = m_mask << m_lowpos;
    }

  protected:
    T                  m_mask;
    const unsigned int m_highpos;
    const unsigned int m_lowpos;
};

enum sr_register_callback_type {
  SR_PRE_READ,
  SR_POST_READ,
  SR_PRE_WRITE,
  SR_POST_WRITE
};

class sr_register_callback_base {
  public:
    virtual ~sr_register_callback_base(){};
    virtual void call() = 0;
};

class sr_register_scireg_callback : public sr_register_callback_base {
  public:
    sr_register_scireg_callback(scireg_ns::scireg_callback &callback, scireg_ns::scireg_region_if &region) : m_callback(callback), m_region(region) {}
    virtual ~sr_register_scireg_callback() {}
    virtual void call() {
      m_callback.do_callback(m_region);
    }
  private:
    scireg_ns::scireg_callback &m_callback;
    scireg_ns::scireg_region_if &m_region;
};

template<typename OWNER>
class sr_register_callback : public sr_register_callback_base {
  public:
    typedef void (OWNER::*callback_t)();
    sr_register_callback(OWNER *owner, callback_t callback) : m_owner(owner), m_callback(callback) {}

    virtual void call() {
      (m_owner->*m_callback)();
    }

  private:
    OWNER *m_owner;
    callback_t m_callback;
};

template<typename DATA_TYPE>
class sr_register : public sc_register_b<DATA_TYPE> {
  public:
    typedef typename std::vector<sr_register_callback_base *> callback_vector_t;
    typedef typename std::map<sr_register_callback_type, callback_vector_t > callback_map_t;
    typedef typename std::map<const char *, sr_register_field<DATA_TYPE> *> field_map_t;

    sr_register(const char *name, DATA_TYPE init_val, DATA_TYPE write_mask)
      : sc_register_b<DATA_TYPE>(name, init_val), m_write_mask(write_mask), m_access_mode(SC_REG_RW_ACCESS) {
    }
    sr_register(const char *name, const char * descr, DATA_TYPE init_val, DATA_TYPE write_mask)
      : sc_register_b<DATA_TYPE>(name, init_val, descr), m_write_mask(write_mask), m_access_mode(SC_REG_RW_ACCESS) {
    }

    ~sr_register() {
      for(callback_map_t::iterator iter = m_callbacks.begin(); iter != m_callbacks.end(); ++iter) {
        for(callback_vector_t::iterator item = iter->second.begin(); item != iter->second.end(); ++item) {
          delete *item;
        }
        iter->second.clear();
      }
      m_callbacks.clear();

      for(typename field_map_t::iterator iter = m_fields.begin(); iter != m_fields.end(); ++iter) {
        delete iter->second;
      }
    }

    template<typename OWNER>
    sr_register &callback(sr_register_callback_type type, OWNER *owner, typename sr_register_callback<OWNER>::callback_t callback) {
      callback_map_t::iterator iter = m_callbacks.find(type);
      if(iter != m_callbacks.end()) {
        iter->second.push_back(new sr_register_callback<OWNER>(owner, callback));
      } else {
        std::pair<sr_register_callback_type, callback_vector_t> pair;
        pair.first = type;
        pair.second.push_back(new sr_register_callback<OWNER>(owner, callback));
        m_callbacks.insert(pair);
      }
      return *this;
    }

    sr_register &create_field(const char *name, size_t start, size_t end) {
      m_fields.insert(std::make_pair(name, new sr_register_field<DATA_TYPE>(name, this, start, end)));
      return *this;
    }

    sr_register_field<DATA_TYPE> &field(const char *name) {
      return m_fields[name];
    }

    const DATA_TYPE &get_write_mask() {
      return m_write_mask;
    }

    void raise_callback(const sr_register_callback_type &type) const {
      callback_map_t::const_iterator map_iter = m_callbacks.find(type);
      if (map_iter != m_callbacks.end()) {
        for (callback_vector_t::const_iterator iter = map_iter->second.begin(); iter != map_iter->second.end(); ++iter) {
          (*iter)->call();
        }
      }
    }

    void bus_read(DATA_TYPE& i) const {
      raise_callback(SR_PRE_READ);
      i = this->read();
      raise_callback(SR_POST_READ);
    }

    DATA_TYPE bus_read() const {
      DATA_TYPE i;
      raise_callback(SR_PRE_READ);
      i = this->read();
      raise_callback(SR_POST_READ);
      return i;
    }

    void bus_write(DATA_TYPE i) {
      raise_callback(SR_PRE_WRITE);
      this->write(i & m_write_mask);
      raise_callback(SR_POST_WRITE);
    }
    bool bit(int pos) const {
      return this->read() & (1 << pos);
    }

    void bit(int pos, bool value) {
      this->write((this->read() & ~(1 << pos)) | (value << pos));
    }

    virtual sc_register_access_mode access_mode() const { return m_access_mode; }

    operator DATA_TYPE() {
      return this->read();
    }

    /// Add/Delete Callback objects associated with this region
    virtual scireg_ns::scireg_response scireg_add_callback(scireg_ns::scireg_callback& cb) {
      if(cb.type == scireg_ns::SCIREG_READ_ACCESS || cb.type == scireg_ns::SCIREG_STATE_CHANGE) {
        callback_map_t::iterator iter = m_callbacks.find(SR_PRE_READ);
        if(iter != m_callbacks.end()) {
          iter->second.push_back(new sr_register_scireg_callback(cb, *this));
        } else {
          std::pair<sr_register_callback_type, callback_vector_t> pair;
          pair.first = SR_PRE_READ;
          pair.second.push_back(new sr_register_scireg_callback(cb, *this));
          m_callbacks.insert(pair);
        }
      } else if(cb.type == scireg_ns::SCIREG_WRITE_ACCESS || cb.type == scireg_ns::SCIREG_STATE_CHANGE) {
        callback_map_t::iterator iter = m_callbacks.find(SR_POST_WRITE);
        if(iter != m_callbacks.end()) {
          iter->second.push_back(new sr_register_scireg_callback(cb, *this));
        } else {
          std::pair<sr_register_callback_type, callback_vector_t> pair;
          pair.first = SR_POST_WRITE;
          pair.second.push_back(new sr_register_scireg_callback(cb, *this));
          m_callbacks.insert(pair);
        }
      }
      return scireg_ns::SCIREG_SUCCESS;
    }

    virtual scireg_ns::scireg_response scireg_remove_callback(scireg_ns::scireg_callback &cb) {
      return scireg_ns::SCIREG_UNSUPPORTED;
    }

    sr_register<DATA_TYPE> &operator = (const DATA_TYPE &val) {
      this->write(val);
      return *this;
    }

  private:
    void before_end_of_elaboration() {
      this->check_and_init();
    }

    callback_map_t m_callbacks;
    field_map_t m_fields;
    DATA_TYPE m_write_mask;
    sc_register_access_mode m_access_mode;
};

template<typename ADDR_TYPE, typename DATA_TYPE>
class sr_register_bank : public sc_register_bank<ADDR_TYPE, DATA_TYPE> {
  public:
    typedef typename std::map<ADDR_TYPE, sr_register<DATA_TYPE> *> register_map_t;

    sr_register_bank(const char* name) :
      sc_register_bank<ADDR_TYPE, DATA_TYPE>(name, 0) {
    }

    ~sr_register_bank() {
      m_registers.clear();
      for(typename register_map_t::iterator iter = m_register.begin(); iter != m_register.end(); iter++) {
        delete iter->second;
      }
      m_register.clear();
    }

    sr_register<DATA_TYPE> &create_register(const char *name, ADDR_TYPE addr, DATA_TYPE init_val, DATA_TYPE write_mask) {
      sr_hierarchy_push(this);
      sr_register<DATA_TYPE> *reg = new sr_register<DATA_TYPE>(name, init_val, write_mask);
      sr_hierarchy_pop();
      m_register[addr] = reg;
      m_registers.push_back(reg);
      this->m_size = m_registers.size();
      return *reg;
    }

    sr_register<DATA_TYPE> &create_register(const char *name, const char *descr, ADDR_TYPE addr, DATA_TYPE init_val, DATA_TYPE write_mask) {
      sr_hierarchy_push(this);
      sr_register<DATA_TYPE> *reg = new sr_register<DATA_TYPE>(name, descr, init_val, write_mask);
      sr_hierarchy_pop();
      m_register.insert(std::make_pair(addr, reg));
      m_registers.push_back(reg);
      this->m_size = m_registers.size();
      return *reg;
    }

    bool bus_read(ADDR_TYPE offset, DATA_TYPE& i) const {
      const sr_register<DATA_TYPE> *reg = get_sr_register(offset);
      if(reg) {
        reg->bus_read(i);
      } else {
        i = 0;
      }
      return true;
    }
    bool bus_write(ADDR_TYPE offset, DATA_TYPE val) {
      sr_register<DATA_TYPE> *reg = get_sr_register(offset);
      if(reg) {
        reg->bus_write(val);
      }
      return true;
    }

    bool bus_read_dbg(ADDR_TYPE offset, DATA_TYPE& i) const {
      const sr_register<DATA_TYPE> *reg = get_sr_register(offset);
      if(reg) {
        reg->bus_read(i);
      } else {
        i = 0;
      }
      return true;
    }

    bool bus_write_dbg(ADDR_TYPE offset, DATA_TYPE val) {
      sr_register<DATA_TYPE> *reg = get_sr_register(offset);
      if(reg) {
        reg->bus_write(val);
      }
      return true;
    }

    bool is_valid_offset(ADDR_TYPE offset) const {
      return get_sr_register(offset) != NULL;
    }

    bool supports_action_type(ADDR_TYPE offset, sc_register_access_type mode) {
      sr_register<DATA_TYPE> *reg = get_sr_register(offset);
      return reg != NULL && reg->access_mode() & mode;
    }

    const sc_register_vec& get_registers() const { return m_registers; }

    const sr_register<DATA_TYPE> *get_sr_register(const ADDR_TYPE &offset) const {
      typename register_map_t::const_iterator item = m_register.find(offset);
      if (item != m_register.end()) {
          return item->second;
      }
      return NULL;
    }

    sr_register<DATA_TYPE> *get_sr_register(const ADDR_TYPE &offset) {
      typename register_map_t::iterator item = m_register.find(offset);
      if (item != m_register.end()) {
          return item->second;
      }
      return NULL;
    }
    sc_register_base* get_register(const ADDR_TYPE offset) {
      return get_sr_register(offset);
    }

    sr_register<DATA_TYPE> &operator[](ADDR_TYPE offset) {
      sr_register<DATA_TYPE> *reg = get_sr_register(offset);
      if(!reg) {
        std::stringstream ss;
        ss << "Register at offset " << offset << " not found";
        throw std::out_of_range(ss.str());
      }
      return *reg;
    }
/*
    sr_register<DATA_TYPE> &operator[](const ADDR_TYPE &offset) {
      sr_register<DATA_TYPE> *reg = get_sr_register(offset);
      if(!reg) {
        v::info << this->name() << "Reg not found " << offset << v::endl;
        throw std::out_of_range("Register at offset not found");
      }
      return *reg;
    }
*/
    bool get_offset(sc_register_base *reg, DATA_TYPE& offset) const {
      if (reg == NULL) {
          return false;
      }
      for (typename register_map_t::const_iterator iter = m_register.begin(); iter!=m_register.end(); iter++) {
        if (reinterpret_cast<void* const*>(&iter->second) == reinterpret_cast<void* const*>(reg)) {
          offset = iter->first;
          return true;
        }
      }
      return false;
    }

    sc_dt::uint64 scireg_get_bit_width() const {
      return this->size() * sizeof(DATA_TYPE) * 8;
    }

    /// Read a vector of "size" bytes at given offset in this region:
    scireg_ns::scireg_response scireg_read(scireg_ns::vector_byte& v, sc_dt::uint64 size, sc_dt::uint64 offset=0) const {
      uint64_t idx = 0;
      for(idx = 0; idx < size; idx += sizeof(DATA_TYPE)) {
        DATA_TYPE data = 0;
        if(bus_read_dbg(offset+idx, data)) {
          memcpy(&v[idx], &data, std::min(sizeof(DATA_TYPE), size_t(size-idx)));
        } else {
          break;
        }
      }
      if(idx>=size) {
        return scireg_ns::SCIREG_SUCCESS;
      } else {
        return scireg_ns::SCIREG_FAILURE;
      }
    }

    /// Write a vector of "size" bytes at given offset in this region:
    scireg_ns::scireg_response scireg_write(const scireg_ns::vector_byte& v, sc_dt::uint64 size, sc_dt::uint64 offset=0) {
      uint64_t idx = 0;
      for(idx = 0; idx < size; idx += sizeof(DATA_TYPE)) {
        DATA_TYPE data = 0;
        if(bus_read_dbg(offset+idx, data)) {
          memcpy(&data, &v[idx], std::min(sizeof(DATA_TYPE), size_t(size-idx)));
          if(!bus_write_dbg(offset+idx, data)) {
            break;
          }
        } else {
          break;
        }
      }
      if(idx>=size) {
        return scireg_ns::SCIREG_SUCCESS;
      } else {
        return scireg_ns::SCIREG_FAILURE;
      }
    }


  protected:
    sc_register_vec m_registers;
    register_map_t m_register;
};

#endif  // CORE_COMMON_SR_REGISTER_H_
/// @}
