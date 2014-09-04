/***************************************************************************\
*
*   This file is part of objcodeFrontend.
*
*   objcodeFrontend is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 3 of the License, or
*   (at your option) any later version.
*
*   This library is distributed in the hope that it will be useful,
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
\ ***************************************************************************/

#ifndef EXECLOADER_HPP
#define EXECLOADER_HPP

#include <string>

#include <fstream>
#include <iostream>

extern "C" {
#include <gelf.h>
}

#include "core/common/trapgen/elfloader/elfFrontend.hpp"

namespace trap {
class ExecLoader {
  private:
    ///Specifies whether a normal binary file (ELF; COFF, etc.) or a plain file (just the
    ///opcodes of the instructions) was used.
    bool plainFile;
    ///Keeps reference to the main elf parser
    ELFFrontend *elfFrontend;
    ///Holds the data of the program to be executed in case it is a simply
    ///sequence of instructions (i.e. not an ELF structured file)
    unsigned char *programData;
    std::ifstream plainExecFile;
  public:
    ///Initializes the loader of executable files by creating
    ///the corresponding bfd image of the executable file
    ///specified as parameter
    ExecLoader(std::string fileName, bool plainFile = false);
    ~ExecLoader();
    ///Returns the entry point of the loaded program
    ///(usually the same as the program start address, but
    ///not always)
    unsigned int getProgStart();
    ///Returns the start address of the program being loaded
    ///(the lowest address of the program)
    unsigned int getDataStart();
    ///Returns the dimension of the loaded program
    unsigned int getProgDim();
    ///Returns a pointer to the array contianing the program data
    unsigned char*getProgData();
};
}

#endif
