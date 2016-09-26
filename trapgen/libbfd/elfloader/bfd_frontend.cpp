/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     bfd_frontend.cpp
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
#include <bfd.h>
}

#include <sys/types.h>
#include <cstdio>
#include <cstdarg>

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

#define DMGL_NO_OPTS     0              // For readability.
#define DMGL_PARAMS      (1 << 0)       // Include function args.
#define DMGL_ANSI        (1 << 1)       // Include const, volatile, etc.
#define DMGL_JAVA        (1 << 2)       // Demangle as Java rather than C++.
#define DMGL_VERBOSE     (1 << 3)       // Include implementation details.
#define DMGL_TYPES       (1 << 4)       // Also try to demangle type encodings.
#define DMGL_RET_POSTFIX (1 << 5)       // Print function return types (when present) after function signature.

#define DMGL_AUTO        (1 << 8)
#define DMGL_GNU         (1 << 9)
#define DMGL_LUCID       (1 << 10)
#define DMGL_ARM         (1 << 11)
#define DMGL_HP          (1 << 12)
#define DMGL_EDG         (1 << 13)
#define DMGL_GNU_V3      (1 << 14)
#define DMGL_GNAT        (1 << 15)

// binutils versions < 2.18 use cplus_demangle instead of the bfd_demangle
// function. Note how this function is only internally used by binutils, so it
// is not available in any header file and it needs to be declared as external.
#ifdef OLD_BFD
extern "C" {
extern char* cplus_demangle (const char* mangled, int options);
}
#endif

std::map<std::string, trap::ELFFrontend*> trap::ELFFrontend::cur_instance;

trap::ELFFrontend::ELFFrontend(std::string binary_name) {
  char** matching = NULL;
  this->sy = NULL;

  bfd_init();
  this->exec_image = bfd_openr(binary_name.c_str(), "default");
  if (this->exec_image == NULL) {
    THROW_ERROR("Cannot open input file " << binary_name << ": " << bfd_errmsg(bfd_get_error()) << '.');
  }
  if (bfd_check_format (this->exec_image, bfd_archive)) {
    THROW_ERROR("Invalid input file " << binary_name << ", expected executable file, not archive.");
  }
  if (!bfd_check_format_matches (this->exec_image, bfd_object, &matching)) {
    THROW_ERROR("Invalid input file " << binary_name << ", the input file is not an object file or the target is ambiguous: " << this->get_matching_formats(matching) << '.');
  }

  this->word_size = bfd_get_arch_size(this->exec_image)/(8*bfd_octets_per_byte(this->exec_image));

  // Read the different sections and save them in a temporary vector.
  struct bfd_section* p = NULL;
  bfd_vma global_start_addr = (bfd_vma)-1;
  bfd_size_type global_end_addr = 0;
  for (p = this->exec_image->sections; p != NULL; p = p->next) {
    flagword flags = bfd_get_section_flags(this->exec_image, p);
    if ((flags & SEC_ALLOC) != 0 && (flags & SEC_DEBUGGING) == 0 && (flags & SEC_THREAD_LOCAL) == 0) {
      bfd_size_type data_size = bfd_section_size(this->exec_image, p);
      bfd_vma vma = bfd_get_section_vma(this->exec_image, p);
/*#ifndef NDEBUG
      std::cerr << "Section " << p->name << " Start Address " << std::hex << vma << " Size " << std::hex << data_size << " End Address " << std::hex << data_size + vma << " flags " << std::hex << std::showbase << flags << std::dec << std::endl;
#endif*/
      if ((data_size + vma) > global_end_addr)
        global_end_addr = data_size + vma;
      if (global_start_addr > vma || global_start_addr == (bfd_vma)-1)
        global_start_addr = vma;
      if ((flags & SEC_HAS_CONTENTS) != 0) {
        Section sec;
        sec.data_size = data_size;
        sec.type = flags;
        sec.start_addr = vma;
        sec.data = new bfd_byte[sec.data_size];
        sec.descriptor = p;
        sec.name = p->name;
        bfd_get_section_contents (this->exec_image, p, sec.data, 0, sec.data_size);
        this->sec_list.push_back(sec);
      }
    }
  }
  this->code_size.first = global_end_addr;
  this->code_size.second = global_start_addr;
  this->exec_name = bfd_get_filename(this->exec_image);

  if (!(bfd_get_file_flags (this->exec_image) & HAS_SYMS)) {
    THROW_ERROR("No symbols found in file " << bfd_get_filename(this->exec_image) << '.');
  }
  int storage = bfd_get_symtab_upper_bound(this->exec_image);
  if (storage < 0) {
    THROW_ERROR("Cannot get symbol table upper bound in file " << bfd_get_filename(this->exec_image) << ": " << bfd_errmsg(bfd_get_error()) << '.');
  }
  if (storage != 0)
    this->sy = (asymbol**)malloc (storage);
  if (this->sy == NULL) {
    THROW_ERROR("Cannot allocate space for symbol storage in file " << bfd_get_filename(this->exec_image) << '.');
  }
  long sym_count = 0;
  sym_count = bfd_canonicalize_symtab (this->exec_image, this->sy);
  if (sym_count < 0) {
    THROW_ERROR("Cannot get symbol count in file " << bfd_get_filename(this->exec_image) << ": " << bfd_errmsg(bfd_get_error()) << '.');
  }

  // Call the various functions which extract all the necessary information
  // form the BFD.
  this->read_syms();
  this->read_src();

  // Deallocate part of the memory.
  std::vector<Section>::iterator sections_it, sections_end;
  for (sections_it = this->sec_list.begin(), sections_end = this->sec_list.end(); sections_it != sections_end; sections_it++) {
    delete [] sections_it->data;
  }
  this->sec_list.clear();
  free(this->sy);
} // ELFFrontend::ELFFrontend()

/// ----------------------------------------------------------------------------

trap::ELFFrontend::~ELFFrontend() {
  if (this->exec_image != NULL) {
    if (!bfd_close_all_done(this->exec_image)) {
      THROW_ERROR("Cannot close binary parser: " << bfd_errmsg(bfd_get_error()) << '.');
    }
    this->exec_image = NULL;
  }
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
  for (it = ELFFrontend::cur_instance.begin(), it_end = ELFFrontend::cur_instance.it_end(); it != it_end; it++) {
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

/// Returns the start address of the loadable code.
unsigned trap::ELFFrontend::get_binary_start() const {
  return this->code_size.second;
} // ELFFrontend::get_binary_start()

/// ----------------------------------------------------------------------------

/// Returns the end address of the loadable code.
unsigned trap::ELFFrontend::get_binary_end() const {
  return (this->code_size.first + this->word_size);
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
  std::map<std::string, unsigned>::const_iterator addr_it = this->sym_to_addr.find(symbol);
  if (addr_it == this->sym_to_addr.end()) {
    valid = false;
    return 0;
  } else {
    valid = true;
    return addr_it->second;
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

/// Accesses the BFD internal structures in order to get the symbols.
void trap::ELFFrontend::read_syms() {
  // Make sure there are symbols in the file.
  if ((bfd_get_file_flags (this->exec_image) & HAS_SYMS) == 0)
    return;

  long sym_count = 0;
  unsigned size = 0;
  void* mini_syms = NULL;
  char* name = NULL;
  sym_count = bfd_read_minisymbols(this->exec_image, 0, &mini_syms, &size);
  asymbol* store = NULL;
  bfd_byte *from_beg = NULL, *from_end = NULL;
  store = bfd_make_empty_symbol(this->exec_image);
  if (store == NULL) {
    THROW_ERROR("Cannot allocate space for symbols from file " << bfd_get_filename(this->exec_image) << ": " << bfd_errmsg(bfd_get_error()) << '.');
  }
  from_beg = (bfd_byte*)mini_syms;
  from_end = from_beg + sym_count * size;
  for (; from_beg < from_end; from_beg += size) {
    asymbol* sym = NULL;
    symbol_info sym_info;

    sym = bfd_minisymbol_to_symbol (this->exec_image, 0, from_beg, store);
    if (sym == NULL) {
      THROW_ERROR("Cannot get symbol from file " << bfd_get_filename(this->exec_image) << ": " << bfd_errmsg(bfd_get_error()) << '.');
    }
    bfd_get_symbol_info (this->exec_image, sym, &sym_info);

    if ((sym->flags & BSF_DEBUGGING) || bfd_is_target_special_symbol(this->exec_image, sym) ||
          (char)sym_info.type == 'b' || (char)sym_info.type == 'r' || (char)sym_info.type == 'a' || bfd_is_undefined_symclass (sym_info.type)) {
      continue;
    }

    name = (char*)sym_info.name;
    if (name[0] == '$')
      continue;

    this->addr_to_sym[sym_info.value].push_back(name);
    this->sym_to_addr[name] = sym_info.value;
  }
} // ELFFrontend::read_syms()

/// ----------------------------------------------------------------------------

/// Accesses the BFD internal structures in order to get correspondence among
/// machine code and the source code.
void trap::ELFFrontend::read_src() {
  std::vector<Section>::iterator sections_it, sections_end;
  for (sections_it = this->sec_list.begin(), sections_end = this->sec_list.end(); sections_it != sections_end; sections_it++) {
    if ((sections_it->type & SEC_DATA) == 0) {
      // I skip DATA sections as they do not contain usefull information
      for (bfd_vma i = 0; i < sections_it->data_size; i += this->word_size) {
        const char* filename = NULL;
        const char* functionname = NULL;
        unsigned line = 0;

        if (!bfd_find_nearest_line(this->exec_image, sections_it->descriptor, this->sy, i, &filename, &functionname, &line)) {
          continue;
        }

        if (filename != NULL && *filename == '\0')
          filename = NULL;
        if (functionname != NULL && *functionname == '\0')
          functionname = NULL;

        //if (functionname != NULL && this->addr_to_func.find(i + sections_it->start_addr) == this->addr_to_func.end()) {
        if (functionname != NULL) {
#ifdef OLD_BFD
          char* name = cplus_demangle(functionname, DMGL_ANSI | DMGL_PARAMS);
#else
          char* name = bfd_demangle (this->exec_image, functionname, DMGL_ANSI | DMGL_PARAMS);
#endif
          if (name == NULL)
            name = (char*)functionname;
          this->addr_to_func[i + sections_it->start_addr] = name;
        }
        //if (line > 0 && this->addr_to_src.find(i + sections_it->start_addr) == this->addr_to_src.end())
        if (line > 0)
          this->addr_to_src[i + sections_it->start_addr] = std::pair<std::string, unsigned>(filename == NULL ? "???" : filename, line);
      }
    }
  }
} // ELFFrontend::read_src()

/// ----------------------------------------------------------------------------

/// In case it is not possible to open the BFD because it is not possible to
/// determine its target, this function extracts the list of possible targets.
std::string trap::ELFFrontend::get_matching_formats(char** p) const {
  std::string match = "";
  if (p != NULL) {
    while (*p) {
      match += *p;
      *p++;
      match += " ";
    }
  }
  return match;
} // ELFFrontend::get_matching_formats()

/// ****************************************************************************
