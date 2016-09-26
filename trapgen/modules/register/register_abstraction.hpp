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

#ifndef TRAP_REGISTER_ABSTRACTION_H
#define TRAP_REGISTER_ABSTRACTION_H

#include "common/report.hpp"

#include <systemc>
//#include <amba_parameters.h>

#include <list>
#include <assert.h>
#include <iomanip>

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
 * 2.The same logic is used for delegating the behavior of special registers. In
 *   particular, constant-value registers, registers with an offset and
 *   registers with a delay (only TLM) again have different read/write behavior
 *   from the regular case. This makes for a second level of delegation:
 *   RegisterAbstraction->RegisterAbstraction<Behavior>. Since a const/offset/
 *   delay completely changes the read/write behavior, inheritance seems like
 *   the correct implementaion choice. The base abstraction is intentionally
 *   not an abstract class and should be used for the general case.
 *
 * 3.A cleaner solution would be layering RegisterAbstraction<Behavior> as
 *   decorators, i.e. enabling things like RegisterTLMDelay ->
 *   RegisterTLMOffset. We would change the variable references to a
 *   RegisterAbstraction&, and initialize the outer layer with a reference to
 *   an inner layer, so RegisterTLMDelay(RegisterTLMOffset(RegisterTLM)). The
 *   outer layers would then need to derive from RegisterAbstraction only, not
 *   RegisterTLM. I found the additional complexity too much for the one case
 *   where this is allowed (TLMDelay + TLMOffset), so I created an extra
 *   combination class instead. It saves us one level of delegation in that
 *   case.
 */
template <typename DATATYPE>
class RegisterAbstraction {
  /// @name Constructors and Destructors
  /// @{

  public:
  virtual ~RegisterAbstraction() {}

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
  /// --------------------------------------------------------------------------
  /// @name Observer Methods
  /// @{

  virtual void set_stage(unsigned stage) {}

  virtual void unset_stage() {}

  virtual void stall(unsigned stage) {}

  virtual void advance() {}

  virtual void flush(unsigned stage) {}

  // Hazard Detection Functions
  virtual unsigned is_locked(unsigned stage, unsigned latency) { return 0; }

  virtual bool lock(void* instr, unsigned stage, unsigned latency) { return true; }

  virtual bool unlock(void* instr) { return true; }

  /// @} Observer Methods
  /// --------------------------------------------------------------------------
  /// @name Information and Helper Methods
  /// @{

  /// sc_object style print() of register value.
  virtual void print(std::ostream& os) const {
    os << std::hex << std::showbase << std::setw((sizeof(DATATYPE)<<1)+2) << this->read_dbg() << std::dec;
  }

  /// @} Information and Helper Methods
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
 * - The write-back stage is chosen by the user using <stage>.setWbStage().
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
 */
template <typename DATATYPE>
class RegisterCA : public RegisterAbstraction<DATATYPE> {
  public:
  typedef bool (*clock_cycle_func_t)(DATATYPE*, DATATYPE*, unsigned long long*, unsigned long long*);
  typedef std::tuple<void* /* instr* */, unsigned /* stage */, unsigned /* latency */> lock_type;
  typedef std::list<lock_type> lock_list_type;
  typedef lock_list_type::iterator lock_list_iterator;

  /// @name Constructors and Destructors
  /// @{

  public:
  RegisterCA(DATATYPE& value, DATATYPE& used_mask, DATATYPE& read_mask, DATATYPE& write_mask, unsigned num_stages, clock_cycle_func_t clock_cycle_func = NULL)
    : m_value(value),
      m_used_mask(used_mask),
      m_read_mask(read_mask),
      m_write_mask(write_mask),
      m_num_stages(num_stages),
      m_current_stage(0),
      m_stall_stage(-1),
      m_current_cycle(1),
      m_clock_cycle_func(clock_cycle_func),
      m_has_to_propagate(false) {

    this->m_values = new DATATYPE[this->m_num_stages];
    this->m_time_stamps = new unsigned long long[this->m_num_stages];
    this->temp_values = new DATATYPE[this->m_num_stages];
    this->temp_time_stamps = new unsigned long long[this->m_num_stages];
    for (unsigned i = 0; i < this->m_num_stages; ++i) {
      this->m_values[i] = this->temp_values[i] = value;
      this->m_time_stamps[i] = this->temp_time_stamps[i] = 0;
    }
  }

  ~RegisterCA() {
    delete [] this->m_values;
    delete [] this->m_time_stamps;
    delete [] this->temp_values;
    delete [] this->temp_time_stamps;
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
    this->m_time_stamps[this->m_current_stage] = m_current_cycle;
    this->m_has_to_propagate = true;
    return true;
  }

  /**
   * @brief Clock Cycle Function
   *
   * Callback implementing propagation logic.
   * This takes into account:
   * - First read (<pipe>.setRegsStage()).
   * - Last write (<pipe>.setWbStage()).
   * - Regular step-wise forward propagation.
   * - Common feedback loops via <processor>Arch.py:<pipe>.setWbStageOrder().
   * - Register-specific feedback loops via <reg>.setWbStageOrder().
   *
   * @see registerWriter.py:getPipeClockCycleFunction() for more details on the
   * propagation logic.
   *
   * @see lock(), unlock() and is_locked() for details on locking logic.
   *
   * @note: Having TRAP generate a hardcoded function is faster than dynamic
   * solutions involving maps or similar.
   */
  void clock_cycle() {
    // Update list of instructions currently writing or intending to write.
    for (lock_list_iterator it = this->m_lock_list.begin(); it != this->m_lock_list.end(); ++it) {
      // Instruction will lock in the future: Decrease lock duration.
      if (std::get<2>(*it) > 0)
        std::get<2>(*it)--;
      // Instruction has lock: Step one stage forward.
      else if (std::get<1>(*it) < this->m_num_stages-1)
        std::get<1>(*it)++;
      // Instruction will leave pipeline: Remove instruction from lock queue.
      else
        it = this->m_lock_list.erase(it);
    }

    // Execute propagation logic generated by TRAP.
    if (this->m_has_to_propagate) {
      bool has_changes = false;

      if (this->m_clock_cycle_func != NULL)
        has_changes = (*this->m_clock_cycle_func)(m_values, temp_values, m_time_stamps, temp_time_stamps);

      // Adopt new values.
      // The propagation logic is hard-coded in m_clock_cycle_func() and saved
      // in m_temp_values. Depending on the stall status, we may adopt only a
      // subset of the calculated values:
      // 1. If the pipeline is not stalled, all values are copied to m_values.
      // 2. If the pipeline is just being stalled, we copy the values only
      // upwards from the stall stage. The values in the stall stage downwards
      // are kept unchanged.
      // 3. If the pipeline is just coming out of a stall, copy the stable value
      // to the stage causing the stall so it can use it the next cycle.
      for (unsigned i = this->m_stall_stage+2; i < this->m_num_stages; ++i) {
        this->m_values[i] = this->temp_values[i];
        this->m_time_stamps[i] = this->temp_time_stamps[i];
      }
      if (this->m_stall_stage < (int)(this->m_num_stages)-1) {
        if (this->m_stall_stage >= 0) {
          // If this stage is causing the stall, propagate zero to the next
          // stage. TODO: We might as well propagate nothing and let it keep
          // its stale value. It runs NOP anyway, so who cares.
          this->m_values[this->m_stall_stage+1] = 0;
          this->m_time_stamps[this->m_stall_stage+1] = 0;
          // If this stage was waiting for a lock, copy the value coming out of
          // the locking stage. TODO: This is dirty and error-prone. The problem
          // is that the way stalling is currently modeled keeps the higher
          // values moving, but does not take them up in the lower stages until
          // the registers are advanced. We only advance the registers after the
          // stalling stage has finished processing, since otherwise the
          // preceding stages, who have by now long since finished their
          // processing, will suddenly see new register values. This is not only
          // wrong, but we might need to propagate the registers several stages
          // up to the stalled stage, without any processing done. All very
          // ugly. The problem needs a different model for the latches, where
          // we make more use of the stable value (m_value). Currently it is not
          // really considered. This is also linked to bypasses, which are
          // currently not handled properly.
/*          if (was_locked) {
            this->m_values[this->m_stall_stage] = this->temp_values[this->m_stall_stage];
            this->m_time_stamps[this->m_stall_stage] = this->temp_time_stamps[this->m_stall_stage];
          }*/
        } else {
          this->m_values[this->m_stall_stage+1] = this->temp_values[this->m_stall_stage+1];
          this->m_time_stamps[this->m_stall_stage+1] = this->temp_time_stamps[this->m_stall_stage+1];
        }
      }

      // Assign stable value.
      this->m_value = this->m_values[this->m_num_stages-1];

      // Increment clock cycle.
      this->m_current_cycle++;
      this->m_has_to_propagate = has_changes;
    }
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

  /**
   * @brief Stall Functions
   *
   * Called whenever the pipeline needs to be stalled. All stages down from and
   * including the given stage retain their values, while the stages upwards
   * propagate in clock_cycle().
   */
  void stall(unsigned stage) {
    assert(stage < this->m_num_stages);
    this->m_stall_stage = (int)stage;
  }

  void advance() {
    this->m_stall_stage = -1;
  }

  void flush(unsigned stage) {
    assert(stage < this->m_num_stages);
    for (unsigned i = 0; i <= stage; ++i) {
      this->m_values[i] = 0;
      this->m_time_stamps[i] = 0;
    }
  }

  /**
   * @brief Hazard Detection Function is_locked()
   *
   * Calling sequence:
   *    Pipeline[decode]::behavior()
   * -> Instruction::check_regs()
   * -> Register::is_locked()
   * Called by instructions before reading a register to check that the register
   * has not been or is not being written by a previous instruction in the
   * pipeline, in which case the pipeline is stalled.
   * This function can only return an upper bound on the required stall cycles.
   * The reason is that the register has no knowledge of pipeline bypasses. So a
   * preceding instruction that is currently writing or will write in the next
   * few cycles might still not necessarily cause a stall if there is an
   * appropriate pipeline bypass.
   *
   * @see isaWriter.py:check_regs().
   * @todo Think about how to incorporate information about bypasses into stall
   * cycle calculations.
   */
  unsigned is_locked(unsigned stage, unsigned latency) {
    assert(stage < this->m_num_stages);
    int read_offset = this->m_num_stages + stage /* read stage */ - latency /* read latency */;
    int instr_lock, max_lock = -1;
    for (lock_list_iterator it = this->m_lock_list.begin(); it != this->m_lock_list.end(); ++it) {
      // An instruction scheduled previously is writing or intends to write.
      // Calculate when the new value will be available. This is the worst case
      // and assumes we can only receive the new value after it has propagated
      // through the whole pipeline and back into the stage where we want to
      // read. The value is calculated as follows:
      // + write latency
      // - read latency
      // + distance(write->read) = num_stages - write_stage + read_stage
      // + 1cc (end of current clock cycles)
      instr_lock = (int)(std::get<2>(*it)) /* write latency */ - (int)(std::get<1>(*it)) /* write stage */ + read_offset;
      if (instr_lock > max_lock)
        max_lock = instr_lock;
    }

    // If an instruction is currently writing, the latency is 0 but we still
    // have to wait until the next clock cycle edge.
    if (max_lock >= 0) return (unsigned)(max_lock+1);
    // A negative latency indicates that the write will have terminated before
    // the read.
    return 0;
  }

  /**
   * @brief Hazard Detection Function lock()
   *
   * Calling sequence:
   *    Pipeline[decode]::behavior()
   * -> Instruction::lock_regs()
   * -> Register::lock()
   * Called by instructions before writing a register to signal the intention to
   * write and lock the register for the duration of the write. Locks are
   * scheduled for the future, so the function will return true even if a
   * preceding instruction had already signalled the intention to write, so long
   * as the latencies are such that the WAW order is preserved.
   *
   * @see isaWriter.py:lock_regs().
   * @todo We could return the required stall similar to is_locked, instead of
   * success/failure, in case we ever implement out-of-order scheduling or so.
   */
  bool lock(void* instr, unsigned stage, unsigned latency) {
    assert(stage < this->m_num_stages);
    for (lock_list_iterator it = this->m_lock_list.begin(); it != this->m_lock_list.end(); ++it) {
      // An instruction scheduled previously will write later than we would. We have to wait.
      // Note that the write stage does not matter, since the register is not
      // pipelined. We therefore have to enforce strict order.
      if (std::get<2>(*it) /* write latency */ <= latency) {
        return false;
      }
    }

    this->m_lock_list.push_back(std::make_tuple(instr, stage, latency));
    return true;
  }

  /**
   * @brief Hazard Detection Function unlock()
   *
   * Calling sequence:
   *    Pipeline[regs..wb]::behavior():catch(annul_exception)
   * -> Instruction::unlock_regs()
   * -> Register::unlock()
   * Unlocks all registers previously unlocked. Called only when an instruction
   * is annulled. The regular write-back case does not require an explicit
   * unlock_regs(), since the registers themselves undertake decrementing the
   * latencies and advancing the locked stages.
   *
   * @see isaWriter.py:unlock_regs().
   */
  bool unlock(void* instr) {
    for (lock_list_iterator it = this->m_lock_list.begin(); it != this->m_lock_list.end(); ++it) {
      // Found locking instruction.
      if (std::get<0>(*it) == instr) {
        this->m_lock_list.erase(it);
        return true;
      }
    }
    return false;
  }

  /// @} Observer Methods
  /// --------------------------------------------------------------------------
  /// @name Information and Helper Methods
  /// @{

  /// sc_object style print() of register value.
  void print(std::ostream& os) const {
    os << std::hex << std::showbase << '(';
    for (unsigned i = 0; i < this->m_num_stages; ++i) {
      os << std::setw((sizeof(DATATYPE)<<1)+2) << this->m_values[i] << ';';
    }
    os << ')' << std::dec;
  }

  /// @} Information and Helper Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  protected:
  DATATYPE& m_value;
  DATATYPE& m_used_mask;
  DATATYPE& m_read_mask;
  DATATYPE& m_write_mask;

  // Stages and propagation
  const unsigned m_num_stages;
  unsigned m_current_stage;
  int m_stall_stage;
  unsigned long long m_current_cycle;
  bool m_has_to_propagate;
  clock_cycle_func_t m_clock_cycle_func;

  // Latch values
  DATATYPE* m_values;
  unsigned long long* m_time_stamps;
  // Temporary array holding pipeline values and timestamps. Eases multiplexing
  // several values and choosing the most recent.
  DATATYPE* temp_values;
  unsigned long long* temp_time_stamps;

  // Locking scheduling queue: Saves a list of (instr*, stage, latency) tuples.
  lock_list_type m_lock_list;

  /// @} Data
}; // class RegisterCA

/// ****************************************************************************

/**
 * @brief RegisterCAGlobal
 *
 * This class models a register on the cycle-accurate level that is not
 * pipelined. The register can be locked for writing for the duration of one
 * cycle. Other instructions requesting to read the register will have to wait
 * until the beginning of the cycle after the write. Since the write can be
 * scheduled for the future, the resulting stall might be larger than one
 * cycle. The register maintains two variables for the value: The stable value
 * is the one read, while the temporary value is used for the duration of the
 * write cycle, and propagated to the stable value at the clock cycle edge.
 */
template <typename DATATYPE>
class RegisterCAGlobal : public RegisterAbstraction<DATATYPE> {
  public:
  typedef std::tuple<void* /* instr* */, unsigned /* stage */, unsigned /* latency */> lock_type;
  typedef std::list<lock_type> lock_list_type;
  typedef lock_list_type::iterator lock_list_iterator;

  /// @name Constructors and Destructors
  /// @{

  public:
  RegisterCAGlobal(DATATYPE& value, DATATYPE& used_mask, DATATYPE& read_mask, DATATYPE& write_mask, unsigned num_stages)
    : m_value(value),
      m_used_mask(used_mask),
      m_read_mask(read_mask),
      m_write_mask(write_mask),
      m_num_stages(num_stages),
      m_has_to_propagate(false),
      m_temp_value(value) {}

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Access and Modification Methods
  /// @{

  public:
  const DATATYPE read_dbg() const {
    return this->m_value & this->m_read_mask;
  }

  bool write_dbg(const DATATYPE& data) {
    this->m_temp_value = data & this->m_write_mask;
    this->m_has_to_propagate = true;
    return true;
  }

void clock_cycle() {
    // Update list of instructions currently writing or intending to write.
    for (lock_list_iterator it = this->m_lock_list.begin(); it != this->m_lock_list.end(); ++it) {
      // Instruction will lock in the future: Decrease lock duration.
      if (std::get<2>(*it) > 0)
        std::get<2>(*it)--;
      // Instruction has lock: Remove instruction from lock queue.
      else
        it = this->m_lock_list.erase(it);
    }

    // Execute propagation logic generated by TRAP.
    if (this->m_has_to_propagate) {
      this->m_value = this->m_temp_value;
      this->m_has_to_propagate = false;
    }
  }

  /// @} Access and Modification Methods
  /// --------------------------------------------------------------------------
  /// @name Observer Methods
  /// @{

  unsigned is_locked(unsigned stage, unsigned latency) {
    assert(stage < this->m_num_stages);
    int instr_lock, max_lock = -1;
    for (lock_list_iterator it = this->m_lock_list.begin(); it != this->m_lock_list.end(); ++it) {
      // An instruction scheduled previously is writing or intends to write.
      // Calculate when the new value will be available. Since this is a global
      // register, the value is available to all stages by the end of the write
      // clock cycle.
      // + write latency
      // - read latency
      // + distance(write->read) = 0 (global register)
      // + 1cc (end of current clock cycles)
      instr_lock = std::get<2>(*it) /* write latency */ - latency /* read latency */;
      if (instr_lock > max_lock)
        max_lock = instr_lock;
    }

    // If an instruction is currently writing, the latency is 0 but we still
    // have to wait until the next clock cycle edge.
    if (max_lock >= 0) return (unsigned)(max_lock+1);
    // A negative latency indicates that the write will have terminated before
    // the read.
    return 0;
  }

  bool lock(void* instr, unsigned stage, unsigned latency) {
    assert(stage < this->m_num_stages);
    // The stage the calling instruction currently is in. This could be negative
    // if an instruction schedules the write far in advance (Will not happen
    // with our current model though).
    int cur_stage = stage - latency;
    for (lock_list_iterator it = this->m_lock_list.begin(); it != this->m_lock_list.end(); ++it) {
      // An instruction scheduled previously will write later than we would. We have to wait.
      if (((int)(std::get<1>(*it) /* stage */) - (int)(std::get<2>(*it) /* latency */)) <= cur_stage) {
        return false;
      }
    }

    this->m_lock_list.push_back(std::make_tuple(instr, stage, latency));
    return true;
  }

  bool unlock(void* instr) {
    for (lock_list_iterator it = this->m_lock_list.begin(); it != this->m_lock_list.end(); ++it) {
      // Found locking instruction.
      if (std::get<0>(*it) == instr) {
        this->m_lock_list.erase(it);
        return true;
      }
    }
    return false;
  }

  /// @} Observer Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  protected:
  DATATYPE& m_value;
  DATATYPE& m_used_mask;
  DATATYPE& m_read_mask;
  DATATYPE& m_write_mask;

  // Stages and propagation
  const unsigned m_num_stages;
  bool m_has_to_propagate;

  // Latch values
  DATATYPE& m_temp_value;

  // Locking scheduling queue: Saves a list of (instr*, stage, latency) tuples.
  lock_list_type m_lock_list;

  /// @} Data
}; // class RegisterCAGlobal

} // namespace trap

/// ****************************************************************************
#endif // TRAP_REGISTER_ABSTRACTION_H
