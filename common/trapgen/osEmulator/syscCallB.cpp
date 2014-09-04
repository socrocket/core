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

#include "core/common/trapgen/elfloader/elfFrontend.hpp"
#include "core/common/trapgen/osEmulator/syscCallB.hpp"

#include <map>
#include <string>
#include <vector>

#define NEWLIB_O_RDONLY          0x0000
#define NEWLIB_O_WRONLY          0x0001
#define NEWLIB_O_RDWR            0x0002
#define NEWLIB_O_APPEND          0x0008
#define NEWLIB_O_CREAT           0x0200
#define NEWLIB_O_TRUNC           0x0400
#define NEWLIB_O_EXCL            0x0800
#define NEWLIB_O_NOCTTY          0x8000
#define NEWLIB_O_NONBLOCK        0x4000

#define CORRECT_O_RDONLY             00
#define CORRECT_O_WRONLY             01
#define CORRECT_O_RDWR               02
#define CORRECT_O_CREAT            0100
#define CORRECT_O_EXCL             0200
#define CORRECT_O_NOCTTY           0400
#define CORRECT_O_TRUNC           01000
#define CORRECT_O_APPEND          02000
#define CORRECT_O_NONBLOCK        04000



void trap::OSEmulatorBase::correct_flags(int &val){
    int flags = 0;

    if( val &  NEWLIB_O_RDONLY )
        flags |= CORRECT_O_RDONLY;
    if( val &  NEWLIB_O_WRONLY )
        flags |= CORRECT_O_WRONLY;
    if( val &  NEWLIB_O_RDWR )
        flags |= CORRECT_O_RDWR;
    if( val & NEWLIB_O_CREAT )
        flags |= CORRECT_O_CREAT;
    if( val & NEWLIB_O_EXCL )
        flags |= CORRECT_O_EXCL;
    if( val & NEWLIB_O_NOCTTY )
        flags |= CORRECT_O_NOCTTY;
    if( val & NEWLIB_O_TRUNC )
        flags |= CORRECT_O_TRUNC;
    if( val & NEWLIB_O_APPEND )
        flags |= CORRECT_O_APPEND;
    if( val & NEWLIB_O_NONBLOCK )
        flags |= CORRECT_O_NONBLOCK;

    val = flags;
} 

void trap::OSEmulatorBase::set_environ(const std::string name, const std::string value){
    env[name] = value;
}

void trap::OSEmulatorBase::set_sysconf(const std::string name, int value){
    sysconfmap[name] = value;
}

void trap::OSEmulatorBase::set_program_args(const std::vector<std::string> args){
    programArgs = args;
}

void trap::OSEmulatorBase::reset(){
    this->programArgs.clear();    
    this->sysconfmap.clear();    
    this->programArgs.clear();    
    this->heapPointer = 0;
    ELFFrontend::reset();
}

std::vector<unsigned int> trap::OSEmulatorBase::groupIDs;
unsigned int trap::OSEmulatorBase::programsCount = 0;

namespace trap{
int exitValue = 0;
}
