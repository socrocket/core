// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_register
/// @{
/// @file sc_register.cpp
/// @date 2011-2015
/// @author Cadence Design Systems, Inc.
/// @author STMicroelectronics
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
/// This file contains an example of a user register class, which can be used
/// to model registers and register banks in a TLM model.
/// This user register register class is not part of the SCIREG API standard.
/// Instead, this user register class is provided as an example of how the
/// SCIREG API standard can be used.
#ifndef SC_REGISTER_H_
#define SC_REGISTER_H_

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <assert.h>

#include "systemc"
#include "scireg.h"

using sc_core::SC_ID_ASSERTION_FAILED_;

#define MAX_DESC_LENGTH 64

/// Register Access Mode
enum sc_register_access_mode {
  SC_REG_RW_ACCESS = 0,
  SC_REG_RO_ACCESS,
  SC_REG_WO_ACCESS
};

enum sc_register_access_type {
  SC_REG_UNKNOWN_ACCESS_TYPE,
  SC_REG_READ,
  SC_REG_WRITE
};

// Forward declaration

/// Register field
/// Non-template base class of register field.
/// It allow non-template function to get fields of a register.
class sc_register_field_base;

/// A template base class derived from sc_register_field_base.
/// T is the data type of register value.
/// read/write methods of a register field is defined in this class.
template<typename T>
class sc_register_field_b;

/// Register field is argumented by T (datatype), bit position or
/// bit position range in T. T is the datatype of the register when it
/// is transferred on the bus.
template<typename T, unsigned int lowpos, unsigned int highpos>
class sc_register_field;

/// Register
/// A non-template register base class.
/// It allow non-template function to get a register.
class sc_register_base;

/// T is the datatype of register. It models the register value transferred
/// over bus.
template<typename T>
class sc_register_b;

/// M is the access mode.
template<typename T, sc_register_access_mode M>
class sc_register;
/// User register class is derived from sc_register<>.
/// It adds sc_register_field<> members.

/// Register field
/// sc_register_field_base is a non-template base class of register field.
/// Its method gives the declarative information of the field.
/// This separates field's declarative and simulation API. The declarative
/// API is in base class, and simulation API is in derived class.
class sc_register_field_base : public scireg_ns::scireg_region_if {
  public:
    sc_register_field_base(const char* nm) : m_name(nm), m_desc() {}

    sc_register_field_base(const char* nm, const char* desc) : m_name(nm), m_desc() {
      if (desc) {
        ::std::string d(desc);
        if (d.length() > MAX_DESC_LENGTH) {
          d.resize(MAX_DESC_LENGTH-4);
          d = d + " ...";
        }
        m_desc = d;
      }
    }
    virtual ~sc_register_field_base() {}

    const char *name() const { return m_name.c_str(); }
    const char *desc() const {
      if (m_desc.empty()) return NULL;
      else                return m_desc.c_str();
    }
    virtual unsigned int low_pos() const = 0;
    virtual unsigned int high_pos() const = 0;
    virtual unsigned int bit_width() const = 0;


    /// return mnemonic and desc, if any, for current value of field
    virtual const char* get_value_mnemonic() const = 0;
    virtual const char* get_value_desc() const = 0;

    /// sc_object style print() - print current value of field
    virtual void print(::std::ostream& os) const = 0;

  protected:

    sc_dt::uint64 scireg_get_low_pos() const { return low_pos(); }
    sc_dt::uint64 scireg_get_high_pos() const { return high_pos(); }
    sc_dt::uint64 scireg_get_bit_width() const { return (1 + high_pos() - low_pos()); }

    virtual scireg_ns::scireg_response scireg_get_region_type(scireg_ns::scireg_region_type& t) const { t = scireg_ns::SCIREG_FIELD; return scireg_ns::SCIREG_SUCCESS; }

    virtual scireg_ns::scireg_response scireg_get_string_attribute(const char *& s, scireg_ns::scireg_string_attribute_type t) const {
      switch (t) {
        case scireg_ns::SCIREG_NAME:
          s = name();
          return scireg_ns::SCIREG_SUCCESS;

        case scireg_ns::SCIREG_DESCRIPTION:
          s = desc();
          return scireg_ns::SCIREG_SUCCESS;

        case scireg_ns::SCIREG_STRING_VALUE:
          return scireg_ns::SCIREG_FAILURE;
      }

      return scireg_ns::SCIREG_FAILURE;
    }

  protected:
    ::std::string m_name;
    ::std::string m_desc;
};

class sc_rf_valuecode {
  public:
    sc_rf_valuecode(const ::std::string& mnemonic) : m_mnemonic(mnemonic), m_desc() {}

    sc_rf_valuecode(const std::string& mnemonic, const ::std::string& desc) : m_mnemonic(mnemonic), m_desc(desc) {
      if (desc.length() > MAX_DESC_LENGTH) {
        m_desc.resize(MAX_DESC_LENGTH-4);
        m_desc = m_desc + " ...";
      }
    }

    ~sc_rf_valuecode() {}

    const char* mnemonic() const {
      if (m_mnemonic.empty()) return NULL;
      else                    return m_mnemonic.c_str();
    }
    const char* desc() const {
      if (m_desc.empty()) return NULL;
      else                return m_desc.c_str();
    }

  protected:
    ::std::string  m_mnemonic;
    ::std::string  m_desc;
};

/// T is the data type of the register that contains this field.
template<typename T>
class sc_register_field_b : public sc_register_field_base {
  public:
    sc_register_field_b(const char* nm, sc_register_b<T> *r)
      : sc_register_field_base(nm),
        m_reg( r ),
        m_value_encoding_map() {
      // Add the field to the register's fields spec.
      m_reg->add_field( this );
    }
    sc_register_field_b(const char* nm, sc_register_b<T> *r, const char* desc)
      : sc_register_field_base(nm, desc),
        m_reg( r ),
        m_value_encoding_map() {
      // Add the field to the register's fields spec.
      m_reg->add_field( this );
    }

    virtual ~sc_register_field_b();

    /// Extract field value. value of the field is returned in [(bit_width-1):0] of T.
    /// The value returned is only valid before the next read of any field of the
    /// containing register.
    const T& read() const;

    /// Write [(bit_width-1):0] of T to field. The written value is retrievable after
    /// the register is updated.
    void write(const T& v);

    /// For value-encoding
    void add_value_code(const T& v, const ::std::string& n);
    void add_value_code(const T& v, const ::std::string& n, const ::std::string& desc);

    /// added by bpriya for debugging

    /// return mnemonic and desc, if any, for current value of field
    const char* get_value_mnemonic() const {
       return get_value_mnemonic(read());
    }
    const char* get_value_desc() const {
      return get_value_desc(read());
    }

    /// sc_object style print() - print current value of field
    /// using hex format
    virtual void print(::std::ostream& os) const {
      os << "0x" << ::std::hex << read() << '\0';
      return;
    }

    scireg_ns::scireg_response scireg_read(scireg_ns::vector_byte& v, sc_dt::uint64 size, sc_dt::uint64 offset) const {
      if (offset != 0) /* || (size != sizeof(T))) */
        return scireg_ns::SCIREG_FAILURE;

      v.resize(sizeof(T));

      *(T *)&v[0] = read();
      return scireg_ns::SCIREG_SUCCESS;
    }

    scireg_ns::scireg_response scireg_write(const scireg_ns::vector_byte& v, sc_dt::uint64 size, sc_dt::uint64 offset) {
      if ((offset != 0) || (size != sizeof(T)))
        return scireg_ns::SCIREG_FAILURE;

      T t = *(T *)&v[0];
      write(t);
      return scireg_ns::SCIREG_SUCCESS;
    }

  protected:
    virtual const T& get_mask() const = 0;
    virtual unsigned int get_shift_bits() const = 0;

    /// For value-encoding
    /// A field's value-encoding is optional. Furthermore, description of
    /// a value-encoding is optional.
    const sc_rf_valuecode* get_value_code(const T& v) const ;
    const char* get_value_mnemonic(const T& v) const ;
    const char* get_value_desc(const T& v) const ;

  protected:
    sc_register_b<T> *m_reg;
    /// Value-encoding map
    ::std::map<T,sc_rf_valuecode*> m_value_encoding_map;
};

/// Vector of register field.
typedef ::std::vector<sc_register_field_base *> sc_register_field_vec;

/// lsb0 mode is used in field position specification.
/// (lsb0 defines the 0 as the least significant bit. It is the default
/// position specification mode in IP-XACT systemRDL.
template<typename T, unsigned int highpos, unsigned int lowpos>
class sc_register_field : public sc_register_field_b<T> {
  public:
    sc_register_field( const char* name, sc_register_b<T>* c)
      : sc_register_field_b<T>( name, c ) {
      // Generate mask.
      gen_mask();
    }

    sc_register_field( const char* name, sc_register_b<T>* c, const char* desc)
      : sc_register_field_b<T>( name, c, desc ) {
      // Generate mask.
      gen_mask();
    }

    ~sc_register_field() {}

    //  const char *name(); // in sc_register_field_base.
    virtual unsigned int low_pos() const { return lowpos; }
    virtual unsigned int high_pos() const { return highpos; }
    virtual unsigned int bit_width() const { return highpos-lowpos+1; }

  protected:
    virtual const T& get_mask() const { return m_mask; }
    virtual unsigned int get_shift_bits() const { return low_pos(); }

  private:

    // Based on lowpos, highpos, generate mask of type T.
    void gen_mask() {
        assert( (lowpos <= highpos) && (highpos < sizeof(T)*8) );
        unsigned int width = highpos - lowpos + 1;
        unsigned int m0 = 0x1;
        m_mask = 0;
        for( unsigned int i = 0; i < width; i++) {
          m_mask = m_mask << 1 | m0;
        }
        m_mask = m_mask << lowpos;
    }

  protected:
    T                      m_mask;
};

/// Register
class sc_register_base : public sc_core::sc_prim_channel , public scireg_ns::scireg_region_if {
  public:
    sc_register_base() : sc_core::sc_prim_channel(sc_core::sc_gen_unique_name("register")), m_desc() {}

    sc_register_base(const char* name) : sc_core::sc_prim_channel(name), m_desc() {}

    sc_register_base(const char* name, const char* desc)
        : sc_prim_channel(name), m_desc() {
      if (desc) {
        ::std::string d(desc);
        if (d.length() > MAX_DESC_LENGTH) {
          d.resize(MAX_DESC_LENGTH);
          d = d + " ...";
        }
        m_desc = d;
      }
    }
    virtual ~sc_register_base() {}

    virtual const char* kind() const { return "sc_register"; }

    const char* desc() const {
      if (m_desc.empty()) return NULL;
      else                return m_desc.c_str();
    }

    virtual sc_register_access_mode access_mode() const = 0;

    virtual const sc_register_field_vec &get_all_fields() const = 0;

    static const char* access_type_to_str(sc_register_access_type accessType);
    static const char* access_mode_to_str(sc_register_access_mode accessMode);

  protected:

     virtual scireg_ns::scireg_response scireg_get_string_attribute(const char *& s, scireg_ns::scireg_string_attribute_type t) const {
       switch (t) {
       case scireg_ns::SCIREG_NAME:
         s = name();
         return scireg_ns::SCIREG_SUCCESS;
       case scireg_ns::SCIREG_DESCRIPTION:
         s = desc();
         return scireg_ns::SCIREG_SUCCESS;

       case scireg_ns::SCIREG_STRING_VALUE:
         return scireg_ns::SCIREG_FAILURE;
       }

       return scireg_ns::SCIREG_FAILURE;
     }

     virtual scireg_ns::scireg_response scireg_get_region_type(scireg_ns::scireg_region_type& t) const { t = scireg_ns::SCIREG_REGISTER; return scireg_ns::SCIREG_SUCCESS; }

    scireg_ns::scireg_response scireg_get_child_regions(std::vector<scireg_ns::scireg_mapped_region>& mapped_regions, sc_dt::uint64 size, sc_dt::uint64 offset) const {
       if ((size != sc_dt::uint64(-1)) || (offset != 0))
         return scireg_ns::SCIREG_FAILURE;

      const sc_register_field_vec& v = get_all_fields();

      sc_register_field_vec::const_iterator it;
      const scireg_region_if* p;

      for (it = v.begin(); it != v.end(); ++it) {
        if ((p = dynamic_cast<scireg_ns::scireg_region_if*>(*it)))
        {
          scireg_ns::scireg_mapped_region mr;
          mr.region = const_cast<scireg_ns::scireg_region_if*>(p);
          static const char* s = "";
          p->scireg_get_string_attribute(s, scireg_ns::SCIREG_NAME);
          mr.name = s;
          mr.offset = 0;
          mapped_regions.push_back(mr);
        }
      }

      return scireg_ns::SCIREG_SUCCESS;
    }

    virtual scireg_ns::scireg_response scireg_get_parent_modules(std::vector<sc_core::sc_module*>& v) const {
      sc_core::sc_module* m = dynamic_cast<sc_core::sc_module*>(get_parent_object());
      v.push_back(m);
      return scireg_ns::SCIREG_SUCCESS;
    }

  protected:
    ::std::string m_desc;
};

/// A register's base class, argumented with T (datatype), implements
/// register APIs.
///
/// Register's read()/write() methods are defined in this class, instead of
/// in sc_register<T,M>, reduces the number of specialazations of these
/// methods.
template<typename T>
class sc_register_b : public sc_register_base {
  public:
    sc_register_b(const char* name, const T& reset_val)
      : sc_register_base(name),
        m_reset_val(reset_val),
        m_probe_event(0)
    { }

    sc_register_b(const char* name, const T& reset_val, const char* desc)
      : sc_register_base(name, desc),
        m_reset_val(reset_val),
        m_probe_event(0)
    { }

    virtual ~sc_register_b() {}

    //void reset() { m_cur_val = m_reset_val & get_mask(); }
    // go thru write() so that any value change callbacks are triggered
    void reset() { write(m_reset_val); }

    // read() and write() methods
    // Register access control is enforced in the sc_module that instantiates
    // sc_register.
    virtual const T& read() const;
    virtual void write(const T&);

    // Other methods declaraed in sc_prim_channel.
    void update() {}

  public:
    unsigned int number_of_fields() const { return m_fields.size(); }
    const sc_register_field_vec &get_all_fields() const { return m_fields; }

  protected:
    friend class sc_register_field_b<T>;

    void add_field(sc_register_field_base *f ) { m_fields.push_back(f); }

    // -- for reg_field read/write, access value with mask.
    virtual const T& mask_read( const T&, unsigned int) const;
    virtual void mask_write(const T&, const T&, unsigned int);

  protected:
    void check_and_init();

  public:
    virtual const T& get_mask() const { return m_mask; }

    scireg_ns::scireg_response scireg_read(scireg_ns::vector_byte& v, sc_dt::uint64 size, sc_dt::uint64 offset) const {
      if ((offset != 0) || (size != sizeof(T)))
        return scireg_ns::SCIREG_FAILURE;

      v.resize(sizeof(T));

      //*(T *)&v[0] = read(); // avoid to trigger callbacks while reading from callback
      *(T *)&v[0] = m_cur_val;
      return scireg_ns::SCIREG_SUCCESS;
    }

    scireg_ns::scireg_response scireg_write(const scireg_ns::vector_byte& v, sc_dt::uint64 size, sc_dt::uint64 offset) {
      if ((offset != 0) || (size != sizeof(T)))
        return scireg_ns::SCIREG_FAILURE;

      //T t = *(T *)&v[0];
      //write(t); avoid endless recursion while writing from callback
      m_cur_val = ((*(T *)&v[0]) & get_mask());

      return scireg_ns::SCIREG_SUCCESS;
    }

    sc_dt::uint64 scireg_get_bit_width() const { return 8 * sizeof(T); }
    scireg_ns::scireg_response scireg_add_callback(scireg_ns::scireg_callback& cb) {
      scireg_callback_vec.push_back(&cb);  return scireg_ns::SCIREG_SUCCESS;
    }

    scireg_ns::scireg_response scireg_remove_callback(scireg_ns::scireg_callback& cb) {
      ::std::vector<scireg_ns::scireg_callback*>::iterator it;
      it = find(scireg_callback_vec.begin(), scireg_callback_vec.end(), &cb);
      if (it != scireg_callback_vec.end())
        scireg_callback_vec.erase(it);
      return scireg_ns::SCIREG_SUCCESS;
    }

  protected:
    T  m_cur_val;
    T  m_reset_val;
    sc_core::sc_event* m_probe_event;
    ::std::vector<scireg_ns::scireg_callback*> scireg_callback_vec;

    // Mask on T to get effective bits of the register. Some register does not
    // use all bits in T.
    T  m_mask;

    sc_register_field_vec m_fields;
};

template< typename T, sc_register_access_mode M >
class sc_register : public sc_register_b<T> {
  private:
    sc_register(const char *name, const T& reset_val )
      : sc_register_b<T>(name, reset_val)
    {}

    sc_register(const char *name, const T& reset_val, const char* desc )
      : sc_register_b<T>(name, reset_val, desc)
    {}

    virtual sc_register_access_mode access_mode() const { return M; }
    void before_end_of_elaboration();

};

/// Register Bank
typedef std::vector<sc_register_base *> sc_register_vec;

class sc_register_bank_base : public sc_core::sc_object , public scireg_ns::scireg_region_if {
  public:
    sc_register_bank_base()
      : sc_core::sc_object(sc_core::sc_gen_unique_name("register_bank")), m_size(0) {}
    sc_register_bank_base(unsigned long long size)
      : sc_core::sc_object(sc_core::sc_gen_unique_name("register_bank")), m_size(size) {}
    sc_register_bank_base(const char* name, const unsigned long long size)
      : sc_core::sc_object(name), m_size(size) {}
    virtual ~sc_register_bank_base() {}

    virtual const char* kind() const { return "sc_register_bank"; }

    // Get register bank size in bytes
    unsigned long long size() const { return m_size; }

    bool add_associate_busport( sc_object* busport );

  protected:
    virtual const sc_register_vec& get_registers() const = 0;

    // print offset of register in bank; need this version in base class
    // because offset is templated by ADDR_TYPE
    virtual void print_offset(sc_register_base* reg, ::std::ostream& os) const = 0;

    const std::vector<sc_object*>& get_associate_busports() const
        { return m_associate_busports; }

  protected:

     virtual scireg_ns::scireg_response scireg_get_string_attribute(const char *& s, scireg_ns::scireg_string_attribute_type t) const
     {
       switch (t)
       {
       case scireg_ns::SCIREG_NAME:
         s = name();
         return scireg_ns::SCIREG_SUCCESS;
       default:
         return scireg_ns::SCIREG_FAILURE;
       }
     }

     virtual sc_dt::uint64 scireg_get_byte_width() const { return size(); }
     virtual scireg_ns::scireg_response scireg_get_region_type(scireg_ns::scireg_region_type& t) const { t = scireg_ns::SCIREG_BANK ; return scireg_ns::SCIREG_SUCCESS; }

     virtual scireg_ns::scireg_response scireg_get_target_sockets(::std::vector<sc_core::sc_object*>& v) const
     {
       v = get_associate_busports();
       return scireg_ns::SCIREG_SUCCESS;
     }

    virtual scireg_ns::scireg_response scireg_get_parent_modules(std::vector<sc_core::sc_module*>& v) const
    {
      sc_core::sc_module* m = dynamic_cast<sc_core::sc_module*>(get_parent_object());
      v.push_back(m);
      return scireg_ns::SCIREG_SUCCESS;
    }

  protected:
    unsigned long long        m_size;
    ::std::vector<sc_core::sc_object*> m_associate_busports;
};

template< typename ADDR_TYPE, typename DATA_TYPE >
class sc_register_bank : public sc_register_bank_base {
  public:
    sc_register_bank(const unsigned long long size)
      : sc_register_bank_base(size) {}
    sc_register_bank(const char* name, const unsigned long long size)
      : sc_register_bank_base(name,size) {}
    virtual ~sc_register_bank() {}

    // Register Read/Write Interface for Bus Access
    // The methods accept offset address that corresponds to a register
    // allowing the access.
    virtual bool bus_read(const ADDR_TYPE offset, DATA_TYPE &data) const = 0;
    virtual bool bus_write(const ADDR_TYPE offset, const DATA_TYPE data) = 0;

    // Register Debug Read/Write Interface for Bus Access
    // These methods accept every valid offset address regardless of the
    // access mode of the corresponding register.
    virtual bool bus_read_dbg(const ADDR_TYPE offset, DATA_TYPE &data) const = 0;
    virtual bool bus_write_dbg(const ADDR_TYPE offset, const DATA_TYPE data) = 0;

    virtual bool is_valid_offset(const ADDR_TYPE offset) const = 0;
    virtual bool supports_action_type(const ADDR_TYPE offset, const sc_register_access_type accType) = 0;

  protected:
    virtual sc_register_base* get_register(const ADDR_TYPE offset) = 0;
    virtual bool get_offset(sc_register_base* reg, ADDR_TYPE& offset) const = 0;
    virtual void print_offset(sc_register_base* reg, ::std::ostream& os) const;

    scireg_ns::scireg_response scireg_get_child_regions(std::vector<scireg_ns::scireg_mapped_region>& mapped_regions, sc_dt::uint64 size, sc_dt::uint64 offset) const
    {
       const sc_register_vec& v = get_registers();

       if ((size != sc_dt::uint64(-1)) || (offset != 0))
         return scireg_ns::SCIREG_FAILURE;

       sc_register_vec::const_iterator it;
       const scireg_region_if* p;

       for (it = v.begin(); it != v.end(); ++it) {
        if ((p = dynamic_cast<scireg_ns::scireg_region_if*>(*it))) {
          scireg_ns::scireg_mapped_region mr;
          mr.region = const_cast<scireg_ns::scireg_region_if*>(p);
          static const char* s = "";
          p->scireg_get_string_attribute(s, scireg_ns::SCIREG_NAME);
          mr.name = s;
          ADDR_TYPE a;
          bool r = get_offset((*it), a);
          sc_assert(r);
          if(r) {
            sc_dt::uint64 offset = a;
            mr.offset = offset;
            mapped_regions.push_back(mr);
          }
        }
      }
      return scireg_ns::SCIREG_SUCCESS;
    }

};

// inline functions

// static functions
inline const char*
sc_register_base::access_type_to_str(sc_register_access_type accessType) {
  if(accessType==SC_REG_READ) return "Read-Access-Type";
  if(accessType==SC_REG_WRITE) return "Write-Access-Type";
  return "Unknown-Access-Type";
}

inline const char*
sc_register_base::access_mode_to_str(sc_register_access_mode accessMode) {
  if(accessMode==SC_REG_RW_ACCESS) return "Read-Write-Mode";
  if(accessMode==SC_REG_RO_ACCESS) return "Read-Only-Mode";
  if(accessMode==SC_REG_WO_ACCESS) return "Write-Only-Mode";
  return "Unknown-Access-Mode";
}

// sc_register_bank class
inline bool
sc_register_bank_base::add_associate_busport(sc_core::sc_object* obj) {
  if( !obj ) return false;
  if( dynamic_cast<sc_core::sc_export_base*>(obj) == NULL ) return false;
  m_associate_busports.push_back(obj);
  return true;
}

template< typename ADDR_TYPE, typename DATA_TYPE >
inline void
sc_register_bank<ADDR_TYPE,DATA_TYPE>::print_offset(sc_register_base* reg,
                                                    ::std::ostream& os) const {
  ADDR_TYPE offs;
  if( get_offset(reg, offs) ) {
    os << "0x" << ::std::hex << offs << '\0';
  }
  else {
    os << "N/A\0";
  }
}

// sc_register class
template<class T>
inline const T&
sc_register_b<T>::read() const {
  // check read access against mode, this->access_mode()
  scireg_ns::scireg_callback* p;
  ::std::vector<scireg_ns::scireg_callback*>::const_iterator it;
  for (it = scireg_callback_vec.begin(); it != scireg_callback_vec.end(); ++it)
  {
    p = *it;
    if (p->type == scireg_ns::SCIREG_READ_ACCESS)
      p->do_callback(*const_cast<scireg_ns::scireg_region_if *>(static_cast<const scireg_ns::scireg_region_if *>(this)));
  }
  return m_cur_val;
}

template<typename T>
inline const T&
sc_register_b<T>::mask_read(const T& mask, unsigned int rightShiftBits ) const {
  static T v;
  v = (m_cur_val & mask) >> rightShiftBits;
  return v;
}

template<typename T>
inline void
sc_register_b<T>::write( const T& v ) {
  // check write access against mode, this->access_mode().
  // Mask the value so only bits allocated to fields are set.
  // The unused bits are set to 0.
  T old_val = m_cur_val;
  m_cur_val = (v & get_mask());
  if (m_probe_event && (m_cur_val != old_val))
    m_probe_event->notify();

  if ((m_cur_val != old_val))
  {
    scireg_ns::scireg_callback* p;
   ::std::vector<scireg_ns::scireg_callback*>::iterator it;
   for (it = scireg_callback_vec.begin(); it != scireg_callback_vec.end(); ++it)
   {
     p = *it;
     if (p->type == scireg_ns::SCIREG_WRITE_ACCESS)
       p->do_callback(*this);
   }
  }
}

template<typename T>
inline void
sc_register_b<T>::mask_write( const T& v, const T& mask, unsigned int leftShiftBits ) {
  //m_cur_val = ((v << leftShiftBits) & mask) | (m_cur_val & ~mask);
  this->write( ((v << leftShiftBits) & mask) | (m_cur_val & ~mask) );
}

template<typename T>
inline void
sc_register_b<T>::check_and_init() {
  // Check fields' bit positions - must not have overlapping; must within
  // the the bit span of T.
  // Determine register bit-width.
  // Generate register mask from its fields' bit ranges.
  sc_register_field_vec sortedFields;
  int numberOfFields = m_fields.size();
  sc_register_field_base *f;
  for (int i = 0; i < numberOfFields; i++) {
    f = m_fields[i];
    sc_register_field_vec::iterator fite = sortedFields.begin();
    while( fite != sortedFields.end() ) {
      // WIP: Report error: two fields take the same bit.
      //// assert (f->high_pos() != (*fite)->high_pos());

      if (f->high_pos() > (*fite)->high_pos()) {
        // Found the idx to insert.
        // WIP: Report error: two fields position overlap.
        assert (f->low_pos() > (*fite)->high_pos());
        break;
      }
      ++fite;
    }
    sortedFields.insert(fite, f);
  }
  for( int j = 0; j < numberOfFields; j++ ) {
    m_fields[j] = sortedFields[j];
  }

  if( numberOfFields > 0 ) {
    m_mask = 0;
    for( int f = 0; f < numberOfFields; f++ ) {
      unsigned int fwidth = m_fields[f]->high_pos() - m_fields[f]->low_pos() + 1;
      T fmask = 0;
      for (unsigned int p = 0; p < fwidth; p++) {
        fmask = fmask << 1 | 0x1;
      }
      fmask = fmask << m_fields[f]->low_pos();
      m_mask = m_mask | fmask;
    }
  }
  else {
    m_mask = (T) ~0x0;
  }

  //if (!m_probe_event)
    //m_probe_event = new sc_event(this, "probe");
  // Set m_cur_val to m_reset_val
  write(m_reset_val);
}

template< typename T, sc_register_access_mode M >
inline void
sc_register<T,M>::before_end_of_elaboration() {
  this->check_and_init();
}

// sc_register_field class

// Destructor
template<typename T>
inline
sc_register_field_b<T>::~sc_register_field_b() {
  typename ::std::map< T, sc_rf_valuecode* >::iterator it;
  it = m_value_encoding_map.begin();
  while( it != m_value_encoding_map.end()) {
    delete it->second;
    ++it;
  }
}

// Extract field value. value of the field is returned in [(bit_width-1):0] of T.
template<typename T>
inline const T&
sc_register_field_b<T>::read() const {
  return( m_reg->mask_read(get_mask(),get_shift_bits()) );
}

// Write [(bit_width-1):0] of T to field. The written value is retrievable after
// the register is updated.
template<typename T>
inline void
sc_register_field_b<T>::write(const T& v) {
  m_reg->mask_write(v,get_mask(),get_shift_bits());
}

template<typename T>
inline void
sc_register_field_b<T>::add_value_code(const T& v, const ::std::string& n) {
  ::std::string d;
  add_value_code(v, n, d);
}

template<typename T>
inline void
sc_register_field_b<T>::add_value_code(const T& v,
                                      const ::std::string& n,
                                      const ::std::string& desc) {
  sc_rf_valuecode *vc = new sc_rf_valuecode(n, desc);
  m_value_encoding_map[v] = vc;
}

template<typename T>
inline const sc_rf_valuecode*
sc_register_field_b<T>::get_value_code(const T& v) const {
  return( ((::std::map<T, sc_rf_valuecode*>)m_value_encoding_map)[v] );
}

template<typename T>
inline const char*
sc_register_field_b<T>::get_value_mnemonic(const T& v) const {
  const sc_rf_valuecode *vc = get_value_code(v);
  if (!vc) return NULL;
  else     return vc->mnemonic();
}

template<typename T>
inline const char*
sc_register_field_b<T>::get_value_desc(const T& v) const {
  const sc_rf_valuecode *vc = get_value_code(v);
  if (!vc) return NULL;
  else     return vc->desc();
}

// Specialize for unsigned char.
template <>
inline
void sc_register_field_b<unsigned char>::print(::std::ostream& os) const {
  os << "0x" << ::std::hex << (unsigned short) read() << '\0';
  return;
}

//
// Specializations of sc_register class
//

template< sc_register_access_mode M >
class sc_register<unsigned char, M> : public sc_register_b<unsigned char> {
  public:
    sc_register(const char *name, const unsigned char& reset_val )
      : sc_register_b<unsigned char>(name, reset_val)
    {}

    sc_register(const char *name, const unsigned char& reset_val, const char* desc )
      : sc_register_b<unsigned char>(name, reset_val, desc)
    {}

    virtual sc_register_access_mode access_mode() const { return M; }
    void before_end_of_elaboration() { this->check_and_init(); }
};

template< sc_register_access_mode M >
class sc_register<unsigned short, M> : public sc_register_b<unsigned short> {
  public:
    sc_register(const char *name, const unsigned short& reset_val )
      : sc_register_b<unsigned short>(name, reset_val)
    {}

    sc_register(const char *name, const unsigned short& reset_val, const char* desc )
      : sc_register_b<unsigned short>(name, reset_val, desc)
    {}

    virtual sc_register_access_mode access_mode() const { return M; }
    void before_end_of_elaboration() { this->check_and_init(); }
};

template< sc_register_access_mode M >
class sc_register<unsigned int, M>: public sc_register_b<unsigned int> {
  public:
    sc_register(const char *name, const unsigned int& reset_val )
      : sc_register_b<unsigned int>(name, reset_val)
    {}

    sc_register(const char *name, const unsigned int& reset_val, const char* desc )
      : sc_register_b<unsigned int>(name, reset_val, desc)
    {}

    virtual sc_register_access_mode access_mode() const { return M; }
    void before_end_of_elaboration() { this->check_and_init(); }
};

template< sc_register_access_mode M >
class sc_register<unsigned long long, M> : public sc_register_b<unsigned long long> {
  public:
    sc_register(const char *name, const unsigned long long& reset_val )
      : sc_register_b<unsigned long long>(name, reset_val)
    {}

    sc_register(const char *name, const unsigned long long& reset_val, const char* desc )
      : sc_register_b<unsigned long long>(name, reset_val, desc)
    {}

    virtual sc_register_access_mode access_mode() const { return M; }
    void before_end_of_elaboration() { this->check_and_init(); }
};

#endif
