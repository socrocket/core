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
*   (c) Luca Fossati, fossati@elet.polimi.it, fossati.l@gmail.com
*
\ ***************************************************************************/

#ifndef ELFFRONTEND_H
#define ELFFRONTEND_H

// Part of the code of this class is taken from the binutils sources

extern "C" {
#include <gelf.h>
}

#include "core/common/vmap.h"

#include <list>
#include <map>
#include <string>
#include <vector>

namespace trap {
class ELFFrontend {
  private:
    ///Size of each assembly instruction in bytes
    unsigned int wordsize;
    ///Name of the executable file
    std::string execName;
    ///runtime representation of the ELF file
    Elf *elf_pointer;
    ///file descriptor representing the open elf file
    int elfFd;

    ///Variables holding what read from the file
    vmap<unsigned int, std::list<std::string> > addrToSym;
    vmap<unsigned int, std::string> addrToFunction;
    std::map<std::string, unsigned int> symToAddr;
    vmap<unsigned int, std::pair<std::string, unsigned int> > addrToSrc;
    unsigned int entryPoint;
    unsigned char *programData;

    // end address and start address (not necessarily the entry point) of the loadable part of the binary file
    std::pair<unsigned int, unsigned int> codeSize;

    static std::map<std::string, ELFFrontend *> curInstance;
    // Private constructor: we want pepole to be only able to use getInstance
    // to get an instance of the frontend
    ELFFrontend(std::string binaryName);

    // Methods for reading the binary instructions from the executable file and
    // for interpreting the symbol table
    void readProgramData();
    void readSymbols();
  public:
    ~ELFFrontend();
    static ELFFrontend&getInstance(std::string fileName);
    static void reset();
    ///Given an address, it returns the symbols found there,(more than one
    ///symbol can be mapped to an address). Note
    ///That if address is in the middle of a function, the symbol
    ///returned refers to the function itself
    std::list<std::string> symbolsAt(unsigned int address) const throw();
    ///Given an address, it returns the first symbol found there
    ///"" if no symbol is found at the specified address; note
    ///That if address is in the middle of a function, the symbol
    ///returned refers to the function itself
    std::string symbolAt(unsigned int address) const throw();
    ///Given the name of a symbol it returns its value
    ///(which usually is its address);
    ///valid is set to false if no symbol with the specified
    ///name is found
    unsigned int getSymAddr(const std::string &symbol, bool &valid) const throw();
    ///Returns the name of the executable file
    std::string getExecName() const;
    ///Specifies whether the address is the first one of a rountine
    bool isRoutineEntry(unsigned int address) const;
    ///Specifies whether the address is the last one of a routine
    bool isRoutineExit(unsigned int address) const;
    ///Returns the end address of the loadable code
    unsigned int getBinaryEnd() const;
    ///Returns the start address of the loadable code
    unsigned int getBinaryStart() const;
    ///Returns the entry point of the executable code
    unsigned int getEntryPoint() const;
    ///Given an address, it sets fileName to the name of the source file
    ///which contains the code and line to the line in that file. Returns
    ///false if the address is not valid
    bool getSrcFile(unsigned int address, std::string &fileName, unsigned int &line) const;
    ///Returns a pointer to the array contianing the program data
    unsigned char*getProgData();
};
}

#endif
