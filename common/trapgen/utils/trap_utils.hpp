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

#ifndef TRAP_UTILS_HPP
#define TRAP_UTILS_HPP

#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "core/common/trapgen/utils/customExceptions.hpp"

#ifndef __GNUC__
#ifndef __PRETTY_FUNCTION__
#ifdef __FUNCDNAME__
#define __PRETTY_FUNCTION__ __FUNCDNAME__
#else
#define __PRETTY_FUNCTION__ "NONAME"
#endif
#endif
#endif

#ifdef MAKE_STRING
#undef MAKE_STRING
#endif
#define MAKE_STRING(msg)  (((std::ostringstream &)((std::ostringstream() << '\x0') << msg)).str().substr(1))

namespace trap {
void throw_exception_helper(std::string message);
}

#ifdef THROW_EXCEPTION
#undef THROW_EXCEPTION
#endif
#define THROW_EXCEPTION(msg) (trap::throw_exception_helper(MAKE_STRING("At: function " << __PRETTY_FUNCTION__ << \
  " file: " << __FILE__ << ":" << __LINE__ << " --> " << msg)))

namespace trap {
void throw_error_helper(std::string message);
}

#ifdef THROW_ERROR
#undef THROW_ERROR
#endif
#define THROW_ERROR(msg) (trap::throw_error_helper(MAKE_STRING("At: function " << __PRETTY_FUNCTION__ << " file: " << \
  __FILE__ << ":" << __LINE__ << " --> " << msg << std::endl)))

#endif
