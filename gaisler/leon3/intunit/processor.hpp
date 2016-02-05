/***************************************************************************\
 *
 *
 *         _/        _/_/_/_/    _/_/    _/      _/   _/_/_/
 *        _/        _/        _/    _/  _/_/    _/         _/
 *       _/        _/_/_/    _/    _/  _/  _/  _/     _/_/
 *      _/        _/        _/    _/  _/    _/_/         _/
 *     _/_/_/_/  _/_/_/_/    _/_/    _/      _/   _/_/_/
 *
 *
 *
 *
 *   This file is part of LEON3.
 *
 *   LEON3 is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *   or see <http://www.gnu.org/licenses/>.
 *
 *
 *
 *   (c) Luca Fossati, fossati.l@gmail.com
 *
\***************************************************************************/


#ifndef LT_PROCESSOR_HPP
#define LT_PROCESSOR_HPP

#include "core/common/sr_param.h"
#include "core/common/trapgen/utils/customExceptions.hpp"
#include "gaisler/leon3/intunit/instructions.hpp"
#include "gaisler/leon3/intunit/decoder.hpp"
#include "gaisler/leon3/intunit/interface.hpp"
#include "core/common/trapgen/ToolsIf.hpp"
#include <tlm_utils/tlm_quantumkeeper.h>
#include "gaisler/leon3/intunit/registers.hpp"
#include "gaisler/leon3/intunit/alias.hpp"
#include "gaisler/leon3/intunit/memory.hpp"
#include <iostream>
#include <fstream>
#include <boost/circular_buffer.hpp>
#include "core/common/trapgen/instructionBase.hpp"
#include "core/common/vmap.h"

#include "gaisler/leon3/intunit/irqPorts.hpp"
#include "gaisler/leon3/intunit/externalPins.hpp"
#include <string>
#include "core/common/systemc.h"

#define FUNC_MODEL
#define LT_IF
using namespace trap;
namespace leon3_funclt_trap{

    class Processor_leon3_funclt : public sc_module{
      private:
        bool resetCalled;
        void beginOp();
        Decoder decoder;
        unsigned int profStartAddr;
        unsigned int profEndAddr;
        //std::ofstream histFile;
        bool instrExecuting;
        sc_event instrEndEvent;
        Instruction **INSTRUCTIONS;
        Instruction *curInstrPtr;
        unsigned int raisedException;
        unsigned int raisedExceptionPC;
        unsigned int raisedExceptionNPC;
        vmap<unsigned int, CacheElem> instrCache;
        static int numInstances;
        unsigned int IRQ;

      public:
        GC_HAS_CALLBACKS();
        SC_HAS_PROCESS(Processor_leon3_funclt);
        Processor_leon3_funclt(sc_module_name name, MemoryInterface *memory = NULL, sc_time latency = sc_time(10, sc_core::SC_NS), bool pow_mon = false);
        void mainLoop();
        void resetOp();
        void start_of_simulation();
        void end_of_simulation();
        void power_model();
        void triggerException(unsigned int exception);
        tlm_utils::tlm_quantumkeeper quantKeeper;
        gs::cnf::callback_return_type sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);
        gs::cnf::callback_return_type int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);
        gs::cnf::callback_return_type swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason);
        void end_of_elaboration();
        Instruction * decode( unsigned int bitString );
        LEON3_ABIIf * abiIf;
        LEON3_ABIIf & getInterface();
        ToolsManager< unsigned int > toolManager;
        Reg32_0 PSR;
        Reg32_1 WIM;
        Reg32_2 TBR;
        Reg32_3 Y;
        Reg32_3 PC;
        Reg32_3 NPC;
        RegisterBankClass GLOBAL;
        Reg32_3 WINREGS[128];
        Reg32_3 ASR[32];
        Alias FP;
        Alias LR;
        Alias SP;
        Alias PCR;
        Alias REGS[32];
        MemoryInterface *mem;
        MemoryInterface &instrMem;
        MemoryInterface &dataMem;
        sc_time latency;
        sc_time profTimeStart;
        sc_time profTimeEnd;
        unsigned int undumpedHistElems;
        unsigned int ENTRY_POINT;
        unsigned int MPROC_ID;
        unsigned int PROGRAM_LIMIT;
        unsigned int PROGRAM_START;
        unsigned int curPC;
        IntrTLMPort_32 IRQ_port;
        PinTLM_out_32 irqAck;
        sr_param<bool> historyEnabled;
        bool m_pow_mon;
        void setProfilingRange( unsigned int startAddr, unsigned int endAddr );
        IRQ_IRQ_Instruction * IRQ_irqInstr;
        ~Processor_leon3_funclt();

      /// ******************************************************************
      /// Power Modeling Parameters

      /// Normalized static power input
      sr_param<double> sta_power_norm;

      /// Normalized internal power input (activation independent)
      sr_param<double> int_power_norm;

      /// Normalized average instruction energy
      sr_param<double> dyn_instr_energy_norm;

      /// Parameter array for power data output
      gs::gs_param_array power;

      /// Static power of module
      sr_param<double> sta_power;

      /// Dynamic power of module (activation independent)
      sr_param<double> int_power;

      /// Switching power of module
      sr_param<double> swi_power;

      /// Power frame starting time
      sr_param<sc_core::sc_time> power_frame_starting_time;

      /// Average dynamic energy per instruction
      sr_param<double> dyn_instr_energy;

      /// Number of instructions processed in time frame
      sr_param<uint64_t> dyn_instr;

      /// Number of instructions processed
      sr_param<uint64_t> numInstructions;
    };

};


#undef LT_IF
#endif
