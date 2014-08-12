// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file report.h
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
#ifndef COMMON_REPORT_H_
#define COMMON_REPORT_H_

#include <systemc.h>
#include <boost/any.hpp>

namespace v {

class pair {
  public:
    pair(const std::string name, int8_t value) : name(name), type(INT32), data(value) {}
    pair(const std::string name, int16_t value) : name(name), type(INT32), data(value) {}
    pair(const std::string name, int32_t value) : name(name), type(INT32), data(value) {}
    pair(const std::string name, uint8_t value) : name(name), type(UINT32), data(value) {}
    pair(const std::string name, uint16_t value) : name(name), type(UINT32), data(value) {}
    pair(const std::string name, uint32_t value) : name(name), type(UINT32), data(value) {}
    pair(const std::string name, int64_t value) : name(name), type(INT64), data(value) {}
    pair(const std::string name, uint64_t value) : name(name), type(UINT64), data(value) {}
    pair(const std::string name, std::string value) : name(name), type(STRING), data(value) {}
    pair(const std::string name, bool value) : name(name), type(BOOL), data(value) {}
    pair(const std::string name, double value) : name(name), type(DOUBLE), data(value) {}
    pair(const std::string name, sc_core::sc_time value) : name(name), type(TIME), data(value) {}

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

}  // namespace v

class sr_report : public sc_core::sc_report {
  public:
    sr_report(): sc_core::sc_report() {};
    sr_report(const sr_report &copy): sc_core::sc_report(copy), actions(copy.actions), pairs(copy.pairs) {}
    explicit sr_report(const sc_core::sc_report &copy): sc_core::sc_report(copy) {}

    sr_report(
        sc_core::sc_severity severity,
	      const sc_core::sc_msg_def* msg_def,
	      const char* msg,
	      const char* file,
	      int line,
	      int verbosity_level,
        const sc_core::sc_actions &actions) :
      sc_core::sc_report(
        severity, msg_def, msg,
	      file, line, verbosity_level),
      actions(actions) {};

    ~sr_report() throw() {}
    sr_report &operator=(const sr_report& other) {
        sr_report copy(other);
        swap(copy);
        return *this;
    }

    void swap(sr_report & that) {
        using std::swap;
        sc_core::sc_report::swap(that);
        swap(actions, that.actions);
        swap(pairs,   that.pairs);
    } 

    inline void set_msg(const char *msg) {
        char* result;
        result = (char*)malloc(strlen(msg)+1);
        strcpy(result, msg);
      this->msg = result;
    }

    inline sr_report &operator()(const std::string &name, int8_t value) {
      pairs.push_back(v::pair(name, value));
      return *this;
    }

    inline sr_report &operator()(const std::string &name, int16_t value) {
      pairs.push_back(v::pair(name, value));
      return *this;
    }

    inline sr_report &operator()(const std::string &name, int32_t value) {
      pairs.push_back(v::pair(name, value));
      return *this;
    }

    inline sr_report &operator()(const std::string &name, uint8_t value) {
      pairs.push_back(v::pair(name, value));
      return *this;
    }

    inline sr_report &operator()(const std::string &name, uint16_t value) {
      pairs.push_back(v::pair(name, value));
      return *this;
    }

    inline sr_report &operator()(const std::string &name, uint32_t value) {
      pairs.push_back(v::pair(name, value));
      return *this;
    }

    inline sr_report &operator()(const std::string &name, int64_t value) {
      pairs.push_back(v::pair(name, value));
      return *this;
    }

    inline sr_report &operator()(const std::string &name, uint64_t value) {
      pairs.push_back(v::pair(name, value));
      return *this;
    }

    inline sr_report &operator()(const std::string &name, std::string value) {
      pairs.push_back(v::pair(name, value));
      return *this;
    }

    inline sr_report &operator()(const std::string &name, bool value) {
      pairs.push_back(v::pair(name, value));
      return *this;
    }

    inline sr_report &operator()(const std::string &name, double value) {
      pairs.push_back(v::pair(name, value));
      return *this;
    }

    inline sr_report &operator()(const std::string &name, sc_core::sc_time value) {
      pairs.push_back(v::pair(name, value));
      return *this;
    }

    inline void operator()(const std::string &name = "");

    sc_core::sc_actions actions;
    std::vector<v::pair> pairs;
};

class sr_report_handler : public sc_core::sc_report_handler {
  public:
    static sr_report &report(
        sc_severity severity_, 
        const char* msg_type_, 
        const char* msg_, 
        int verbosity_, 
        const char* file_, 
        int line_) {
        sc_msg_def * md = mdlookup(msg_type_);

      // If the severity of the report is SC_INFO and the specified verbosity 
      // level is greater than the maximum verbosity level of the simulator then 
      // return without any action.

      if ((severity_ == SC_INFO) && (verbosity_ > verbosity_level)) {
        return null;
      }

      // Process the report:

      if (!md) {
        md = add_msg_type(msg_type_);
      }

      sc_actions actions = execute(md, severity_);
      rep = sr_report(severity_, md, msg_, file_, line_, verbosity_, actions);

      //if (actions & SC_CACHE_REPORT) {
      //  cache_report(rep);
      //}

      //handler(rep, actions);
      return rep;
    }
    using sc_core::sc_report_handler::handler;
  friend void sr_report::operator()(const std::string &name);
  private:
    static sr_report rep;
    static sr_report null;
};

void sr_report::operator()(const std::string &name) {
  set_msg(name.c_str());
  if (this != &sr_report_handler::null) {
    sr_report_handler::handler(*this, actions);
  }
}

#define srDebug(id) \
    sr_report_handler::report( \
      sc_core::SC_INFO, id, "", \
      sc_core::SC_DEBUG, __FILE__ , __LINE__)

#define srConfig(id) \
    sr_report_handler::report( \
      sc_core::SC_INFO, id, "", \
      sc_core::SC_FULL, __FILE__ , __LINE__)

#define srPower(id) \
    sr_report_handler::report( \
      sc_core::SC_INFO, id, "", \
      sc_core::SC_HIGH, __FILE__ , __LINE__)

#define srReport(id) \
    sr_report_handler::report( \
      sc_core::SC_INFO, id, "", \
      sc_core::SC_HIGH, __FILE__ , __LINE__)

#define srAnalyse(id) \
    sr_report_handler::report( \
      sc_core::SC_INFO, id, "", \
      sc_core::SC_LOW, __FILE__ , __LINE__)

#define srInfo(id) \
    sr_report_handler::report( \
      sc_core::SC_INFO, id, "", \
      sc_core::SC_MEDIUM, __FILE__ , __LINE__)

#define srMessage(id, verbosity) \
    sr_report_handler::report( \
      sc_core::SC_INFO, id, "", \
      verbosity, __FILE__ , __LINE__)

#define srWarn(id) \
    sr_report_handler::report( \
      sc_core::SC_WARNING, id, "", \
      sc_core::SC_MEDIUM, __FILE__ , __LINE__)

#define srError(id) \
    sr_report_handler::report( \
      sc_core::SC_ERROR, id, "", \
      sc_core::SC_MEDIUM, __FILE__ , __LINE__)

#define srFatal(id) \
    sr_report_handler::report( \
      sc_core::SC_FATAL, id, "", \
      sc_core::SC_MEDIUM, __FILE__ , __LINE__)


#endif  // COMMON_REPORT_H_
/// @}
