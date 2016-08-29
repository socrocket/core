/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     elf_frontend.cpp
* @brief    This file is part of the TRAP runtime library.
* @details
* @author   Luca Fossati
* @author   Lillian Tadros (Technische Universitaet Dortmund)
* @date     2008-2013 Luca Fossati
*           2015-2016 Technische Universitaet Dortmund
* @copyright
*
* This file is part of TRAP.
*
* TRAP is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
* or see <http://www.gnu.org/licenses/>.
*
* (c) Luca Fossati, fossati@elet.polimi.it, fossati.l@gmail.com
*
*******************************************************************************/
#include "elf_frontend.hpp"
#include "common/report.hpp"

extern "C" {
#include <gelf.h>
}

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

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

#ifdef __GNUC__
#ifdef __GNUC_MINOR__
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 3)
#include <tr1/unordered_map>
#define template_map std::tr1::unordered_map
#else
#include <ext/hash_map>
#define  template_map __gnu_cxx::hash_map
#endif
#else
#include <ext/hash_map>
#define  template_map __gnu_cxx::hash_map
#endif
#else
#ifdef _WIN32
#include <hash_map>
#define  template_map stdext::hash_map
#else
#include <map>
#define  template_map std::map
#endif
#endif

#include <map>
#include <string>
#include <vector>
#include <list>
#include <iostream>

std::map<std::string, trap::ELFFrontend*> trap::ELFFrontend::cur_instance;

trap::ELFFrontend::ELFFrontend(std::string binary_name) : exec_name(binary_name) {
  //Let's open the elf parser and check that everything is all right
  if (elf_version(EV_CURRENT) == EV_NONE) {
    THROW_ERROR("Wrong ELF library version.");
  }
  boost::filesystem::path filepath = boost::filesystem::system_complete(boost::filesystem::path(binary_name));
  if (!boost::filesystem::exists(filepath)) {
    THROW_EXCEPTION("Path " << binary_name << " does not exist.");
  }
  this->elf_image = open(binary_name.c_str(), O_RDONLY, 0);
  if (this->elf_image < 0)
    THROW_EXCEPTION("Cannot open file " << binary_name << '.');
  if ((this->elf_ptr = elf_begin(this->elf_image, ELF_C_READ, NULL)) == NULL) {
    THROW_ERROR("Cannot initialize ELF descriptor belonging to file " << binary_name << '.');
  }
  if (elf_kind (this->elf_ptr) != ELF_K_ELF) {
    THROW_ERROR("Invalid ELF file " << binary_name << ", expected executable file.");
  }

  //Now it's time to start reading the content: the program headers for the actual code and
  //the symbols sections for the correspondence routines - addresses (I do not need any other
  //symbol)
  GElf_Ehdr elf_exec_header;
  if (gelf_getehdr(this->elf_ptr, &elf_exec_header) == NULL) {
    THROW_ERROR("Cannot read executable header " << elf_errmsg (-1) << '.');
  }
  switch (gelf_getclass(this->elf_ptr)) {
    case ELFCLASS32:
      this->word_size = 4;
    break;
    case ELFCLASS64:
      this->word_size = 8;
    break;
    default:
      THROW_ERROR("Invalid ELF file " << binary_name << ", expected executable file.");
    break;
  }
  this->entry_point = elf_exec_header.e_entry;
  if (elf_exec_header.e_type != ET_EXEC) {
    THROW_ERROR("Invalid ELF file " << binary_name << ", expected executable file.");
  }
  read_program_data();
  read_symbols();

  if (this->elf_ptr != NULL) {
    if (elf_end(this->elf_ptr) != 0) {
      if (this->elf_image > 0) {
        close(this->elf_image);
      }
      //An Error has occurred; lets see what it is
      THROW_ERROR("Cannot close ELF parser: " << elf_errmsg(-1) << '.');
    }
    this->elf_ptr = NULL;
  }
  if (this->elf_image > 0) {
    close(this->elf_image);
  }
} // ELFFrontend::ELFFrontend()

/// ----------------------------------------------------------------------------

trap::ELFFrontend::~ELFFrontend() {
} // ELFFrontend::~ELFFrontend()

/// ----------------------------------------------------------------------------

trap::ELFFrontend& trap::ELFFrontend::get_instance(std::string filename) {
  if (ELFFrontend::cur_instance.find(filename) == ELFFrontend::cur_instance.end()) {
    ELFFrontend::cur_instance[filename] = new ELFFrontend(filename);
  }
  return *ELFFrontend::cur_instance[filename];
} // ELFFrontend::get_instance()

/// ----------------------------------------------------------------------------

void trap::ELFFrontend::reset() {
  std::map<std::string, trap::ELFFrontend*>::iterator it, it_end;
  for (it = ELFFrontend::cur_instance.begin(), it_end = ELFFrontend::cur_instance.end(); it != it_end; it++) {
    delete it->second;
  }
  ELFFrontend::cur_instance.clear();
} // ELFFrontend::reset()

/// ----------------------------------------------------------------------------

/// Returns the name of the executable file.
std::string trap::ELFFrontend::get_exec_name() const {
  return this->exec_name;
} // ELFFrontend::get_exec_name()

/// ----------------------------------------------------------------------------

/// Given an address, it sets filename to the name of the source file which
/// contains the code and line to the line in that file. Returns false if the
/// address is not valid.
bool trap::ELFFrontend::get_src_file(unsigned address, std::string& filename, unsigned& line) const {
  template_map<unsigned, std::pair<std::string, unsigned> >::const_iterator src_map = this->addr_to_src.find(address);
  if (src_map == this->addr_to_src.end()) {
    return false;
  } else {
    filename = src_map->second.first;
    line = src_map->second.second;
    return true;
  }
} // ELFFrontend::get_src_file()

/// ----------------------------------------------------------------------------

/// Returns the entry point of the executable code.
unsigned trap::ELFFrontend::get_entry_point() const {
  return this->entry_point;
} // ELFFrontend::get_entry_point()

/// ----------------------------------------------------------------------------

/// Returns a pointer to the array contianing the program data.
unsigned char* trap::ELFFrontend::get_program_data() {
  return this->program_data;
} // ELFFrontend::get_program_data()

/// ----------------------------------------------------------------------------

/// Returns the start address of the loadable code.
unsigned trap::ELFFrontend::get_binary_start() const {
  return this->code_size.second;
} // ELFFrontend::get_binary_start()

/// ----------------------------------------------------------------------------

/// Returns the end address of the loadable code.
unsigned trap::ELFFrontend::get_binary_end() const {
  return (this->code_size.first + this->word_size + (this->word_size - (this->code_size.first % this->word_size)));
} // ELFFrontend::get_binary_end()

/// ----------------------------------------------------------------------------

/// Given an address, it returns the first symbol found there, "" if no symbol
/// is found at the specified address. Note that if address is in the middle of
/// a function, the symbol returned refers to the function itself.
std::string trap::ELFFrontend::symbol_at(unsigned address) const throw() {
  template_map<unsigned, std::list<std::string> >::const_iterator sym_it1 = this->addr_to_sym.find(address);
  if (sym_it1 == this->addr_to_sym.end()) {
    template_map<unsigned, std::string>::const_iterator sym_it2 = this->addr_to_func.find(address);
    if (sym_it2 != this->addr_to_func.end()) {
      return sym_it2->second;
    } else {
      return "";
    }
  }
  return sym_it1->second.front();
} // ELFFrontend::symbol_at()

/// ----------------------------------------------------------------------------

/// Given an address, it returns the symbols found there,(more than one symbol
/// can be mapped to an address). Note that if address is in the middle of a
/// function, the symbol returned refers to the function itself.
std::list<std::string> trap::ELFFrontend::symbols_at(unsigned address) const throw() {
  template_map<unsigned, std::list<std::string> >::const_iterator sym_it1 = this->addr_to_sym.find(address);
  if (sym_it1 == this->addr_to_sym.end()) {
    template_map<unsigned, std::string>::const_iterator sym_it2 = this->addr_to_func.find(address);
    std::list<std::string> func_list;
    if (sym_it2 != this->addr_to_func.end())
      func_list.push_back(sym_it2->second);
    return func_list;
  }
  return sym_it1->second;
} // ELFFrontend::symbols_at()

/// ----------------------------------------------------------------------------

/// Given the name of a symbol it returns its value (which usually is its
/// address). valid is set to false if no symbol with the specified name is
/// found.
unsigned trap::ELFFrontend::get_sym_addr(const std::string& symbol, bool& valid) const throw() {
  std::map<std::string, unsigned>::const_iterator addr_map = this->sym_to_addr.find(symbol);
  if (addr_map == this->sym_to_addr.end()) {
    valid = false;
    return 0;
  } else {
    valid = true;
    return addr_map->second;
  }
} // ELFFrontend::get_sym_addr()

/// ----------------------------------------------------------------------------

/// Specifies whether the address is the first one of a rountine.
bool trap::ELFFrontend::is_routine_entry(unsigned address) const {
  template_map<unsigned, std::string>::const_iterator fun_name_it = this->addr_to_func.find(address);
  template_map<unsigned, std::string>::const_iterator fun_name_end = this->addr_to_func.end();
  if (fun_name_it == fun_name_end)
    return false;
  std::string cur_name = fun_name_it->second;
  fun_name_it = this->addr_to_func.find(address + this->word_size);
  if (fun_name_it != fun_name_end && cur_name == fun_name_it->second) {
    fun_name_it = this->addr_to_func.find(address - this->word_size);
    if (fun_name_it == fun_name_end || cur_name != fun_name_it->second)
      return true;
  }
  return false;
} // ELFFrontend::is_routine_entry()

/// ----------------------------------------------------------------------------

/// Specifies whether the address is the last one of a rountine.
bool trap::ELFFrontend::is_routine_exit(unsigned address) const {
  template_map<unsigned, std::string>::const_iterator fun_name_it = this->addr_to_func.find(address);
  template_map<unsigned, std::string>::const_iterator fun_name_end = this->addr_to_func.end();
  if (fun_name_it == fun_name_end)
    return false;
  std::string cur_name = fun_name_it->second;
  fun_name_it = this->addr_to_func.find(address - this->word_size);
  if (fun_name_it != fun_name_end && cur_name == fun_name_it->second) {
    fun_name_it = this->addr_to_func.find(address + this->word_size);
    if (fun_name_it == fun_name_end || cur_name != fun_name_it->second)
      return true;
  }
  return false;
} // ELFFrontend::is_routine_exit()

/// ----------------------------------------------------------------------------

// Read the binary instructions from the executable file.
void trap::ELFFrontend::read_program_data() {
  size_t num_program_segments = 0;
  GElf_Phdr elf_program_header;
  if (elf_getphdrnum(this->elf_ptr, &num_program_segments) != 0) {
    THROW_ERROR("Cannot retrieve number of program headers: " << elf_errmsg (-1) << '.');
  }

  std::map<unsigned, unsigned char> mem_map;
  for (size_t i = 0; i < num_program_segments; i++) {
    if (gelf_getphdr(this->elf_ptr, i, &elf_program_header) == NULL) {
      THROW_ERROR("Cannot retireve program header " << i << '.');
    }
    if (elf_program_header.p_type == PT_LOAD) {
      // Found a standard loadable segment. Put its content into the executable
      // image.
      std::map<unsigned, unsigned char>::iterator mem_it = mem_map.end();
      if (elf_program_header.p_filesz > 0) {
        unsigned char* file_content = new unsigned char[elf_program_header.p_filesz];
        // Read the stream of bytes from the executable file.
        lseek(this->elf_image, elf_program_header.p_offset, SEEK_SET);
        if (read(this->elf_image, (void*)file_content, elf_program_header.p_filesz) != static_cast<int>(elf_program_header.p_filesz)) {
          THROW_ERROR("Cannot read content of program section at virtual address " << std::hex << std::showbase << elf_program_header.p_vaddr << " of size " << elf_program_header.p_filesz << std::dec << '.');
        }
        for (size_t i = 0; i < elf_program_header.p_filesz; i++) {
          mem_it = mem_map.insert(mem_it, std::pair<unsigned, unsigned char>(elf_program_header.p_vaddr + i, file_content[i]));
        }
        delete [] file_content;
      }
      for (size_t i = elf_program_header.p_filesz; i < elf_program_header.p_memsz; i++) {
        mem_it = mem_map.insert(mem_it, std::pair<unsigned, unsigned char>(elf_program_header.p_vaddr + i, 0));
      }
    }
  }
  // Convert the map into the unsigned char* array representing the various
  // bytes in memory.
  this->code_size.second = mem_map.begin()->first;
  this->code_size.first = mem_map.rbegin()->first;
  this->program_data = new unsigned char[this->code_size.first - this->code_size.second];
  std::memset(this->program_data, 0, this->code_size.first - this->code_size.second);
  std::map<unsigned, unsigned char>::iterator mem_it, mem_end;
  for (mem_it = mem_map.begin(), mem_end = mem_map.end(); mem_it != mem_end; mem_it++) {
    this->program_data[mem_it->first - this->code_size.second] = mem_it->second;
  }
} // ELFFrontend::read_program_data()

/// ----------------------------------------------------------------------------

// Read the symbols from the executable file and map them to their address.
void trap::ELFFrontend::read_symbols() {
  size_t sec_name_idx = 0;
  Elf_Scn* elf_section = NULL;
  GElf_Shdr elf_section_header;
  if (elf_getshdrstrndx(this->elf_ptr, &sec_name_idx) != 0) {
    THROW_ERROR("Cannot read section string index: " << elf_errmsg (-1) << '.');
  }
  while((elf_section = elf_nextscn(this->elf_ptr, elf_section)) != NULL) {
    if (gelf_getshdr(elf_section, &elf_section_header) == NULL) {
      THROW_ERROR("Cannot retrieve section header: " << elf_errmsg (-1) << '.');
    }
    if (elf_section_header.sh_type == SHT_SYMTAB) {
      Elf_Data* sec_data = NULL;
      if ((sec_data = elf_getdata(elf_section, sec_data)) == NULL) {
        THROW_ERROR("Cannot retrieve section data: " << elf_errmsg (-1) << '.');
      }
      // how many symbols are there? this number comes from the size of
      // the section divided by the entry size
      unsigned symbol_count = elf_section_header.sh_size / elf_section_header.sh_entsize;

      // loop through to grab all symbols
      for (unsigned i = 0; i < symbol_count; i++) {
        GElf_Sym sym;
        // libelf grabs the symbol data using gelf_getsym()
        gelf_getsym(sec_data, i, &sym);
        //now I get the symbol; I am only interested in
        // the functions
        if (ELF32_ST_TYPE(sym.st_info) == STT_FUNC) {
          // the name of the symbol is somewhere in a string table
          // we know which one using the shdr.sh_link member
          // libelf grabs the string using elf_strptr()
          char* name_original = elf_strptr(this->elf_ptr, elf_section_header.sh_link, sym.st_name);
          char* name_demangled = NULL;
#if defined(HAVE_ABI____CXA_DEMANGLE) && defined(HAVE_CXXABI_H)
          int demangle_status = 0;
          name_demangled = abi::__cxa_demangle(name_original, NULL, NULL, &demangle_status);
#endif
          if (name_demangled != NULL) {
            this->addr_to_sym[sym.st_value].push_back(name_demangled);
            this->sym_to_addr[name_demangled] = sym.st_value;
            free(name_demangled);
          } else {
            this->addr_to_sym[sym.st_value].push_back(name_original);
            this->sym_to_addr[name_original] = sym.st_value;
          }
        }
      }
    }
  }
} // ELFFrontend::read_symbols()

/// ****************************************************************************
