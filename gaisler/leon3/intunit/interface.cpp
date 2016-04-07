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



#include "gaisler/leon3/intunit/interface.hpp"
#include "core/common/trapgen/ABIIf.hpp"
#include "gaisler/leon3/intunit/memory.hpp"
#include "gaisler/leon3/intunit/registers.hpp"
#include "gaisler/leon3/intunit/alias.hpp"
#include <boost/circular_buffer.hpp>
#include "core/common/trapgen/instructionBase.hpp"
#include <vector>
#include <string>
#include "core/common/trapgen/utils/trap_utils.hpp"
#include "core/common/systemc.h"

using namespace leon3_funclt_trap;
using namespace trap;
bool leon3_funclt_trap::LEON3_ABIIf::isLittleEndian() const throw(){
    return false;
}

int leon3_funclt_trap::LEON3_ABIIf::getProcessorID() const throw(){
    return ((ASR[17] & 0xF0000000) >> 28);
}

bool leon3_funclt_trap::LEON3_ABIIf::isInstrExecuting() const throw(){
    return this->instrExecuting;
}

void leon3_funclt_trap::LEON3_ABIIf::waitInstrEnd() const throw(){
    if(this->instrExecuting){
        wait(this->instrEndEvent);
    }
}

void leon3_funclt_trap::LEON3_ABIIf::preCall() throw(){

    unsigned int newCwp = ((unsigned int)(PSR[key_CWP] - 1)) % 8;
    PSR.immediateWrite((PSR & 0xFFFFFFE0) | newCwp);

    //ABI model: we simply immediately update the alias
    for(int i = 8; i < 32; i++){
        REGS[i].updateAlias(WINREGS[(newCwp*16 + i - 8) % (128)]);
    }
}

void leon3_funclt_trap::LEON3_ABIIf::postCall() throw(){

    unsigned int newCwp = ((unsigned int)(PSR[key_CWP] + 1)) % 8;
    PSR.immediateWrite((PSR & 0xFFFFFFE0) | newCwp);

    //ABI model: we simply immediately update the alias
    for(int i = 8; i < 32; i++){
        REGS[i].updateAlias(WINREGS[(newCwp*16 + i - 8) % (128)]);
    }
}

void leon3_funclt_trap::LEON3_ABIIf::returnFromCall() throw(){
    PC.immediateWrite(LR + 8);
    NPC.immediateWrite(LR + 12);
}

bool leon3_funclt_trap::LEON3_ABIIf::isRoutineEntry( const InstructionBase * instr \
    ) throw(){
    std::vector<std::string> nextNames = this->routineEntrySequence[this->routineEntryState];
    std::vector<std::string>::const_iterator namesIter, namesEnd;
    std::string curName = instr->getInstructionName();
    for(namesIter = nextNames.begin(), namesEnd = nextNames.end(); namesIter != namesEnd; \
        namesIter++){
        if(curName == *namesIter || *namesIter == ""){
            if(this->routineEntryState == 2){
                this->routineEntryState = 0;
                return true;
            }
            this->routineEntryState++;
            return false;
        }
    }
    this->routineEntryState = 0;
    return false;
}

bool leon3_funclt_trap::LEON3_ABIIf::isRoutineExit( const InstructionBase * instr \
    ) throw(){
    std::vector<std::string> nextNames = this->routineExitSequence[this->routineExitState];
    std::vector<std::string>::const_iterator namesIter, namesEnd;
    std::string curName = instr->getInstructionName();
    for(namesIter = nextNames.begin(), namesEnd = nextNames.end(); namesIter != namesEnd; \
        namesIter++){
        if(curName == *namesIter || *namesIter == ""){
            if(this->routineExitState == 1){
                this->routineExitState = 0;
                return true;
            }
            this->routineExitState++;
            return false;
        }
    }
    this->routineExitState = 0;
    return false;
}

unsigned char * leon3_funclt_trap::LEON3_ABIIf::getState() const throw(){
    unsigned char * curState = new unsigned char[696];
    unsigned char * curStateTemp = curState;
    *((unsigned int *)curStateTemp) = this->PSR.readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WIM.readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->TBR.readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->Y.readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->PC.readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->NPC.readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->GLOBAL[0].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->GLOBAL[1].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->GLOBAL[2].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->GLOBAL[3].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->GLOBAL[4].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->GLOBAL[5].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->GLOBAL[6].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->GLOBAL[7].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[0].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[1].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[2].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[3].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[4].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[5].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[6].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[7].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[8].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[9].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[10].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[11].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[12].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[13].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[14].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[15].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[16].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[17].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[18].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[19].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[20].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[21].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[22].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[23].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[24].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[25].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[26].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[27].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[28].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[29].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[30].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[31].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[32].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[33].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[34].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[35].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[36].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[37].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[38].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[39].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[40].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[41].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[42].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[43].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[44].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[45].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[46].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[47].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[48].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[49].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[50].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[51].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[52].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[53].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[54].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[55].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[56].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[57].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[58].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[59].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[60].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[61].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[62].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[63].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[64].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[65].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[66].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[67].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[68].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[69].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[70].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[71].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[72].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[73].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[74].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[75].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[76].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[77].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[78].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[79].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[80].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[81].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[82].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[83].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[84].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[85].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[86].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[87].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[88].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[89].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[90].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[91].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[92].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[93].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[94].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[95].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[96].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[97].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[98].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[99].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[100].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[101].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[102].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[103].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[104].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[105].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[106].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[107].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[108].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[109].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[110].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[111].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[112].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[113].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[114].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[115].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[116].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[117].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[118].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[119].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[120].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[121].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[122].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[123].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[124].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[125].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[126].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->WINREGS[127].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[0].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[1].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[2].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[3].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[4].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[5].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[6].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[7].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[8].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[9].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[10].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[11].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[12].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[13].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[14].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[15].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[16].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[18].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[19].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[20].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[21].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[22].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[23].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[24].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[25].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[26].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[27].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[28].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[29].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[30].readNewValue();
    curStateTemp += 4;
    *((unsigned int *)curStateTemp) = this->ASR[31].readNewValue();
    curStateTemp += 4;
    return curState;
}

void leon3_funclt_trap::LEON3_ABIIf::setState( unsigned char * state ) throw(){
    unsigned char * curStateTemp = state;
    this->PSR.immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WIM.immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->TBR.immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->Y.immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->PC.immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->NPC.immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->GLOBAL[0].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->GLOBAL[1].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->GLOBAL[2].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->GLOBAL[3].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->GLOBAL[4].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->GLOBAL[5].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->GLOBAL[6].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->GLOBAL[7].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[0].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[1].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[2].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[3].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[4].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[5].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[6].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[7].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[8].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[9].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[10].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[11].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[12].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[13].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[14].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[15].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[16].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[17].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[18].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[19].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[20].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[21].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[22].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[23].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[24].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[25].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[26].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[27].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[28].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[29].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[30].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[31].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[32].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[33].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[34].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[35].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[36].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[37].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[38].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[39].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[40].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[41].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[42].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[43].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[44].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[45].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[46].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[47].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[48].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[49].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[50].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[51].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[52].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[53].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[54].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[55].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[56].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[57].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[58].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[59].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[60].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[61].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[62].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[63].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[64].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[65].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[66].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[67].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[68].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[69].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[70].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[71].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[72].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[73].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[74].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[75].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[76].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[77].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[78].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[79].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[80].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[81].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[82].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[83].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[84].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[85].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[86].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[87].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[88].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[89].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[90].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[91].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[92].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[93].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[94].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[95].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[96].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[97].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[98].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[99].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[100].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[101].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[102].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[103].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[104].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[105].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[106].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[107].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[108].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[109].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[110].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[111].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[112].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[113].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[114].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[115].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[116].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[117].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[118].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[119].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[120].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[121].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[122].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[123].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[124].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[125].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[126].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->WINREGS[127].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[0].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[1].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[2].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[3].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[4].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[5].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[6].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[7].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[8].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[9].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[10].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[11].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[12].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[13].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[14].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[15].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[16].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[18].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[19].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[20].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[21].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[22].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[23].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[24].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[25].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[26].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[27].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[28].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[29].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[30].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
    this->ASR[31].immediateWrite(*((unsigned int *)curStateTemp));
    curStateTemp += 4;
}

void leon3_funclt_trap::LEON3_ABIIf::setExitValue( unsigned int value ) throw(){
  this->exitValue = value;
}
unsigned int leon3_funclt_trap::LEON3_ABIIf::getExitValue() throw(){
  return this->exitValue;
}

unsigned int leon3_funclt_trap::LEON3_ABIIf::getCodeLimit(){
    return this->PROGRAM_LIMIT;
}

unsigned int leon3_funclt_trap::LEON3_ABIIf::readLR() const throw(){
    return this->LR;
}

void leon3_funclt_trap::LEON3_ABIIf::setLR( const unsigned int & newValue ) throw(){
    this->LR.immediateWrite(newValue);
}

unsigned int leon3_funclt_trap::LEON3_ABIIf::readPC() const throw(){
    return this->PC;
}

void leon3_funclt_trap::LEON3_ABIIf::setPC( const unsigned int & newValue ) throw(){
    this->PC.immediateWrite(newValue);
}

unsigned int leon3_funclt_trap::LEON3_ABIIf::readSP() const throw(){
    return this->SP;
}

void leon3_funclt_trap::LEON3_ABIIf::setSP( const unsigned int & newValue ) throw(){
    this->SP.immediateWrite(newValue);
}

unsigned int leon3_funclt_trap::LEON3_ABIIf::readFP() const throw(){
    return this->FP;
}

void leon3_funclt_trap::LEON3_ABIIf::setFP( const unsigned int & newValue ) throw(){
    this->FP.immediateWrite(newValue);
}

unsigned int leon3_funclt_trap::LEON3_ABIIf::readRetVal() const throw(){
    return this->REGS[24];
}

void leon3_funclt_trap::LEON3_ABIIf::setRetVal( const unsigned int & newValue ) throw(){
    this->REGS[24].immediateWrite(newValue);
}

std::vector< unsigned int > leon3_funclt_trap::LEON3_ABIIf::readArgs() const throw(){
    std::vector< unsigned int > args;
    args.push_back(this->REGS[24]);
    args.push_back(this->REGS[25]);
    args.push_back(this->REGS[26]);
    args.push_back(this->REGS[27]);
    args.push_back(this->REGS[28]);
    args.push_back(this->REGS[29]);
    return args;
}

void leon3_funclt_trap::LEON3_ABIIf::setArgs( const std::vector< unsigned int > & \
    args ) throw(){
    if(args.size() > 6){
        THROW_EXCEPTION("ABI of processor supports up to 6 arguments: " << args.size() << \
            " given");
    }
    std::vector< unsigned int >::const_iterator argIter = args.begin(), argEnd = args.end();
    if(argIter != argEnd){
        this->REGS[24].immediateWrite(*argIter);
        argIter++;
    }
    if(argIter != argEnd){
        this->REGS[25].immediateWrite(*argIter);
        argIter++;
    }
    if(argIter != argEnd){
        this->REGS[26].immediateWrite(*argIter);
        argIter++;
    }
    if(argIter != argEnd){
        this->REGS[27].immediateWrite(*argIter);
        argIter++;
    }
    if(argIter != argEnd){
        this->REGS[28].immediateWrite(*argIter);
        argIter++;
    }
    if(argIter != argEnd){
        this->REGS[29].immediateWrite(*argIter);
        argIter++;
    }
}

unsigned int leon3_funclt_trap::LEON3_ABIIf::readGDBReg( const unsigned int & gdbId \
    ) const throw(){
    switch(gdbId){
        case 0:{
            return REGS[0];
        break;}
        case 1:{
            return REGS[1];
        break;}
        case 2:{
            return REGS[2];
        break;}
        case 3:{
            return REGS[3];
        break;}
        case 4:{
            return REGS[4];
        break;}
        case 5:{
            return REGS[5];
        break;}
        case 6:{
            return REGS[6];
        break;}
        case 7:{
            return REGS[7];
        break;}
        case 8:{
            return REGS[8];
        break;}
        case 9:{
            return REGS[9];
        break;}
        case 10:{
            return REGS[10];
        break;}
        case 11:{
            return REGS[11];
        break;}
        case 12:{
            return REGS[12];
        break;}
        case 13:{
            return REGS[13];
        break;}
        case 14:{
            return REGS[14];
        break;}
        case 15:{
            return REGS[15];
        break;}
        case 16:{
            return REGS[16];
        break;}
        case 17:{
            return REGS[17];
        break;}
        case 18:{
            return REGS[18];
        break;}
        case 19:{
            return REGS[19];
        break;}
        case 20:{
            return REGS[20];
        break;}
        case 21:{
            return REGS[21];
        break;}
        case 22:{
            return REGS[22];
        break;}
        case 23:{
            return REGS[23];
        break;}
        case 24:{
            return REGS[24];
        break;}
        case 25:{
            return REGS[25];
        break;}
        case 26:{
            return REGS[26];
        break;}
        case 27:{
            return REGS[27];
        break;}
        case 28:{
            return REGS[28];
        break;}
        case 29:{
            return REGS[29];
        break;}
        case 30:{
            return REGS[30];
        break;}
        case 31:{
            return REGS[31];
        break;}
        case 64:{
            return Y;
        break;}
        case 65:{
            return PSR;
        break;}
        case 66:{
            return WIM;
        break;}
        case 67:{
            return TBR;
        break;}
        case 68:{
            return PC;
        break;}
        case 69:{
            return NPC;
        break;}
        default:{
            return 0;
        }
    }
}

unsigned int leon3_funclt_trap::LEON3_ABIIf::nGDBRegs() const throw(){
    return 70;
}

void leon3_funclt_trap::LEON3_ABIIf::setGDBReg( const unsigned int & newValue, const \
    unsigned int & gdbId ) throw(){
    switch(gdbId){
        case 0:{
            REGS[0].immediateWrite(newValue);
        break;}
        case 1:{
            REGS[1].immediateWrite(newValue);
        break;}
        case 2:{
            REGS[2].immediateWrite(newValue);
        break;}
        case 3:{
            REGS[3].immediateWrite(newValue);
        break;}
        case 4:{
            REGS[4].immediateWrite(newValue);
        break;}
        case 5:{
            REGS[5].immediateWrite(newValue);
        break;}
        case 6:{
            REGS[6].immediateWrite(newValue);
        break;}
        case 7:{
            REGS[7].immediateWrite(newValue);
        break;}
        case 8:{
            REGS[8].immediateWrite(newValue);
        break;}
        case 9:{
            REGS[9].immediateWrite(newValue);
        break;}
        case 10:{
            REGS[10].immediateWrite(newValue);
        break;}
        case 11:{
            REGS[11].immediateWrite(newValue);
        break;}
        case 12:{
            REGS[12].immediateWrite(newValue);
        break;}
        case 13:{
            REGS[13].immediateWrite(newValue);
        break;}
        case 14:{
            REGS[14].immediateWrite(newValue);
        break;}
        case 15:{
            REGS[15].immediateWrite(newValue);
        break;}
        case 16:{
            REGS[16].immediateWrite(newValue);
        break;}
        case 17:{
            REGS[17].immediateWrite(newValue);
        break;}
        case 18:{
            REGS[18].immediateWrite(newValue);
        break;}
        case 19:{
            REGS[19].immediateWrite(newValue);
        break;}
        case 20:{
            REGS[20].immediateWrite(newValue);
        break;}
        case 21:{
            REGS[21].immediateWrite(newValue);
        break;}
        case 22:{
            REGS[22].immediateWrite(newValue);
        break;}
        case 23:{
            REGS[23].immediateWrite(newValue);
        break;}
        case 24:{
            REGS[24].immediateWrite(newValue);
        break;}
        case 25:{
            REGS[25].immediateWrite(newValue);
        break;}
        case 26:{
            REGS[26].immediateWrite(newValue);
        break;}
        case 27:{
            REGS[27].immediateWrite(newValue);
        break;}
        case 28:{
            REGS[28].immediateWrite(newValue);
        break;}
        case 29:{
            REGS[29].immediateWrite(newValue);
        break;}
        case 30:{
            REGS[30].immediateWrite(newValue);
        break;}
        case 31:{
            REGS[31].immediateWrite(newValue);
        break;}
        case 64:{
            Y.immediateWrite(newValue);
        break;}
        case 65:{
            PSR.immediateWrite(newValue);
        break;}
        case 66:{
            WIM.immediateWrite(newValue);
        break;}
        case 67:{
            TBR.immediateWrite(newValue);
        break;}
        case 68:{
            PC.immediateWrite(newValue);
        break;}
        case 69:{
            NPC.immediateWrite(newValue);
        break;}
        default:{
            THROW_EXCEPTION("No register corresponding to GDB id " << gdbId);
        }
    }
}

unsigned int leon3_funclt_trap::LEON3_ABIIf::readMem( const unsigned int & address ){
    return this->dataMem.read_word_dbg(address);
}

unsigned char leon3_funclt_trap::LEON3_ABIIf::readCharMem( const unsigned int & address \
    ){
    return this->dataMem.read_byte_dbg(address);
}

void leon3_funclt_trap::LEON3_ABIIf::writeMem( const unsigned int & address, unsigned \
    int datum ){
    this->dataMem.write_word_dbg(address, datum);
}

void leon3_funclt_trap::LEON3_ABIIf::writeCharMem( const unsigned int & address, \
    unsigned char datum ){
    this->dataMem.write_byte_dbg(address, datum);
}


leon3_funclt_trap::LEON3_ABIIf::~LEON3_ABIIf(){

}
leon3_funclt_trap::LEON3_ABIIf::LEON3_ABIIf( unsigned int & PROGRAM_LIMIT, MemoryInterface \
    & dataMem, Reg32_0 & PSR, Reg32_1 & WIM, Reg32_2 & TBR, Reg32_3 & Y, Reg32_3 & PC, \
    Reg32_3 & NPC, RegisterBankClass & GLOBAL, Reg32_3 * WINREGS, Reg32_3 * ASR, Alias \
    & FP, Alias & LR, Alias & SP, Alias & PCR, Alias * REGS, bool & instrExecuting, sc_event \
    & instrEndEvent ) \
    : PROGRAM_LIMIT(PROGRAM_LIMIT), dataMem(dataMem), PSR(PSR), WIM(WIM), TBR(TBR), Y(Y), \
    PC(PC), NPC(NPC), GLOBAL(GLOBAL), WINREGS(WINREGS), ASR(ASR), FP(FP), LR(LR), SP(SP), \
    PCR(PCR), REGS(REGS), instrExecuting(instrExecuting), instrEndEvent(instrEndEvent){
    this->routineExitState = 0;
    this->routineEntryState = 0;
    std::vector<std::string> tempVec;
    tempVec.clear();
    tempVec.push_back("CALL");
    this->routineEntrySequence.push_back(tempVec);
    tempVec.clear();
    tempVec.push_back("");
    this->routineEntrySequence.push_back(tempVec);
    tempVec.clear();
    tempVec.push_back("");
    this->routineEntrySequence.push_back(tempVec);
    tempVec.clear();
    tempVec.push_back("RESTORE_imm");
    tempVec.push_back("RESTORE_reg");
    tempVec.push_back("JUMP_imm");
    tempVec.push_back("JUMP_reg");
    this->routineExitSequence.push_back(tempVec);
    tempVec.clear();
    tempVec.push_back("JUMP_imm");
    tempVec.push_back("JUMP_reg");
    tempVec.push_back("RESTORE_imm");
    tempVec.push_back("RESTORE_reg");
    this->routineExitSequence.push_back(tempVec);
}

MemoryInterface & leon3_funclt_trap::LEON3_ABIIf::get_data_memory(){
  return this->dataMem;
}
