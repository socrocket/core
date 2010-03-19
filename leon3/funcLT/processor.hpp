/***************************************************************************\
 *
 *   
 *            ___        ___           ___           ___
 *           /  /\      /  /\         /  /\         /  /\
 *          /  /:/     /  /::\       /  /::\       /  /::\
 *         /  /:/     /  /:/\:\     /  /:/\:\     /  /:/\:\
 *        /  /:/     /  /:/~/:/    /  /:/~/::\   /  /:/~/:/
 *       /  /::\    /__/:/ /:/___ /__/:/ /:/\:\ /__/:/ /:/
 *      /__/:/\:\   \  \:\/:::::/ \  \:\/:/__\/ \  \:\/:/
 *      \__\/  \:\   \  \::/~~~~   \  \::/       \  \::/
 *           \  \:\   \  \:\        \  \:\        \  \:\
 *            \  \ \   \  \:\        \  \:\        \  \:\
 *             \__\/    \__\/         \__\/         \__\/
 *   
 *
 *
 *   
 *   This file is part of TRAP.
 *   
 *   TRAP is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
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
 *   (c) Luca Fossati, fossati@elet.polimi.it
 *
\***************************************************************************/


#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <systemc.h>
#include <customExceptions.hpp>
#include <instructions.hpp>
#include <decoder.hpp>
#include <interface.hpp>
#include <ToolsIf.hpp>
#include <tlm_utils/tlm_quantumkeeper.h>
#include <registers.hpp>
#include <alias.hpp>
#include <externalPorts.hpp>
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

#include <irqPorts.hpp>
#include <externalPins.hpp>

#define FUNC_MODEL
#define LT_IF
using namespace trap;
namespace leon3_funclt_trap{

    class Processor : public sc_module{
        private:
        Decoder decoder;
        tlm_utils::tlm_quantumkeeper quantKeeper;
        bool instrExecuting;
        sc_event instrEndEvent;
        static Instruction * * INSTRUCTIONS;
        template_map< unsigned int, CacheElem > instrCache;
        static int numInstances;
        unsigned int IRQ;

        public:
        SC_HAS_PROCESS( Processor );
        Processor( sc_module_name name, sc_time latency );
        void mainLoop();
        void resetOp();
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
        TLMMemory instrMem;
        TLMMemory dataMem;
        sc_time latency;
        unsigned int numInstructions;
        unsigned int ENTRY_POINT;
        unsigned int PROGRAM_LIMIT;
        unsigned int PROGRAM_START;
        IntrTLMPort_32 IRQ_port;
        PinTLM_out_32 irqAck;
        IRQ_IRQ_Instruction * IRQ_irqInstr;
        ~Processor();
    };

};



#endif
