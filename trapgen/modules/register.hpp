/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     register.hpp
* @brief    This file is part of the TRAP runtime library.
* @details  Provides a framework for registers.
*           - The abstract interface is split between scireg_region_if
*             (datatype independent) and RegisterInterface (templated on
*             datatype).
*           - The implementation is in the RegisterField, Register and
*             RegisterBank classes (templated on datatype).
*           - The RegisterBank class is provided for consistency but not used
*             further in TRAP since it does not supply significant benefits over
*             a simple array of registers.
*           - The implementation classes can be configured for multiple levels
*             of abstraction. Abstraction-specific behavior is delegated to
*             RegisterAbstraction.
*           - The set of interfaces and classes implement to some degree the
*             Composite (Field/Register/Bank), Iterator (RegisterIterator),
*             Strategy (RegisterAbstraction), Decorator (RegisterAlias) and
*             Observer (scireg_callback) design patterns.
* @author   Lillian Tadros (Technische Universitaet Dortmund)
* @date     2016 Technische Universitaet Dortmund
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
*******************************************************************************/

#ifndef TRAP_REGISTER_H
#define TRAP_REGISTER_H

#include "modules/register/register_field.hpp"
#include "modules/register/register_register.hpp"
#include "modules/register/register_alias.hpp"
#include "modules/register/register_bank.hpp"

/// ****************************************************************************
#endif // TRAP_REGISTER_H
