// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_report
/// @{
/// @file sr_report.i
/// @date 2013-2015
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
%module sr_report

%include "usi.i"
%include "std_string.i"

%{
USI_REGISTER_MODULE(sr_report);
%}

namespace sc_core {
enum sc_severity {
    SC_INFO = 0,        // informative only
    SC_WARNING, // indicates potentially incorrect condition
    SC_ERROR,   // indicates a definite problem
    SC_FATAL,   // indicates a problem from which we cannot recover
    SC_MAX_SEVERITY
};
}

void set_filter_to_whitelist(bool value);
void add_sc_object_to_filter(sc_core::sc_object *obj, sc_core::sc_severity severity, int verbosity);
void remove_sc_object_from_filter(sc_core::sc_object *obj);


%{
#include "sr_report.h"

void set_filter_to_whitelist(bool value) {
  sr_report_handler::set_filter_to_whitelist(value);
}

void add_sc_object_to_filter(sc_core::sc_object *obj, sc_severity severity, int verbosity) {
  if(obj) {
    sr_report_handler::add_sc_object_to_filter(obj, severity, verbosity);
  }
}

void remove_sc_object_from_filter(sc_core::sc_object *obj) {
  if(obj) {
    sr_report_handler::remove_sc_object_from_filter(obj);
  }
}

/*
std::vector<sc_core::sc_object *> show_sc_object_in_filter() {
  sr_report_handler::show_sc_object_in_filter();
}
*/

%}


