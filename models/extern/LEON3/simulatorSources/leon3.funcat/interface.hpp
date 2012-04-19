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


#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <ABIIf.hpp>
#include <leon3.funcat/memory.hpp>
#include <leon3.funcat/registers.hpp>
#include <leon3.funcat/alias.hpp>
#include <boost/circular_buffer.hpp>
#include <instructionBase.hpp>
#include <vector>
#include <string>
#include <trap_utils.hpp>
#include <systemc.h>

#define FUNC_MODEL
#define AT_IF
using namespace trap;
namespace leon3_funcat_trap{

    class LEON3_ABIIf : public ABIIf< unsigned int >{
        private:
        unsigned int & PROGRAM_LIMIT;
        MemoryInterface & dataMem;
        Reg32_0 & PSR;
        Reg32_1 & WIM;
        Reg32_2 & TBR;
        Reg32_3 & Y;
        Reg32_3 & PC;
        Reg32_3 & NPC;
        RegisterBankClass & GLOBAL;
        Reg32_3 * WINREGS;
        Reg32_3 * ASR;
        Alias & FP;
        Alias & LR;
        Alias & SP;
        Alias & PCR;
        Alias * REGS;
        bool & instrExecuting;
        sc_event & instrEndEvent;
        boost::circular_buffer< HistoryInstrType > & instHistoryQueue;
        int routineEntryState;
        int routineExitState;
        std::vector< std::vector< std::string > > routineEntrySequence;
        std::vector< std::vector< std::string > > routineExitSequence;

        public:
        LEON3_ABIIf( unsigned int & PROGRAM_LIMIT, MemoryInterface & dataMem, Reg32_0 & PSR, \
            Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, Reg32_3 & NPC, RegisterBankClass \
            & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias & FP, Alias & LR, Alias & SP, Alias \
            & PCR, Alias * REGS, bool & instrExecuting, sc_event & instrEndEvent, boost::circular_buffer< \
            HistoryInstrType > & instHistoryQueue );
        bool isLittleEndian() const throw();
        int getProcessorID() const throw();
        bool isInstrExecuting() const throw();
        void waitInstrEnd() const throw();
        void preCall() throw();
        void postCall() throw();
        void returnFromCall() throw();
        bool isRoutineEntry( const InstructionBase * instr ) throw();
        bool isRoutineExit( const InstructionBase * instr ) throw();
        unsigned char * getState() const throw();
        void setState( unsigned char * state ) throw();
        unsigned int getCodeLimit();
        unsigned int readLR() const throw();
        void setLR( const unsigned int & newValue ) throw();
        unsigned int readPC() const throw();
        void setPC( const unsigned int & newValue ) throw();
        unsigned int readSP() const throw();
        void setSP( const unsigned int & newValue ) throw();
        unsigned int readFP() const throw();
        void setFP( const unsigned int & newValue ) throw();
        unsigned int readRetVal() const throw();
        void setRetVal( const unsigned int & newValue ) throw();
        std::vector< unsigned int > readArgs() const throw();
        void setArgs( const std::vector< unsigned int > & args ) throw();
        unsigned int readGDBReg( const unsigned int & gdbId ) const throw();
        unsigned int nGDBRegs() const throw();
        void setGDBReg( const unsigned int & newValue, const unsigned int & gdbId ) throw();
        unsigned int readMem( const unsigned int & address );
        unsigned char readCharMem( const unsigned int & address );
        void writeMem( const unsigned int & address, unsigned int datum );
        void writeCharMem( const unsigned int & address, unsigned char datum );
        boost::circular_buffer< HistoryInstrType > & getInstructionHistory();
        virtual ~LEON3_ABIIf();
    };

};



#endif
