/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     abi_if.hpp
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
#ifndef TRAP_ABIIF_HPP
#define TRAP_ABIIF_HPP

#include "modules/instruction.hpp"
#include "common/report.hpp"

#include <vector>

namespace trap {

/**
 * @brief ABIIf
 */
template<class RegWidth>
class ABIIf {

  public:
  virtual ~ABIIf() {}

  virtual RegWidth read_PC() const throw() = 0;

  virtual void set_PC(const RegWidth& new_value) throw() = 0;

  virtual RegWidth read_LR() const throw() {
    THROW_ERROR("The LR register is not defined in the processor ABI.");
    return 0;
  }

  virtual void set_LR(const RegWidth& new_value) throw() {
    THROW_ERROR("The LR register is not defined in the processor ABI.");
  }

  virtual RegWidth read_SP() const throw() {
    THROW_ERROR("The SP register is not defined in the processor ABI.");
    return 0;
  }

  virtual void set_SP(const RegWidth& new_value) throw() {
    THROW_ERROR("The SP register is not defined in the processor ABI.");
  }

  virtual RegWidth read_FP() const throw() {
    THROW_ERROR("The FP register is not defined in the processor ABI.");
    return 0;
  }

  virtual void set_FP(const RegWidth& new_value) throw() {
    THROW_ERROR("The FP register is not defined in the processor ABI.");
  }

  virtual std::vector<RegWidth> read_args() const throw() = 0;

  virtual void set_args(const std::vector<RegWidth>& args) throw() = 0;

  virtual RegWidth read_return_value() const throw() = 0;

  virtual void set_return_value(const RegWidth& new_value) throw() = 0;

  virtual RegWidth read_gdb_reg(const unsigned& gdb_id) const throw() = 0;

  virtual void set_gdb_reg(const RegWidth& new_value, const unsigned& gdb_id) throw() = 0;

  virtual unsigned num_gdb_regs() const throw() = 0;

  virtual RegWidth read_mem(const RegWidth& address) = 0;

  virtual unsigned char read_char_mem(const RegWidth& address) = 0;

  virtual void write_mem(const RegWidth& address, RegWidth datum) = 0;

  virtual void write_char_mem(const RegWidth& address, unsigned char datum) = 0;

  virtual void pre_call() throw() {
  }

  virtual void post_call() throw() {
  }

  virtual void return_from_call() throw() {
    this->set_PC(this->read_LR());
  }

  virtual bool is_executing_instr() const throw() = 0;

  virtual void wait_instr_end() const throw() = 0;

  virtual bool is_routine_entry(const InstructionBase* instr) throw() = 0;

  virtual bool is_routine_exit(const InstructionBase* instr) throw() = 0;

  virtual unsigned char* get_state() const throw() = 0;

  virtual void set_state(unsigned char* state) throw() = 0;

  virtual unsigned get_exit_value() throw() = 0;

  virtual void set_exit_value(unsigned value) throw() = 0;

  virtual RegWidth get_code_limit() = 0;

  virtual int get_id() const throw() {
    return 0;
  }

  inline bool match_endian() const throw() {
#ifdef LITTLE_ENDIAN_BO
    return this->is_little_endian();
#else
    return !this->is_little_endian();
#endif
  }

  virtual bool is_little_endian() const throw() = 0;
}; // class ABIIf

} // namespace trap

/// ****************************************************************************
#endif
