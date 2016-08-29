/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     osemu_base.cpp
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
#include "osemu_base.hpp"
#include "elfloader/elf_frontend.hpp"

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

std::vector<unsigned> trap::OSEmulatorBase::group_ids;
unsigned trap::OSEmulatorBase::num_programs = 0;

void trap::OSEmulatorBase::set_env(const std::string name, const std::string value) {
  env[name] = value;
} // OSEmulatorBase::set_env()

/// ----------------------------------------------------------------------------

void trap::OSEmulatorBase::set_sysconf(const std::string name, int value) {
  sysconf[name] = value;
} // OSEmulatorBase::set_sysconf()

/// ----------------------------------------------------------------------------

void trap::OSEmulatorBase::set_program_args(const std::vector<std::string> args) {
  program_args = args;
} // OSEmulatorBase::set_program_args()

/// ----------------------------------------------------------------------------

void trap::OSEmulatorBase::correct_flags(int& val) {
  int flags = 0;

  if (val & NEWLIB_O_RDONLY)
    flags |= CORRECT_O_RDONLY;
  if (val & NEWLIB_O_WRONLY)
    flags |= CORRECT_O_WRONLY;
  if (val & NEWLIB_O_RDWR)
    flags |= CORRECT_O_RDWR;
  if (val & NEWLIB_O_CREAT)
    flags |= CORRECT_O_CREAT;
  if (val & NEWLIB_O_EXCL)
    flags |= CORRECT_O_EXCL;
  if (val & NEWLIB_O_NOCTTY)
    flags |= CORRECT_O_NOCTTY;
  if (val & NEWLIB_O_TRUNC)
    flags |= CORRECT_O_TRUNC;
  if (val & NEWLIB_O_APPEND)
    flags |= CORRECT_O_APPEND;
  if (val & NEWLIB_O_NONBLOCK)
    flags |= CORRECT_O_NONBLOCK;

  val = flags;
} // OSEmulatorBase::correct_flags()

/// ----------------------------------------------------------------------------

void trap::OSEmulatorBase::reset() {
  this->env.clear();
  this->sysconf.clear();
  this->program_args.clear();
  this->heap_ptr = 0;
  ELFFrontend::reset();
} // OSEmulatorBase::reset()

/// ****************************************************************************
