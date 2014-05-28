// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file verbose_cult.cpp
/// Implements a unified output system for messages and debunging.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#include "verbose.h"

namespace v {

void logFile(char *name) {}

void logApplication(char *name) {}

logstream<0> error;
logstream<1> warn;
logstream<2> report;
logstream<3> info;
logstream<4> debug;

/** Linux internal consol color pattern */
Color bgBlack("\e[40m");
Color bgWhite("\e[47m");
Color bgYellow("\e[43m");
Color bgRed("\e[41m");
Color bgGreen("\e[42m");
Color bgBlue("\e[44m");
Color bgMagenta("\e[45m");
Color bgCyan("\e[46m");

Color Black("\e[30m");
Color White("\e[37m");
Color Yellow("\e[33m");
Color Red("\e[31m");
Color Green("\e[32m");
Color Blue("\e[34m");
Color Magenta("\e[35m");
Color Cyan("\e[36m");

Color Normal("\e[0m");
Color Bold("\033[1m");
Color Blink("\e[36m");
Color Beep("\e[36m");

Number uint64("0x", '0', 16, true);
Number uint32("0x", '0', 8, true);
Number uint16("0x", '0', 4, true);
Number uint8("0x", '0', 2, true);
Number noint("", ' ', 0, false);

} // namespace

/// @}