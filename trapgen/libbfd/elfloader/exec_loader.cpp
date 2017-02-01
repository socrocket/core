/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     exec_loader.cpp
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

#include <string>
#include <map>
#include <iostream>

#include "exec_loader.hpp"
#include "common/report.hpp"

extern "C" {
#include <bfd.h>
}

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

/// Initializes the loader of executable files by creating the corresponding bfd
/// image of the executable file specified as parameter.
trap::ExecLoader::ExecLoader(std::string filename, bool plain_file) : plain_file(plain_file) {
  this->program_data = NULL;
  this->exec_image = NULL;
  this->program_dim = 0;
  this->data_start = 0;
  char** matching = NULL;
  if (plain_file) {
    /// Read the input file, putting all the bytes of its content in the
    /// program_data array.
    boost::filesystem::path file_path = boost::filesystem::system_complete(boost::filesystem::path(filename));
    if (!boost::filesystem::exists(file_path)) {
      THROW_EXCEPTION("Path " << filename << " specified in the executable loader does not exist.");
    }
    std::ifstream plain_exec_file(filename.c_str(), std::ifstream::in | std::ifstream::binary);
    if (!plain_exec_file.good())
      THROW_EXCEPTION("Cannot open file " << filename << '.');

    // Determine the size of the program being loaded.
    plain_exec_file.seekg (0, std::ios::end);
    this->program_dim = plain_exec_file.tellg();
    plain_exec_file.seekg (0, std::ios::beg);
    this->program_data = new unsigned char[this->program_dim];
    // Read the whole file
    plain_exec_file.read((char*)this->program_data, this->program_dim);
    this->data_start = 0;
    plain_exec_file.close();
  } else {
    bfd_init();
    this->exec_image = bfd_openr(filename.c_str(), "default");
    if (this->exec_image == NULL) {
      THROW_ERROR("Cannot open input file " << filename << ": " << bfd_errmsg(bfd_get_error()) << '.');
    }
    if (bfd_check_format (this->exec_image, bfd_archive)) {
      THROW_ERROR("Invalid input file " << filename << ", expected executable file, not archive.");
    }
    if (!bfd_check_format_matches (this->exec_image, bfd_object, &matching)) {
      THROW_ERROR("Invalid input file " << filename << ", the input file is not an object file or the target is ambiguous: " << this->get_matching_formats(matching) << '.');
    }
    this->load_program_data();
  }
} // ExecLoader::ExecLoader()

/// ----------------------------------------------------------------------------

trap::ExecLoader::~ExecLoader() {
  if (this->exec_image != NULL) {
    if (!bfd_close_all_done(this->exec_image)) {
      //An Error has occurred; lets see what it is
      THROW_ERROR("Cannot close binary parser: " << bfd_errmsg(bfd_get_error()) << '.');
    }
  }
  if (this->program_data != NULL) {
    delete [] this->program_data;
  }
} // ExecLoader::~ExecLoader()

/// ----------------------------------------------------------------------------

/// Returns the entry point of the loaded program (usually the same as the
/// program start address, but not always).
unsigned trap::ExecLoader::get_program_start() {
  if (this->exec_image == NULL && !this->plain_file) {
    THROW_ERROR("Binary parser was not created correctly.");
  }
  if (this->plain_file)
    return this->data_start;
  else
    return bfd_get_start_address(this->exec_image);
} // ExecLoader::get_program_start()

/// ----------------------------------------------------------------------------

/// Returns the dimension of the loaded program.
unsigned trap::ExecLoader::get_program_dim() {
  if (this->exec_image == NULL && !this->plain_file) {
    THROW_ERROR("Binary parser was not created correctly.");
  }
  return this->program_dim;
} // ExecLoader::get_program_dim()

/// ----------------------------------------------------------------------------

/// Returns the start address of the program being loaded (the lowest address of
/// the program).
unsigned trap::ExecLoader::get_data_start() {
  if (this->exec_image == NULL && !this->plain_file) {
    THROW_ERROR("Binary parser was not created correctly.");
  }
  return this->data_start;
} // ExecLoader::get_data_start()

/// ----------------------------------------------------------------------------

/// Returns a pointer to the array containing the program data.
unsigned char* trap::ExecLoader::get_program_data() {
  if (this->exec_image == NULL && !this->plain_file) {
    THROW_ERROR("Binary parser was not created correctly.");
  }
  if (this->program_data == NULL) {
    THROW_ERROR("Program data was not computed correctly.");
  }
  return this->program_data;
} // ExecLoader::get_program_data()

/// ----------------------------------------------------------------------------

/// Examines the bfd in order to find the sections containing data to be loaded.
/// Also fills the program_data array.
void trap::ExecLoader::load_program_data() {
  bfd_section* p = NULL;
  std::map<unsigned long, unsigned char> mem_map;
  for (p = this->exec_image->sections; p != NULL; p = p->next) {
    flagword flags = bfd_get_section_flags(this->exec_image, p);
    if ((flags & SEC_ALLOC) != 0 && (flags & SEC_DEBUGGING) == 0 && (flags & SEC_THREAD_LOCAL) == 0) {
      //Ok, this is a section which must be in the final executable;
      //Lets see if it has content: if not I have to pad the section with zeros,
      //otherwise I load it
      bfd_size_type datasize = bfd_section_size(this->exec_image, p);
      bfd_vma vma = bfd_get_section_vma(this->exec_image, p);
      std::map<unsigned long, unsigned char>::iterator mem_it = mem_map.begin();
      if ((flags & SEC_HAS_CONTENTS) != 0) {
/*#ifndef NDEBUG
        std::cerr << "Loading data fom section " << p->name << " Start Address " << std::showbase << std::hex << vma << " Size " << std::hex << datasize << " End Address " << std::hex << datasize + vma << std::dec << " Swap Endianess " << swap_endianess << " flags " << std::hex << std::showbase << flags << std::dec << std::endl;
#endif*/
        bfd_byte* data = new bfd_byte[datasize];
        bfd_get_section_contents (this->exec_image, p, data, 0, datasize);
        for (unsigned i = 0; i < datasize; i++) {
          mem_it = mem_map.insert(mem_it, std::pair<unsigned long, unsigned char>(vma + i, data[i]));
        }
        delete [] data;
      } else {
/*#ifndef NDEBUG
        std::cerr << "Filling with 0s section " << p->name << " Start Address " << std::showbase << std::hex << vma << " Size " << std::hex << datasize << " End Address " << std::hex << datasize + vma << std::dec << std::endl;
#endif*/
        for (unsigned i = 0; i < datasize; i++)
          mem_it = mem_map.insert(mem_it, std::pair<unsigned long, unsigned char>(vma + i, 0));
      }
    }
  }
  //ok,  I now have all the map of the memory; I simply have to fill in the
  //this->program_data  array
  this->data_start = mem_map.begin()->first;
  this->program_dim = mem_map.rbegin()->first - this->data_start + 1;
  this->program_data = new unsigned char[this->program_dim];
  std::map<unsigned long, unsigned char>::iterator mem_it,  mem_end;
  for (mem_it = mem_map.begin(),  mem_end = mem_map.end(); mem_it != mem_end; mem_it++) {
    this->program_data[mem_it->first - this->data_start] = mem_it->second;
  }
} // ExecLoader::load_program_data()

/// ----------------------------------------------------------------------------

std::string trap::ExecLoader::get_matching_formats(char** p) {
  std::string match = "";
  if (p != NULL) {
    while (*p) {
      match += *p;
      *p++;
      match += " ";
    }
  }
  return match;
} // ExecLoader::get_matching_formats()

/// ****************************************************************************
