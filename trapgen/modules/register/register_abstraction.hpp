/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     register_abstraction.hpp
* @brief    This file is part of the TRAP runtime library.
* @details  @see RegisterAbstraction
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
#ifndef TRAP_REGISTER_ABSTRACTION_H_
#define TRAP_REGISTER_ABSTRACTION_H_

#include "common/report.hpp"

#include <systemc>
//#include <amba_parameters.h>

#include <assert.h>

namespace trap {

/// Types

// NOTE: Not worth an amba dependency. Just keep synced with amba.
enum amba_layer_ids {
  amba_CT = 0x01,   // cycle-timed or cycle-accurate
  amba_AT = 0x04,   // approximately-timed
  amba_LT = 0x08,   // loosely-timed
  amba_pv = 0x10    // programmer's view
};

/// ****************************************************************************

/**
 * @brief RegisterAbstraction
 *
 * Serves two purposes:
 * 1.The register classes can be used for multiple levels of abstraction (cycle-
 *   accurate, etc), if the effect of the abstraction can be reduced down to a
 *   few operations. Since all access operations of RegisterField, RegisterBank
 *   as well as Register delegate to Register::read/write, it suffices to
 *   encapsulate these operations away from Register and into a strategy
 *   appropriate for the chosen abstraction level. The heavy-weight Register
 *   class can thus be used for all abstractions by just varying the strategy.
 *   Abstraction could be provided as a compile-time choice, but we use run-time
 *   delegation instead for more flexibility.
 *
 * 2.The same logic is used for delegating the behavior of special Registers. In
 *   particular, constant-value registers, registers with an offset and
 *   registers with a delay (only TLM) again have slightly different read/write
 *   behavior from the regular case. This makes for a second level of
 *   delegation: RegisterAbstraction->RegisterAbstraction<Behavior>. Since a
 *   const/offset/delay completely changes the read/write behavior, inheritance
 *   seems like the correct implementaion choice. The base abstraction is
 *   intentionally not an abstract class and should be used for the general
 *   case.
 *
 * 3.A cleaner solution would be layering RegisterAbstraction<Behavior> as
 *   decorators, i.e. enabling things like RegisterTLMDelay ->
 *   RegisterTLMOffset. We would change the variable references to a
 *   RegisterAbstraction&, and initialize the outer layer with a reference to
 *   an inner layer, so RegisterTLMDelay(RegisterTLMOffset(RegisterTLM)). The
 *   outer layers would then need to derive from RegisterAbstraction only, not
 *   RegisterTLM. I found the additional complexity too much for the one case
 *   where this is allowed (TLMDelay + TLMOffset), so I created an extra
 *   combination class instead. It saves us one level of delegation in that case.
 */
template <typename DATATYPE>
class RegisterAbstraction {
  /// @name Constructors and Destructors
  /// @{

  public:
  virtual ~RegisterAbstraction()
  {}

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Access and Modification Methods
  /// @{

  public:
  virtual const DATATYPE read_dbg() const = 0;

  // Reads last written value.
  virtual const DATATYPE read_force() const { return this->read_dbg(); }

  virtual bool write_dbg(const DATATYPE& data) = 0;

  // Writes value immediately discarding delay.
  virtual bool write_force(const DATATYPE& data) { return this->write_dbg(data); }

  virtual void clock_cycle() {}
  /// @} Access and Modification Methods
}; // class RegisterAbstraction

/// ****************************************************************************

template <typename DATATYPE>
class RegisterTLM : public RegisterAbstraction<DATATYPE> {
  /// @name Constructors and Destructors
  /// @{

  public:
  RegisterTLM(DATATYPE& value, DATATYPE& used_mask, DATATYPE& read_mask, DATATYPE& write_mask)
    : m_value(value),
      m_used_mask(used_mask),
      m_read_mask(read_mask),
      m_write_mask(write_mask)
  {}

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Access and Modification Methods
  /// @{

  public:
  const DATATYPE read_dbg() const {
    return this->m_value & this->m_read_mask;
  }

  bool write_dbg(const DATATYPE& data) {
    this->m_value = data & this->m_write_mask;
    return true;
  }

  /// @} Access and Modification Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  protected:
  DATATYPE& m_value;
  DATATYPE& m_used_mask;
  DATATYPE& m_read_mask;
  DATATYPE& m_write_mask;

  /// @} Data
}; // class RegisterTLM

/// ****************************************************************************

template <typename DATATYPE>
class RegisterTLMConst : public RegisterTLM<DATATYPE> {
  /// @name Constructors and Destructors
  /// @{

  public:
  RegisterTLMConst(DATATYPE& value, DATATYPE& used_mask, DATATYPE& read_mask, DATATYPE& write_mask, DATATYPE& reset_value)
    : RegisterTLM<DATATYPE>(value, used_mask, read_mask, write_mask),
      m_reset_value(reset_value) {
    assert(value == reset_value);
  }

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Access and Modification Methods
  /// @{

  public:
  bool write_dbg(const DATATYPE&) {
    THROW_WARNING("Cannot write a const register.");
    return false;
  }

  /// @} Access and Modification Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  protected:
  DATATYPE& m_reset_value;

  /// @} Data
}; // class RegisterTLMConst

/// ****************************************************************************

template <typename DATATYPE>
class RegisterTLMOffset : public RegisterTLM<DATATYPE> {
  /// @name Constructors and Destructors
  /// @{

  public:
  RegisterTLMOffset(DATATYPE& value, DATATYPE& used_mask, DATATYPE& read_mask, DATATYPE& write_mask, const unsigned& offset)
    : RegisterTLM<DATATYPE>(value, used_mask, read_mask, write_mask),
      m_offset(offset) {
    // Offsets cannot be used with masks since it is not obviuos whether the
    // initial or the offset value should be masked.
    assert(read_mask == ((unsigned long)1 << (sizeof(DATATYPE) * 8) - 1)
    && read_mask == write_mask);
  }

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Access and Modification Methods
  /// @{

  public:
  const DATATYPE read_dbg() const {
    assert(this->m_value+this->m_offset < ((unsigned long)1 << (sizeof(DATATYPE) * 8)));
    return this->m_value + this->m_offset;
  }

  /// @} Access and Modification Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  protected:
  const DATATYPE& m_offset;

  /// @} Data
}; // class RegisterTLMOffset

/// ****************************************************************************

template <typename DATATYPE>
class RegisterTLMDelay : public RegisterTLM<DATATYPE> {
  /// @name Constructors and Destructors
  /// @{

  public:
  RegisterTLMDelay(DATATYPE& value, DATATYPE& used_mask, DATATYPE& read_mask, DATATYPE& write_mask, const unsigned& delay)
    : RegisterTLM<DATATYPE>(value, used_mask, read_mask, write_mask),
      m_delay(delay) {
    this->m_values = new DATATYPE[this->m_delay];
    this->m_update_slots = new bool[this->m_delay];
    for (unsigned i = 0; i < this->m_delay; ++i) {
      this->m_values[i] = 0;
      this->m_update_slots[i] = false;
    }
  }

  ~RegisterTLMDelay() {
    delete [] this->m_values;
    delete [] this->m_update_slots;
  }

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Access and Modification Methods
  /// @{

  public:
  const DATATYPE read_force() {
    for (unsigned i = this->m_delay-1; i >= 0; --i) {
      if (this->m_update_slots[i]) {
        return this->m_values[i] & this->m_read_mask;
      }
    }
    return this->m_value;
  }

  bool write_dbg(const DATATYPE& data) {
    this->m_values[this->m_delay-1] = data & this->m_write_mask;
    this->m_update_slots[this->m_delay-1] = true;
    return true;
  }

  bool write_force(const DATATYPE& data) {
    this->m_value = data;
    for (unsigned i = 0; i < this->m_delay; ++i) {
      this->m_update_slots[i] = false;
    }
    return true;
  }

  void clock_cycle() {
    if (this->m_update_slots[0]) {
      this->m_value = this->m_values[0];
      this->m_update_slots[0] = false;
    }
    for (unsigned i = 1; i < this->m_delay; ++i) {
      if (this->m_update_slots[i]) {
        this->m_values[i-1] = this->m_values[i];
        this->m_update_slots[i] = false;
        this->m_update_slots[i-1] = true;
      }
    }
  }

  /// @} Access and Modification Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  protected:
  DATATYPE* m_values;
  bool* m_update_slots;
  const unsigned& m_delay;

  /// @} Data
}; // class RegisterTLMDelay

/// ****************************************************************************


template <typename DATATYPE>
class RegisterTLMDelayOffset : public RegisterTLM<DATATYPE> {
  /// @name Constructors and Destructors
  /// @{

  public:
  RegisterTLMDelayOffset(DATATYPE& value, DATATYPE& used_mask, DATATYPE& read_mask, DATATYPE& write_mask, const unsigned& delay, const unsigned& offset)
    : RegisterTLM<DATATYPE>(value, used_mask, read_mask, write_mask),
      m_offset(offset),
      m_delay(delay) {
    // Offsets cannot be used with masks since it is not obviuos whether the
    // initial or the offset value should be masked.
    assert(read_mask == ((unsigned long)1 << (sizeof(DATATYPE) * 8) - 1)
    && read_mask == write_mask);

    this->m_update_slots = new bool[this->m_delay];
    this->m_values = new DATATYPE[this->m_delay];
    for (unsigned i = 0; i < this->m_delay; ++i) {
      this->m_values[i] = 0;
      this->m_update_slots[i] = false;
    }
  }

  ~RegisterTLMDelayOffset() {
    delete [] this->m_values;
    delete [] this->m_update_slots;
  }

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Access and Modification Methods
  /// @{

  public:
  const DATATYPE read_dbg() const {
    assert(this->m_value+this->m_offset < ((unsigned long)1 << (sizeof(DATATYPE) * 8)));
    return this->m_value + this->m_offset;
  }

  const DATATYPE read_force() {
    for (unsigned i = this->m_delay-1; i >= 0; --i) {
      if (this->m_update_slots[i]) {
        return this->m_values[i] + this->m_offset;
      }
    }
    return this->m_value + this->m_offset;
  }

  bool write_dbg(const DATATYPE& data) {
    this->m_values[this->m_delay-1] = data;
    this->m_update_slots[this->m_delay-1] = true;
    return true;
  }

  bool write_force(const DATATYPE& data) {
    this->m_value = data;
    for (unsigned i = 0; i < this->m_delay; ++i) {
      this->m_update_slots[i] = false;
    }
    return true;
  }

  void clock_cycle() {
    if (this->m_update_slots[0]) {
      this->m_value = this->m_values[0];
      this->m_update_slots[0] = false;
    }
    for (unsigned i = 1; i < this->m_delay; ++i) {
      if (this->m_update_slots[i]) {
        this->m_values[i-1] = this->m_values[i];
        this->m_update_slots[i] = false;
        this->m_update_slots[i-1] = true;
      }
    }
  }

  /// @} Access and Modification Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  protected:
  DATATYPE* m_values;
  bool* m_update_slots;
  const unsigned& m_offset;
  const unsigned& m_delay;

  /// @} Data
}; // class RegisterTLMDelayOffset

/// ****************************************************************************

/**
 * @brief RegisterCA
 *
 * A cycle-accurate register model has to consider the effect of the pipeline on
 * the register value:
 * - Each two consecutive pipeline stages share a latched version of the
 *   register, which is written by the first and read by the second stage. The
 *   first latch is between stages 0 and 1, and the last is after the write-
 *   back stage, which is not necessarily the last pipeline stage. Latch values
 *   are propagated on cycles edges, so that each stage is assigned the
 *   latch following it for both reading and writing. The last latch (after
 *   write-back) is written back to the first.
 * - The write-back stage is chosen by the user using <stage>.setWriteBack().
 * - The value written in the write-back stage is considered the stable value.
 *   All other values can potentially be flushed if the instruction is annulled.
 *   Flushes propagate backwards, causing all preceding stages to flush.
 * - Special write-back sequences are given via <reg>.setWbStageOrder(). In
 *   that case, the register value is not propagated forward, but according to
 *   the given order of stages.
 * Implementation
 * - Composite Pattern:
 *   A straight-forward implementation could implement a composite pattern, with
 *   the latches as members. Each member would have to implement the whole
 *   register interface, operators and all, just to provide a different value.
 *   The instruction needs to store pointers to all stages of a register and
 *   update an alias for each behavior stage, so that the stages are kept
 *   transparent to the user. This solution is fast but produces large code.
 * - One Combined Register with Stage Parameter:
 *   A much more compact solution is for the pipeline register to contain a
 *   simple array of values. An instruction calls the access functions with an
 *   additional parameter for the pipeline stage. While simple, this solution
 *   implies changing all the interfaces of the access functions to include an
 *   optional stage parameter.
 * - Callbacks:
 *   This is essentially a variation on double dispatch. We (mis-)use callbacks
 *   to enable the register to find out the caller and accordingly manipulate
 *   the correct stage. The flow is as follows: Each instruction defines
 *   set_stage()/get_stage() inline functions. TRAP generates calls to register.
 *   set_callback() for all registers in registers.cpp with instruction::
 *   get_stage() as the callback function for both reads and writes. A call to
 *   instruction::set_stage() at the beginning of each instruction::
 *   behavior_stage() is also generated, where the instruction sets the current
 *   stage. Thus, instruction::get_stage() will be called by the register at
 *   each register access. While aesthetically beautiful, I am not sure about
 *   the incurred dynamic overhead.
 * - State Pattern:
 *   This describes our requirements perfectly. It has RegisterCA implementing
 *   the skeleton of the access methods and only delegating get_value() to the
 *   correct object that only stores the current value. In our case, we would
 *   not even need costly delegation, but just saving the correct array index.
 *   Again, the problem remains that the "state", i.e. current pipeline stage,
 *   is known by the instruction, but not by the register. A possible solution
 *   is having the instruction call Register::set_state() at the beginning of
 *   each instruction::behavior_stage(). This is similar to the previous
 *   solution, but reduces the number of extra function calls to only one
 *   set_stage() at the beginning of the instruction behavior, instead of one
 *   set_stage() + one get_stage() callback per register access. The minor
 *   downside is that the program flow has to be guaranteed sequential, i.e. no
 *   other instruction is allowed to access the register anywhere between
 *   set_state() and up to the end of the instruction behavior. To provide
 *   robust results, if the state is unset, the value of the last stage, i.e.
 *   the write-back value is returned.
 * TODO: This is incomplete and untested. The additional functions have to be
 * defined and delegated in the Register class. clock_cycle_func also needs to
 * be generated by TRAP.
 */
template <typename DATATYPE>
class RegisterCA : public RegisterAbstraction<DATATYPE> {
  public:
  typedef bool (*clock_cycle_func_t)();

  /// @name Constructors and Destructors
  /// @{

  public:
  RegisterCA(DATATYPE& value, DATATYPE& used_mask, DATATYPE& read_mask, DATATYPE& write_mask, unsigned num_stages, clock_cycle_func_t clock_cycle_func = NULL)
    : m_value(value),
      m_used_mask(used_mask),
      m_read_mask(read_mask),
      m_write_mask(write_mask),
      m_num_stages(num_stages),
      m_current_stage(num_stages-1),
      m_clock_cycle_func(clock_cycle_func),
      m_has_to_propagate(false) {

    this->m_values = new DATATYPE[this->m_num_stages];
    this->m_num_locked = new int[this->m_num_stages];
    this->m_locked_latency = new int[this->m_num_stages];
    this->m_time_stamp = new sc_core::sc_time[this->m_num_stages];
    for (unsigned i = 0; i < this->m_num_stages; ++i) {
      this->m_values[i] = value;
      this->m_num_locked[i] = 0;
      this->m_locked_latency[i] = -1;
      this->m_time_stamp[i] = sc_core::SC_ZERO_TIME;
    }
  }

  ~RegisterCA() {
    delete [] this->m_values;
    delete [] this->m_num_locked;
    delete [] this->m_locked_latency;
    delete [] this->m_time_stamp;
  }

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Access and Modification Methods
  /// @{

  public:
  const DATATYPE read_dbg() const {
    return this->m_values[this->m_current_stage] & this->m_read_mask;
  }

  bool write_dbg(const DATATYPE& data) {
    this->m_values[this->m_current_stage] = data & this->m_write_mask;
    if (this->m_current_stage == this->m_num_stages-1)
      this->m_value = this->m_values[this->m_current_stage];
    this->m_time_stamp[this->m_current_stage] = sc_core::sc_time_stamp();
    this->m_has_to_propagate = true;
    return true;
  }

  // lock(), unlock() and is_locked() are used for hazard detection.
  void lock() {
    if (this->m_current_stage == this->m_num_stages-1) {
      for (unsigned i = 0; i < this->m_num_stages; ++i) {
        this->m_locked_latency[i] = -1;
        this->m_num_locked[i]++;
      }
    } else {
      this->m_locked_latency[this->m_current_stage] = -1;
      this->m_num_locked[this->m_current_stage]++;
    }
  }

  void unlock() {
    if (this->m_current_stage == this->m_num_stages-1) {
      for (unsigned i = 0; i < this->m_num_stages; ++i) {
        this->m_locked_latency[i] = -1;
        if (this->m_num_locked[i] > 0) {
          this->m_num_locked[i]--;
        }
      }
    } else {
      this->m_locked_latency[this->m_current_stage] = -1;
      if (this->m_num_locked[this->m_current_stage] > 0) {
        this->m_num_locked[this->m_current_stage]--;
      }
    }
  }

  void unlock(int wb_latency) {
    if (this->m_current_stage == this->m_num_stages-1) {
      for (unsigned i = 0; i < this->m_num_stages; ++i) {
        if (wb_latency > 0) {
          // NOTE: Luca does not set m_locked_latency[i].
          this->m_locked_latency[i] = wb_latency;
        } else {
          this->m_locked_latency[i] = -1;
          if (this->m_num_locked[i] > 0) {
            this->m_num_locked[i]--;
          }
        }
      }
    } else {
      if (wb_latency > 0) {
        this->m_locked_latency[this->m_current_stage] = wb_latency;
      } else {
        this->m_locked_latency[this->m_current_stage] = -1;
        if (this->m_num_locked[this->m_current_stage] > 0) {
          this->m_num_locked[this->m_current_stage]--;
        }
      }
    }
  }

  // Used during forwarding, when a specific pipeline stage feeds the value to a
  // preceding stage.
  bool is_locked() {
    if (this->m_locked_latency[this->m_current_stage] > 0) {
      this->m_locked_latency[this->m_current_stage]--;
      if (this->m_locked_latency[this->m_current_stage] == 0) {
        this->m_num_locked[this->m_current_stage]--;
      }
    }
    if (this->m_num_locked[this->m_current_stage] <= 0) {
      this->m_num_locked[this->m_current_stage] = 0;
      return false;
    }
    return true;
  }

  void clock_cycle() {
    bool has_changes = false;
    sc_core::sc_time time_stamp;

    this->m_value = this->m_values[this->m_num_stages-1];
    time_stamp = this->m_time_stamp[this->m_num_stages-1];

    for (int i = this->m_num_stages-2; i > 0; --i) {
      if (this->m_time_stamp[i] > this->m_time_stamp[i+1]) {
        this->m_values[i+1] = this->m_values[i];
        this->m_time_stamp[i+1] = this->m_time_stamp[i];
        has_changes = true;
      }
    }

    if (time_stamp > this->m_time_stamp[0]) {
      this->m_values[0] = this->m_value;
      this->m_time_stamp[0] = time_stamp;
      has_changes = true;
    }

    // Callback implementing special write-back sequences.
    // Having TRAP generate a hardcoded function is faster than reading a map of
    // stage -> stages every cycle.
    if (this->m_clock_cycle_func != NULL)
      has_changes = has_changes || this->m_clock_cycle_func();

    this->m_has_to_propagate = has_changes;
  }

  /// @} Access and Modification Methods
  /// --------------------------------------------------------------------------
  /// @name Observer Methods
  /// @{

  void set_stage(unsigned stage) {
    assert(stage < this->m_num_stages);
    this->m_current_stage = stage;
  }

  void unset_stage() { this->m_current_stage = this->m_num_stages-1; }

  /// @} Observer Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  protected:
  DATATYPE& m_value;
  DATATYPE& m_used_mask;
  DATATYPE& m_read_mask;
  DATATYPE& m_write_mask;

  const unsigned m_num_stages;
  int m_current_stage;
  bool m_has_to_propagate;
  clock_cycle_func_t m_clock_cycle_func;

  DATATYPE* m_values;
  int* m_num_locked;
  int* m_locked_latency;
  sc_core::sc_time* m_time_stamp;
  void** m_observers;

  /// @} Data
}; // class RegisterCA

} // namespace trap

/// ****************************************************************************
#endif
