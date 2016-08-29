/***************************************************************************//**
*
*  _/_/_/_/_/  _/_/_/           _/        _/_/_/
*     _/      _/    _/        _/_/       _/    _/
*    _/      _/    _/       _/  _/      _/    _/
*   _/      _/_/_/        _/_/_/_/     _/_/_/
*  _/      _/    _/     _/      _/    _/
* _/      _/      _/  _/        _/   _/
*
* @file     tools_if.hpp
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
#ifndef TRAP_TOOLSIF_HPP
#define TRAP_TOOLSIF_HPP

#include "modules/instruction.hpp"

#include <cstdlib>

namespace trap {

/**
 * @brief MemoryToolsIf
 *
 * Base class for the tools which need to interact with memory, i.e. to be
 * called for every write operation which happens in memory. Note that only one
 * tool at a time can interact with memory.
 */
template<class AddressType>
class MemoryToolsIf {
  public:
  virtual ~MemoryToolsIf() {}
#ifndef NDEBUG
  virtual void notify_address(AddressType address, unsigned size) throw() = 0;
#else
  virtual void notify_address(AddressType address, unsigned size) = 0;
#endif
}; // class MemoryToolsIf

/// ****************************************************************************

/**
 * @brief ToolsIf
 *
 * Base class for all tools (profilers, debugger, etc...).
 */
template<class IssueWidth>
class ToolsIf {
  public:
  virtual ~ToolsIf() {}

  /**
   * The only method which is called to activate the tool. It signals to the
   * tool that a new instruction issue has started. The tool can then take the
   * appropriate actions.
   *
   * @return
   * Indicates whether the processor should skip the issue of the current
   * instruction.
   */
  virtual bool issue(const IssueWidth& cur_PC, const InstructionBase* cur_instr) throw() = 0;

  /**
   * @return
   * True if the pipeline has to be empty before being able to call the current
   * tool, false otherwise.
   */
  virtual bool is_pipeline_empty(const IssueWidth& cur_PC) const throw() = 0;
}; // class ToolsIf

/// ****************************************************************************

/**
 * @brief ToolsManager
 */
template<class IssueWidth>
class ToolsManager {
  /// @name Constructors and Destructors
  /// @{

  public:
  ToolsManager() {
    active_tools = NULL;
    num_active_tools = 0;
  }

  /// @} Constructors and Destructors
  /// --------------------------------------------------------------------------
  /// @name Interface Methods
  /// @{

  /**
   * Add a tool to the list of the tools which are activated when there is a new
   * instruction issue.
   */
  void add_tool(ToolsIf<IssueWidth>& tool) {
    this->num_active_tools++;
    ToolsIf<IssueWidth>** active_tools_temp = new ToolsIf<IssueWidth> *[num_active_tools];
    if (this->active_tools != NULL) {
      for (int i = 0; i < (this->num_active_tools - 1); i++) {
        active_tools_temp[i] = this->active_tools[i];
      }
      delete[] this->active_tools;
    }
    this->active_tools = active_tools_temp;
    this->active_tools[this->num_active_tools - 1] = &tool;
  }

  /**
   * The only method which is called to activate the tool. It signals to the
   * tool that a new instruction issue has started. The tool can then take the
   * appropriate actions.
   *
   * @return
   * Indicates whether the processor should skip the issue of the current
   * instruction.
   */
  inline bool issue(const IssueWidth& cur_PC, const InstructionBase* cur_instr) const throw() {
    bool skip_instruction = false;
    for (int i = 0; i < this->num_active_tools; i++) {
      skip_instruction |= this->active_tools[i]->issue(cur_PC, cur_instr);
    }
    return skip_instruction;
  }

  /**
   * @return
   * True if the pipeline has to be empty before being able to call the current
   * tool, false otherwise.
   */
  inline bool is_pipeline_empty(const IssueWidth& cur_PC) const throw() {
    bool need_to_empty = false;
    for (int i = 0; i < this->num_active_tools; i++) {
      need_to_empty |= this->active_tools[i]->is_pipeline_empty(cur_PC);
    }
    return need_to_empty;
  }

  /// @} Interface Methods
  /// --------------------------------------------------------------------------
  /// @name Data
  /// @{

  private:
  /// List of the active tools which are activated at every instruction.
  ToolsIf<IssueWidth>** active_tools;
  int num_active_tools;

  /// @} Data
}; // class ToolsManager

} // namespace trap

/// ****************************************************************************
#endif
