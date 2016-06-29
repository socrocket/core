// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup sr_register
/// @{
/// @file scireg.cpp
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
#include <cstdlib>
#include "scireg.h"

namespace scireg_ns {

std::vector<scireg_notification_if *> *scireg_tool_registry::tool_vec_ptr = 0;

void scireg_tool_registry::add_tool(scireg_notification_if &notification) { 
  check_allocation(); 
  tool_vec_ptr->push_back(&notification); 
}

void scireg_tool_registry::remove_tool(scireg_notification_if &notification) {
  check_allocation();
  std::vector<scireg_notification_if*>::iterator it;
  it = std::find(tool_vec_ptr->begin(), tool_vec_ptr->end(), &notification);
  if (it != tool_vec_ptr->end())
    tool_vec_ptr->erase(it);
}

void scireg_tool_registry::add_region(scireg_region_if &r) {
  check_allocation();
  std::vector<scireg_notification_if*>::iterator it;
  for (it = tool_vec_ptr->begin(); it != tool_vec_ptr->end(); ++it)
    (*it)->add_region(r);
}

void scireg_tool_registry::remove_region(scireg_region_if &r) {
  check_allocation();
  std::vector<scireg_notification_if*>::iterator it;
  for (it = tool_vec_ptr->begin(); it != tool_vec_ptr->end(); ++it)
    (*it)->remove_region(r);
}

void scireg_tool_registry::check_allocation() {
  if (!tool_vec_ptr) {
    tool_vec_ptr = new std::vector<scireg_notification_if *>();
    std::atexit(&scireg_tool_registry::destroy);
  }
}

void scireg_tool_registry::destroy() {
  delete tool_vec_ptr;
  tool_vec_ptr = 0;
}

}  // scireg_ns


