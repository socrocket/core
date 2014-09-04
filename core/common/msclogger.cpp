// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file msclogger.cpp
/// Utility class for timing verification. Keeps track of TLM communication by
/// writing a mscgen control file.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include <tlm.h>
#include <fstream>  // NOLINT(readability/streams)

std::ofstream msc;
sc_core::sc_time msclogger_start;
sc_core::sc_time msclogger_end;
/// @}
