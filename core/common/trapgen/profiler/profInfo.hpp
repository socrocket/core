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

#ifndef PROFINFO_HPP
#define PROFINFO_HPP

#include <string>
#include "core/common/systemc.h"

namespace trap {
///Represents all the profiling data which can be
///associated with a single assembly instruction
struct ProfInstruction {
  ///Name of the assembly instruction (MOV, ADD ...)
  std::string name;
  ///Number of times this instruction is called
  unsigned long long numCalls;
  ///Total number of instructions executed
  static unsigned long long numTotalCalls;
  ///Total time spent in executing the instruction
  sc_time time;
  ///dump these information to a string, in the command separated values (CVS) format
  std::string printCsv();
  ///Prints the description of the informations which describe an instruction, in the command separated values (CVS) format
  static std::string printCsvHeader();
  ///Prints the summary of all the executed instructions, in the command separated values (CVS) format
  static std::string printCsvSummary();
  ///Empty constructor, performs the initialization of the statistics
  ProfInstruction();
};

///Represents all the profiling data which can be
///associated with a single function
struct ProfFunction {
  ///Address of the function
  unsigned int address;
  ///Name of the function
  std::string name;
  ///Number of times this function is called
  unsigned long long numCalls;
  ///Total number of function calls
  static unsigned long long numTotalCalls;
  ///The number of assembly instructions executed in total inside the function
  unsigned long long totalNumInstr;
  ///The number of assembly instructions executed exclusively inside the function
  unsigned long long exclNumInstr;
  ///Total time spent in the function
  sc_time totalTime;
  ///Time spent exclusively in the function
  sc_time exclTime;
  ///Used to coorectly keep track of the increment of the time, instruction count, etc.
  ///in recursive functions
  bool alreadyExamined;
  ///dump these information to a string, in the command separated values (CVS) format
  std::string printCsv();
  ///Prints the description of the informations which describe a function, in the command separated values (CVS) format
  static std::string printCsvHeader();
  ///Empty constructor, performs the initialization of the statistics
  ProfFunction();
};
}

#endif
