/***************************************************************************\
 *
 *   This file is part of TRAP.
 *
 *   TRAP is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *   or see <http://www.gnu.org/licenses/>.
 *
 *
 *
 *   (c) Luca Fossati, fossati@elet.polimi.it, fossati.l@gmail.com
 *
\***************************************************************************/

#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>
//#ifdef __GNUC__
//extern "C" {
//#include <execinfo.h>
//}
//#endif

#include "core/common/trapgen/utils/trap_utils.hpp"

void trap::throw_error_helper(std::string message){
    std::cerr << message << std::endl;
    ::exit(0);
}

void trap::throw_exception_helper(std::string message){
    throw std::runtime_error(message);
}

// #ifdef __GNUC__
// void throw_exception_helper(std::string message){
//     void * array[25];
//     int nSize = backtrace(array, 25);
//     char ** symbols = backtrace_symbols(array, nSize);
//     std::ostringstream traceMex;
//
//     if (symbols != NULL){
//         for (int i = 0; i < nSize; i++){
//             traceMex << symbols[i] << std::endl;
//         }
//         traceMex << std::endl;
//         free(symbols);
//     }
//     traceMex << message;
//
//     throw std::runtime_error(traceMex.str());
// }
// #else
// void throw_exception_helper(std::string message){
//     throw std::runtime_error(message);
// }
// #endif

