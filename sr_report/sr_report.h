// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file report.h
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
#ifndef SR_REPORT_H_
#define SR_REPORT_H_

#include <boost/any.hpp>
#include <systemc.h>
#include <map> // gcc-5 doesn't like vmap within this file
#include <string>
#include <iomanip>
#include <iostream>  // NOLINT(readability/streams)
#include <streambuf>

namespace v {

/// Standard verbosity is 3.
/// All messages but debug.
#ifndef VERBOSITY
#ifdef GLOBALVERBOSITY
#define VERBOSITY GLOBALVERBOSITY
#else
#define VERBOSITY 4
#endif
#endif

class pair {
  public:
    pair(const std::string name, int8_t value) : name(name), type(INT32), data(value) {
      // std::cout << "+ P" << this << " " << name << std::endl;
    }
    pair(const std::string name, int16_t value) : name(name), type(INT32), data(value) {
      // std::cout << "+ P" << this << " " << name << std::endl;
    }
    pair(const std::string name, int32_t value) : name(name), type(INT32), data(value) {
      // std::cout << "+ P" << this << " " << name << std::endl;
    }
    pair(const std::string name, uint8_t value) : name(name), type(UINT32), data(value) {
      // std::cout << "+ P" << this << " " << name << std::endl;
    }
    pair(const std::string name, uint16_t value) : name(name), type(UINT32), data(value) {
      // std::cout << "+ P" << this << " " << name << std::endl;
    }
    pair(const std::string name, uint32_t value) : name(name), type(UINT32), data(value) {
      // std::cout << "+ P" << this << " " << name << std::endl;
    }
    pair(const std::string name, int64_t value) : name(name), type(INT64), data(value) {
      // std::cout << "+ P" << this << " " << name << std::endl;
    }
    pair(const std::string name, uint64_t value) : name(name), type(UINT64), data(value) {
      // std::cout << "+ P" << this << " " << name << std::endl;
    }
    pair(const std::string name, std::string value) : name(name), type(STRING), data(value) {
      // std::cout << "+ P" << this << " " << name << std::endl;
    }
    pair(const std::string name, bool value) : name(name), type(BOOL), data(value) {
      // std::cout << "+ P" << this << " " << name << std::endl;
    }
    pair(const std::string name, double value) : name(name), type(DOUBLE), data(value) {
      // std::cout << "+ P" << this << " " << name << std::endl;
    }
    pair(const std::string name, sc_core::sc_time value) : name(name), type(TIME), data(value) {
      // std::cout << "+ P" << this << " " << name << std::endl;
    }
    pair(const pair &copy) : name(copy.name), type(copy.type), data(copy.data) {
      // std::cout << "+ P" << this << " C " << &copy << " " << name << std::endl;
    }

    ~pair() {
      // std::cout << "- P " << this << " " << name << std::endl;
    }
  std::string name;
  enum type {
    INT32,
    UINT32,
    INT64,
    UINT64,
    STRING,
    BOOL,
    DOUBLE,
    TIME
  } type;
  boost::any data;
};

};  // namespace v

class sr_report : public sc_core::sc_report {
  public:
#if SYSTEMC_VERSION != 20140417
    sr_report() : sc_core::sc_report(), enabled(false) {};
#endif
    sr_report(const sr_report &copy) : sc_core::sc_report(copy), enabled(copy.enabled), actions(copy.actions), pairs(copy.pairs) {}
    explicit sr_report(const sc_core::sc_report &copy) : sc_core::sc_report(copy), enabled(true) {}

    sr_report(
        sc_core::sc_severity severity,
        const sc_core::sc_msg_def* msg_def,
        const char* msg,
        const char* file,
        int line,
        int verbosity_level,
        const sc_core::sc_actions &actions) :
#ifdef NC_SYSTEMC
      sc_core::sc_report(
        severity, msg_def, msg,
        verbosity_level, file, line),
#else
      sc_core::sc_report(
        severity, msg_def, msg,
        file, line, verbosity_level),
#endif
      enabled(false),
      actions(actions) {
    };

    ~sr_report() throw() {
      pairs.clear();
    }

    sr_report &operator=(const sr_report& other) {
      sr_report copy(other);
      swap(copy);
      return *this;
    }

    void swap(sr_report & that) {
      using std::swap;
      sc_core::sc_report::swap(that);
      swap(enabled, that.enabled);
      swap(actions, that.actions);
      swap(pairs,   that.pairs);
    }

    inline void set_msg(const char *msg) {
      char* result;
      result = (char *)new char[strlen(msg)+1];
      strcpy(result, msg);
      if(this->msg && *this->msg) {
          delete this->msg;
      }

      this->msg = result;
    }

    inline sr_report &operator()(const std::string &name, int8_t value) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, value));
      }
      return *this;
    }

    inline sr_report &operator()(const std::string &name, int16_t value) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, value));
      }
      return *this;
    }

    inline sr_report &operator()(const std::string &name, int32_t value) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, value));
      }
      return *this;
    }

    inline sr_report &operator()(const std::string &name, uint8_t value) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, value));
      }
      return *this;
    }

    inline sr_report &operator()(const std::string &name, uint16_t value) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, value));
      }
      return *this;
    }

    inline sr_report &operator()(const std::string &name, uint32_t value) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, value));
      }
      return *this;
    }

    inline sr_report &operator()(const std::string &name, int64_t value) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, value));
      }
      return *this;
    }

#if not defined(NC_SYSTEMC) and not defined(_WIN32)
    inline sr_report &operator()(const std::string &name, sc_dt::int64 value) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, (int64_t)value));
      }
      return *this;
    }
#endif

    inline sr_report &operator()(const std::string &name, uint64_t value) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, value));
      }
      return *this;
    }

#if not defined(NC_SYSTEMC) and not defined(_WIN32)
    inline sr_report &operator()(const std::string &name, sc_dt::uint64 value) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, (uint64_t)value));
      }
      return *this;
    }
#endif

    inline sr_report &operator()(const std::string &name, std::string value) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, value));
      }
      return *this;
    }

    inline sr_report &operator()(const std::string &name, const char value[]) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, std::string(value)));
      }
      return *this;
    }

    inline sr_report &operator()(const std::string &name, char value[]) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, std::string(value)));
      }
      return *this;
    }

    inline sr_report &operator()(const std::string &name, bool value) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, value));
      }
      return *this;
    }

    inline sr_report &operator()(const std::string &name, double value) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, value));
      }
      return *this;
    }

    inline sr_report &operator()(const std::string &name, sc_core::sc_time value) {
      if( __builtin_expect( enabled, 0 ) ) {
        pairs.push_back(v::pair(name, value));
      }
      return *this;
    }

    inline void operator()(const std::string &name = "");

    bool enabled;
    sc_core::sc_actions actions;
    std::vector<v::pair> pairs;
};

class sr_report_handler : public sc_core::sc_report_handler {
  public:
    static sr_report report(
        sc_severity severity_,
        const sc_core::sc_object *obj,
        const char *msg_type_,
        const char *msg_,
        int verbosity_,
        const char *file_,
        int line_) {

      bool enabled = true;
      if(obj) {
        if(sr_report_handler::blacklist) {
          // Blacklist
          if(!sr_report_handler::filter.empty()) {
            std::map< const sc_core::sc_object *, std::pair<sc_core::sc_severity, int> >::iterator iter = sr_report_handler::filter.find( obj );
            //sr_report_handler::filter_t::iterator iter = sr_report_handler::filter.find(obj); // deprecated with GCC 5
            if(iter != sr_report_handler::filter.end()) {
              if(severity_ <= iter->second.first && verbosity_ >= iter->second.second) {
                enabled = false;
              }
            }
          }
        } else {
          // Whitelist
          std::map< const sc_core::sc_object *, std::pair<sc_core::sc_severity, int> >::iterator iter = sr_report_handler::filter.find( obj );
          //sr_report_handler::filter_t::iterator iter = sr_report_handler::filter.find(obj); // deprecated with GCC 5
          enabled = (iter != sr_report_handler::filter.end() && (severity_ > iter->second.first && verbosity_ >= iter->second.second));
        }
      }

      // If the severity of the report is SC_INFO and the specified verbosity
      // level is greater than the maximum verbosity level of the simulator then
      // return without any action.
      if ( __builtin_expect( (severity_ == sc_core::SC_INFO) && (verbosity_ > sr_report_handler::get_verbosity_level()), 1 ) ) {
        return null;
      }

      // Process the report:
      sc_msg_def *md = mdlookup(msg_type_);
      if (!md) {
        md = add_msg_type(msg_type_);
      }

      sc_actions actions = execute(md, severity_);
      sr_report rep = sr_report(severity_, md, msg_, file_, line_, verbosity_, actions);
      rep.pairs.clear();
      rep.enabled = enabled;

      return rep;
    }

    static void set_filter_to_whitelist(bool value) {
      sr_report_handler::blacklist = !value;
    }

    static void add_sc_object_to_filter(sc_core::sc_object *obj, sc_severity severity, int verbosity) {
      sr_report_handler::filter.insert(std::make_pair(obj, std::make_pair(severity, verbosity)));
    }

    static void remove_sc_object_from_filter(sc_core::sc_object *obj) {
      std::map< const sc_core::sc_object *, std::pair<sc_core::sc_severity, int> >::iterator iter = sr_report_handler::filter.find(obj);
      //sr_report_handler::filter_t::iterator iter = sr_report_handler::filter.find(obj); // deprecated with GCC 5
      if(iter != sr_report_handler::filter.end()) {
        sr_report_handler::filter.erase(iter);
      }
    }

    using sc_core::sc_report_handler::handler;

    static void default_handler(const sc_core::sc_report &rep, const sc_core::sc_actions &actions);

  friend void sr_report::operator()(const std::string &name);
  private:
    static sr_report rep;
    static sr_report null;
    static bool blacklist;
//    typedef vmap<const sc_core::sc_object *, std::pair<sc_core::sc_severity, int> > filter_t; // deprecated with GCC 5
//    static filter_t filter; // deprecated with GCC 5
    static std::map< const sc_core::sc_object *, std::pair<sc_core::sc_severity, int> > filter;
};

void sr_report::operator()(const std::string &name) {
  if ( __builtin_expect( this != &sr_report_handler::null && enabled, 0 )  ) {
    if(name != "") {
      set_msg(name.c_str());
    }
    sr_report_handler::handler(*this, actions);
  }
}

#define _GET_MACRO_(dummy,_1,NAME,...) NAME
#define _GET_MACRO_2_(dummy,_1,_2,NAME,...) NAME

#define srDebug(...) \
  if(5 >= VERBOSITY) {} else \
    _GET_MACRO_(dummy,##__VA_ARGS__,srDebug_1(__VA_ARGS__),srDebug_0())

#define srDebug_0() \
    sr_report_handler::report( \
      sc_core::SC_INFO, this, this->name(), "", \
      sc_core::SC_DEBUG, __FILE__ , __LINE__)

#define srDebug_1(id) \
    sr_report_handler::report( \
      sc_core::SC_INFO, NULL, id, "", \
      sc_core::SC_DEBUG, __FILE__ , __LINE__)

// DEPRECATED: for configurations at initialization -> will be moved to Python
#define srConfig(...) _GET_MACRO_(dummy,##__VA_ARGS__,srConfig_1(__VA_ARGS__),srConfig_0())

#define srConfig_0() \
    sr_report_handler::report( \
      sc_core::SC_INFO, this, this->name(), "", \
      sc_core::SC_MEDIUM, __FILE__ , __LINE__)

#define srConfig_1(id) \
    sr_report_handler::report( \
      sc_core::SC_INFO, NULL, id, "", \
      sc_core::SC_MEDIUM, __FILE__ , __LINE__)

// STRONGLY DEPRECATED: for report after end of simulation -> will be moved to
// Python
#define srReport(...) _GET_MACRO_(dummy,##__VA_ARGS__,srReport_1(__VA_ARGS__),srReport_0())

#define srReport_0() \
    sr_report_handler::report( \
      sc_core::SC_INFO, this, this->name(), "", \
      sc_core::SC_MEDIUM, __FILE__ , __LINE__)

#define srReport_1(id) \
    sr_report_handler::report( \
      sc_core::SC_INFO, NULL, id, "", \
      sc_core::SC_MEDIUM, __FILE__ , __LINE__)

// for data to be analysed by ipython
#define srAnalyse(...) \
  if(4 >= VERBOSITY) {} else \
    _GET_MACRO_(dummy,##__VA_ARGS__,srAnalyse_1(__VA_ARGS__),srAnalyse_0())

#define srAnalyse_0() \
    sr_report_handler::report( \
      sc_core::SC_INFO, this, this->name(), "", \
      sc_core::SC_FULL, __FILE__ , __LINE__)

#define srAnalyse_1(id) \
    sr_report_handler::report( \
      sc_core::SC_INFO, NULL, id, "", \
      sc_core::SC_FULL, __FILE__ , __LINE__)

#define srInfo(...) \
  if(3 >= VERBOSITY) {} else \
    _GET_MACRO_(dummy,##__VA_ARGS__,srInfo_1(__VA_ARGS__),srInfo_0())

#define srInfo_0() \
    sr_report_handler::report( \
      sc_core::SC_INFO, this, this->name(), "", \
      sc_core::SC_LOW, __FILE__ , __LINE__)

#define srInfo_1(id) \
    sr_report_handler::report( \
      sc_core::SC_INFO, NULL, id, "", \
      sc_core::SC_LOW, __FILE__ , __LINE__)

#define srMessage(...) _GET_MACRO_2_(dummy,##__VA_ARGS__,srMessage_2(__VA_ARGS__),srMessage_1(__VA_ARGS__))

#define srMessage_1(verbosity) \
    sr_report_handler::report( \
      sc_core::SC_INFO, this, this->name(), "", \
      verbosity, __FILE__ , __LINE__)

#define srMessage_2(id, verbosity) \
    sr_report_handler::report( \
      sc_core::SC_INFO, NULL, id, "", \
      verbosity, __FILE__ , __LINE__)

#define srWarn(...) _GET_MACRO_(dummy,##__VA_ARGS__,srWarn_1(__VA_ARGS__),srWarn_0())

#define srWarn_0() \
    sr_report_handler::report( \
      sc_core::SC_WARNING, this, this->name(), "", \
      sc_core::SC_MEDIUM, __FILE__ , __LINE__)

#define srWarn_1(id) \
    sr_report_handler::report( \
      sc_core::SC_WARNING, NULL, id, "", \
      sc_core::SC_MEDIUM, __FILE__ , __LINE__)

#define srError(...) _GET_MACRO_(dummy,##__VA_ARGS__,srError_1(__VA_ARGS__),srError_0())

#define srError_0() \
    sr_report_handler::report( \
      sc_core::SC_ERROR, this, this->name(), "", \
      sc_core::SC_LOW, __FILE__ , __LINE__)

#define srError_1(id) \
    sr_report_handler::report( \
      sc_core::SC_ERROR, NULL, id, "", \
      sc_core::SC_LOW, __FILE__ , __LINE__)

#define srFatal(...) _GET_MACRO_(dummy,##__VA_ARGS__,srFatal_1(__VA_ARGS__),srFatal_0())

#define srFatal_0() \
    sr_report_handler::report( \
      sc_core::SC_FATAL, this, this->name(), "", \
      sc_core::SC_LOW, __FILE__ , __LINE__)

#define srFatal_1(id) \
    sr_report_handler::report( \
      sc_core::SC_FATAL, NULL, id, "", \
      sc_core::SC_LOW, __FILE__ , __LINE__)

#define srCommand(...) _GET_MACRO_2_(dummy,##__VA_ARGS__,srCommand_2(__VA_ARGS__),srCommand_1(__VA_ARGS__))

#define srCommand_1(type) \
    sr_report_handler::report( \
      sc_core::SC_MAX_SEVERITY, NULL, this->name(), "command", \
      0x0FFFFFFF, __FILE__, __LINE__)("typename", type)

#define srCommand_2(id, type) \
    sr_report_handler::report( \
      sc_core::SC_MAX_SEVERITY, NULL, id, "command", \
      0x0FFFFFFF, __FILE__, __LINE__)("typename", type)

#endif  // SR_REPORT_H_
/// @}
