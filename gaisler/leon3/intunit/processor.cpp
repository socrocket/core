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



#include "gaisler/leon3/intunit/processor.hpp"
#include "core/common/trapgen/utils/customExceptions.hpp"
#include "gaisler/leon3/intunit/instructions.hpp"
#include "gaisler/leon3/intunit/decoder.hpp"
#include "gaisler/leon3/intunit/interface.hpp"
#include "core/common/trapgen/ToolsIf.hpp"
#include <tlm_utils/tlm_quantumkeeper.h>
#include "gaisler/leon3/intunit/registers.hpp"
#include "gaisler/leon3/intunit/alias.hpp"
#include "gaisler/leon3/intunit/memory.hpp"
//#include "gaisler/leon3/intunit/externalPorts.hpp"
#include <iostream>
#include <fstream>
#include <boost/circular_buffer.hpp>
#include "core/common/trapgen/instructionBase.hpp"
#include "gaisler/leon3/intunit/irqPorts.hpp"
#include "gaisler/leon3/intunit/externalPins.hpp"
#include <string>
#include "core/common/systemc.h"
#include "core/common/verbose.h"

using namespace leon3_funclt_trap;
using namespace trap;
void leon3_funclt_trap::Processor_leon3_funclt::mainLoop() {
    wait(SC_ZERO_TIME); // wait for SystemC infrastructure.
                        // if you don't wait the register callbacks will crash

    bool startMet = false;
    vmap< unsigned int, CacheElem >::iterator instrCacheEnd = this->instrCache.end();

    unsigned int firstPC = this->PC + 0;
    unsigned int firstbitString = this->instrMem.read_instr(firstPC, 0x8 | (PSR[key_S]? 1 : 0), 0);
    int firstinstrId = this->decoder.decode(firstbitString);
    Instruction *firstinstr = this->INSTRUCTIONS[firstinstrId];
    raisedException = 0;
    while(true) {
        unsigned int numCycles = 0;
        this->instrExecuting = true;

        if(irqAck.stopped) {
          while(irqAck.stopped) {
            //if(sc_time_stamp()>sc_time(0, SC_NS)) {
              //wait(irqAck.start);
              this->toolManager.newIssue(firstPC, firstinstr);
              wait(100, SC_NS);
            //}
          }
          v::info << name() << "Starting ... " << v::endl;
          resetOp();
        }

        // Log instruction count for power monitoring
        if (m_pow_mon) {
            dyn_instr++;
        }

        if((IRQ != 0xFFFFFFFF) && (PSR[key_ET] && (IRQ == 15 || IRQ > PSR[key_PIL]))){
            this->IRQ_irqInstr->setInterruptValue(IRQ);
            try{
                numCycles = this->IRQ_irqInstr->behavior();
                //this->IRQ_irqInstr->behavior(); // Replacement for ^^
            }
            catch(annull_exception &etc){
              numCycles = 0;
            }

        } else {
            try {
                curPC = this->PC + 0;
                if(!startMet && curPC == this->profStartAddr){
                    this->profTimeStart = sc_time_stamp();
                } else if(startMet && curPC == this->profEndAddr){
                    this->profTimeEnd = sc_time_stamp();
                }

                int instrId = 0;
                unsigned int bitString = this->instrMem.read_instr(curPC, 0x8 | (PSR[key_S]? 1 : 0),0);
                if(raisedException) {
                    unsigned int exception = raisedException;
                    raisedException = 0;
                    curInstrPtr->RaiseException(raisedExceptionPC, raisedExceptionNPC, exception);
                }
                vmap< unsigned int, CacheElem >::iterator cachedInstr = this->instrCache.find(bitString);
                unsigned int *curCount = NULL;
                if(cachedInstr != instrCacheEnd) {
                    curInstrPtr = cachedInstr->second.instr;
                    // I can call the instruction, I have found it
                    if(curInstrPtr == NULL) {
                        curCount = &cachedInstr->second.count;
                        instrId = this->decoder.decode(bitString);
                        curInstrPtr = this->INSTRUCTIONS[instrId];
                        curInstrPtr->setParams(bitString);
                    }
                } else {
                    // The current instruction is not present in the cache:
                    // I have to perform the normal decoding phase ...
                    instrId = this->decoder.decode(bitString);
                    curInstrPtr = this->INSTRUCTIONS[instrId];
                    curInstrPtr->setParams(bitString);
                }
                if (this->historyEnabled) {
                    srInfo()
                        ("Address",curPC)
                        ("Name",curInstrPtr->getInstructionName())
                        ("Mnemonic",curInstrPtr->getMnemonic())
                        ("Instruction History");
                }
                try {
                    #ifndef DISABLE_TOOLS
                    if (!(this->toolManager.newIssue(curPC, curInstrPtr))) {
                        #endif
                        numCycles = curInstrPtr->behavior();
                        //curInstrPtr->behavior(); // Replacement for ^^
                        #ifndef DISABLE_TOOLS
                    }
                    #endif
                } catch (annull_exception &etc) {
                    numCycles = 0;
                }
                if (cachedInstr != instrCacheEnd) {
                    if (curCount && *curCount < 256) {
//                        *curCount++; // ????
                    } else if (curCount) {
                        // ... and then add the instruction to the cache
                        cachedInstr->second.instr = curInstrPtr;
                        this->INSTRUCTIONS[instrId] = curInstrPtr->replicate();
                    }
                } else {
                    this->instrCache.insert(std::pair< unsigned int, CacheElem >(bitString, CacheElem()));
                    instrCacheEnd = this->instrCache.end();
                }
            } catch (annull_exception &etc) {
                numCycles = 0;
            }
        }
        this->quantKeeper.inc((numCycles + 1)*this->latency);
        if (this->quantKeeper.need_sync()){
            this->quantKeeper.sync();
        }
        this->instrExecuting = false;
        this->instrEndEvent.notify();
        this->numInstructions++;
    }
}

void leon3_funclt_trap::Processor_leon3_funclt::triggerException(unsigned int exception) {
    raisedException = exception;
    raisedExceptionPC = this->PC;
    raisedExceptionNPC = this->NPC;
}

void leon3_funclt_trap::Processor_leon3_funclt::beginOp(){
    #ifdef TSIM_COMPATIBILITY
    PSR.immediateWrite(0xf30000E0L);
    WIM.immediateWrite(2);
    #endif
}

void leon3_funclt_trap::Processor_leon3_funclt::resetOp(){
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
    ASR[17].immediateWrite(MPROC_ID + 0x307);
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
    NPC.immediateWrite(ENTRY_POINT + 0x4);
    this->IRQ = -1;

    //user-defined initialization
    this->beginOp();
    this->resetCalled = true;
}

// Calculate power/energy values from normalized input data
void leon3_funclt_trap::Processor_leon3_funclt::power_model() {

  // Static power calculation (pW)
  sta_power = sta_power_norm;

  // Cell internal power (uW)
  int_power = int_power_norm * 1/(latency.to_seconds());

  // Average energy per instruction
  dyn_instr_energy = dyn_instr_energy_norm;

}

// Static power callback
gs::cnf::callback_return_type leon3_funclt_trap::Processor_leon3_funclt::sta_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // Static power of AHBMem is constant !!
  return GC_RETURN_OK;
}

// Internal power callback
gs::cnf::callback_return_type leon3_funclt_trap::Processor_leon3_funclt::int_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  // Nothing to do !!
  // AHBMem internal power is constant !!
  return GC_RETURN_OK;
}

// Switching power callback
gs::cnf::callback_return_type leon3_funclt_trap::Processor_leon3_funclt::swi_power_cb(gs::gs_param_base& changed_param, gs::cnf::callback_type reason) {

  swi_power = (dyn_instr_energy * dyn_instr) / (sc_time_stamp() - power_frame_starting_time).to_seconds();
  return GC_RETURN_OK;
}

// Automatically called at the beginning of the simulation
void leon3_funclt_trap::Processor_leon3_funclt::start_of_simulation() {

  // Initialize power model
  if (m_pow_mon) {

    power_model();

  }
}

void leon3_funclt_trap::Processor_leon3_funclt::end_of_simulation() {
    v::report << name() << " ********************************************" << v::endl;
    v::report << name() << " * LEON3 Statistic:" << v::endl;
    v::report << name() << " * ------------------" << v::endl;
    v::report << name() << " * Total number of processed instructions: " << numInstructions << v::endl;
    v::report << name() << " ******************************************** " << v::endl;
}

void leon3_funclt_trap::Processor_leon3_funclt::end_of_elaboration(){
    if(!this->resetCalled){
        this->resetOp();
    }
    this->resetCalled = false;
}

Instruction * leon3_funclt_trap::Processor_leon3_funclt::decode( unsigned int bitString \
    ){
    int instrId = this->decoder.decode(bitString);
    if(instrId >= 0){
        Instruction * instr = this->INSTRUCTIONS[instrId];
        instr->setParams(bitString);
        return instr;
    }
    return NULL;
}

LEON3_ABIIf & leon3_funclt_trap::Processor_leon3_funclt::getInterface(){
    return *this->abiIf;
}

int leon3_funclt_trap::Processor_leon3_funclt::numInstances = 0;
void leon3_funclt_trap::Processor_leon3_funclt::setProfilingRange( unsigned int startAddr, \
    unsigned int endAddr ){
    this->profStartAddr = startAddr;
    this->profEndAddr = endAddr;
}

leon3_funclt_trap::Processor_leon3_funclt::Processor_leon3_funclt(
    sc_module_name name,
    MemoryInterface *memory,
    sc_time latency,
    bool pow_mon ) :
      sc_module(name),
      PSR("PSR"),
      WIM("WIM"),
      TBR("TBR"),
      Y("Y"),
      PC("PC"),
      NPC("NPC"),
#if !defined(MTI_SYSTEMC) and !defined(NC_SYSTEMC)
      WINREGS{
        Reg32_3("WINREG0"), Reg32_3("WINREG1"), Reg32_3("WINREG2"),
        Reg32_3("WINREG3"), Reg32_3("WINREG4"), Reg32_3("WINREG5"),
        Reg32_3("WINREG6"), Reg32_3("WINREG7"), Reg32_3("WINREG8"),
        Reg32_3("WINREG9"), Reg32_3("WINREG10"), Reg32_3("WINREG11"),
        Reg32_3("WINREG12"), Reg32_3("WINREG13"), Reg32_3("WINREG14"),
        Reg32_3("WINREG15"), Reg32_3("WINREG16"), Reg32_3("WINREG17"),
        Reg32_3("WINREG18"), Reg32_3("WINREG19"), Reg32_3("WINREG20"),
        Reg32_3("WINREG21"), Reg32_3("WINREG22"), Reg32_3("WINREG23"),
        Reg32_3("WINREG24"), Reg32_3("WINREG25"), Reg32_3("WINREG26"),
        Reg32_3("WINREG27"), Reg32_3("WINREG28"), Reg32_3("WINREG29"),
        Reg32_3("WINREG30"), Reg32_3("WINREG31"), Reg32_3("WINREG32"),
        Reg32_3("WINREG33"), Reg32_3("WINREG34"), Reg32_3("WINREG35"),
        Reg32_3("WINREG36"), Reg32_3("WINREG37"), Reg32_3("WINREG38"),
        Reg32_3("WINREG39"), Reg32_3("WINREG40"), Reg32_3("WINREG41"),
        Reg32_3("WINREG42"), Reg32_3("WINREG43"), Reg32_3("WINREG44"),
        Reg32_3("WINREG45"), Reg32_3("WINREG46"), Reg32_3("WINREG47"),
        Reg32_3("WINREG48"), Reg32_3("WINREG49"), Reg32_3("WINREG50"),
        Reg32_3("WINREG51"), Reg32_3("WINREG52"), Reg32_3("WINREG53"),
        Reg32_3("WINREG54"), Reg32_3("WINREG55"), Reg32_3("WINREG56"),
        Reg32_3("WINREG57"), Reg32_3("WINREG58"), Reg32_3("WINREG59"),
        Reg32_3("WINREG60"), Reg32_3("WINREG61"), Reg32_3("WINREG62"),
        Reg32_3("WINREG63"), Reg32_3("WINREG64"), Reg32_3("WINREG65"),
        Reg32_3("WINREG66"), Reg32_3("WINREG67"), Reg32_3("WINREG68"),
        Reg32_3("WINREG69"), Reg32_3("WINREG70"), Reg32_3("WINREG71"),
        Reg32_3("WINREG72"), Reg32_3("WINREG73"), Reg32_3("WINREG74"),
        Reg32_3("WINREG75"), Reg32_3("WINREG76"), Reg32_3("WINREG77"),
        Reg32_3("WINREG78"), Reg32_3("WINREG79"), Reg32_3("WINREG80"),
        Reg32_3("WINREG81"), Reg32_3("WINREG82"), Reg32_3("WINREG83"),
        Reg32_3("WINREG84"), Reg32_3("WINREG85"), Reg32_3("WINREG86"),
        Reg32_3("WINREG87"), Reg32_3("WINREG88"), Reg32_3("WINREG89"),
        Reg32_3("WINREG90"), Reg32_3("WINREG91"), Reg32_3("WINREG92"),
        Reg32_3("WINREG93"), Reg32_3("WINREG94"), Reg32_3("WINREG95"),
        Reg32_3("WINREG96"), Reg32_3("WINREG97"), Reg32_3("WINREG98"),
        Reg32_3("WINREG99"), Reg32_3("WINREG100"), Reg32_3("WINREG101"),
        Reg32_3("WINREG102"), Reg32_3("WINREG103"), Reg32_3("WINREG104"),
        Reg32_3("WINREG105"), Reg32_3("WINREG106"), Reg32_3("WINREG107"),
        Reg32_3("WINREG108"), Reg32_3("WINREG109"), Reg32_3("WINREG110"),
        Reg32_3("WINREG111"), Reg32_3("WINREG112"), Reg32_3("WINREG113"),
        Reg32_3("WINREG114"), Reg32_3("WINREG115"), Reg32_3("WINREG116"),
        Reg32_3("WINREG117"), Reg32_3("WINREG118"), Reg32_3("WINREG119"),
        Reg32_3("WINREG120"), Reg32_3("WINREG121"), Reg32_3("WINREG122"),
        Reg32_3("WINREG123"), Reg32_3("WINREG124"), Reg32_3("WINREG125"),
        Reg32_3("WINREG126"), Reg32_3("WINREG127")},
      ASR{
        Reg32_3("ASR0"), Reg32_3("ASR1"), Reg32_3("ASR2"),
        Reg32_3("ASR3"), Reg32_3("ASR4"), Reg32_3("ASR5"), Reg32_3("ASR6"),
        Reg32_3("ASR7"), Reg32_3("ASR8"), Reg32_3("ASR9"), Reg32_3("ASR10"),
        Reg32_3("ASR11"), Reg32_3("ASR12"), Reg32_3("ASR13"),
        Reg32_3("ASR14"), Reg32_3("ASR15"), Reg32_3("ASR16"),
        Reg32_3("ASR17"), Reg32_3("ASR18"), Reg32_3("ASR19"),
        Reg32_3("ASR20"), Reg32_3("ASR21"), Reg32_3("ASR22"),
        Reg32_3("ASR23"), Reg32_3("ASR24"), Reg32_3("ASR25"),
        Reg32_3("ASR26"), Reg32_3("ASR27"), Reg32_3("ASR28"),
        Reg32_3("ASR29"), Reg32_3("ASR30"), Reg32_3("ASR31")},
#endif
      mem(memory),
      instrMem(*mem),
      dataMem(*mem),
      latency(latency),
      IRQ_port("IRQ_port", IRQ),
      irqAck("irqAck"),
      historyEnabled("historyEnabled", false),
      m_pow_mon(pow_mon),
      sta_power_norm("power.leon3.sta_power_norm", 5.27e+8, true), // norm. static power
      int_power_norm("power.leon3.int_power_norm", 5.497e-6, true), // norm. dynamic power
      dyn_instr_energy_norm("power.leon3.dyn_instr_energy_norm", 3.95e-5, true), // norm. average energy per instruction
      power("power"),
      sta_power("sta_power", 0.0, power), // Static power output
      int_power("int_power", 0.0, power), // Internal power of module
      swi_power("swi_power", 0.0, power), // Switching power of module
      power_frame_starting_time("power_frame_starting_time", SC_ZERO_TIME, power),
      dyn_instr_energy("dyn_instr_energy", 0.0, power), // average instruction energy
      dyn_instr("dyn_instr", 0ull, power), // number of instructions
      numInstructions("instruction_count", 0ull)
{
    this->resetCalled = false;
    Processor_leon3_funclt::numInstances++;
    // Initialization of the array holding the initial instance of the instructions
    this->INSTRUCTIONS = new Instruction *[145];
    this->INSTRUCTIONS[126] = new READasr(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[130] = new WRITEY_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[58] = new XNOR_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[44] = new ANDNcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[0] = new LDSB_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[135] = new WRITEpsr_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[125] = new READy(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, ASR, \
        FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[60] = new XNORcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[127] = new READpsr(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[41] = new ANDN_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[40] = new ANDcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[87] = new TSUBcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[12] = new LDSBA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[6] = new LDUH_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[28] = new STA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[50] = new ORN_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[13] = new LDSHA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[26] = new STBA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[22] = new ST_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, ASR, \
        FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[129] = new READtbr(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[109] = new UDIVcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[35] = new SWAPA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[73] = new ADDXcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[18] = new STB_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[85] = new SUBXcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[21] = new STH_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[63] = new SRL_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[133] = new WRITEasr_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[98] = new UMULcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[31] = new LDSTUB_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[53] = new XOR_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[104] = new SMAC_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[132] = new WRITEasr_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[9] = new LD_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, ASR, \
        FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[23] = new ST_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, ASR, \
        FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[82] = new SUBcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[11] = new LDD_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[69] = new ADDcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[7] = new LDUH_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[64] = new SRL_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[113] = new SAVE_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[92] = new MULScc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[45] = new OR_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, ASR, \
        FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[24] = new STD_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[86] = new SUBXcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[71] = new ADDX_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[33] = new SWAP_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[94] = new UMUL_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[131] = new WRITEY_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[38] = new AND_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[143] = new FLUSH_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[66] = new SRA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[20] = new STH_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[137] = new WRITEwim_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[10] = new LDD_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[61] = new SLL_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[15] = new LDUHA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[76] = new TADDcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[75] = new TADDcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[107] = new SDIV_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[89] = new TSUBccTV_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[142] = new FLUSH_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[52] = new ORNcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[121] = new RETT_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[112] = new SDIVcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[68] = new ADD_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[123] = new TRAP_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[139] = new WRITEtbr_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[5] = new LDUB_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[116] = new RESTORE_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[74] = new ADDXcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[19] = new STB_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[37] = new AND_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[95] = new SMUL_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[67] = new ADD_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[93] = new UMUL_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[128] = new READwim(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[30] = new LDSTUB_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[103] = new SMAC_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[1] = new LDSB_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[42] = new ANDN_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[90] = new TSUBccTV_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[36] = new SETHI(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, ASR, \
        FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[65] = new SRA_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[3] = new LDSH_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[110] = new UDIVcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[49] = new ORN_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[25] = new STD_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[43] = new ANDNcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[77] = new TADDccTV_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[138] = new WRITEtbr_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[84] = new SUBX_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[57] = new XNOR_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[105] = new UDIV_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[2] = new LDSH_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[141] = new UNIMP(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, ASR, \
        FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[32] = new LDSTUBA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[97] = new UMULcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[48] = new ORcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[91] = new MULScc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[56] = new XORcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[80] = new SUB_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[136] = new WRITEwim_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[101] = new UMAC_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[88] = new TSUBcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[117] = new BRANCH(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[100] = new SMULcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[79] = new SUB_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[70] = new ADDcc_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[54] = new XOR_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[81] = new SUBcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[78] = new TADDccTV_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[108] = new SDIV_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[99] = new SMULcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[34] = new SWAP_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[83] = new SUBX_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[29] = new STDA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[102] = new UMAC_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[119] = new JUMP_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[96] = new SMUL_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[55] = new XORcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[51] = new ORNcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[14] = new LDUBA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[120] = new JUMP_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[72] = new ADDX_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[106] = new UDIV_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[59] = new XNORcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[140] = new STBAR(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, ASR, \
        FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[16] = new LDA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[27] = new STHA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[17] = new LDDA_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[62] = new SLL_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[115] = new RESTORE_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[8] = new LD_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, ASR, \
        FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[124] = new TRAP_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[4] = new LDUB_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[122] = new RETT_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[111] = new SDIVcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[114] = new SAVE_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[46] = new OR_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, ASR, \
        FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[47] = new ORcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[118] = new CALL(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, ASR, \
        FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[134] = new WRITEpsr_reg(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[39] = new ANDcc_imm(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->INSTRUCTIONS[144] = new InvalidInstr(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck);
    this->IRQ_irqInstr = new IRQ_IRQ_Instruction(PSR, WIM, TBR, Y, PC, NPC, GLOBAL, WINREGS, \
        ASR, FP, LR, SP, PCR, REGS, instrMem, dataMem, irqAck, this->IRQ);
    this->quantKeeper.set_global_quantum( this->latency*100);
    this->quantKeeper.reset();
    // Initialization of the standard registers
    // Initialization of the register banks
    this->GLOBAL.setSize(8);
    this->GLOBAL.setNewRegister(0, new Reg32_3_const_0("GLOBAL0"));
    this->GLOBAL.setNewRegister(1, new Reg32_3("GLOBAL1"));
    this->GLOBAL.setNewRegister(2, new Reg32_3("GLOBAL2"));
    this->GLOBAL.setNewRegister(3, new Reg32_3("GLOBAL3"));
    this->GLOBAL.setNewRegister(4, new Reg32_3("GLOBAL4"));
    this->GLOBAL.setNewRegister(5, new Reg32_3("GLOBAL5"));
    this->GLOBAL.setNewRegister(6, new Reg32_3("GLOBAL6"));
    this->GLOBAL.setNewRegister(7, new Reg32_3("GLOBAL7"));
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
    this->FP.updateAlias(this->REGS[30], 0);
    this->LR.updateAlias(this->REGS[31], 0);
    this->PCR.updateAlias(this->ASR[17], 0);
    this->SP.updateAlias(this->REGS[14], 0);
    this->profTimeStart = SC_ZERO_TIME;
    this->profTimeEnd = SC_ZERO_TIME;
    this->profStartAddr = (unsigned int)-1;
    this->profEndAddr = (unsigned int)-1;
    this->undumpedHistElems = 0;
    this->numInstructions = 0;
    this->ENTRY_POINT = 0;
    this->MPROC_ID = 0;
    this->PROGRAM_LIMIT = 0;
    this->PROGRAM_START = 0;
    this->abiIf = new LEON3_ABIIf(this->PROGRAM_LIMIT, this->dataMem, this->PSR, this->WIM, \
        this->TBR, this->Y, this->PC, this->NPC, this->GLOBAL, this->WINREGS, this->ASR, \
        this->FP, this->LR, this->SP, this->PCR, this->REGS, this->instrExecuting, this->instrEndEvent );
    SC_THREAD(mainLoop);

    // Register power callback functions
    if (m_pow_mon) {

      GC_REGISTER_TYPED_PARAM_CALLBACK(&sta_power, gs::cnf::pre_read, Processor_leon3_funclt, sta_power_cb);
      GC_REGISTER_TYPED_PARAM_CALLBACK(&int_power, gs::cnf::pre_read, Processor_leon3_funclt, int_power_cb);
      GC_REGISTER_TYPED_PARAM_CALLBACK(&swi_power, gs::cnf::pre_read, Processor_leon3_funclt, swi_power_cb);

    }

    end_module();
}

leon3_funclt_trap::Processor_leon3_funclt::~Processor_leon3_funclt(){

    GC_UNREGISTER_CALLBACKS();

    Processor_leon3_funclt::numInstances--;
    for(int i = 0; i < 145; i++){
        delete this->INSTRUCTIONS[i];
    }
    delete [] this->INSTRUCTIONS;
    vmap< unsigned int, CacheElem >::const_iterator cacheIter, cacheEnd;
    for(cacheIter = this->instrCache.begin(), cacheEnd = this->instrCache.end(); cacheIter \
        != cacheEnd; cacheIter++){
        delete cacheIter->second.instr;
    }
    delete this->abiIf;
    delete this->IRQ_irqInstr;
}

