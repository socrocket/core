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
#include "exec_loader.hpp"
#include "elf_frontend.hpp"
#include "common/report.hpp"

extern "C" {
#include <gelf.h>
}

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <string>
#include <map>
#include <iostream>
#include <fstream>

/// Initializes the loader of executable files by creating the corresponding ELF
/// image of the executable file specified as parameter.
trap::ExecLoader::ExecLoader(std::string filename, bool plain_file) : plain_file(plain_file), elf_frontend(NULL), program_data(NULL) {
  if (plain_file) {
    /// Here I simply have to read the input file, putting all the bytes
    /// of its content in the program_data array
    boost::filesystem::path file_path = boost::filesystem::system_complete(boost::filesystem::path(filename));
    if (!boost::filesystem::exists(file_path)) {
      THROW_EXCEPTION("Path " << filename << " specified in the executable loader does not exist.");
    }
    this->plain_exec_file.open(filename.c_str(), std::ifstream::in | std::ifstream::binary);
    if (!this->plain_exec_file.good())
      THROW_EXCEPTION("Cannot open file " << filename << '.');

    unsigned program_dim = this->get_program_dim();
    this->plain_exec_file.seekg (0, std::ios::beg);
    this->program_data = new unsigned char[program_dim];
    //Now I read the whole file
    this->plain_exec_file.read((char*)this->program_data, program_dim);
  } else {
    this->elf_frontend = &ELFFrontend::get_instance(filename);
  }
} // ExecLoader::ExecLoader()

/// ----------------------------------------------------------------------------

trap::ExecLoader::~ExecLoader() {
  if (this->program_data != NULL) {
    delete [] this->program_data;
    this->program_data = NULL;
  }
  if (this->plain_exec_file.is_open()) {
    this->plain_exec_file.close();
  }
} // ExecLoader::~ExecLoader()

/// ----------------------------------------------------------------------------

/// Returns the entry point of the loaded program (usually the same as the
/// program start address, but not always).
unsigned trap::ExecLoader::get_program_start() {
  if (this->elf_frontend == NULL && !this->plain_file) {
    THROW_ERROR("Binary parser was not created correctly.");
  }
  if (this->plain_file)
    return 0;
  else
    return this->elf_frontend->get_entry_point();
} // ExecLoader::get_program_start()

/// ----------------------------------------------------------------------------

/// Returns the dimension of the loaded program.
unsigned trap::ExecLoader::get_program_dim() {
  if (this->elf_frontend == NULL && !this->plain_file) {
    THROW_ERROR("Binary parser was not created correctly.");
  }
  if (this->plain_file) {
    plain_exec_file.seekg (0, std::ios::end);
    return plain_exec_file.tellg();
  } else {
    return this->elf_frontend->get_binary_end() - this->elf_frontend->get_binary_start();
  }
} // ExecLoader::get_program_dim()

/// ----------------------------------------------------------------------------

/// Returns the start address of the program being loaded (the lowest address of
/// the program).
unsigned trap::ExecLoader::get_data_start() {
  if (this->elf_frontend == NULL && !this->plain_file) {
    THROW_ERROR("Binary parser was not created correctly.");
  }
  if (this->plain_file)
    return 0;
  else
    return this->elf_frontend->get_binary_start();
} // ExecLoader::get_data_start()

/// ----------------------------------------------------------------------------

/// Returns a pointer to the array containing the program data.
unsigned char* trap::ExecLoader::get_program_data() {
  if (this->elf_frontend == NULL && !this->plain_file) {
    THROW_ERROR("Binary parser was not created correctly.");
  }
  if (this->plain_file) {
    return this->program_data;
  } else {
    return this->elf_frontend->get_program_data();
  }
} // ExecLoader::get_program_data()

/// ****************************************************************************
