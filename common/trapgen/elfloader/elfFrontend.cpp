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

extern "C" {
#include <gelf.h>
}

// **************************** HAVE_ABI____CXA_DEMANGLE

#ifdef HAVE_CXXABI_H
#include <cxxabi.h>
#endif

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

#include "core/common/vmap.h"

#include <map>
#include <string>
#include <vector>
#include <list>
#include <iostream>

#include "core/common/trapgen/utils/trap_utils.hpp"

#include "core/common/trapgen/elfloader/elfFrontend.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

std::map<std::string, trap::ELFFrontend *> trap::ELFFrontend::curInstance;

trap::ELFFrontend & trap::ELFFrontend::getInstance(std::string fileName){
    if(ELFFrontend::curInstance.find(fileName) == ELFFrontend::curInstance.end()){
        ELFFrontend::curInstance[fileName] = new ELFFrontend(fileName);
    }
    return *ELFFrontend::curInstance[fileName];
}

void trap::ELFFrontend::reset(){
    std::map<std::string, trap::ELFFrontend *>::iterator beg, end;
    for(beg = ELFFrontend::curInstance.begin(), end = ELFFrontend::curInstance.end(); beg != end; beg++){
        delete beg->second;
    }
    ELFFrontend::curInstance.clear();
}

trap::ELFFrontend::ELFFrontend(std::string binaryName) : execName(binaryName){
    //Let's open the elf parser and check that everything is all right
    if(elf_version(EV_CURRENT) == EV_NONE){
        THROW_ERROR("Error, wrong version of the ELF library");
    }
    boost::filesystem::path fileNamePath = boost::filesystem::system_complete(boost::filesystem::path(binaryName));
    if ( !boost::filesystem::exists( fileNamePath ) ){
        THROW_EXCEPTION("Path " << binaryName << " does not exists");
    }
    this->elfFd = open(binaryName.c_str(), O_RDONLY, 0);
    if(this->elfFd < 0)
        THROW_EXCEPTION("Error in opening file " << binaryName);
    if((this->elf_pointer = elf_begin(this->elfFd, ELF_C_READ, NULL)) == NULL){
        THROW_ERROR("Error in initializing the ELF descriptor related to file " << binaryName);
    }
    if(elf_kind (this->elf_pointer) != ELF_K_ELF){
        THROW_ERROR(" File " << binaryName << " is not a valid ELF type: only executable files are allowed");
    }

    //Now it's time to start reading the content: the program headers for the actual code and
    //the symbols sections for the correspondence routines - addresses (I do not need any other
    //symbol)
    GElf_Ehdr elfExecHeader;
    if(gelf_getehdr(this->elf_pointer, &elfExecHeader) == NULL){
        THROW_ERROR("Error in reading the executable header " << elf_errmsg ( -1));
    }
    switch(gelf_getclass(this->elf_pointer)){
        case ELFCLASS32:
            this->wordsize = 4;
        break;
        case ELFCLASS64:
            this->wordsize = 8;
        break;
        default:
            THROW_ERROR(" File " << binaryName << " is not a valid ELF type: only executable files are allowed");
        break;
    }
    this->entryPoint = elfExecHeader.e_entry;
    if(elfExecHeader.e_type != ET_EXEC){
        THROW_ERROR(" File " << binaryName << " is not a valid ELF type: only executable files are allowed");
    }
    readProgramData();
    readSymbols();

    if(this->elf_pointer != NULL){
        if(elf_end(this->elf_pointer) != 0){
            if(this->elfFd > 0){
                close(this->elfFd);
            }
            //An Error has occurred; lets see what it is
            THROW_ERROR("Error in closing the elf parser --> " << elf_errmsg(-1));
        }
        this->elf_pointer = NULL;
    }
    if(this->elfFd > 0){
        close(this->elfFd);
    }
}

trap::ELFFrontend::~ELFFrontend(){
}

///Reads the program instructions contained in the file
void trap::ELFFrontend::readProgramData(){
    size_t numProgSegments = 0;
    GElf_Phdr elfProgHeader;
    if(elf_getphdrnum(this->elf_pointer, &numProgSegments) != 0){
        THROW_ERROR("Error in retrieving the number of program headers: " << elf_errmsg ( -1));
    }

    std::map<unsigned int, unsigned char> memMap;
    for(size_t i = 0; i < numProgSegments; i++){
        if(gelf_getphdr(this->elf_pointer, i, &elfProgHeader) == NULL){
            THROW_ERROR("Error in retireving program header " << i);
        }
        if(elfProgHeader.p_type == PT_LOAD){
            //Found a standard loadable segment: I can put its content into the executable
            //image
            std::map<unsigned int, unsigned char>::iterator curMapPos = memMap.end();
            if(elfProgHeader.p_filesz > 0){
                unsigned char * fileContent = new unsigned char[elfProgHeader.p_filesz];
                //Now I have to actually read the strean if bytes from the executable file
                lseek(this->elfFd, elfProgHeader.p_offset, SEEK_SET);
                if(read(this->elfFd, (void *)fileContent, elfProgHeader.p_filesz) != static_cast<int>(elfProgHeader.p_filesz)){
                    THROW_ERROR("Error in reading the content of program section at virtual address " << std::hex << std::showbase << elfProgHeader.p_vaddr << " of size " << elfProgHeader.p_filesz << std::dec);
                }
                for(size_t curPos = 0; curPos < elfProgHeader.p_filesz; curPos++){
                    curMapPos = memMap.insert(curMapPos, std::pair<unsigned int, unsigned char>(elfProgHeader.p_vaddr + curPos, fileContent[curPos]));
                }
                delete [] fileContent;
            }
            for(size_t curPos = elfProgHeader.p_filesz; curPos < elfProgHeader.p_memsz; curPos++){
                curMapPos = memMap.insert(curMapPos, std::pair<unsigned int, unsigned char>(elfProgHeader.p_vaddr + curPos, 0));
            }
        }
    }
    //Ok: now finally, I can take the map and convert it into the unsigned char * array representing the various
    //bytes in memory
    this->codeSize.second = memMap.begin()->first;
    this->codeSize.first = memMap.rbegin()->first;
    this->programData = new unsigned char[this->codeSize.first - this->codeSize.second];
    std::memset(this->programData, 0, this->codeSize.first - this->codeSize.second);
    std::map<unsigned int, unsigned char>::iterator memBeg,  memEnd;
    for(memBeg = memMap.begin(),  memEnd = memMap.end(); memBeg != memEnd; memBeg++){
        this->programData[memBeg->first - this->codeSize.second] = memBeg->second;
    }
}

///Now I have to read the symbols contained into the file, mapping them
///to thei address
void trap::ELFFrontend::readSymbols(){
    size_t secNameIndex = 0;
    Elf_Scn *elfSection = NULL;
    GElf_Shdr elfSecHeader;
    if(elf_getshdrstrndx(this->elf_pointer, &secNameIndex) != 0){
        THROW_ERROR("Error while reading the section string index: " << elf_errmsg ( -1));
    }
    while((elfSection = elf_nextscn(this->elf_pointer, elfSection)) != NULL){
        if(gelf_getshdr(elfSection, &elfSecHeader) == NULL){
            THROW_ERROR("Error in retrieving the section header: " << elf_errmsg ( -1));
        }
        if(elfSecHeader.sh_type == SHT_SYMTAB){
            Elf_Data *secData = NULL;
            if((secData = elf_getdata(elfSection, secData)) == NULL){
                THROW_ERROR("Error in retrieving the section data: " << elf_errmsg ( -1));
            }
            // how many symbols are there? this number comes from the size of
            // the section divided by the entry size
            unsigned int symbol_count = elfSecHeader.sh_size / elfSecHeader.sh_entsize;

            // loop through to grab all symbols
            for(unsigned int i = 0; i < symbol_count; i++){
                GElf_Sym sym;
                // libelf grabs the symbol data using gelf_getsym()
                gelf_getsym(secData, i, &sym);
                //now I get the symbol; I am only interested in
                // the functions
                if(ELF32_ST_TYPE(sym.st_info) == STT_FUNC){
                    // the name of the symbol is somewhere in a string table
                    // we know which one using the shdr.sh_link member
                    // libelf grabs the string using elf_strptr()
                    char * originalName = elf_strptr(this->elf_pointer, elfSecHeader.sh_link, sym.st_name);
                    char * demangledName = NULL;
#if defined(HAVE_ABI____CXA_DEMANGLE) && defined(HAVE_CXXABI_H)
                    int demangleStatus = 0;
                    demangledName = abi::__cxa_demangle(originalName, NULL, NULL, &demangleStatus);
#endif
                    if(demangledName != NULL){
                        this->addrToSym[sym.st_value].push_back(demangledName);
                        this->symToAddr[demangledName] = sym.st_value;
                        free(demangledName);
                    }
                    else{
                        this->addrToSym[sym.st_value].push_back(originalName);
                        this->symToAddr[originalName] = sym.st_value;
                    }
                }
            }
        }
    }
}

///Given an address, it returns the symbols found there,(more than one
///symbol can be mapped to an address). Note
///That if address is in the middle of a function, the symbol
///returned refers to the function itself
std::list<std::string> trap::ELFFrontend::symbolsAt(unsigned int address) const throw(){
    vmap<unsigned int, std::list<std::string> >::const_iterator symMap1 = this->addrToSym.find(address);
    if(symMap1 == this->addrToSym.end()){
        vmap<unsigned int, std::string>::const_iterator symMap2 = this->addrToFunction.find(address);
        std::list<std::string> functionsList;
        if(symMap2 != this->addrToFunction.end())
            functionsList.push_back(symMap2->second);
        return functionsList;
    }
    return symMap1->second;
}

///Given an address, it returns the first symbol found there
///"" if no symbol is found at the specified address; note
///That if address is in the middle of a function, the symbol
///returned refers to the function itself
std::string trap::ELFFrontend::symbolAt(unsigned int address) const throw(){
    vmap<unsigned int, std::list<std::string> >::const_iterator symMap1 = this->addrToSym.find(address);
    if(symMap1 == this->addrToSym.end()){
        vmap<unsigned int, std::string>::const_iterator symMap2 = this->addrToFunction.find(address);
        if(symMap2 != this->addrToFunction.end()){
            return symMap2->second;
        }
        else{
            return "";
        }
    }
    return symMap1->second.front();
}

///Given the name of a symbol it returns its value
///(which usually is its address);
///valid is set to false if no symbol with the specified
///name is found
unsigned int trap::ELFFrontend::getSymAddr(const std::string &symbol, bool &valid) const throw(){
    std::map<std::string, unsigned int>::const_iterator addrMap = this->symToAddr.find(symbol);
    if(addrMap == this->symToAddr.end()){
        valid = false;
        return 0;
    }
    else{
        valid = true;
        return addrMap->second;
    }
}

///Returns the name of the executable file
std::string trap::ELFFrontend::getExecName() const{
    return this->execName;
}

///Returns the end address of the loadable code
unsigned int trap::ELFFrontend::getBinaryEnd() const{
    return (this->codeSize.first + this->wordsize + (this->wordsize - (this->codeSize.first % this->wordsize)));
}

///Specifies whether the address is the first one of a rountine
bool trap::ELFFrontend::isRoutineEntry(unsigned int address) const{
    vmap<unsigned int, std::string>::const_iterator funNameIter = this->addrToFunction.find(address);
    vmap<unsigned int, std::string>::const_iterator endFunNames = this->addrToFunction.end();
    if(funNameIter == endFunNames)
        return false;
    std::string curName = funNameIter->second;
    funNameIter = this->addrToFunction.find(address + this->wordsize);
    if(funNameIter != endFunNames && curName == funNameIter->second){
        funNameIter = this->addrToFunction.find(address - this->wordsize);
        if(funNameIter == endFunNames || curName != funNameIter->second)
            return true;
    }
    return false;
}

///Specifies whether the address is the last one of a routine
bool trap::ELFFrontend::isRoutineExit(unsigned int address) const{
    vmap<unsigned int, std::string>::const_iterator funNameIter = this->addrToFunction.find(address);
    vmap<unsigned int, std::string>::const_iterator endFunNames = this->addrToFunction.end();
    if(funNameIter == endFunNames)
        return false;
    std::string curName = funNameIter->second;
    funNameIter = this->addrToFunction.find(address - this->wordsize);
    if(funNameIter != endFunNames && curName == funNameIter->second){
        funNameIter = this->addrToFunction.find(address + this->wordsize);
        if(funNameIter == endFunNames || curName != funNameIter->second)
            return true;
    }
    return false;
}

///Given an address, it sets fileName to the name of the source file
///which contains the code and line to the line in that file. Returns
///false if the address is not valid
bool trap::ELFFrontend::getSrcFile(unsigned int address, std::string &fileName, unsigned int &line) const{
    vmap<unsigned int, std::pair<std::string, unsigned int> >::const_iterator srcMap = this->addrToSrc.find(address);
    if(srcMap == this->addrToSrc.end()){
        return false;
    }
    else{
        fileName = srcMap->second.first;
        line = srcMap->second.second;
        return true;
    }
}

///Returns the start address of the loadable code
unsigned int trap::ELFFrontend::getBinaryStart() const{
    return this->codeSize.second;
}

///Returns the entry point of the executable code
unsigned int trap::ELFFrontend::getEntryPoint() const{
    return this->entryPoint;
}

///Returns a pointer to the array contianing the program data
unsigned char * trap::ELFFrontend::getProgData(){
    return this->programData;
}
