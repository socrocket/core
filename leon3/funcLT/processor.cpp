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



#include <processor.hpp>
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

using namespace leon3_funclt_trap;
using namespace trap;
void leon3_funclt_trap::Processor::mainLoop(){
    template_map< unsigned int, CacheElem >::iterator instrCacheEnd = this->instrCache.end();while(true){
        unsigned int numCycles = 0;
        this->instrExecuting = true;
        if((IRQ != -1) && (PSR[key_ET] && (IRQ == 15 || IRQ > PSR[key_PIL]))){
            this->IRQ_irqInstr->setInterruptValue(IRQ);
            try{
                numCycles = this->IRQ_irqInstr->behavior();
            }
            catch(annull_exception &etc){
                numCycles = 0;
            }

        }
        else{
            unsigned int curPC = this->PC + 0;
            unsigned int bitString = this->instrMem.read_word(curPC);
            template_map< unsigned int, CacheElem >::iterator cachedInstr = this->instrCache.find(bitString);
            if(cachedInstr != instrCacheEnd){
                Instruction * curInstrPtr = cachedInstr->second.instr;
                // I can call the instruction, I have found it
                if(curInstrPtr != NULL){
                    try{
                        #ifndef DISABLE_TOOLS
                        if(!(this->toolManager.newIssue(curPC, curInstrPtr))){
                            #endif
                            numCycles = curInstrPtr->behavior();
                            #ifndef DISABLE_TOOLS
                        }
                        #endif
                    }
                    catch(annull_exception &etc){
                        numCycles = 0;
                    }
                }
                else{
                    unsigned int & curCount = cachedInstr->second.count;
                    int instrId = this->decoder.decode(bitString);
                    Instruction * instr = Processor::INSTRUCTIONS[instrId];
                    instr->setParams(bitString);
                    try{
                        #ifndef DISABLE_TOOLS
                        if(!(this->toolManager.newIssue(curPC, instr))){
                            #endif
                            numCycles = instr->behavior();
                            #ifndef DISABLE_TOOLS
                        }
                        #endif
                    }
                    catch(annull_exception &etc){
                        numCycles = 0;
                    }
                    if(curCount < 256){
                        curCount++;
                    }
                    else{
                        // ... and then add the instruction to the cache
                        cachedInstr->second.instr = instr;
                        Processor::INSTRUCTIONS[instrId] = instr->replicate();
                    }
                }
            }
            else{
                // The current instruction is not present in the cache:
                // I have to perform the normal decoding phase ...
                int instrId = this->decoder.decode(bitString);
                Instruction * instr = Processor::INSTRUCTIONS[instrId];
                instr->setParams(bitString);
                try{
                    #ifndef DISABLE_TOOLS
                    if(!(this->toolManager.newIssue(curPC, instr))){
                        #endif
                        numCycles = instr->behavior();
                        #ifndef DISABLE_TOOLS
                    }
                    #endif
                }
                catch(annull_exception &etc){
                    numCycles = 0;
                }
                this->instrCache.insert(std::pair< unsigned int, CacheElem >(bitString, CacheElem()));
                instrCacheEnd = this->instrCache.end();
            }
        }
        this->quantKeeper.inc((numCycles + 1)*this->latency);
        if(this->quantKeeper.need_sync()){
            this->quantKeeper.sync();
        }
        this->instrExecuting = false;
        this->instrEndEvent.notify();
        this->numInstructions++;

    }
}

void leon3_funclt_trap::Processor::resetOp(){
    for(int i = 0; i < 8; i++){
        GLOBAL[i] = 0;
    }
    for(int i = 0; i < 128; i++){
        WINREGS[i] = 0;
    }
    ASR[0].immediateWrite(0x0);
    ASR[1].immediateWrite(0x0);
    ASR[2].immediateWrite(0x0);
    ASR[3].immediateWrite(0x0);
    ASR[4].immediateWrite(0x0);
    ASR[5].immediateWrite(0x0);
    ASR[6].immediateWrite(0x0);
    ASR[7].immediateWrite(0x0);
    ASR[8].immediateWrite(0x0);
    ASR[9].immediateWrite(0x0);
    ASR[10].immediateWrite(0x0);
    ASR[11].immediateWrite(0x0);
    ASR[12].immediateWrite(0x0);
    ASR[13].immediateWrite(0x0);
    ASR[14].immediateWrite(0x0);
    ASR[15].immediateWrite(0x0);
    ASR[16].immediateWrite(0x0);
    ASR[17].immediateWrite(0x308);
    ASR[18].immediateWrite(0x0);
    ASR[19].immediateWrite(0x0);
    ASR[20].immediateWrite(0x0);
    ASR[21].immediateWrite(0x0);
    ASR[22].immediateWrite(0x0);
    ASR[23].immediateWrite(0x0);
    ASR[24].immediateWrite(0x0);
    ASR[25].immediateWrite(0x0);
    ASR[26].immediateWrite(0x0);
    ASR[27].immediateWrite(0x0);
    ASR[28].immediateWrite(0x0);
    ASR[29].immediateWrite(0x0);
    ASR[30].immediateWrite(0x0);
    ASR[31].immediateWrite(0x0);
    PSR.immediateWrite(0xf3000080L);
    WIM.immediateWrite(0x0);
    TBR.immediateWrite(0x0);
    Y.immediateWrite(0x0);
    PC.immediateWrite(ENTRY_POINT);
    NPC.immediateWrite(ENTRY_POINT + 4);
    this->IRQ = -1;
}

void leon3_funclt_trap::Processor::end_of_elaboration(){
    this->resetOp();
}

Instruction * leon3_funclt_trap::Processor::decode( unsigned int bitString ){
    int instrId = this->decoder.decode(bitString);
    if(instrId >= 0){
        Instruction * instr = Processor::INSTRUCTIONS[instrId];
        instr->setParams(bitString);
        return instr;
    }
    return NULL;
}

LEON3_ABIIf & leon3_funclt_trap::Processor::getInterface(){
    return *this->abiIf;
}

Instruction * * leon3_funclt_trap::Processor::INSTRUCTIONS = NULL;
int leon3_funclt_trap::Processor::numInstances = 0;
leon3_funclt_trap::Processor::Processor( sc_module_name name, sc_time latency ) : \
    sc_module(name), latency(latency), instrMem("instrMem", this->quantKeeper), dataMem("dataMem", \
    this->quantKeeper), IRQ_port("IRQ_IRQ", IRQ), irqAck("irqAck_PIN"){
    Processor::numInstances++;
    if(Processor::INSTRUCTIONS == NULL){
        // Initialization of the array holding the initial instance of the instructions
        Processor::INSTRUCTIONS = new Instruction *[145];
        Processor::INSTRUCTIONS[126] = new READasr(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[130] = new WRITEY_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[58] = new XNOR_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[44] = new ANDNcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[0] = new LDSB_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[135] = new WRITEpsr_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[125] = new READy(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[60] = new XNORcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[127] = new READpsr(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[41] = new ANDN_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[40] = new ANDcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[87] = new TSUBcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[12] = new LDSBA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[6] = new LDUH_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[28] = new STA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[50] = new ORN_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[13] = new LDSHA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[26] = new STBA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[22] = new ST_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[129] = new READtbr(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[109] = new UDIVcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[35] = new SWAPA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[73] = new ADDXcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[18] = new STB_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[85] = new SUBXcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[21] = new STH_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[63] = new SRL_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[133] = new WRITEasr_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[98] = new UMULcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[31] = new LDSTUB_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[53] = new XOR_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[104] = new SMAC_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[132] = new WRITEasr_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[9] = new LD_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[23] = new ST_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[82] = new SUBcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[11] = new LDD_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[69] = new ADDcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[7] = new LDUH_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[64] = new SRL_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[113] = new SAVE_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[92] = new MULScc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[45] = new OR_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[24] = new STD_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[86] = new SUBXcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[71] = new ADDX_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[33] = new SWAP_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[94] = new UMUL_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[131] = new WRITEY_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[38] = new AND_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[143] = new FLUSH_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[66] = new SRA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[20] = new STH_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[137] = new WRITEwim_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[10] = new LDD_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[61] = new SLL_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[15] = new LDUHA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[76] = new TADDcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[75] = new TADDcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[107] = new SDIV_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[89] = new TSUBccTV_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[142] = new FLUSH_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[52] = new ORNcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[121] = new RETT_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[112] = new SDIVcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[68] = new ADD_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[123] = new TRAP_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[139] = new WRITEtbr_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[5] = new LDUB_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[116] = new RESTORE_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[74] = new ADDXcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[19] = new STB_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[37] = new AND_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[95] = new SMUL_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[67] = new ADD_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[93] = new UMUL_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[128] = new READwim(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[30] = new LDSTUB_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[103] = new SMAC_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[1] = new LDSB_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[42] = new ANDN_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[90] = new TSUBccTV_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[36] = new SETHI(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[65] = new SRA_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[3] = new LDSH_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[110] = new UDIVcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[49] = new ORN_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[25] = new STD_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[43] = new ANDNcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[77] = new TADDccTV_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[138] = new WRITEtbr_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[84] = new SUBX_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[57] = new XNOR_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[105] = new UDIV_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[2] = new LDSH_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[141] = new UNIMP(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[32] = new LDSTUBA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[97] = new UMULcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[48] = new ORcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[91] = new MULScc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[56] = new XORcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[80] = new SUB_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[136] = new WRITEwim_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[101] = new UMAC_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[88] = new TSUBcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[117] = new BRANCH(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[100] = new SMULcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[79] = new SUB_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[70] = new ADDcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[54] = new XOR_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[81] = new SUBcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[78] = new TADDccTV_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[108] = new SDIV_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[99] = new SMULcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[34] = new SWAP_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[83] = new SUBX_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[29] = new STDA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[102] = new UMAC_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[119] = new JUMP_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[96] = new SMUL_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[55] = new XORcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[51] = new ORNcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[14] = new LDUBA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[120] = new JUMP_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[72] = new ADDX_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[106] = new UDIV_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[59] = new XNORcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[140] = new STBAR(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[16] = new LDA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[27] = new STHA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[17] = new LDDA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[62] = new SLL_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[115] = new RESTORE_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[8] = new LD_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[124] = new TRAP_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[4] = new LDUB_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[122] = new RETT_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[111] = new SDIVcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[114] = new SAVE_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[46] = new OR_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[47] = new ORcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[118] = new CALL(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[134] = new WRITEpsr_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[39] = new ANDcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
            ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
        Processor::INSTRUCTIONS[144] = new InvalidInstr(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, \
            WINREGS, ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    }
    this->IRQ_irqInstr = new IRQ_IRQ_Instruction(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck, this->IRQ);
    quantKeeper.set_global_quantum( this->latency*100 );
    quantKeeper.reset();
    // Initialization of the standard registers
    // Initialization of the register banks
    this->GLOBAL.setSize(8);
    this->GLOBAL.setNewRegister(0, new Reg32_3_const_0());
    this->GLOBAL.setNewRegister(1, new Reg32_3());
    this->GLOBAL.setNewRegister(2, new Reg32_3());
    this->GLOBAL.setNewRegister(3, new Reg32_3());
    this->GLOBAL.setNewRegister(4, new Reg32_3());
    this->GLOBAL.setNewRegister(5, new Reg32_3());
    this->GLOBAL.setNewRegister(6, new Reg32_3());
    this->GLOBAL.setNewRegister(7, new Reg32_3());
    // Initialization of the aliases (plain and banks)
    this->REGS[0].updateAlias(this->GLOBAL[0]);
    this->REGS[1].updateAlias(this->GLOBAL[1]);
    this->REGS[2].updateAlias(this->GLOBAL[2]);
    this->REGS[3].updateAlias(this->GLOBAL[3]);
    this->REGS[4].updateAlias(this->GLOBAL[4]);
    this->REGS[5].updateAlias(this->GLOBAL[5]);
    this->REGS[6].updateAlias(this->GLOBAL[6]);
    this->REGS[7].updateAlias(this->GLOBAL[7]);
    this->REGS[8].updateAlias(this->WINREGS[0]);
    this->REGS[9].updateAlias(this->WINREGS[1]);
    this->REGS[10].updateAlias(this->WINREGS[2]);
    this->REGS[11].updateAlias(this->WINREGS[3]);
    this->REGS[12].updateAlias(this->WINREGS[4]);
    this->REGS[13].updateAlias(this->WINREGS[5]);
    this->REGS[14].updateAlias(this->WINREGS[6]);
    this->REGS[15].updateAlias(this->WINREGS[7]);
    this->REGS[16].updateAlias(this->WINREGS[8]);
    this->REGS[17].updateAlias(this->WINREGS[9]);
    this->REGS[18].updateAlias(this->WINREGS[10]);
    this->REGS[19].updateAlias(this->WINREGS[11]);
    this->REGS[20].updateAlias(this->WINREGS[12]);
    this->REGS[21].updateAlias(this->WINREGS[13]);
    this->REGS[22].updateAlias(this->WINREGS[14]);
    this->REGS[23].updateAlias(this->WINREGS[15]);
    this->REGS[24].updateAlias(this->WINREGS[16]);
    this->REGS[25].updateAlias(this->WINREGS[17]);
    this->REGS[26].updateAlias(this->WINREGS[18]);
    this->REGS[27].updateAlias(this->WINREGS[19]);
    this->REGS[28].updateAlias(this->WINREGS[20]);
    this->REGS[29].updateAlias(this->WINREGS[21]);
    this->REGS[30].updateAlias(this->WINREGS[22]);
    this->REGS[31].updateAlias(this->WINREGS[23]);
    this->PCR.updateAlias(this->ASR[17], 0);
    this->LR.updateAlias(this->REGS[31], 0);
    this->SP.updateAlias(this->REGS[14], 0);
    this->FP.updateAlias(this->REGS[30], 0);
    this->numInstructions = 0;
    this->ENTRY_POINT = 0;
    this->PROGRAM_LIMIT = 0;
    this->PROGRAM_START = 0;
    this->abiIf = new LEON3_ABIIf(this->PROGRAM_LIMIT, this->dataMem, this->PSR, this->WIM, \
        this->TBR, this->Y, this->PC, this->NPC, this->GLOBAL, this->WINREGS, this->ASR, \
        this->FP, this->LR, this->SP, this->PCR, this->REGS, this->instrExecuting, this->instrEndEvent);
    SC_THREAD(mainLoop);
    end_module();
}

leon3_funclt_trap::Processor::~Processor(){
    Processor::numInstances--;
    if(Processor::numInstances == 0){
        for(int i = 0; i < 145; i++){
            delete Processor::INSTRUCTIONS[i];
        }
        delete [] Processor::INSTRUCTIONS;
        Processor::INSTRUCTIONS = NULL;
    }
    template_map< unsigned int, CacheElem >::const_iterator cacheIter, cacheEnd;
    for(cacheIter = this->instrCache.begin(), cacheEnd = this->instrCache.end(); cacheIter \
        != cacheEnd; cacheIter++){
        delete cacheIter->second.instr;
    }
    delete this->abiIf;
    delete this->IRQ_irqInstr;
}

