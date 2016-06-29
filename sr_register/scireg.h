// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_register
/// @{
/// @file scireg.h
/// @date 2011-2012
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
/// This file contains the SCIREG API ("SystemC Inspect Region API") which is a SystemC tool to model API
/// that enables tools to discover and monitor the various registers, register banks, fields, and memories
/// in a design. 
#ifndef CORE_COMMON_SCIREG_H_
#define CORE_COMMON_SCIREG_H_
#include <algorithm>
#include <vector>
#include "systemc"

#ifndef NC_SYSTEMC // Only use this file if we are not running in incisiv
namespace scireg_ns {

class scireg_region_if;
typedef std::vector<unsigned char> vector_byte;

/// The scireg_notification_if is used to notify tools of additions/deletions of scireg region objects
class scireg_notification_if {
  public:
    virtual ~scireg_notification_if() {}

    virtual void add_region(scireg_region_if & region) = 0;
    virtual void remove_region(scireg_region_if & region) = 0;
};

/// The scireg_tool_registry is a global singleton that broadcasts all additions and deletions
/// of all regions to all tools that have requested notification
class scireg_tool_registry {
  public:
    static void add_tool(scireg_notification_if & notification);

    static void remove_tool(scireg_notification_if & notification);

    static void add_region(scireg_region_if & region);

    static void remove_region(scireg_region_if & region);

  protected:
    static std::vector<scireg_notification_if*>* tool_vec_ptr;

    static void check_allocation();
    static void destroy();
};

/// A region represents a SystemC object that occupies a region (ie portion) of the address space within a SystemC model.
/// The SCIREG API uses a single interface (scireg_region_if) to represent all types of regions.
/// The types of regions are:
enum scireg_region_type {
    SCIREG_MEMORY           ///@< A Memory
  , SCIREG_BANK             ///@< A Register Bank
  , SCIREG_REGISTER         ///@< A Register
  , SCIREG_FIELD            ///@< A Register Field
  , SCIREG_STRING_REGISTER  ///@< A string register (i.e. an object in a model which contains textual string values to be made available to tools)
} ;

/// SCIREG API response codes:
enum scireg_response {
    SCIREG_SUCCESS          ///@< Function succeeded.
  , SCIREG_FAILURE          ///@< Function failed at runtime.
  , SCIREG_UNSUPPORTED      ///@< Function failed since it is unsupported at compile time.
};

/// Attributes assocated with individual bits within regions:
enum scireg_bit_attributes_type {
    SCIREG_READABLE         ///@< Bit is readable from perspective of HW Bus
  , SCIREG_WRITEABLE        ///@< Bit is writeable from perspective of HW Bus
};

/// String attributes associated with region objects:
enum scireg_string_attribute_type {
    SCIREG_NAME             ///@< Name of object (typically short identifier, not fully rooted name)
  , SCIREG_DESCRIPTION      ///@< Textual description (typically short phrase or sentence).
  , SCIREG_STRING_VALUE     ///@< String Value stored in a string register
};


/// A mapped region represents a child region that has been mapped into a parent region at a particular offset.
/// Each mapping may be associated with a unique name for the child region, therefore a name is included in the mapped region object.
struct scireg_mapped_region {
 scireg_region_if* region;  ///@< the child region
 const char* name;          ///@< name of the child region associated with this mapping
 sc_dt::uint64 offset;      ///@< offset of the child region associated with this mapping
};

/// Registers and register fields may have textual "mnemonics" associated with particular values. The value info object
/// represents the textual mnemonic and description (ie arbitrary descriptive text) associated with particular values.
struct scireg_value_info {
  vector_byte value;        ///@< The specific value for the register or register field.
  const char* mnemonic;     ///@< The textual mnemonic. (Typically a short identifier)
  const char* description;  ///@< The textual description. (Typically a descriptive phrase).
};


/// The types of callbacks that can be assocated with regions:
enum scireg_callback_type {
    SCIREG_READ_ACCESS      ///@< Execute callback when read occurs in region - does not include reads executed via DMI
  , SCIREG_WRITE_ACCESS     ///@< Execute callback when write occurs in region - does not include writes executed via DMI
  , SCIREG_STATE_CHANGE     ///@< Execute callback when there is ANY kind of state change associated with region
} ;


/// The scireg_callback is an abstract base class that is used to implement callbacks to tools
class scireg_callback {
  public:
    virtual ~scireg_callback() {}
    virtual void do_callback(scireg_region_if & region) = 0;   /// r represents the region object where the callback was originally added

  scireg_callback_type type;   ///@< The type of the callback that the tool is requesting
  sc_dt::uint64 offset;        ///@< offset: used by tool requesting the callback to constrain the range of the callback
  sc_dt::uint64 size;          ///@< size: used by tool requesting the callback to constrain the range of the callback
};


class scireg_region_if {
  public:
    scireg_region_if() {
      scireg_tool_registry::add_region(*this);
    }

    virtual ~scireg_region_if() {
      scireg_tool_registry::remove_region(*this);
    }

    /// Get the region_type of this region:
    virtual scireg_response scireg_get_region_type(scireg_region_type& t) const {
      return SCIREG_UNSUPPORTED;
    }

    /// Read a vector of "size" bytes at given offset in this region:
    virtual scireg_response scireg_read(vector_byte& v, sc_dt::uint64 size, sc_dt::uint64 offset=0) const {
      return SCIREG_UNSUPPORTED;
    }

    /// Write a vector of "size" bytes at given offset in this region:
    virtual scireg_response scireg_write(const vector_byte& v, sc_dt::uint64 size, sc_dt::uint64 offset=0) {
      return SCIREG_UNSUPPORTED;
    }

    /// Get bit attributes of type "t" into "v". "size" represents number of bytes returned, offset can be used to constrain the range.
    virtual scireg_response scireg_get_bit_attributes(vector_byte& v, scireg_bit_attributes_type t, sc_dt::uint64 size, sc_dt::uint64 offset=0) const {
      return SCIREG_UNSUPPORTED;
    }

    /// Get string attributes of type "t" associated with this region. The returned string is assigned to "s"
    virtual scireg_response scireg_get_string_attribute(const char *& s, scireg_string_attribute_type t) const {
      return SCIREG_UNSUPPORTED;
    }

    /// Get bit width and byte width of this region
    virtual sc_dt::uint64 scireg_get_bit_width() const {
      return 0;
    }
    virtual sc_dt::uint64 scireg_get_byte_width() const {
      return (7 + scireg_get_bit_width()) / 8;
    }

    /// If this region is a register field, these functions return low bit and high bit positions:
    virtual sc_dt::uint64 scireg_get_low_pos() const {
      return 0;
    }
    virtual sc_dt::uint64 scireg_get_high_pos() const {
      return 0;
    }

    /// Get child regions mapped into this region, by returning a mapped region object representing each mapping. 
    /// The size and offset parameters can be used to constrain the range of the search
    virtual scireg_response scireg_get_child_regions(std::vector<scireg_mapped_region>& mapped_regions, sc_dt::uint64 size=sc_dt::uint64(-1), sc_dt::uint64 offset=0) const {
      return SCIREG_UNSUPPORTED;
    }

    /// Get parent regions of this region
    virtual scireg_response scireg_get_parent_regions(std::vector<scireg_region_if*>& v) const {
      return SCIREG_UNSUPPORTED;
    }

    /// Add/Delete Callback objects associated with this region
    virtual scireg_response scireg_add_callback(scireg_callback & cb) {
      return SCIREG_UNSUPPORTED;
    }
    virtual scireg_response scireg_remove_callback(scireg_callback & cb) {
      return SCIREG_UNSUPPORTED;
    }

    /// Get SC TLM2 Target socket associated with this region
    virtual scireg_response scireg_get_target_sockets(std::vector<sc_core::sc_object*>& v) const {
      return SCIREG_UNSUPPORTED;
    }

    /// Get parent SystemC modules associated with this region
    virtual scireg_response scireg_get_parent_modules(std::vector<sc_core::sc_module*>& v) const {
      return SCIREG_UNSUPPORTED;
    }

    /// Get a vector of value_info objects for this region
    virtual scireg_response scireg_get_value_info(std::vector<scireg_value_info>& v) const {
      return SCIREG_UNSUPPORTED;
    }

    /// Query to see if DMI access has been granted to this region. "size" and offset can be used to constrain the range.
    virtual scireg_response scireg_get_dmi_granted(bool& granted, sc_dt::uint64 size, sc_dt::uint64 offset=0) const {
      return SCIREG_UNSUPPORTED;
    }
};

}

#endif  // We do not run in incisive
#endif  // CORE_COMMON_SCIREG_H_
///@}
