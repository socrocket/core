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
#include "common/report.h"

sr_report sr_report_handler::rep(sc_core::SC_INFO, sc_core::sc_report_handler::add_msg_type("/initial/msg"), "null", 
                                 __FILE__, __LINE__, sc_core::SC_NONE, SC_UNSPECIFIED);
sr_report sr_report_handler::null(sc_core::SC_INFO, sc_core::sc_report_handler::add_msg_type("/null"), "null",
                                  __FILE__, __LINE__, sc_core::SC_NONE, SC_UNSPECIFIED);

/// @}
