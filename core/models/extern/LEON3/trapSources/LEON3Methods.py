# -*- coding: iso-8859-1 -*-

import trap
import cxx_writer

#-------------------------------------------------------
# Miscellaneous operations which can be used and
# accessed by any instruction
#-------------------------------------------------------
# *******
# Here we define some helper methods, which are not directly part of the
# instructions, but which can be called by the instruction body
# *******

from LEONDefs import *

def updateAliasCode_abi():
    return """
    //ABI model: we simply immediately update the alias
    for(int i = 8; i < 32; i++){
        REGS[i].updateAlias(WINREGS[(newCwp*16 + i - 8) % (""" + str(16*numRegWindows) + """)]);
    }
    """

def updateAliasCode_decode():
    import math
    if math.modf(math.log(16*numRegWindows, 2))[0] == 0:
        modCode = '& ' + hex(16*numRegWindows - 1)
    else:
        modCode = '% ' + str(16*numRegWindows)

    code = """#ifndef ACC_MODEL
    //Functional model: we simply immediately update the alias
    for(int i = 8; i < 32; i++){
        REGS[i].updateAlias(WINREGS[(newCwp*16 + i - 8) """ + modCode + """]);
    }
    #else
    //Cycle accurate model: we have to update the alias using the pipeline register
    //We update the aliases for this stage and for all the preceding ones (we are in the
    //decode stage and we need to update fetch, and decode)
    for(int i = 8; i < 32; i++){
        REGS_fetch[i].updateAlias(WINREGS_pipe[(newCwp*16 + i - 8) """ + modCode + """]);
        REGS_decode[i].updateAlias(WINREGS_pipe[(newCwp*16 + i - 8) """ + modCode + """]);
    }
    #endif
    """
    return code

def updateAliasCode_exception():
    import math
    if math.modf(math.log(16*numRegWindows, 2))[0] == 0:
        modCode = '& ' + hex(16*numRegWindows - 1)
    else:
        modCode = '% ' + str(16*numRegWindows)

    code = """#ifndef ACC_MODEL
    //Functional model: we simply immediately update the alias
    for(int i = 8; i < 32; i++){
        REGS[i].updateAlias(WINREGS[(newCwp*16 + i - 8) """ + modCode + """]);
    }
    #else
    //Cycle accurate model: we have to update the alias using the pipeline register
    //We update the aliases for this stage and for all the preceding ones (we are in the
    //execute stage and we need to update fetch, decode, and register read and execute)
    for(int i = 8; i < 32; i++){
        REGS_fetch[i].updateAlias(WINREGS_pipe[(newCwp*16 + i - 8) """ + modCode + """]);
        REGS_decode[i].updateAlias(WINREGS_pipe[(newCwp*16 + i - 8) """ + modCode + """]);
        REGS_regs[i].updateAlias(WINREGS_pipe[(newCwp*16 + i - 8) """ + modCode + """]);
        REGS_execute[i].updateAlias(WINREGS_pipe[(newCwp*16 + i - 8) """ + modCode + """]);
        REGS_memory[i].updateAlias(WINREGS_pipe[(newCwp*16 + i - 8) """ + modCode + """]);
        REGS_exception[i].updateAlias(WINREGS_pipe[(newCwp*16 + i - 8) """ + modCode + """]);
    }
    #endif
    """
    return code

# Methods used (just by the cycle accurate processor) to check that a register window is valid
# when a decrement or an increment are performed
checkIncrementWin_code = """
unsigned int newCwp = ((unsigned int)(PSR[key_CWP] + 1)) % NUM_REG_WIN;
if(((0x01 << (newCwp)) & WIM) != 0){
    return false;
}
else{
    return true;
}
"""
opCode = cxx_writer.writer_code.Code(checkIncrementWin_code)
checkIncrementWin_method = trap.HelperMethod('checkIncrementWin', opCode, 'decode', exception = False, const = True)
checkIncrementWin_method.setSignature(cxx_writer.writer_code.boolType)
checkDecrementWin_code = """
unsigned int newCwp = ((unsigned int)(PSR[key_CWP] - 1)) % NUM_REG_WIN;
if(((0x01 << (newCwp)) & WIM) != 0){
    return false;
}
else{
    return true;
}
"""
opCode = cxx_writer.writer_code.Code(checkDecrementWin_code)
checkDecrementWin_method = trap.HelperMethod('checkDecrementWin', opCode, 'decode', exception = False, const = True)
checkDecrementWin_method.setSignature(cxx_writer.writer_code.boolType)

# Method used to move to the next register window; this simply consists in
# the check that there is an empty valid window and in the update of
# the window aliases
IncrementRegWindow_code = """
newCwp = ((unsigned int)(PSR[key_CWP] + 1)) % NUM_REG_WIN;
if(((0x01 << (newCwp)) & WIM) != 0){
    return false;
}
PSR = (PSR & 0xFFFFFFE0) | newCwp;
"""
IncrementRegWindow_code += updateAliasCode_decode()
IncrementRegWindow_code += 'return true;'
opCode = cxx_writer.writer_code.Code(IncrementRegWindow_code)
IncrementRegWindow_method = trap.HelperMethod('IncrementRegWindow', opCode, 'decode', exception = False)
IncrementRegWindow_method.setSignature(cxx_writer.writer_code.boolType)
IncrementRegWindow_method.addVariable(('newCwp', 'BIT<32>'))
# Method used to move to the previous register window; this simply consists in
# the check that there is an empty valid window and in the update of
# the window aliases
DecrementRegWindow_code = """
newCwp = ((unsigned int)(PSR[key_CWP] - 1)) % NUM_REG_WIN;
if(((0x01 << (newCwp)) & WIM) != 0){
    return false;
}
PSR = (PSR & 0xFFFFFFE0) | newCwp;
"""
DecrementRegWindow_code += updateAliasCode_decode()
DecrementRegWindow_code += 'return true;'
opCode = cxx_writer.writer_code.Code(DecrementRegWindow_code)
DecrementRegWindow_method = trap.HelperMethod('DecrementRegWindow', opCode, 'decode', exception = False)
DecrementRegWindow_method.setSignature(cxx_writer.writer_code.boolType)
DecrementRegWindow_method.addVariable(('newCwp', 'BIT<32>'))

# Sign extends the input bitstring
opCode = cxx_writer.writer_code.Code("""
if((bitSeq & (1 << (bitSeq_length - 1))) != 0)
    bitSeq |= (((unsigned int)0xFFFFFFFF) << bitSeq_length);
return bitSeq;
""")
SignExtend_method = trap.HelperMethod('SignExtend', opCode, 'execute', exception = False, const = True)
SignExtend_method.setSignature(cxx_writer.writer_code.intType, [('bitSeq', 'BIT<32>'), cxx_writer.writer_code.Parameter('bitSeq_length', cxx_writer.writer_code.uintType)])

# Normal PC increment, used when not in a branch instruction; in a branch instruction
# I will directly modify both PC and nPC in case we are in a the cycle accurate model,
# while just nPC in case we are in the functional one; if the branch has the annulling bit
# set, then also in the functional model both the PC and nPC will be modified
raiseExcCode = """
if(PSR[key_ET] == 0){
    if(exceptionId < IRQ_LEV_15){
        // I print a core dump and then I signal an error: an exception happened while
        // exceptions were disabled in the processor core
        THROW_EXCEPTION("Exception " << exceptionId << " happened while the PSR[ET] = 0; PC = " << std::hex << std::showbase << PC << std::endl << "Instruction " << getMnemonic());
    }
}
else{
    unsigned int curPSR = PSR;
    curPSR = (curPSR & 0xffffffbf) | (PSR[key_S] << 6);
    curPSR = (curPSR & 0xffffff7f) | 0x00000080;
    curPSR &= 0xffffffdf;
    unsigned int newCwp = ((unsigned int)(PSR[key_CWP] - 1)) % NUM_REG_WIN;
"""
raiseExcCode += updateAliasCode_exception()
raiseExcCode +=  """
    curPSR = (curPSR & 0xffffffe0) + newCwp;
    PSR = curPSR;
    #ifdef ACC_MODEL
    PSR_execute = curPSR;
    #endif
    REGS[17] = pcounter;
    REGS[18] = npcounter;
    switch(exceptionId){
        case RESET:{
        }break;
        case DATA_STORE_ERROR:{
            TBR[key_TT] = 0x2b;
        }break;
        case INSTR_ACCESS_MMU_MISS:{
            TBR[key_TT] = 0x3c;
        }break;
        case INSTR_ACCESS_ERROR:{
            TBR[key_TT] = 0x21;
        }break;
        case R_REGISTER_ACCESS_ERROR:{
            TBR[key_TT] = 0x20;
        }break;
        case INSTR_ACCESS_EXC:{
            TBR[key_TT] = 0x01;
        }break;
        case PRIVILEDGE_INSTR:{
            TBR[key_TT] = 0x03;
        }break;
        case ILLEGAL_INSTR:{
            TBR[key_TT] = 0x02;
        }break;
        case FP_DISABLED:{
            TBR[key_TT] = 0x04;
        }break;
        case CP_DISABLED:{
            TBR[key_TT] = 0x24;
        }break;
        case UNIMPL_FLUSH:{
            TBR[key_TT] = 0x25;
        }break;
        case WATCHPOINT_DETECTED:{
            TBR[key_TT] = 0x0b;
        }break;
        case WINDOW_OVERFLOW:{
            TBR[key_TT] = 0x05;
        }break;
        case WINDOW_UNDERFLOW:{
            TBR[key_TT] = 0x06;
        }break;
        case MEM_ADDR_NOT_ALIGNED:{
            TBR[key_TT] = 0x07;
        }break;
        case FP_EXCEPTION:{
            TBR[key_TT] = 0x08;
        }break;
        case CP_EXCEPTION:{
            TBR[key_TT] = 0x28;
        }break;
        case DATA_ACCESS_ERROR:{
            TBR[key_TT] = 0x29;
        }break;
        case DATA_ACCESS_MMU_MISS:{
            TBR[key_TT] = 0x2c;
        }break;
        case DATA_ACCESS_EXC:{
            TBR[key_TT] = 0x09;
        }break;
        case TAG_OVERFLOW:{
            TBR[key_TT] = 0x0a;
        }break;
        case DIV_ZERO:{
            TBR[key_TT] = 0x2a;
        }break;
        case TRAP_INSTRUCTION:{
            TBR[key_TT] = 0x80 + customTrapOffset;
        }break;
        case IRQ_LEV_15:{
            TBR[key_TT] = 0x1f;
        }break;
        case IRQ_LEV_14:{
            TBR[key_TT] = 0x1e;
        }break;
        case IRQ_LEV_13:{
            TBR[key_TT] = 0x1d;
        }break;
        case IRQ_LEV_12:{
            TBR[key_TT] = 0x1c;
        }break;
        case IRQ_LEV_11:{
            TBR[key_TT] = 0x1b;
        }break;
        case IRQ_LEV_10:{
            TBR[key_TT] = 0x1a;
        }break;
        case IRQ_LEV_9:{
            TBR[key_TT] = 0x19;
        }break;
        case IRQ_LEV_8:{
            TBR[key_TT] = 0x18;
        }break;
        case IRQ_LEV_7:{
            TBR[key_TT] = 0x17;
        }break;
        case IRQ_LEV_6:{
            TBR[key_TT] = 0x16;
        }break;
        case IRQ_LEV_5:{
            TBR[key_TT] = 0x15;
        }break;
        case IRQ_LEV_4:{
            TBR[key_TT] = 0x14;
        }break;
        case IRQ_LEV_3:{
            TBR[key_TT] = 0x13;
        }break;
        case IRQ_LEV_2:{
            TBR[key_TT] = 0x12;
        }break;
        case IRQ_LEV_1:{
            TBR[key_TT] = 0x11;
        }break;
        case IMPL_DEP_EXC:{
            TBR[key_TT] = 0x60 + customTrapOffset;
        }break;
        default:{
        }break;
    }
    if(exceptionId == RESET){
        // I have to jump to address 0 and restart execution
        PC = 0;
        NPC = 4;
    }
    else{
        // I have to jump to the address contained in the TBR register
        PC = TBR;
        NPC = TBR + 4;
    }
    if(exceptionId > TRAP_INSTRUCTION && exceptionId < IMPL_DEP_EXC){
        // finally I acknowledge the interrupt on the external pin port
        irqAck.send_pin_req(IMPL_DEP_EXC - exceptionId, 0);
    }
    flush();
    annull();
}
"""
RaiseException_method = trap.HelperMethod('RaiseException', cxx_writer.writer_code.Code(raiseExcCode), 'exception')
RaiseException_methodParams = []
RaiseException_methodParams.append(cxx_writer.writer_code.Parameter('pcounter', cxx_writer.writer_code.uintType))
RaiseException_methodParams.append(cxx_writer.writer_code.Parameter('npcounter', cxx_writer.writer_code.uintType))
RaiseException_methodParams.append(cxx_writer.writer_code.Parameter('exceptionId', cxx_writer.writer_code.uintType))
RaiseException_methodParams.append(cxx_writer.writer_code.Parameter('customTrapOffset', cxx_writer.writer_code.uintType, initValue = '0'))
RaiseException_method.setSignature(cxx_writer.writer_code.voidType, RaiseException_methodParams)

# Code used increment the program counter, moving it to the next instruction in
# the instruction stream
opCode = cxx_writer.writer_code.Code("""unsigned int npc = NPC;
PC = npc;
npc += 4;
NPC = npc;
""")
IncrementPC = trap.HelperOperation('IncrementPC', opCode, exception = False)

# Write back of the result of most operations, expecially ALUs;
# such operations do not modify the PSR
opCode = cxx_writer.writer_code.Code("""
rd = result;
""")
WB_plain = trap.HelperOperation('WB_plain', opCode, exception = False)
WB_plain.addInstuctionVar(('result', 'BIT<32>'))
WB_plain.addUserInstructionElement('rd')

# Write back of the result of most operations, expecially ALUs;
# such operations also modify the PSR
opCode = cxx_writer.writer_code.Code("""
if(!temp_V){
    rd = result;
}
""")
WB_tv = trap.HelperOperation('WB_tv', opCode, exception = False)
WB_tv.addInstuctionVar(('result', 'BIT<32>'))
WB_tv.addInstuctionVar(('temp_V', 'BIT<1>'))
WB_tv.addUserInstructionElement('rd')

# Modification of the Integer Condition Codes of the Processor Status Register
# after an logical operation or after the multiply operation
opCode = cxx_writer.writer_code.Code("""
PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
PSR[key_ICC_z] = (result == 0);
PSR[key_ICC_v] = 0;
PSR[key_ICC_c] = 0;
""")
ICC_writeLogic = trap.HelperOperation('ICC_writeLogic', opCode, exception = False)
ICC_writeLogic.addInstuctionVar(('result', 'BIT<32>'))

# Modification of the Integer Condition Codes of the Processor Status Register
# after an addition operation
opCode = cxx_writer.writer_code.Code("""
PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
PSR[key_ICC_z] = (result == 0);
PSR[key_ICC_v] = ((unsigned int)((rs1_op & rs2_op & (~result)) | ((~rs1_op) & (~rs2_op) & result))) >> 31;
PSR[key_ICC_c] = ((unsigned int)((rs1_op & rs2_op) | ((rs1_op | rs2_op) & (~result)))) >> 31;
""")
ICC_writeAdd = trap.HelperOperation('ICC_writeAdd', opCode, exception = False)
ICC_writeAdd.addInstuctionVar(('result', 'BIT<32>'))
ICC_writeAdd.addInstuctionVar(('rs1_op', 'BIT<32>'))
ICC_writeAdd.addInstuctionVar(('rs2_op', 'BIT<32>'))

# Modification of the Integer Condition Codes of the Processor Status Register
# after a tagged addition operation
opCode = cxx_writer.writer_code.Code("""
PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
PSR[key_ICC_z] = (result == 0);
PSR[key_ICC_v] = temp_V;
PSR[key_ICC_c] = ((unsigned int)((rs1_op & rs2_op) | ((rs1_op | rs2_op) & (~result)))) >> 31;
""")
ICC_writeTAdd = trap.HelperOperation('ICC_writeTAdd', opCode, exception = False)
ICC_writeTAdd.addInstuctionVar(('result', 'BIT<32>'))
ICC_writeTAdd.addInstuctionVar(('temp_V', 'BIT<1>'))
ICC_writeTAdd.addInstuctionVar(('rs1_op', 'BIT<32>'))
ICC_writeTAdd.addInstuctionVar(('rs2_op', 'BIT<32>'))

# Modification of the Integer Condition Codes of the Processor Status Register
# after a division operation
opCode = cxx_writer.writer_code.Code("""
if(!exception){
    PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
    PSR[key_ICC_z] = (result == 0);
    PSR[key_ICC_v] = temp_V;
    PSR[key_ICC_c] = 0;
}
""")
ICC_writeDiv = trap.HelperOperation('ICC_writeDiv', opCode, exception = False)
ICC_writeDiv.addInstuctionVar(('exception', 'BIT<1>'))
ICC_writeDiv.addInstuctionVar(('result', 'BIT<32>'))
ICC_writeDiv.addInstuctionVar(('temp_V', 'BIT<1>'))

# Modification of the Integer Condition Codes of the Processor Status Register
# after a tagged addition operation
opCode = cxx_writer.writer_code.Code("""
if(!temp_V){
    PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
    PSR[key_ICC_z] = (result == 0);
    PSR[key_ICC_v] = 0;
    PSR[key_ICC_c] = ((unsigned int)((rs1_op & rs2_op) | ((rs1_op | rs2_op) & (~result)))) >> 31;
}
""")
ICC_writeTVAdd = trap.HelperOperation('ICC_writeTVAdd', opCode, exception = False)
ICC_writeTVAdd.addInstuctionVar(('result', 'BIT<32>'))
ICC_writeTVAdd.addInstuctionVar(('temp_V', 'BIT<1>'))
ICC_writeTVAdd.addInstuctionVar(('rs1_op', 'BIT<32>'))
ICC_writeTVAdd.addInstuctionVar(('rs2_op', 'BIT<32>'))

# Modification of the Integer Condition Codes of the Processor Status Register
# after a subtraction operation
opCode = cxx_writer.writer_code.Code("""
PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
PSR[key_ICC_z] = (result == 0);
PSR[key_ICC_v] = ((unsigned int)((rs1_op & (~rs2_op) & (~result)) | ((~rs1_op) & rs2_op & result))) >> 31;
PSR[key_ICC_c] = ((unsigned int)(((~rs1_op) & rs2_op) | (((~rs1_op) | rs2_op) & result))) >> 31;
""")
ICC_writeSub = trap.HelperOperation('ICC_writeSub', opCode, exception = False)
ICC_writeSub.addInstuctionVar(('result', 'BIT<32>'))
ICC_writeSub.addInstuctionVar(('rs1_op', 'BIT<32>'))
ICC_writeSub.addInstuctionVar(('rs2_op', 'BIT<32>'))

# Modification of the Integer Condition Codes of the Processor Status Register
# after a tagged subtraction operation
opCode = cxx_writer.writer_code.Code("""
PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
PSR[key_ICC_z] = (result == 0);
PSR[key_ICC_v] = temp_V;
PSR[key_ICC_c] = ((unsigned int)(((~rs1_op) & rs2_op) | (((~rs1_op) | rs2_op) & result))) >> 31;
""")
ICC_writeTSub = trap.HelperOperation('ICC_writeTSub', opCode, exception = False)
ICC_writeTSub.addInstuctionVar(('result', 'BIT<32>'))
ICC_writeTSub.addInstuctionVar(('temp_V', 'BIT<1>'))
ICC_writeTSub.addInstuctionVar(('rs1_op', 'BIT<32>'))
ICC_writeTSub.addInstuctionVar(('rs2_op', 'BIT<32>'))

# Modification of the Integer Condition Codes of the Processor Status Register
# after a tagged subtraction operation
opCode = cxx_writer.writer_code.Code("""
if(!temp_V){
    PSR[key_ICC_n] = ((result & 0x80000000) >> 31);
    PSR[key_ICC_z] = (result == 0);
    PSR[key_ICC_v] = temp_V;
    PSR[key_ICC_c] = ((unsigned int)(((~rs1_op) & rs2_op) | (((~rs1_op) | rs2_op) & result))) >> 31;
}
""")
ICC_writeTVSub = trap.HelperOperation('ICC_writeTVSub', opCode, exception = False)
ICC_writeTVSub.addInstuctionVar(('result', 'BIT<32>'))
ICC_writeTVSub.addInstuctionVar(('temp_V', 'BIT<1>'))
ICC_writeTVSub.addInstuctionVar(('rs1_op', 'BIT<32>'))
ICC_writeTVSub.addInstuctionVar(('rs2_op', 'BIT<32>'))
