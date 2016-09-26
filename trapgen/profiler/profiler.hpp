/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     profiler.hpp
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

#ifndef TRAP_PROFILER_H
#define TRAP_PROFILER_H

#include "profiler_elements.hpp"
#include "elfloader/elf_frontend.hpp"
#include "modules/abi_if.hpp"
#include "modules/instruction.hpp"
#include "common/tools_if.hpp"

#include <systemc.h>

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

#include <set>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

namespace trap {

/**
 * @brief Profiler
 *
 * Keeps track of many runtime statistics on:
 * - number and percentage of instructions executed of each type,
 * - function stats: time and percentage in each function,
 * - call graph: time of each call.
 */
template<class IssueWidth>
class Profiler : public ToolsIf<IssueWidth> {
  /// @name Constructors and Destructors
  /// @{

  public:
  Profiler(ABIIf<IssueWidth>* processor, std::string exec_name, bool disable_func_profiling) :
    processor(processor), elf_frontend(&ELFFrontend::get_instance(exec_name)), disable_func_profiling(disable_func_profiling) {
    this->old_instr = NULL;
    this->old_instr_time = SC_ZERO_TIME;
    this->instr_end = this->instructions.end();
    this->old_func_time = SC_ZERO_TIME;
    this->func_end = this->functions.end();
    this->old_func_instr = 0;
    this->exited = false;
    this->lower_addr = 0;
    this->higher_addr = (IssueWidth) - 1;
    this->stats_running = false;
  } // Profiler()

  ~Profiler() {
  }

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Interface Methods
  /// @{

  public:
  /// Prints the computed statistics to a csv file.
  void print_csv_stats(std::string filename) {
    // Iterate over the encountered functions and instructions and print the
    // relative statistics. Two files will be created: fileName_fun.csv and
    // fileName_instr.csv.
    std::ofstream instruction_file((filename + "_instr.csv").c_str());
    instruction_file << ProfInstruction::print_csv_header() << std::endl;
    template_map<unsigned, ProfInstruction>::iterator instr_it, instr_end;
    for (instr_it = this->instructions.begin(), instr_end = this->instructions.end(); instr_it != instr_end; instr_it++) {
      instruction_file << instr_it->second.print_csv() << std::endl;
    }
    instruction_file << ProfInstruction::print_csv_summary() << std::endl;
    instruction_file.close();

    if (!this->disable_func_profiling) {
      std::ofstream function_file((filename + "_fun.csv").c_str());
      function_file << ProfFunction::print_csv_header() << std::endl;
      typename template_map<IssueWidth, ProfFunction>::iterator func_it, func_end;
      for (func_it = this->functions.begin(), func_end = this->functions.end(); func_it != func_end; func_it++) {
        function_file << func_it->second.print_csv() << std::endl;
      }
      function_file.close();
    }
  } // print_csv_stats()

  /// ..........................................................................

  /// Function called by the processor at every new instruction issue.
  bool issue(const IssueWidth& cur_PC, const InstructionBase* cur_instr) throw() {
    // Perform the update of the statistics only if they are included in the
    // predefined range
    if (!this->stats_running && (cur_PC == this->lower_addr)) {
      this->stats_running = true;
    } else if (this->stats_running && (cur_PC == this->higher_addr)) {
      this->stats_running = false;
    }

    if (this->stats_running) {
      this->update_instr_stats(cur_PC, cur_instr);
    }

    if (!this->disable_func_profiling) {
      this->update_func_stats(cur_PC, cur_instr);
    }

    return false;
  } // issue()

  /// ..........................................................................

  /// Since the profiler does not perform any modification to the registers and
  /// does not use any registers but the current program counter, it does not
  /// need the pipeline to be empty.
  bool is_pipeline_empty(const IssueWidth& cur_PC) const throw() {
    return false;
  } // is_pipeline_empty()

  /// ..........................................................................

  void add_ignored_function(std::string& to_ignore) {
    this->ignored.insert(to_ignore);
  } // add_ignored_function()

  /// ..........................................................................

  void add_ignored_functions(const std::set<std::string>& to_ignore) {
    this->ignored.insert(to_ignore.begin(), to_ignore.end());
  } // add_ignored_functions()

  /// ..........................................................................

  void set_profiling_range(const IssueWidth& lower_addr, const IssueWidth& higher_addr) {
    this->lower_addr = lower_addr;
    this->higher_addr = higher_addr;
  } // set_profiling_range()

  /// @} Interface Methods
  /// --------------------------------------------------------------------------
  /// @name Internal Methods
  /// @{

  /// Based on the new instruction just issued, the statistics on the
  /// instructions are updated.
  inline void update_instr_stats(const IssueWidth& cur_PC, const InstructionBase* cur_instr) throw() {
    // Update the total number of instructions executed.
    ProfInstruction::num_total_calls++;
    // Update the old instruction elapsed time.
    if (this->old_instr != NULL) {
      this->old_instr->time += sc_time_stamp() - this->old_instr_time;
      this->old_instr_time = sc_time_stamp();
    }
    // Update the new instruction statistics.
    unsigned instr_id = cur_instr->get_id();
      template_map<unsigned, ProfInstruction>::iterator instr_found = this->instructions.find(instr_id);
    if (instr_found != this->instr_end) {
      instr_found->second.num_calls++;
      this->old_instr = &(instr_found->second);
    } else {
      this->instructions[instr_id].name = cur_instr->get_name();
      this->old_instr = &(this->instructions[instr_id]);
      this->instr_end = this->instructions.end();
    }
  } // update_instr_stats()

  /// ..........................................................................

  /// Based on the new instruction just issued, the statistics on the functions
  /// are updated.
  inline void update_func_stats(const IssueWidth& cur_PC, const InstructionBase* cur_instr) throw() {
    std::vector<ProfFunction*>::iterator stack_it, stack_end;

    if (this->exited) {
      std::string cur_func_name = this->elf_frontend->symbol_at(cur_PC);
      if ((this->cur_stack.size() > 1) && (this->cur_stack.back()->name != cur_func_name)) {
        //std::cerr << "Exiting into " << cur_func_name << ", expected " << this->cur_stack.back()->name << ".\n";;
        // Error: We have not returned to where we came from.
        std::vector<ProfFunction*>::reverse_iterator stack_it, stack_end;
        stack_it = this->cur_stack.rbegin();
        bool have_to_pop = false;
        unsigned num_to_pop = 0;
        for (stack_it++, stack_end = this->cur_stack.rend();
        stack_it != stack_end;
        stack_it++) {
          if ((*stack_it)->name == cur_func_name) {
            have_to_pop = true;
            break;
          }
          num_to_pop++;
        }
        if (have_to_pop) {
          this->cur_stack.erase(
            this->cur_stack.begin() + (this->cur_stack.size() - num_to_pop - 1), this->cur_stack.end());
        }/*else
          std::cerr << "just exited I should have gone into " << cur_func_name << ".\n";*/
      }
      this->exited = false;
    }

    // Check whether we are entering in a new function. If not, check whether we
    // are exiting from the current function. If neither, do nothing.
    if (this->processor->is_routine_entry(cur_instr)) {
      std::string func_name = this->elf_frontend->symbol_at(cur_PC);
      if (this->ignored.find(func_name) != this->ignored.end()) {
        this->old_func_instr++;
        return;
      }
      ProfFunction::num_total_calls++;
      ProfFunction* cur_func_ptr = NULL;
      typename template_map<IssueWidth, ProfFunction>::iterator cur_func_it = this->functions.find(cur_PC);
      if (cur_func_it != this->func_end) {
        cur_func_ptr = &(cur_func_it->second);
        cur_func_ptr->num_calls++;
      } else {
        cur_func_ptr = &(this->functions[cur_PC]);
        this->func_end = this->functions.end();
        cur_func_ptr->name = func_name;
        cur_func_ptr->address = cur_PC;
      }

      //std::cerr << "Entering in " << cur_func_ptr->name << " " << std::hex << std::showbase << cur_PC << " function at cur_PC " << func_name << ".\n";

      // Now I have to update the statistics on the number of instructions executed on the
      // instruction stack so far
      sc_time cur_delta_time = sc_time_stamp() - this->old_func_time;

      if (this->cur_stack.size() > 0) {
        //std::cerr << "From " << this->cur_stack.back()->name << " " << std::hex << std::showbase << this->prev_PC << ".\n";;
        this->cur_stack.back()->num_excl_instr += this->old_func_instr;
        this->cur_stack.back()->excl_time += cur_delta_time;
      }

      for (stack_it = this->cur_stack.begin(), stack_end = this->cur_stack.end();
      stack_it != stack_end;
      stack_it++) {
        if (!(*stack_it)->already_examined) {
          (*stack_it)->num_total_instr += this->old_func_instr;
          (*stack_it)->total_time += cur_delta_time;
          (*stack_it)->already_examined = true;
        }
      }
      // Push the element on the stack.
      this->cur_stack.push_back(cur_func_ptr);
      // Record the call time of the function.
      this->old_func_time = sc_time_stamp();
      this->old_func_instr = 0;
      // Reset the already examined flag.
      for (stack_it = this->cur_stack.begin(), stack_end = this->cur_stack.end();
      stack_it != stack_end;
      stack_it++) {
        (*stack_it)->already_examined = false;
      }
    } else if (this->processor->is_routine_exit(cur_instr)) {
      std::string func_name = this->elf_frontend->symbol_at(cur_PC);
      if (this->ignored.find(func_name) != this->ignored.end()) {
        this->old_func_instr++;
        return;
      }
      // Update the timing statistics for the function on the top of the stack
      // and pop it from the stack.
      if (this->cur_stack.size() == 0) {
        THROW_ERROR("Empty stack while exiting from routine " << func_name << " at address " << std::hex << std::showbase << cur_PC << ".");
      }
      // Update the statistics for the current instruction.
      ProfFunction* cur_func_ptr = this->cur_stack.back();
      cur_func_ptr->num_excl_instr += this->old_func_instr;
      sc_time cur_delta_time = sc_time_stamp() - this->old_func_time;
      cur_func_ptr->excl_time += cur_delta_time;

      //std::cerr << "Exiting from " << cur_func_ptr->name << " " << std::hex << std::showbase << cur_PC << ", currently in " << func_name << ".\n";

      // Update the statistics on the number of instructions executed on the
      // instruction stack.
      for (stack_it = this->cur_stack.begin(), stack_end = this->cur_stack.end();
      stack_it != stack_end;
      stack_it++) {
        if (!(*stack_it)->already_examined) {
          (*stack_it)->num_total_instr += this->old_func_instr;
          (*stack_it)->total_time += cur_delta_time;
          (*stack_it)->already_examined = true;
        }
      }
      // Restore the already examined flag.
      for (stack_it = this->cur_stack.begin(), stack_end = this->cur_stack.end();
      stack_it != stack_end;
      stack_it++) {
        (*stack_it)->already_examined = false;
      }
      // Pop the instruction from the stack.
      this->cur_stack.pop_back();
      this->exited = true;
      this->old_func_instr = 0;
      this->old_func_time = sc_time_stamp();
    } else {
      this->old_func_instr++;
    }
    //this->prev_PC = cur_PC;
  } // update_func_stats()

  /// @} Internal Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  private:
  // Interface with the processor.
  ABIIf<IssueWidth>* processor;
  // Instance of the ELF parser containing information about the software
  // running on the processor
  ELFFrontend* elf_frontend;
  // Statistic on the instructions.
  template_map<unsigned, ProfInstruction> instructions;
  ProfInstruction* old_instr;
  sc_time old_instr_time;
  template_map<unsigned, ProfInstruction>::iterator instr_end;
  // Statistic on the functions.
  typename template_map<IssueWidth, ProfFunction> functions;
  std::vector<ProfFunction*> cur_stack;
  sc_time old_func_time;
  unsigned old_func_instr;
  typename template_map<IssueWidth, ProfFunction>::iterator func_end;
  // Names of the routines which should be ignored from entry or exit.
  std::set<std::string> ignored;
  bool exited;
  // Address range inside which the instruction statistics are updated.
  IssueWidth lower_addr;
  IssueWidth higher_addr;
  bool stats_running;
  bool disable_func_profiling;

  /// @} Data
}; // class Profiler

} // namespace trap

/// ****************************************************************************
#endif // TRAP_PROFILER_H
