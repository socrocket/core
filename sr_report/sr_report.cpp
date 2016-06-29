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
///
/// In this file the default backend hander is defined.
#include "sr_report.h"

sr_report sr_report_handler::rep(sc_core::SC_INFO, sc_core::sc_report_handler::add_msg_type("/initial/msg"), "null", 
                                 __FILE__, __LINE__, sc_core::SC_NONE, sc_core::SC_UNSPECIFIED);
sr_report sr_report_handler::null(sc_core::SC_INFO, sc_core::sc_report_handler::add_msg_type("/null"), "null",
                                  __FILE__, __LINE__, sc_core::SC_NONE, sc_core::SC_UNSPECIFIED);

bool sr_report_handler::blacklist = true;
//sr_report_handler::filter_t sr_report_handler::filter = sr_report_handler::filter_t(); // deprecated with GCC 5
std::map< const sc_core::sc_object *, std::pair<sc_core::sc_severity, int> > sr_report_handler::filter = std::map< const sc_core::sc_object *, std::pair<sc_core::sc_severity, int> >();

void sr_report_handler::default_handler(const sc_core::sc_report &rep, const sc_core::sc_actions &actions) {
  std::ostringstream sstr;
  sr_report *srr = dynamic_cast<sr_report *>(const_cast<sc_report *>(&rep));
  if(srr) {
    sstr.str("");
    sstr << srr->get_msg();
    sstr << std::endl;
    for(std::vector<v::pair>::const_iterator iter = srr->pairs.begin(); iter!=srr->pairs.end(); iter++) {
      sstr << iter->name << " : ";
      switch(iter->type) {
        case v::pair::INT32:  sstr << std::hex << std::setfill('0') << "0x" << std::setw(8) 
                                   << boost::any_cast<int32_t>(iter->data); break;
        case v::pair::UINT32: sstr << std::hex << std::setfill('0') << "0x" << std::setw(8) 
                                   << boost::any_cast<uint32_t>(iter->data); break;
        case v::pair::INT64:  sstr << std::hex << std::setfill('0') << "0x" << std::setw(16) 
                                   << boost::any_cast<int64_t>(iter->data); break;
        case v::pair::UINT64: sstr << std::hex << std::setfill('0') << "0x" << std::setw(16) 
                                   << boost::any_cast<uint64_t>(iter->data); break;
        case v::pair::STRING: sstr << boost::any_cast<std::string>(iter->data).c_str(); break;
        case v::pair::BOOL:   sstr << (boost::any_cast<bool>(iter->data)? "true" : "false"); break;
        case v::pair::DOUBLE: sstr << boost::any_cast<double>(iter->data); break;
        case v::pair::TIME:   sstr << boost::any_cast<sc_core::sc_time>(iter->data).to_string(); break;
        default:              sstr << boost::any_cast<int32_t>(iter->data);
      }
      if((iter+1)!=srr->pairs.end()) {
        sstr << ", ";
      }
    }
    srr->set_msg(sstr.str().c_str());
  }
  sc_report_handler::default_handler(rep, actions);
}

/// @}
