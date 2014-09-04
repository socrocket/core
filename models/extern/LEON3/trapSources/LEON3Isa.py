# -*- coding: iso-8859-1 -*-

# Lets first of all import the necessary files for the
# creation of the processor
import trap
import cxx_writer
from LEON3Coding import *
from LEON3Methods import *
from LEONDefs import *

# ISA declaration: it is the container for all the single instructions
isa = trap.ISA()

# Now I add to the ISA all the helper methods and operations which will be
# called from the instructions
isa.addMethod(IncrementRegWindow_method)
isa.addMethod(DecrementRegWindow_method)
isa.addMethod(SignExtend_method)
isa.addMethod(RaiseException_method)
isa.addMethod(checkIncrementWin_method)
isa.addMethod(checkDecrementWin_method)

# Now I add some useful definitions to be used inside the instructions; they will be
# inserted as defines in the hpp and file of the instructions
isa.addDefines("""
#define RESET 0
#define DATA_STORE_ERROR 1
#define INSTR_ACCESS_MMU_MISS 2
#define INSTR_ACCESS_ERROR 3
#define R_REGISTER_ACCESS_ERROR 4
#define INSTR_ACCESS_EXC 5
#define PRIVILEDGE_INSTR 6
#define ILLEGAL_INSTR 7
#define FP_DISABLED 8
#define CP_DISABLED 9
#define UNIMPL_FLUSH 10
#define WATCHPOINT_DETECTED 11
#define WINDOW_OVERFLOW 12
#define WINDOW_UNDERFLOW 13
#define MEM_ADDR_NOT_ALIGNED 14
#define FP_EXCEPTION 15
#define CP_EXCEPTION 16
#define DATA_ACCESS_ERROR 17
#define DATA_ACCESS_MMU_MISS 18
#define DATA_ACCESS_EXC 19
#define TAG_OVERFLOW 20
#define DIV_ZERO 21
#define TRAP_INSTRUCTION 22
#define IRQ_LEV_15 23
#define IRQ_LEV_14 24
#define IRQ_LEV_13 25
#define IRQ_LEV_12 26
#define IRQ_LEV_11 27
#define IRQ_LEV_10 28
#define IRQ_LEV_9 29
#define IRQ_LEV_8 30
#define IRQ_LEV_7 31
#define IRQ_LEV_6 32
#define IRQ_LEV_5 33
#define IRQ_LEV_4 34
#define IRQ_LEV_3 35
#define IRQ_LEV_2 36
#define IRQ_LEV_1 37
#define IMPL_DEP_EXC 38
""")

# Here lests set the NOP behavior: it is used only for the cycle accurate processor; the functional
# one does not even define such instruction
isa.setNOPBehavior("""unsigned int npc = NPC;
PC = npc;
npc += 4;
NPC = npc;
""", 'fetch')

#-------------------------------------------------------------------------------------
# Let's now procede to set the behavior of the instructions
#-------------------------------------------------------------------------------------
#
# Note the special operations:
#
# -- annull(): transforms the current instruction in a NOP; if we are
# in the middle of the execution of some code, it also terminates the
# execution of that part of code (it is like an exception)
# -- flush(): flushes the pipeline stages preceding the one in which
# the flush method has been called
# -- stall(n): stalls the current stage and the preceding ones for n clock
# cycles. If we issue this operation in the middle of the execution of an
# instruction, anyway the execution of that code finished before the stall
# operation has any effect; if that code contains another call to stall(m),
# the pipeline stages are stalled for a total of n+m. Note that if the instruction
# taken n cycles, than the stall has to be issued for n-1, since one cycle
# is intrinsic
# -- THROW_EXCEPTION: a macro for throwing C++ exceptions
#

#____________________________________________________________________________________________________
#----------------------------------------------------------------------------------------------------
# Now using all the defined operations, instruction codings, etc
# I can actually declare the processor instructions
#----------------------------------------------------------------------------------------------------
#____________________________________________________________________________________________________
#

ReadPCFetch = """pcounter = PC;
#ifndef ACC_MODEL
npcounter = NPC;
#endif
"""
ReadNPCDecode = """#ifdef ACC_MODEL
npcounter = PC;
#endif
"""

opCodeReadPC = cxx_writer.writer_code.Code(ReadPCFetch)
opCodeReadNPC = cxx_writer.writer_code.Code(ReadNPCDecode)

opCodeRegsImm = cxx_writer.writer_code.Code("""
address = rs1 + SignExtend(simm13, 13);
""")
opCodeRegsRegs = cxx_writer.writer_code.Code("""
address = rs1 + rs2;
""")
opCodeMem = cxx_writer.writer_code.Code("""
readValue = SignExtend(dataMem.read_byte(address), 8);
""")
opCodeWb = cxx_writer.writer_code.Code("""
rd = readValue;
""")
# Load Integer Instruction Family
ldsb_imm_Instr = trap.Instruction('LDSB_imm', True, frequency = 2)
ldsb_imm_Instr.setMachineCode(mem_format2, {'op3': [0, 0, 1, 0, 0, 1]}, ('ldsb r', '%rs1', '+', '%simm13', ' r', '%rd'))
ldsb_imm_Instr.setVarField('rd', ('REGS', 0), 'out')
ldsb_imm_Instr.setCode(opCodeRegsImm, 'regs')
ldsb_imm_Instr.setCode(opCodeMem, 'memory')
ldsb_imm_Instr.setCode(opCodeWb, 'wb')
ldsb_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ldsb_imm_Instr.addVariable(('address', 'BIT<32>'))
ldsb_imm_Instr.addVariable(('readValue', 'BIT<32>'))
isa.addInstruction(ldsb_imm_Instr)
ldsb_reg_Instr = trap.Instruction('LDSB_reg', True, frequency = 3)
ldsb_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 0, 1, 0, 0, 1]}, ('ldsb r', '%rs1', '+r', '%rs2', ' r', '%rd'))
ldsb_reg_Instr.setVarField('rd', ('REGS', 0), 'out')
ldsb_reg_Instr.setCode(opCodeRegsRegs, 'regs')
ldsb_reg_Instr.setCode(opCodeMem, 'memory')
ldsb_reg_Instr.setCode(opCodeWb, 'wb')
ldsb_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ldsb_reg_Instr.addVariable(('address', 'BIT<32>'))
ldsb_reg_Instr.addVariable(('readValue', 'BIT<32>'))
isa.addInstruction(ldsb_reg_Instr)
opCodeExec = cxx_writer.writer_code.Code("""
notAligned = (address & 0x00000001) != 0;
#ifdef ACC_MODEL
if(notAligned){
    flush();
}
#endif
""")
opCodeMem = cxx_writer.writer_code.Code("""
if(!notAligned){
    readValue = SignExtend(dataMem.read_half(address), 16);
}
#ifdef ACC_MODEL
else{
    flush();
}
#endif
""")
opCodeException = cxx_writer.writer_code.Code("""
if(notAligned){
    RaiseException(pcounter, npcounter, MEM_ADDR_NOT_ALIGNED);
}
""")
ldsh_imm_Instr = trap.Instruction('LDSH_imm', True, frequency = 1)
ldsh_imm_Instr.setMachineCode(mem_format2, {'op3': [0, 0, 1, 0, 1, 0]}, ('ldsh r', '%rs1', '+', '%simm13', ' r', '%rd'))
ldsh_imm_Instr.setVarField('rd', ('REGS', 0), 'out')
ldsh_imm_Instr.setCode(opCodeRegsImm, 'regs')
ldsh_imm_Instr.setCode(opCodeExec, 'execute')
ldsh_imm_Instr.setCode(opCodeMem, 'memory')
ldsh_imm_Instr.setCode(opCodeException, 'exception')
ldsh_imm_Instr.setCode(opCodeWb, 'wb')
ldsh_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ldsh_imm_Instr.setCode(opCodeReadPC, 'fetch')
ldsh_imm_Instr.setCode(opCodeReadNPC, 'decode')
ldsh_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
ldsh_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
ldsh_imm_Instr.addVariable(('address', 'BIT<32>'))
ldsh_imm_Instr.addVariable(('readValue', 'BIT<32>'))
ldsh_imm_Instr.addVariable(('notAligned', 'BIT<1>'))
isa.addInstruction(ldsh_imm_Instr)
ldsh_reg_Instr = trap.Instruction('LDSH_reg', True, frequency = 1)
ldsh_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 0, 1, 0, 1, 0]}, ('ldsh r', '%rs1', '+r', '%rs2', ' r', '%rd'))
ldsh_reg_Instr.setVarField('rd', ('REGS', 0), 'out')
ldsh_reg_Instr.setCode(opCodeRegsRegs, 'regs')
ldsh_reg_Instr.setCode(opCodeExec, 'execute')
ldsh_reg_Instr.setCode(opCodeMem, 'memory')
ldsh_reg_Instr.setCode(opCodeException, 'exception')
ldsh_reg_Instr.setCode(opCodeWb, 'wb')
ldsh_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ldsh_reg_Instr.setCode(opCodeReadPC, 'fetch')
ldsh_reg_Instr.setCode(opCodeReadNPC, 'decode')
ldsh_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
ldsh_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
ldsh_reg_Instr.addVariable(('address', 'BIT<32>'))
ldsh_reg_Instr.addVariable(('readValue', 'BIT<32>'))
ldsh_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
isa.addInstruction(ldsh_reg_Instr)
opCodeMem = cxx_writer.writer_code.Code("""
readValue = dataMem.read_byte(address);
""")
ldub_imm_Instr = trap.Instruction('LDUB_imm', True, frequency = 5)
ldub_imm_Instr.setMachineCode(mem_format2, {'op3': [0, 0, 0, 0, 0, 1]}, ('ldub r', '%rs1', '+', '%simm13', ' r', '%rd'))
ldub_imm_Instr.setVarField('rd', ('REGS', 0), 'out')
ldub_imm_Instr.setCode(opCodeRegsImm, 'regs')
ldub_imm_Instr.setCode(opCodeMem, 'memory')
ldub_imm_Instr.setCode(opCodeWb, 'wb')
ldub_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ldub_imm_Instr.addVariable(('address', 'BIT<32>'))
ldub_imm_Instr.addVariable(('readValue', 'BIT<32>'))
isa.addInstruction(ldub_imm_Instr)
ldub_reg_Instr = trap.Instruction('LDUB_reg', True, frequency = 6)
ldub_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 0, 0, 0, 0, 1]}, ('ldub r', '%rs1', '+r', '%rs2', ' r', '%rd'))
ldub_reg_Instr.setVarField('rd', ('REGS', 0), 'out')
ldub_reg_Instr.setCode(opCodeRegsRegs, 'regs')
ldub_reg_Instr.setCode(opCodeMem, 'memory')
ldub_reg_Instr.setCode(opCodeWb, 'wb')
ldub_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ldub_reg_Instr.addVariable(('address', 'BIT<32>'))
ldub_reg_Instr.addVariable(('readValue', 'BIT<32>'))
isa.addInstruction(ldub_reg_Instr)
opCodeMem = cxx_writer.writer_code.Code("""
readValue = dataMem.read_half(address);
""")
lduh_imm_Instr = trap.Instruction('LDUH_imm', True, frequency = 7)
lduh_imm_Instr.setMachineCode(mem_format2, {'op3': [0, 0, 0, 0, 1, 0]}, ('lduh r', '%rs1', '+', '%simm13', ' r', '%rd'))
lduh_imm_Instr.setVarField('rd', ('REGS', 0), 'out')
lduh_imm_Instr.setCode(opCodeRegsImm, 'regs')
lduh_imm_Instr.setCode(opCodeExec, 'execute')
lduh_imm_Instr.setCode(opCodeMem, 'memory')
lduh_imm_Instr.setCode(opCodeException, 'exception')
lduh_imm_Instr.setCode(opCodeWb, 'wb')
lduh_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
lduh_imm_Instr.setCode(opCodeReadPC, 'fetch')
lduh_imm_Instr.setCode(opCodeReadNPC, 'decode')
lduh_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
lduh_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
lduh_imm_Instr.addVariable(('address', 'BIT<32>'))
lduh_imm_Instr.addVariable(('readValue', 'BIT<32>'))
lduh_imm_Instr.addVariable(('notAligned', 'BIT<1>'))
isa.addInstruction(lduh_imm_Instr)
lduh_reg_Instr = trap.Instruction('LDUH_reg', True, frequency = 6)
lduh_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 0, 0, 0, 1, 0]}, ('lduh r', '%rs1', '+r', '%rs2', ' r', '%rd'))
lduh_reg_Instr.setVarField('rd', ('REGS', 0), 'out')
lduh_reg_Instr.setCode(opCodeRegsRegs, 'regs')
lduh_reg_Instr.setCode(opCodeExec, 'execute')
lduh_reg_Instr.setCode(opCodeMem, 'memory')
lduh_reg_Instr.setCode(opCodeException, 'exception')
lduh_reg_Instr.setCode(opCodeWb, 'wb')
lduh_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
lduh_reg_Instr.setCode(opCodeReadPC, 'fetch')
lduh_reg_Instr.setCode(opCodeReadNPC, 'decode')
lduh_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
lduh_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
lduh_reg_Instr.addVariable(('address', 'BIT<32>'))
lduh_reg_Instr.addVariable(('readValue', 'BIT<32>'))
lduh_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
isa.addInstruction(lduh_reg_Instr)
opCodeMem = cxx_writer.writer_code.Code("""
readValue = dataMem.read_word(address);
""")
opCodeExec = cxx_writer.writer_code.Code("""
notAligned = (address & 0x00000003) != 0;
#ifdef ACC_MODEL
if(notAligned){
    flush();
}
#endif
""")
ld_imm_Instr = trap.Instruction('LD_imm', True, frequency = 15)
ld_imm_Instr.setMachineCode(mem_format2, {'op3': [0, 0, 0, 0, 0, 0]}, ('ld r', '%rs1', '+', '%simm13', ' r', '%rd'))
ld_imm_Instr.setVarField('rd', ('REGS', 0), 'out')
ld_imm_Instr.setCode(opCodeRegsImm, 'regs')
ld_imm_Instr.setCode(opCodeExec, 'execute')
ld_imm_Instr.setCode(opCodeMem, 'memory')
ld_imm_Instr.setCode(opCodeException, 'exception')
ld_imm_Instr.setCode(opCodeWb, 'wb')
ld_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ld_imm_Instr.setCode(opCodeReadPC, 'fetch')
ld_imm_Instr.setCode(opCodeReadNPC, 'decode')
ld_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
ld_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
ld_imm_Instr.addVariable(('address', 'BIT<32>'))
ld_imm_Instr.addVariable(('readValue', 'BIT<32>'))
ld_imm_Instr.addVariable(('notAligned', 'BIT<1>'))
isa.addInstruction(ld_imm_Instr)
ld_reg_Instr = trap.Instruction('LD_reg', True, frequency = 10)
ld_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 0, 0, 0, 0, 0]}, ('ld r', '%rs1', '+r', '%rs2', ' r', '%rd'))
ld_reg_Instr.setVarField('rd', ('REGS', 0), 'out')
ld_reg_Instr.setCode(opCodeRegsRegs, 'regs')
ld_reg_Instr.setCode(opCodeExec, 'execute')
ld_reg_Instr.setCode(opCodeMem, 'memory')
ld_reg_Instr.setCode(opCodeException, 'exception')
ld_reg_Instr.setCode(opCodeWb, 'wb')
ld_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ld_reg_Instr.setCode(opCodeReadPC, 'fetch')
ld_reg_Instr.setCode(opCodeReadNPC, 'decode')
ld_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
ld_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
ld_reg_Instr.addVariable(('address', 'BIT<32>'))
ld_reg_Instr.addVariable(('readValue', 'BIT<32>'))
ld_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
isa.addInstruction(ld_reg_Instr)
opCodeDecode = cxx_writer.writer_code.Code(ReadNPCDecode + """
#ifdef ACC_MODEL
REGS[rd_bit ^ 0x1].lock();
#endif
""")
flushCode = """#ifdef ACC_MODEL
if(notAligned){
    flush();
}
#endif
"""
notAlignComputeCode = """notAligned = (address & 0x00000007) != 0;
#ifdef ACC_MODEL
if(notAligned){
    flush();
}
#endif
"""
opCodeRegsImm = cxx_writer.writer_code.Code('address = rs1 + SignExtend(simm13, 13);\n' + notAlignComputeCode)
opCodeRegsRegs = cxx_writer.writer_code.Code('address = rs1 + rs2;\n' + notAlignComputeCode)
opCodeExec = cxx_writer.writer_code.Code(flushCode)
opCodeMem = cxx_writer.writer_code.Code("""
if(!notAligned){
    readValue = dataMem.read_dword(address);
    stall(1);
}
""" + flushCode)
opCodeWb = cxx_writer.writer_code.Code("""
if(rd_bit % 2 == 0){
    rd = (unsigned int)(readValue & 0x00000000FFFFFFFFLL);
    REGS[rd_bit + 1] = (unsigned int)((readValue >> 32) & 0x00000000FFFFFFFFLL);
}
else{
    REGS[rd_bit - 1] = (unsigned int)(readValue & 0x00000000FFFFFFFFLL);
    rd = (unsigned int)((readValue >> 32) & 0x00000000FFFFFFFFLL);
}
#ifdef ACC_MODEL
unlockQueue[0].push_back(REGS[rd_bit ^ 0x1].getPipeReg());
#endif
""")
ldd_imm_Instr = trap.Instruction('LDD_imm', True, frequency = 6)
ldd_imm_Instr.setMachineCode(mem_format2, {'op3': [0, 0, 0, 0, 1, 1]}, ('ldd r', '%rs1', '+', '%simm13', ' r', '%rd'))
ldd_imm_Instr.setVarField('rd', ('REGS', 0), 'out')
ldd_imm_Instr.setCode(opCodeDecode, 'decode')
ldd_imm_Instr.setCode(opCodeRegsImm, 'regs')
ldd_imm_Instr.setCode(opCodeExec, 'execute')
ldd_imm_Instr.setCode(opCodeMem, 'memory')
ldd_imm_Instr.setCode(opCodeException, 'exception')
ldd_imm_Instr.setCode(opCodeWb, 'wb')
ldd_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ldd_imm_Instr.setCode(opCodeReadPC, 'fetch')
ldd_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
ldd_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
ldd_imm_Instr.addVariable(('address', 'BIT<32>'))
ldd_imm_Instr.addVariable(('readValue', 'BIT<64>'))
ldd_imm_Instr.addVariable(('notAligned', 'BIT<1>'))
isa.addInstruction(ldd_imm_Instr)
ldd_reg_Instr = trap.Instruction('LDD_reg', True, frequency = 5)
ldd_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 0, 0, 0, 1, 1]}, ('ldd r', '%rs1', '+r', '%rs2', ' r', '%rd'))
ldd_reg_Instr.setVarField('rd', ('REGS', 0), 'out')
ldd_reg_Instr.setCode(opCodeDecode, 'decode')
ldd_reg_Instr.setCode(opCodeRegsRegs, 'regs')
ldd_reg_Instr.setCode(opCodeExec, 'execute')
ldd_reg_Instr.setCode(opCodeMem, 'memory')
ldd_reg_Instr.setCode(opCodeException, 'exception')
ldd_reg_Instr.setCode(opCodeWb, 'wb')
ldd_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ldd_reg_Instr.setCode(opCodeReadPC, 'fetch')
ldd_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
ldd_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
ldd_reg_Instr.addVariable(('address', 'BIT<32>'))
ldd_reg_Instr.addVariable(('readValue', 'BIT<64>'))
ldd_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
isa.addInstruction(ldd_reg_Instr)
# Here are the load operations accessing alternate space
opCodeRegsRegs = cxx_writer.writer_code.Code("""
address = rs1 + rs2;
supervisor = PSR[key_S];
""")
opCodeWb = cxx_writer.writer_code.Code("""
rd = readValue;
""")
opCodeMem = cxx_writer.writer_code.Code("""
#ifdef ACC_MODEL
if(!supervisor){
    flush();
}
else{
#endif
    readValue = SignExtend(dataMem.read_byte(address), 8);
#ifdef ACC_MODEL
}
#endif
""")
opCodeExec = cxx_writer.writer_code.Code("""
#ifdef ACC_MODEL
if(!supervisor){
    flush();
}
#endif
""")
opCodeException = cxx_writer.writer_code.Code("""
if(!supervisor){
    RaiseException(pcounter, npcounter, PRIVILEDGE_INSTR);
}
""")
ldsba_reg_Instr = trap.Instruction('LDSBA_reg', True, frequency = 1)
ldsba_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 1, 1, 0, 0, 1]}, ('ldba r', '%rs1', '+r', '%rs2', ' ', '%asi', ' r', '%rd'))
ldsba_reg_Instr.setVarField('rd', ('REGS', 0), 'out')
ldsba_reg_Instr.setCode(opCodeRegsRegs, 'regs')
ldsba_reg_Instr.setCode(opCodeExec, 'execute')
ldsba_reg_Instr.setCode(opCodeMem, 'memory')
ldsba_reg_Instr.setCode(opCodeException, 'exception')
ldsba_reg_Instr.setCode(opCodeWb, 'wb')
ldsba_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ldsba_reg_Instr.setCode(opCodeReadPC, 'fetch')
ldsba_reg_Instr.setCode(opCodeReadNPC, 'decode')
ldsba_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
ldsba_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
ldsba_reg_Instr.addVariable(('address', 'BIT<32>'))
ldsba_reg_Instr.addVariable(('readValue', 'BIT<32>'))
ldsba_reg_Instr.addVariable(('supervisor', 'BIT<1>'))
isa.addInstruction(ldsba_reg_Instr)
opCodeMem = cxx_writer.writer_code.Code("""
#ifdef ACC_MODEL
if(notAligned || !supervisor){
    flush();
}
else{
#endif
    readValue = SignExtend(dataMem.read_half(address), 16);
#ifdef ACC_MODEL
}
#endif
""")
opCodeExec = cxx_writer.writer_code.Code("""
notAligned = (address & 0x00000001) != 0;
#ifdef ACC_MODEL
if(notAligned || !supervisor){
    flush();
}
#endif
""")
opCodeException = cxx_writer.writer_code.Code("""
if(!supervisor){
    RaiseException(pcounter, npcounter, PRIVILEDGE_INSTR);
}
if(notAligned){
    RaiseException(pcounter, npcounter, MEM_ADDR_NOT_ALIGNED);
}
""")
ldsha_reg_Instr = trap.Instruction('LDSHA_reg', True, frequency = 1)
ldsha_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 1, 1, 0, 1, 0]}, ('ldsha r', '%rs1', '+r', '%rs2', ' ', '%asi', ' r', '%rd'))
ldsha_reg_Instr.setVarField('rd', ('REGS', 0), 'out')
ldsha_reg_Instr.setCode(opCodeRegsRegs, 'regs')
ldsha_reg_Instr.setCode(opCodeExec, 'execute')
ldsha_reg_Instr.setCode(opCodeMem, 'memory')
ldsha_reg_Instr.setCode(opCodeException, 'exception')
ldsha_reg_Instr.setCode(opCodeWb, 'wb')
ldsha_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ldsha_reg_Instr.setCode(opCodeReadPC, 'fetch')
ldsha_reg_Instr.setCode(opCodeReadNPC, 'decode')
ldsha_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
ldsha_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
ldsha_reg_Instr.addVariable(('address', 'BIT<32>'))
ldsha_reg_Instr.addVariable(('readValue', 'BIT<32>'))
ldsha_reg_Instr.addVariable(('supervisor', 'BIT<1>'))
ldsha_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
isa.addInstruction(ldsha_reg_Instr)
opCodeMem = cxx_writer.writer_code.Code("""
#ifdef ACC_MODEL
if(!supervisor){
    flush();
}
else{
#endif
    readValue = dataMem.read_byte(address);
#ifdef ACC_MODEL
}
#endif
""")
opCodeExec = cxx_writer.writer_code.Code("""
#ifdef ACC_MODEL
if(!supervisor){
    flush();
}
#endif
""")
opCodeException = cxx_writer.writer_code.Code("""
if(!supervisor){
    RaiseException(pcounter, npcounter, PRIVILEDGE_INSTR);
}
""")
lduba_reg_Instr = trap.Instruction('LDUBA_reg', True, frequency = 1)
lduba_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 1, 0, 0, 0, 1]}, ('lduba r', '%rs1', '+r', '%rs2', ' ', '%asi', ' r', '%rd'))
lduba_reg_Instr.setVarField('rd', ('REGS', 0), 'out')
lduba_reg_Instr.setCode(opCodeRegsRegs, 'regs')
lduba_reg_Instr.setCode(opCodeExec, 'execute')
lduba_reg_Instr.setCode(opCodeMem, 'memory')
lduba_reg_Instr.setCode(opCodeException, 'exception')
lduba_reg_Instr.setCode(opCodeWb, 'wb')
lduba_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
lduba_reg_Instr.setCode(opCodeReadPC, 'fetch')
lduba_reg_Instr.setCode(opCodeReadNPC, 'decode')
lduba_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
lduba_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
lduba_reg_Instr.addVariable(('address', 'BIT<32>'))
lduba_reg_Instr.addVariable(('readValue', 'BIT<32>'))
lduba_reg_Instr.addVariable(('supervisor', 'BIT<1>'))
isa.addInstruction(lduba_reg_Instr)
opCodeMem = cxx_writer.writer_code.Code("""
#ifdef ACC_MODEL
if(notAligned || !supervisor){
    flush();
}
else{
#endif
    readValue = dataMem.read_half(address);
#ifdef ACC_MODEL
}
#endif
""")
opCodeExec = cxx_writer.writer_code.Code("""
notAligned = (address & 0x00000001) != 0;
#ifdef ACC_MODEL
if(notAligned || !supervisor){
    flush();
}
#endif
""")
opCodeException = cxx_writer.writer_code.Code("""
if(!supervisor){
    RaiseException(pcounter, npcounter, PRIVILEDGE_INSTR);
}
if(notAligned){
    RaiseException(pcounter, npcounter, MEM_ADDR_NOT_ALIGNED);
}
""")
lduha_reg_Instr = trap.Instruction('LDUHA_reg', True, frequency = 1)
lduha_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 1, 0, 0, 1, 0]}, ('lduha r', '%rs1', '+r', '%rs2', ' ', '%asi', ' r', '%rd'))
lduha_reg_Instr.setVarField('rd', ('REGS', 0), 'out')
lduha_reg_Instr.setCode(opCodeRegsRegs, 'regs')
lduha_reg_Instr.setCode(opCodeExec, 'execute')
lduha_reg_Instr.setCode(opCodeMem, 'memory')
lduha_reg_Instr.setCode(opCodeException, 'exception')
lduha_reg_Instr.setCode(opCodeWb, 'wb')
lduha_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
lduha_reg_Instr.setCode(opCodeReadPC, 'fetch')
lduha_reg_Instr.setCode(opCodeReadNPC, 'decode')
lduha_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
lduha_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
lduha_reg_Instr.addVariable(('address', 'BIT<32>'))
lduha_reg_Instr.addVariable(('readValue', 'BIT<32>'))
lduha_reg_Instr.addVariable(('supervisor', 'BIT<1>'))
lduha_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
isa.addInstruction(lduha_reg_Instr)
opCodeMem = cxx_writer.writer_code.Code("""
#ifdef ACC_MODEL
if(notAligned || !supervisor){
    flush();
}
else{
#endif
    readValue = dataMem.read_word(address);
#ifdef ACC_MODEL
}
#endif
""")
opCodeExec = cxx_writer.writer_code.Code("""
notAligned = (address & 0x00000003) != 0;
#ifdef ACC_MODEL
if(notAligned || !supervisor){
    flush();
}
#endif
""")
lda_reg_Instr = trap.Instruction('LDA_reg', True, frequency = 1)
lda_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 1, 0, 0, 0, 0]}, ('lda r', '%rs1', '+r', '%rs2', ' ', '%asi', ' r', '%rd'))
lda_reg_Instr.setVarField('rd', ('REGS', 0), 'out')
lda_reg_Instr.setCode(opCodeRegsRegs, 'regs')
lda_reg_Instr.setCode(opCodeExec, 'execute')
lda_reg_Instr.setCode(opCodeMem, 'memory')
lda_reg_Instr.setCode(opCodeException, 'exception')
lda_reg_Instr.setCode(opCodeWb, 'wb')
lda_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
lda_reg_Instr.setCode(opCodeReadPC, 'fetch')
lda_reg_Instr.setCode(opCodeReadNPC, 'decode')
lda_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
lda_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
lda_reg_Instr.addVariable(('address', 'BIT<32>'))
lda_reg_Instr.addVariable(('readValue', 'BIT<32>'))
lda_reg_Instr.addVariable(('supervisor', 'BIT<1>'))
lda_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
isa.addInstruction(lda_reg_Instr)
opCodeDecode = cxx_writer.writer_code.Code(ReadNPCDecode + """
#ifdef ACC_MODEL
REGS[rd_bit ^ 0x1].lock();
#endif
""")
opCodeExec = cxx_writer.writer_code.Code("""
notAligned = (address & 0x00000007) != 0;
#ifdef ACC_MODEL
if(notAligned || !supervisor){
    flush();
}
#endif
""")
opCodeMem = cxx_writer.writer_code.Code("""
#ifdef ACC_MODEL
if(notAligned || !supervisor){
    flush();
}
else{
#endif
    readValue = dataMem.read_dword(address);
#ifdef ACC_MODEL
}
#endif
""")
opCodeWb = cxx_writer.writer_code.Code("""
if(rd_bit % 2 == 0){
    rd = (unsigned int)(readValue & 0x00000000FFFFFFFFLL);
    REGS[rd_bit + 1] = (unsigned int)((readValue >> 32) & 0x00000000FFFFFFFFLL);
}
else{
    REGS[rd_bit - 1] = (unsigned int)(readValue & 0x00000000FFFFFFFFLL);
    rd = (unsigned int)((readValue >> 32) & 0x00000000FFFFFFFFLL);
}
#ifdef ACC_MODEL
unlockQueue[0].push_back(REGS[rd_bit ^ 0x1].getPipeReg());
#endif
""")
ldda_reg_Instr = trap.Instruction('LDDA_reg', True, frequency = 1)
ldda_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 1, 0, 0, 1, 1]}, ('ldda r', '%rs1', '+r', '%rs2', ' ', '%asi', ' r', '%rd'))
ldda_reg_Instr.setVarField('rd', ('REGS', 0), 'out')
ldda_reg_Instr.setCode(opCodeDecode, 'decode')
ldda_reg_Instr.setCode(opCodeRegsRegs, 'regs')
ldda_reg_Instr.setCode(opCodeExec, 'execute')
ldda_reg_Instr.setCode(opCodeMem, 'memory')
ldda_reg_Instr.setCode(opCodeException, 'exception')
ldda_reg_Instr.setCode(opCodeWb, 'wb')
ldda_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ldda_reg_Instr.setCode(opCodeReadPC, 'fetch')
ldda_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
ldda_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
ldda_reg_Instr.addVariable(('address', 'BIT<32>'))
ldda_reg_Instr.addVariable(('readValue', 'BIT<64>'))
ldda_reg_Instr.addVariable(('supervisor', 'BIT<1>'))
ldda_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
isa.addInstruction(ldda_reg_Instr)

# Store integer instructions
opCodeRegsImm = cxx_writer.writer_code.Code("""
address = rs1 + SignExtend(simm13, 13);
toWrite = (unsigned char)(rd & 0x000000FF);
""")
opCodeRegsRegs = cxx_writer.writer_code.Code("""
address = rs1 + rs2;
toWrite = (unsigned char)(rd & 0x000000FF);
""")
opCodeMem = cxx_writer.writer_code.Code("""
dataMem.write_byte(address, toWrite);
stall(1);
""")
stb_imm_Instr = trap.Instruction('STB_imm', True, frequency = 5)
stb_imm_Instr.setMachineCode(mem_format2, {'op3': [0, 0, 0, 1, 0, 1]}, ('stb r', '%rd', ' r', '%rs1', '+', '%simm13'))
stb_imm_Instr.setVarField('rd', ('REGS', 0), 'in')
stb_imm_Instr.setCode(opCodeRegsImm, 'regs')
stb_imm_Instr.setCode(opCodeMem, 'memory')
stb_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
stb_imm_Instr.addVariable(('address', 'BIT<32>'))
stb_imm_Instr.addVariable(('toWrite', 'BIT<8>'))
isa.addInstruction(stb_imm_Instr)
stb_reg_Instr = trap.Instruction('STB_reg', True, frequency = 5)
stb_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 0, 0, 1, 0, 1]}, ('stb r', '%rd', ' r', '%rs1', '+r', '%rs2'))
stb_reg_Instr.setVarField('rd', ('REGS', 0), 'in')
stb_reg_Instr.setCode(opCodeRegsRegs, 'regs')
stb_reg_Instr.setCode(opCodeMem, 'memory')
stb_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
stb_reg_Instr.addVariable(('address', 'BIT<32>'))
stb_reg_Instr.addVariable(('toWrite', 'BIT<8>'))
isa.addInstruction(stb_reg_Instr)
opCodeRegsImm = cxx_writer.writer_code.Code("""
address = rs1 + SignExtend(simm13, 13);
toWrite = (unsigned short int)(rd & 0x0000FFFF);
""")
opCodeRegsRegs = cxx_writer.writer_code.Code("""
address = rs1 + rs2;
toWrite = (unsigned short int)(rd & 0x0000FFFF);
""")
opCodeMem = cxx_writer.writer_code.Code("""
if(!notAligned){
    dataMem.write_half(address, toWrite);
}
else{
    flush();
}
stall(1);
""")
opCodeExec = cxx_writer.writer_code.Code("""
notAligned = (address & 0x00000001) != 0;
#ifdef ACC_MODEL
if(notAligned){
    flush();
}
#endif
""")
opCodeException = cxx_writer.writer_code.Code("""
if(notAligned){
    RaiseException(pcounter, npcounter, MEM_ADDR_NOT_ALIGNED);
}
""")
sth_imm_Instr = trap.Instruction('STH_imm', True, frequency = 6)
sth_imm_Instr.setMachineCode(mem_format2, {'op3': [0, 0, 0, 1, 1, 0]}, ('sth r', '%rd', ' r', '%rs1', '+', '%simm13'))
sth_imm_Instr.setVarField('rd', ('REGS', 0), 'in')
sth_imm_Instr.setCode(opCodeRegsImm, 'regs')
sth_imm_Instr.setCode(opCodeMem, 'memory')
sth_imm_Instr.setCode(opCodeExec, 'execute')
sth_imm_Instr.setCode(opCodeException, 'exception')
sth_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
sth_imm_Instr.setCode(opCodeReadPC, 'fetch')
sth_imm_Instr.setCode(opCodeReadNPC, 'decode')
sth_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
sth_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
sth_imm_Instr.addVariable(('notAligned', 'BIT<1>'))
sth_imm_Instr.addVariable(('address', 'BIT<32>'))
sth_imm_Instr.addVariable(('toWrite', 'BIT<16>'))
isa.addInstruction(sth_imm_Instr)
sth_reg_Instr = trap.Instruction('STH_reg', True, frequency = 5)
sth_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 0, 0, 1, 1, 0]}, ('sth r', '%rd', ' r', '%rs1', '+r', '%rs2'))
sth_reg_Instr.setVarField('rd', ('REGS', 0), 'in')
sth_reg_Instr.setCode(opCodeRegsRegs, 'regs')
sth_reg_Instr.setCode(opCodeMem, 'memory')
sth_reg_Instr.setCode(opCodeExec, 'execute')
sth_reg_Instr.setCode(opCodeException, 'exception')
sth_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
sth_reg_Instr.setCode(opCodeReadPC, 'fetch')
sth_reg_Instr.setCode(opCodeReadNPC, 'decode')
sth_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
sth_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
sth_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
sth_reg_Instr.addVariable(('address', 'BIT<32>'))
sth_reg_Instr.addVariable(('toWrite', 'BIT<16>'))
isa.addInstruction(sth_reg_Instr)
opCodeRegsImm = cxx_writer.writer_code.Code("""
address = rs1 + SignExtend(simm13, 13);
toWrite = rd;
""")
opCodeRegsRegs = cxx_writer.writer_code.Code("""
address = rs1 + rs2;
toWrite = rd;
""")
opCodeMem = cxx_writer.writer_code.Code("""
if(!notAligned){
    dataMem.write_word(address, toWrite);
}
else{
    flush();
}
stall(1);
""")
opCodeExec = cxx_writer.writer_code.Code("""
notAligned = (address & 0x00000003) != 0;
#ifdef ACC_MODEL
if(notAligned){
    flush();
}
#endif
""")
st_imm_Instr = trap.Instruction('ST_imm', True, frequency = 13)
st_imm_Instr.setMachineCode(mem_format2, {'op3': [0, 0, 0, 1, 0, 0]}, ('st r', '%rd', ' r', '%rs1', '+', '%simm13'))
st_imm_Instr.setVarField('rd', ('REGS', 0), 'in')
st_imm_Instr.setCode(opCodeRegsImm, 'regs')
st_imm_Instr.setCode(opCodeMem, 'memory')
st_imm_Instr.setCode(opCodeExec, 'execute')
st_imm_Instr.setCode(opCodeException, 'exception')
st_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
st_imm_Instr.setCode(opCodeReadPC, 'fetch')
st_imm_Instr.setCode(opCodeReadNPC, 'decode')
st_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
st_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
st_imm_Instr.addVariable(('notAligned', 'BIT<1>'))
st_imm_Instr.addVariable(('address', 'BIT<32>'))
st_imm_Instr.addVariable(('toWrite', 'BIT<32>'))
isa.addInstruction(st_imm_Instr)
st_reg_Instr = trap.Instruction('ST_reg', True, frequency = 10)
st_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 0, 0, 1, 0, 0]}, ('st r', '%rd', ' r', '%rs1', '+r', '%rs2'))
st_reg_Instr.setVarField('rd', ('REGS', 0), 'in')
st_reg_Instr.setCode(opCodeRegsRegs, 'regs')
st_reg_Instr.setCode(opCodeMem, 'memory')
st_reg_Instr.setCode(opCodeExec, 'execute')
st_reg_Instr.setCode(opCodeException, 'exception')
st_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
st_reg_Instr.setCode(opCodeReadPC, 'fetch')
st_reg_Instr.setCode(opCodeReadNPC, 'decode')
st_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
st_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
st_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
st_reg_Instr.addVariable(('address', 'BIT<32>'))
st_reg_Instr.addVariable(('toWrite', 'BIT<32>'))
isa.addInstruction(st_reg_Instr)
readReg = """if(rd_bit % 2 == 0){
    toWrite = rd | (((unsigned long long)REGS[rd_bit + 1]) << 32);
}
else{
    toWrite = REGS[rd_bit - 1] | (((unsigned long long)rd) << 32);
}
"""
opCodeRegsImm = cxx_writer.writer_code.Code('address = rs1 + SignExtend(simm13, 13);\n' + readReg)
opCodeRegsRegs = cxx_writer.writer_code.Code('address = rs1 + rs2;\n' + readReg)
opCodeMem = cxx_writer.writer_code.Code("""
if(!notAligned){
    dataMem.write_dword(address, toWrite);
}
else{
    flush();
}
stall(2);
""")
opCodeExec = cxx_writer.writer_code.Code("""
notAligned = (address & 0x00000007) != 0;
#ifdef ACC_MODEL
if(notAligned){
    flush();
}
#endif
""")
std_imm_Instr = trap.Instruction('STD_imm', True, frequency = 6)
std_imm_Instr.setMachineCode(mem_format2, {'op3': [0, 0, 0, 1, 1, 1]}, ('std r', '%rd', ' r', '%rs1', '+', '%simm13'))
std_imm_Instr.setVarField('rd', ('REGS', 0), 'in')
std_imm_Instr.setCode(opCodeRegsImm, 'regs')
std_imm_Instr.setCode(opCodeMem, 'memory')
std_imm_Instr.setCode(opCodeExec, 'execute')
std_imm_Instr.setCode(opCodeException, 'exception')
std_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
std_imm_Instr.setCode(opCodeReadPC, 'fetch')
std_imm_Instr.setCode(opCodeReadNPC, 'decode')
std_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
std_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
std_imm_Instr.addVariable(('notAligned', 'BIT<1>'))
std_imm_Instr.addVariable(('address', 'BIT<32>'))
std_imm_Instr.addVariable(('toWrite', 'BIT<64>'))
std_imm_Instr.addCheckHazardCode('REGS_decode[rd_bit ^ 0x1].isLocked()', 'decode')
isa.addInstruction(std_imm_Instr)
std_reg_Instr = trap.Instruction('STD_reg', True, frequency = 5)
std_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 0, 0, 1, 1, 1]}, ('std r', '%rd', ' r', '%rs1', '+r', '%rs2'))
std_reg_Instr.setVarField('rd', ('REGS', 0), 'in')
std_reg_Instr.setCode(opCodeRegsRegs, 'regs')
std_reg_Instr.setCode(opCodeMem, 'memory')
std_reg_Instr.setCode(opCodeExec, 'execute')
std_reg_Instr.setCode(opCodeException, 'exception')
std_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
std_reg_Instr.setCode(opCodeReadPC, 'fetch')
std_reg_Instr.setCode(opCodeReadNPC, 'decode')
std_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
std_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
std_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
std_reg_Instr.addVariable(('address', 'BIT<32>'))
std_reg_Instr.addVariable(('toWrite', 'BIT<64>'))
std_reg_Instr.addCheckHazardCode('REGS_decode[rd_bit ^ 0x1].isLocked()', 'decode')
isa.addInstruction(std_reg_Instr)
opCodeRegsRegs = cxx_writer.writer_code.Code("""
address = rs1 + rs2;
toWrite = (unsigned char)(rd & 0x000000FF);
supervisor = PSR[key_S];
""")
opCodeMem = cxx_writer.writer_code.Code("""
if(supervisor){
    dataMem.write_byte(address, toWrite);
}
else{
    flush();
}
stall(1);
""")
opCodeExec = cxx_writer.writer_code.Code("""
#ifdef ACC_MODEL
if(!supervisor){
    flush();
}
#endif
""")
opCodeException = cxx_writer.writer_code.Code("""
if(!supervisor){
    RaiseException(pcounter, npcounter, PRIVILEDGE_INSTR);
}
""")
stba_reg_Instr = trap.Instruction('STBA_reg', True, frequency = 1)
stba_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 1, 0, 1, 0, 1]}, ('stba r', '%rd', ' r', '%rs1', '+r', '%rs2', ' ', '%asi'))
stba_reg_Instr.setVarField('rd', ('REGS', 0), 'in')
stba_reg_Instr.setCode(opCodeRegsRegs, 'regs')
stba_reg_Instr.setCode(opCodeMem, 'memory')
stba_reg_Instr.setCode(opCodeExec, 'execute')
stba_reg_Instr.setCode(opCodeException, 'exception')
stba_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
stba_reg_Instr.setCode(opCodeReadPC, 'fetch')
stba_reg_Instr.setCode(opCodeReadNPC, 'decode')
stba_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
stba_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
stba_reg_Instr.addVariable(('supervisor', 'BIT<1>'))
stba_reg_Instr.addVariable(('address', 'BIT<32>'))
stba_reg_Instr.addVariable(('toWrite', 'BIT<8>'))
isa.addInstruction(stba_reg_Instr)
opCodeRegsRegs = cxx_writer.writer_code.Code("""
address = rs1 + rs2;
toWrite = (unsigned short int)(rd & 0x0000FFFF);
supervisor = PSR[key_S];
""")
opCodeMem = cxx_writer.writer_code.Code("""
if(supervisor || !notAligned){
    dataMem.write_half(address, toWrite);
}
else{
    flush();
}
stall(1);
""")
opCodeExec = cxx_writer.writer_code.Code("""
notAligned = (address & 0x00000001) != 0;
#ifdef ACC_MODEL
if(!supervisor || notAligned){
    flush();
}
#endif
""")
opCodeException = cxx_writer.writer_code.Code("""
if(!supervisor){
    RaiseException(pcounter, npcounter, PRIVILEDGE_INSTR);
}
if(notAligned){
    RaiseException(pcounter, npcounter, MEM_ADDR_NOT_ALIGNED);
}
""")
stha_reg_Instr = trap.Instruction('STHA_reg', True, frequency = 1)
stha_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 1, 0, 1, 1, 0]}, ('stha r', '%rd', ' r', '%rs1', '+r', '%rs2', ' ', '%asi'))
stha_reg_Instr.setVarField('rd', ('REGS', 0), 'in')
stha_reg_Instr.setCode(opCodeRegsRegs, 'regs')
stha_reg_Instr.setCode(opCodeMem, 'memory')
stha_reg_Instr.setCode(opCodeExec, 'execute')
stha_reg_Instr.setCode(opCodeException, 'exception')
stha_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
stha_reg_Instr.setCode(opCodeReadPC, 'fetch')
stha_reg_Instr.setCode(opCodeReadNPC, 'decode')
stha_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
stha_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
stha_reg_Instr.addVariable(('supervisor', 'BIT<1>'))
stha_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
stha_reg_Instr.addVariable(('address', 'BIT<32>'))
stha_reg_Instr.addVariable(('toWrite', 'BIT<16>'))
isa.addInstruction(stha_reg_Instr)
opCodeRegsRegs = cxx_writer.writer_code.Code("""
address = rs1 + rs2;
toWrite = rd;
supervisor = PSR[key_S];
""")
opCodeMem = cxx_writer.writer_code.Code("""
if(supervisor || !notAligned){
    dataMem.write_word(address, toWrite);
}
else{
    flush();
}
stall(1);
""")
opCodeExec = cxx_writer.writer_code.Code("""
notAligned = (address & 0x00000003) != 0;
#ifdef ACC_MODEL
if(!supervisor || notAligned){
    flush();
}
#endif
""")
sta_reg_Instr = trap.Instruction('STA_reg', True, frequency = 1)
sta_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 1, 0, 1, 0, 0]}, ('sta r', '%rd', ' r', '%rs1', '+r', '%rs2', ' ', '%asi'))
sta_reg_Instr.setVarField('rd', ('REGS', 0), 'in')
sta_reg_Instr.setCode(opCodeRegsRegs, 'regs')
sta_reg_Instr.setCode(opCodeMem, 'memory')
sta_reg_Instr.setCode(opCodeExec, 'execute')
sta_reg_Instr.setCode(opCodeException, 'exception')
sta_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
sta_reg_Instr.setCode(opCodeReadPC, 'fetch')
sta_reg_Instr.setCode(opCodeReadNPC, 'decode')
sta_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
sta_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
sta_reg_Instr.addVariable(('supervisor', 'BIT<1>'))
sta_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
sta_reg_Instr.addVariable(('address', 'BIT<32>'))
sta_reg_Instr.addVariable(('toWrite', 'BIT<32>'))
isa.addInstruction(sta_reg_Instr)
opCodeRegsRegs = cxx_writer.writer_code.Code("""
address = rs1 + rs2;
if(rd_bit % 2 == 0){
    toWrite = rd || (((unsigned long long)REGS[rd_bit + 1]) << 32);
}
else{
    toWrite = REGS[rd_bit + 1] || (((unsigned long long)rd) << 32);
}
supervisor = PSR[key_S];
""")
opCodeMem = cxx_writer.writer_code.Code("""
if(supervisor || !notAligned){
    dataMem.write_dword(address, toWrite);
}
else{
    flush();
}
stall(1);
""")
opCodeExec = cxx_writer.writer_code.Code("""
notAligned = (address & 0x00000003) != 0;
#ifdef ACC_MODEL
if(!supervisor || notAligned){
    flush();
}
#endif
""")
stda_reg_Instr = trap.Instruction('STDA_reg', True, frequency = 1)
stda_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 1, 0, 1, 1, 1]}, ('stda r', '%rd', ' r', '%rs1', '+r', '%rs2', ' ', '%asi'))
stda_reg_Instr.setVarField('rd', ('REGS', 0), 'in')
stda_reg_Instr.setCode(opCodeRegsRegs, 'regs')
stda_reg_Instr.setCode(opCodeMem, 'memory')
stda_reg_Instr.setCode(opCodeExec, 'execute')
stda_reg_Instr.setCode(opCodeException, 'exception')
stda_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
stda_reg_Instr.setCode(opCodeReadPC, 'fetch')
stda_reg_Instr.setCode(opCodeReadNPC, 'decode')
stda_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
stda_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
stda_reg_Instr.addVariable(('supervisor', 'BIT<1>'))
stda_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
stda_reg_Instr.addVariable(('address', 'BIT<32>'))
stda_reg_Instr.addVariable(('toWrite', 'BIT<64>'))
stda_reg_Instr.addCheckHazardCode('REGS_decode[rd_bit ^ 0x1].isLocked()', 'decode')
isa.addInstruction(stda_reg_Instr)

# Atomic Load/Store
opCodeRegsImm = cxx_writer.writer_code.Code("""
address = rs1 + SignExtend(simm13, 13);
""")
opCodeRegsRegs = cxx_writer.writer_code.Code("""
address = rs1 + rs2;
""")
opCodeMem = cxx_writer.writer_code.Code("""
readValue = dataMem.read_byte(address);
dataMem.write_byte(address, 0xff);
stall(2);
""")
opCodeWb = cxx_writer.writer_code.Code("""
rd = readValue;
""")
ldstub_imm_Instr = trap.Instruction('LDSTUB_imm', True, frequency = 1)
ldstub_imm_Instr.setMachineCode(mem_format2, {'op3': [0, 0, 1, 1, 0, 1]}, ('ldastub r', '%rs1', '+', '%simm13', ' r', '%rd'))
ldstub_imm_Instr.setVarField('rd', ('REGS', 0), 'out')
ldstub_imm_Instr.setCode(opCodeRegsImm, 'regs')
ldstub_imm_Instr.setCode(opCodeMem, 'memory')
ldstub_imm_Instr.setCode(opCodeWb, 'wb')
ldstub_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ldstub_imm_Instr.addVariable(('address', 'BIT<32>'))
ldstub_imm_Instr.addVariable(('readValue', 'BIT<32>'))
isa.addInstruction(ldstub_imm_Instr)
ldstub_reg_Instr = trap.Instruction('LDSTUB_reg', True, frequency = 1)
ldstub_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 0, 1, 1, 0, 1]}, ('ldastub r', '%rs1', '+r', '%rs2', ' r', '%rd'))
ldstub_reg_Instr.setVarField('rd', ('REGS', 0), 'out')
ldstub_reg_Instr.setCode(opCodeRegsRegs, 'regs')
ldstub_reg_Instr.setCode(opCodeMem, 'memory')
ldstub_reg_Instr.setCode(opCodeWb, 'wb')
ldstub_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ldstub_reg_Instr.addVariable(('address', 'BIT<32>'))
ldstub_reg_Instr.addVariable(('readValue', 'BIT<32>'))
isa.addInstruction(ldstub_reg_Instr)
opCodeRegsRegs = cxx_writer.writer_code.Code("""
address = rs1 + rs2;
supervisor = PSR[key_S];
""")
opCodeExec = cxx_writer.writer_code.Code("""
#ifdef ACC_MODEL
if(!supervisor){
    flush();
}
#endif
""")
opCodeMem = cxx_writer.writer_code.Code("""
if(supervisor){
    readValue = dataMem.read_byte(address);
    dataMem.write_byte(address, 0xff);
}
else{
    flush();
}
stall(2);
""")
opCodeException = cxx_writer.writer_code.Code("""
if(!supervisor){
    RaiseException(pcounter, npcounter, PRIVILEDGE_INSTR);
}
""")
ldstuba_reg_Instr = trap.Instruction('LDSTUBA_reg', True, frequency = 1)
ldstuba_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 1, 1, 1, 0, 1]}, ('ldastub r', '%rs1', '+r', '%rs2', ' ', '%asi', ' r', '%rd'))
ldstuba_reg_Instr.setVarField('rd', ('REGS', 0), 'out')
ldstuba_reg_Instr.setCode(opCodeRegsRegs, 'regs')
ldstuba_reg_Instr.setCode(opCodeMem, 'memory')
ldstuba_reg_Instr.setCode(opCodeExec, 'execute')
ldstuba_reg_Instr.setCode(opCodeException, 'exception')
ldstuba_reg_Instr.setCode(opCodeWb, 'wb')
ldstuba_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
ldstuba_reg_Instr.setCode(opCodeReadPC, 'fetch')
ldstuba_reg_Instr.setCode(opCodeReadNPC, 'decode')
ldstuba_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
ldstuba_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
ldstuba_reg_Instr.addVariable(('supervisor', 'BIT<1>'))
ldstuba_reg_Instr.addVariable(('address', 'BIT<32>'))
ldstuba_reg_Instr.addVariable(('readValue', 'BIT<32>'))
isa.addInstruction(ldstuba_reg_Instr)

# Swap
opCodeRegsImm = cxx_writer.writer_code.Code("""
address = rs1 + SignExtend(simm13, 13);
toWrite = rd;
""")
opCodeRegsRegs = cxx_writer.writer_code.Code("""
address = rs1 + rs2;
toWrite = rd;
""")
opCodeExec = cxx_writer.writer_code.Code("""
notAligned = (address & 0x00000003) != 0;
#ifdef ACC_MODEL
if(notAligned){
    flush();
}
#endif
""")
opCodeMem = cxx_writer.writer_code.Code("""
if(notAligned){
    flush();
}
else{
    readValue = dataMem.read_word(address);
    dataMem.write_word(address, toWrite);
}
stall(2);
""")
opCodeWb = cxx_writer.writer_code.Code("""
rd = readValue;
""")
opCodeException = cxx_writer.writer_code.Code("""
if(notAligned){
    RaiseException(pcounter, npcounter, MEM_ADDR_NOT_ALIGNED);
}
""")
swap_imm_Instr = trap.Instruction('SWAP_imm', True, frequency = 1)
swap_imm_Instr.setMachineCode(mem_format2, {'op3': [0, 0, 1, 1, 1, 1]}, ('swap r', '%rs1', '+', '%simm13', ' r', '%rd'))
swap_imm_Instr.setVarField('rd', ('REGS', 0), 'inout')
swap_imm_Instr.setCode(opCodeRegsImm, 'regs')
swap_imm_Instr.setCode(opCodeMem, 'memory')
swap_imm_Instr.setCode(opCodeExec, 'execute')
swap_imm_Instr.setCode(opCodeException, 'exception')
swap_imm_Instr.setCode(opCodeWb, 'wb')
swap_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
swap_imm_Instr.setCode(opCodeReadPC, 'fetch')
swap_imm_Instr.setCode(opCodeReadNPC, 'decode')
swap_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
swap_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
swap_imm_Instr.addVariable(('notAligned', 'BIT<1>'))
swap_imm_Instr.addVariable(('address', 'BIT<32>'))
swap_imm_Instr.addVariable(('readValue', 'BIT<32>'))
swap_imm_Instr.addVariable(('toWrite', 'BIT<32>'))
isa.addInstruction(swap_imm_Instr)
swap_reg_Instr = trap.Instruction('SWAP_reg', True, frequency = 1)
swap_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 0, 1, 1, 1, 1]}, ('swap r', '%rs1', '+r', '%rs2', ' r', '%rd'))
swap_reg_Instr.setVarField('rd', ('REGS', 0), 'inout')
swap_reg_Instr.setCode(opCodeRegsRegs, 'regs')
swap_reg_Instr.setCode(opCodeMem, 'memory')
swap_reg_Instr.setCode(opCodeExec, 'execute')
swap_reg_Instr.setCode(opCodeException, 'exception')
swap_reg_Instr.setCode(opCodeWb, 'wb')
swap_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
swap_reg_Instr.setCode(opCodeReadPC, 'fetch')
swap_reg_Instr.setCode(opCodeReadNPC, 'decode')
swap_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
swap_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
swap_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
swap_reg_Instr.addVariable(('address', 'BIT<32>'))
swap_reg_Instr.addVariable(('readValue', 'BIT<32>'))
swap_reg_Instr.addVariable(('toWrite', 'BIT<32>'))
isa.addInstruction(swap_reg_Instr)
opCodeRegsRegs = cxx_writer.writer_code.Code("""
address = rs1 + rs2;
toWrite = rd;
supervisor = PSR[key_S];
""")
opCodeExec = cxx_writer.writer_code.Code("""
notAligned = (address & 0x00000003) != 0;
#ifdef ACC_MODEL
if(!supervisor || notAligned){
    flush();
}
#endif
""")
opCodeMem = cxx_writer.writer_code.Code("""
if(!supervisor || notAligned){
    flush();
}
else{
    readValue = dataMem.read_word(address);
    dataMem.write_word(address, toWrite);
}
stall(2);
""")
opCodeWb = cxx_writer.writer_code.Code("""
rd = readValue;
""")
opCodeException = cxx_writer.writer_code.Code("""
if(!supervisor){
    RaiseException(pcounter, npcounter, PRIVILEDGE_INSTR);
}
if(notAligned){
    RaiseException(pcounter, npcounter, MEM_ADDR_NOT_ALIGNED);
}
""")
swapa_reg_Instr = trap.Instruction('SWAPA_reg', True, frequency = 1)
swapa_reg_Instr.setMachineCode(mem_format1, {'op3': [0, 1, 1, 1, 1, 1]}, ('swapa r', '%rs1', '+r', '%rs2', ' ', '%asi', ' r', '%rd'))
swapa_reg_Instr.setVarField('rd', ('REGS', 0), 'inout')
swapa_reg_Instr.setCode(opCodeRegsRegs, 'regs')
swapa_reg_Instr.setCode(opCodeMem, 'memory')
swapa_reg_Instr.setCode(opCodeExec, 'execute')
swapa_reg_Instr.setCode(opCodeException, 'exception')
swapa_reg_Instr.setCode(opCodeWb, 'wb')
swapa_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
swapa_reg_Instr.setCode(opCodeReadPC, 'fetch')
swapa_reg_Instr.setCode(opCodeReadNPC, 'decode')
swapa_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
swapa_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
swapa_reg_Instr.addVariable(('supervisor', 'BIT<1>'))
swapa_reg_Instr.addVariable(('notAligned', 'BIT<1>'))
swapa_reg_Instr.addVariable(('address', 'BIT<32>'))
swapa_reg_Instr.addVariable(('readValue', 'BIT<32>'))
swapa_reg_Instr.addVariable(('toWrite', 'BIT<32>'))
isa.addInstruction(swapa_reg_Instr)

# sethi
opCodeExec = cxx_writer.writer_code.Code("""
result = 0xfffffc00 & (imm22 << 10);
""")
sethi_Instr = trap.Instruction('SETHI', True, frequency = 14)
sethi_Instr.setMachineCode(b_sethi_format1, {'op2': [1, 0, 0]}, ('sethi ', '%imm22', ' r', '%rd'))
sethi_Instr.setCode(opCodeExec, 'execute')
sethi_Instr.addBehavior(WB_plain, 'wb')
sethi_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
sethi_Instr.addVariable(('result', 'BIT<32>'))
isa.addInstruction(sethi_Instr)

# Logical Instructions
opCodeReadRegs1 = cxx_writer.writer_code.Code("""
rs1_op = rs1;
""")
opCodeReadRegs2 = cxx_writer.writer_code.Code("""
rs1_op = rs1;
rs2_op = rs2;
""")
opCodeExecImm = cxx_writer.writer_code.Code("""
result = rs1_op & SignExtend(simm13, 13);
""")
and_imm_Instr = trap.Instruction('AND_imm', True, frequency = 10)
and_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 0, 0, 0, 0, 1]}, ('and r', '%rs1', ' ', '%simm13', ' r', '%rd'))
and_imm_Instr.setCode(opCodeExecImm, 'execute')
and_imm_Instr.setCode(opCodeReadRegs1, 'regs')
and_imm_Instr.addBehavior(WB_plain, 'wb')
and_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
and_imm_Instr.addVariable(('result', 'BIT<32>'))
and_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
isa.addInstruction(and_imm_Instr)
opCodeExecReg = cxx_writer.writer_code.Code("""
result = rs1_op & rs2_op;
""")
and_reg_Instr = trap.Instruction('AND_reg', True, frequency = 7)
and_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 0, 0, 0, 0, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('and r', '%rs1', ' r', '%rs2', ' r', '%rd'))
and_reg_Instr.setCode(opCodeExecReg, 'execute')
and_reg_Instr.setCode(opCodeReadRegs2, 'regs')
and_reg_Instr.addBehavior(WB_plain, 'wb')
and_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
and_reg_Instr.addVariable(('result', 'BIT<32>'))
and_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
and_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
isa.addInstruction(and_reg_Instr)
andcc_imm_Instr = trap.Instruction('ANDcc_imm', True, frequency = 6)
andcc_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 1, 0, 0, 0, 1]}, ('andcc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
andcc_imm_Instr.setCode(opCodeExecImm, 'execute')
andcc_imm_Instr.setCode(opCodeReadRegs1, 'regs')
andcc_imm_Instr.addBehavior(WB_plain, 'wb')
andcc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
andcc_imm_Instr.addBehavior(ICC_writeLogic, 'execute', False)
andcc_imm_Instr.addVariable(('result', 'BIT<32>'))
andcc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
andcc_imm_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(andcc_imm_Instr)
andcc_reg_Instr = trap.Instruction('ANDcc_reg', True, frequency = 2)
andcc_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 1, 0, 0, 0, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('andcc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
andcc_reg_Instr.setCode(opCodeExecReg, 'execute')
andcc_reg_Instr.setCode(opCodeReadRegs2, 'regs')
andcc_reg_Instr.addBehavior(WB_plain, 'wb')
andcc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
andcc_reg_Instr.addBehavior(ICC_writeLogic, 'execute', False)
andcc_reg_Instr.addVariable(('result', 'BIT<32>'))
andcc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
andcc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
andcc_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(andcc_reg_Instr)
opCodeExecImm = cxx_writer.writer_code.Code("""
result = rs1_op & ~(SignExtend(simm13, 13));
""")
andn_imm_Instr = trap.Instruction('ANDN_imm', True, frequency = 2)
andn_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 0, 0, 1, 0, 1]}, ('andn r', '%rs1', ' ', '%simm13', ' r', '%rd'))
andn_imm_Instr.setCode(opCodeExecImm, 'execute')
andn_imm_Instr.setCode(opCodeReadRegs1, 'regs')
andn_imm_Instr.addBehavior(WB_plain, 'wb')
andn_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
andn_imm_Instr.addVariable(('result', 'BIT<32>'))
andn_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
isa.addInstruction(andn_imm_Instr)
opCodeExecReg = cxx_writer.writer_code.Code("""
result = rs1_op & ~rs2_op;
""")
andn_reg_Instr = trap.Instruction('ANDN_reg', True, frequency = 5)
andn_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 0, 0, 1, 0, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('andn r', '%rs1', ' r', '%rs2', ' r', '%rd'))
andn_reg_Instr.setCode(opCodeExecReg, 'execute')
andn_reg_Instr.setCode(opCodeReadRegs2, 'regs')
andn_reg_Instr.addBehavior(WB_plain, 'wb')
andn_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
andn_reg_Instr.addVariable(('result', 'BIT<32>'))
andn_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
andn_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
isa.addInstruction(andn_reg_Instr)
andncc_imm_Instr = trap.Instruction('ANDNcc_imm', True, frequency = 2)
andncc_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 1, 0, 1, 0, 1]}, ('andncc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
andncc_imm_Instr.setCode(opCodeExecImm, 'execute')
andncc_imm_Instr.setCode(opCodeReadRegs1, 'regs')
andncc_imm_Instr.addBehavior(WB_plain, 'wb')
andncc_imm_Instr.addBehavior(ICC_writeLogic, 'execute', False)
andncc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
andncc_imm_Instr.addVariable(('result', 'BIT<32>'))
andncc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
andncc_imm_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(andncc_imm_Instr)
andncc_reg_Instr = trap.Instruction('ANDNcc_reg', True, frequency = 2)
andncc_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 1, 0, 1, 0, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('andncc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
andncc_reg_Instr.setCode(opCodeExecReg, 'execute')
andncc_reg_Instr.setCode(opCodeReadRegs2, 'regs')
andncc_reg_Instr.addBehavior(WB_plain, 'wb')
andncc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
andncc_reg_Instr.addBehavior(ICC_writeLogic, 'execute', False)
andncc_reg_Instr.addVariable(('result', 'BIT<32>'))
andncc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
andncc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
andncc_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(andncc_reg_Instr)
opCodeExecImm = cxx_writer.writer_code.Code("""
result = rs1_op | SignExtend(simm13, 13);
""")
or_imm_Instr = trap.Instruction('OR_imm', True, frequency = 12)
or_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 0, 0, 0, 1, 0]}, ('or r', '%rs1', ' ', '%simm13', ' r', '%rd'))
or_imm_Instr.setCode(opCodeExecImm, 'execute')
or_imm_Instr.setCode(opCodeReadRegs1, 'regs')
or_imm_Instr.addBehavior(WB_plain, 'wb')
or_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
or_imm_Instr.addVariable(('result', 'BIT<32>'))
or_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
isa.addInstruction(or_imm_Instr)
opCodeExecReg = cxx_writer.writer_code.Code("""
result = rs1_op | rs2_op;
""")
or_reg_Instr = trap.Instruction('OR_reg', True, frequency = 12)
or_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 0, 0, 0, 1, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('or r', '%rs1', ' r', '%rs2', ' r', '%rd'))
or_reg_Instr.setCode(opCodeExecReg, 'execute')
or_reg_Instr.setCode(opCodeReadRegs2, 'regs')
or_reg_Instr.addBehavior(WB_plain, 'wb')
or_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
or_reg_Instr.addVariable(('result', 'BIT<32>'))
or_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
or_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
isa.addInstruction(or_reg_Instr)
orcc_imm_Instr = trap.Instruction('ORcc_imm', True, frequency = 4)
orcc_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 1, 0, 0, 1, 0]}, ('orcc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
orcc_imm_Instr.setCode(opCodeExecImm, 'execute')
orcc_imm_Instr.setCode(opCodeReadRegs1, 'regs')
orcc_imm_Instr.addBehavior(WB_plain, 'wb')
orcc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
orcc_imm_Instr.addBehavior(ICC_writeLogic, 'execute', False)
orcc_imm_Instr.addVariable(('result', 'BIT<32>'))
orcc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
orcc_imm_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(orcc_imm_Instr)
orcc_reg_Instr = trap.Instruction('ORcc_reg', True, frequency = 5)
orcc_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 1, 0, 0, 1, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('orcc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
orcc_reg_Instr.setCode(opCodeExecReg, 'execute')
orcc_reg_Instr.setCode(opCodeReadRegs2, 'regs')
orcc_reg_Instr.addBehavior(WB_plain, 'wb')
orcc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
orcc_reg_Instr.addBehavior(ICC_writeLogic, 'execute', False)
orcc_reg_Instr.addVariable(('result', 'BIT<32>'))
orcc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
orcc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
orcc_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(orcc_reg_Instr)
opCodeExecImm = cxx_writer.writer_code.Code("""
result = rs1_op | ~(SignExtend(simm13, 13));
""")
orn_imm_Instr = trap.Instruction('ORN_imm', True, frequency = 2)
orn_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 0, 0, 1, 1, 0]}, ('orn r', '%rs1', ' ', '%simm13', ' r', '%rd'))
orn_imm_Instr.setCode(opCodeExecImm, 'execute')
orn_imm_Instr.setCode(opCodeReadRegs1, 'regs')
orn_imm_Instr.addBehavior(WB_plain, 'wb')
orn_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
orn_imm_Instr.addVariable(('result', 'BIT<32>'))
orn_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
isa.addInstruction(orn_imm_Instr)
opCodeExecReg = cxx_writer.writer_code.Code("""
result = rs1_op | ~rs2_op;
""")
orn_reg_Instr = trap.Instruction('ORN_reg', True, frequency = 2)
orn_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 0, 0, 1, 1, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('orn r', '%rs1', ' r', '%rs2', ' r', '%rd'))
orn_reg_Instr.setCode(opCodeExecReg, 'execute')
orn_reg_Instr.setCode(opCodeReadRegs2, 'regs')
orn_reg_Instr.addBehavior(WB_plain, 'wb')
orn_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
orn_reg_Instr.addVariable(('result', 'BIT<32>'))
orn_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
orn_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
isa.addInstruction(orn_reg_Instr)
orncc_imm_Instr = trap.Instruction('ORNcc_imm', True, frequency = 2)
orncc_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 1, 0, 1, 1, 0]}, ('orncc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
orncc_imm_Instr.setCode(opCodeExecImm, 'execute')
orncc_imm_Instr.setCode(opCodeReadRegs1, 'regs')
orncc_imm_Instr.addBehavior(WB_plain, 'wb')
orncc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
orncc_imm_Instr.addBehavior(ICC_writeLogic, 'execute', False)
orncc_imm_Instr.addVariable(('result', 'BIT<32>'))
orncc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
isa.addInstruction(orncc_imm_Instr)
orncc_reg_Instr = trap.Instruction('ORNcc_reg', True, frequency = 2)
orncc_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 1, 0, 1, 1, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('orncc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
orncc_reg_Instr.setCode(opCodeExecReg, 'execute')
orncc_reg_Instr.setCode(opCodeReadRegs2, 'regs')
orncc_reg_Instr.addBehavior(WB_plain, 'wb')
orncc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
orncc_reg_Instr.addBehavior(ICC_writeLogic, 'execute', False)
orncc_reg_Instr.addVariable(('result', 'BIT<32>'))
orncc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
orncc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
orncc_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(orncc_reg_Instr)
opCodeExecImm = cxx_writer.writer_code.Code("""
result = rs1_op ^ SignExtend(simm13, 13);
""")
xor_imm_Instr = trap.Instruction('XOR_imm', True, frequency = 3)
xor_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 0, 0, 0, 1, 1]}, ('xor r', '%rs1', ' ', '%simm13', ' r', '%rd'))
xor_imm_Instr.setCode(opCodeExecImm, 'execute')
xor_imm_Instr.setCode(opCodeReadRegs1, 'regs')
xor_imm_Instr.addBehavior(WB_plain, 'wb')
xor_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
xor_imm_Instr.addVariable(('result', 'BIT<32>'))
xor_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
isa.addInstruction(xor_imm_Instr)
opCodeExecReg = cxx_writer.writer_code.Code("""
result = rs1_op ^ rs2_op;
""")
xor_reg_Instr = trap.Instruction('XOR_reg', True, frequency = 7)
xor_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 0, 0, 0, 1, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('xor r', '%rs1', ' r', '%rs2', ' r', '%rd'))
xor_reg_Instr.setCode(opCodeExecReg, 'execute')
xor_reg_Instr.setCode(opCodeReadRegs2, 'regs')
xor_reg_Instr.addBehavior(WB_plain, 'wb')
xor_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
xor_reg_Instr.addVariable(('result', 'BIT<32>'))
xor_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
xor_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
isa.addInstruction(xor_reg_Instr)
xorcc_imm_Instr = trap.Instruction('XORcc_imm', True, frequency = 2)
xorcc_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 1, 0, 0, 1, 1]}, ('xorcc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
xorcc_imm_Instr.setCode(opCodeExecImm, 'execute')
xorcc_imm_Instr.setCode(opCodeReadRegs1, 'regs')
xorcc_imm_Instr.addBehavior(WB_plain, 'wb')
xorcc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
xorcc_imm_Instr.addBehavior(ICC_writeLogic, 'execute', False)
xorcc_imm_Instr.addVariable(('result', 'BIT<32>'))
xorcc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
xorcc_imm_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(xorcc_imm_Instr)
xorcc_reg_Instr = trap.Instruction('XORcc_reg', True, frequency = 2)
xorcc_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 1, 0, 0, 1, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('xorcc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
xorcc_reg_Instr.setCode(opCodeExecReg, 'execute')
xorcc_reg_Instr.setCode(opCodeReadRegs2, 'regs')
xorcc_reg_Instr.addBehavior(WB_plain, 'wb')
xorcc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
xorcc_reg_Instr.addBehavior(ICC_writeLogic, 'execute', False)
xorcc_reg_Instr.addVariable(('result', 'BIT<32>'))
xorcc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
xorcc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
xorcc_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(xorcc_reg_Instr)
opCodeExecImm = cxx_writer.writer_code.Code("""
result = rs1_op ^ ~(SignExtend(simm13, 13));
""")
xnor_imm_Instr = trap.Instruction('XNOR_imm', True, frequency = 2)
xnor_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 0, 0, 1, 1, 1]}, ('xnor r', '%rs1', ' ', '%simm13', ' r', '%rd'))
xnor_imm_Instr.setCode(opCodeExecImm, 'execute')
xnor_imm_Instr.setCode(opCodeReadRegs1, 'regs')
xnor_imm_Instr.addBehavior(WB_plain, 'wb')
xnor_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
xnor_imm_Instr.addVariable(('result', 'BIT<32>'))
xnor_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
isa.addInstruction(xnor_imm_Instr)
opCodeExecReg = cxx_writer.writer_code.Code("""
result = rs1_op ^ ~rs2_op;
""")
xnor_reg_Instr = trap.Instruction('XNOR_reg', True, frequency = 2)
xnor_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 0, 0, 1, 1, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('xnor r', '%rs1', ' r', '%rs2', ' r', '%rd'))
xnor_reg_Instr.setCode(opCodeExecReg, 'execute')
xnor_reg_Instr.setCode(opCodeReadRegs2, 'regs')
xnor_reg_Instr.addBehavior(WB_plain, 'wb')
xnor_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
xnor_reg_Instr.addVariable(('result', 'BIT<32>'))
xnor_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
xnor_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
isa.addInstruction(xnor_reg_Instr)
xnorcc_imm_Instr = trap.Instruction('XNORcc_imm', True, frequency = 2)
xnorcc_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 1, 0, 1, 1, 1]}, ('xnorcc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
xnorcc_imm_Instr.setCode(opCodeExecImm, 'execute')
xnorcc_imm_Instr.setCode(opCodeReadRegs1, 'regs')
xnorcc_imm_Instr.addBehavior(WB_plain, 'wb')
xnorcc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
xnorcc_imm_Instr.addBehavior(ICC_writeLogic, 'execute', False)
xnorcc_imm_Instr.addVariable(('result', 'BIT<32>'))
xnorcc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
xnorcc_imm_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(xnorcc_imm_Instr)
xnorcc_reg_Instr = trap.Instruction('XNORcc_reg', True, frequency = 2)
xnorcc_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 1, 0, 1, 1, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('xnorcc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
xnorcc_reg_Instr.setCode(opCodeExecReg, 'execute')
xnorcc_reg_Instr.setCode(opCodeReadRegs2, 'regs')
xnorcc_reg_Instr.addBehavior(WB_plain, 'wb')
xnorcc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
xnorcc_reg_Instr.addBehavior(ICC_writeLogic, 'execute', False)
xnorcc_reg_Instr.addVariable(('result', 'BIT<32>'))
xnorcc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
xnorcc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
xnorcc_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(xnorcc_reg_Instr)

# Shift
opCodeRegsImm = cxx_writer.writer_code.Code("""
rs1_op = rs1;
""")
opCodeRegsRegs = cxx_writer.writer_code.Code("""
rs1_op = rs1;
rs2_op = rs2;
""")
opCodeExec = cxx_writer.writer_code.Code("""
result = rs1_op << simm13;
""")
sll_imm_Instr = trap.Instruction('SLL_imm', True, frequency = 9)
sll_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 0, 0, 1, 0, 1]}, ('sll r', '%rs1', ' ', '%simm13', ' r', '%rd'))
sll_imm_Instr.setCode(opCodeExec, 'execute')
sll_imm_Instr.setCode(opCodeRegsImm, 'regs')
sll_imm_Instr.addBehavior(WB_plain, 'wb')
sll_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
sll_imm_Instr.addVariable(('result', 'BIT<32>'))
sll_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
isa.addInstruction(sll_imm_Instr)
opCodeExec = cxx_writer.writer_code.Code("""
result = rs1_op << (rs2_op & 0x0000001f);
""")
sll_reg_Instr = trap.Instruction('SLL_reg', True, frequency = 6)
sll_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 0, 0, 1, 0, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('sll r', '%rs1', ' r', '%rs2', ' r', '%rd'))
sll_reg_Instr.setCode(opCodeExec, 'execute')
sll_reg_Instr.setCode(opCodeRegsRegs, 'regs')
sll_reg_Instr.addBehavior(WB_plain, 'wb')
sll_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
sll_reg_Instr.addVariable(('result', 'BIT<32>'))
sll_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
sll_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
isa.addInstruction(sll_reg_Instr)
opCodeExec = cxx_writer.writer_code.Code("""
result = ((unsigned int)rs1_op) >> simm13;
""")
srl_imm_Instr = trap.Instruction('SRL_imm', True, frequency = 9)
srl_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 0, 0, 1, 1, 0]}, ('srl r', '%rs1', ' ', '%simm13', ' r', '%rd'))
srl_imm_Instr.setCode(opCodeExec, 'execute')
srl_imm_Instr.setCode(opCodeRegsImm, 'regs')
srl_imm_Instr.addBehavior(WB_plain, 'wb')
srl_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
srl_imm_Instr.addVariable(('result', 'BIT<32>'))
srl_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
isa.addInstruction(srl_imm_Instr)
opCodeExec = cxx_writer.writer_code.Code("""
result = ((unsigned int)rs1_op) >> (rs2_op & 0x0000001f);
""")
srl_reg_Instr = trap.Instruction('SRL_reg', True, frequency = 3)
srl_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 0, 0, 1, 1, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('srl r', '%rs1', ' r', '%rs2', ' r', '%rd'))
srl_reg_Instr.setCode(opCodeExec, 'execute')
srl_reg_Instr.setCode(opCodeRegsRegs, 'regs')
srl_reg_Instr.addBehavior(WB_plain, 'wb')
srl_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
srl_reg_Instr.addVariable(('result', 'BIT<32>'))
srl_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
srl_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
isa.addInstruction(srl_reg_Instr)
opCodeExec = cxx_writer.writer_code.Code("""
result = ((int)rs1_op) >> simm13;
""")
sra_imm_Instr = trap.Instruction('SRA_imm', True, frequency = 7)
sra_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 0, 0, 1, 1, 1]}, ('sra r', '%rs1', ' ', '%simm13', ' r', '%rd'))
sra_imm_Instr.setCode(opCodeExec, 'execute')
sra_imm_Instr.setCode(opCodeRegsImm, 'regs')
sra_imm_Instr.addBehavior(WB_plain, 'wb')
sra_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
sra_imm_Instr.addVariable(('result', 'BIT<32>'))
sra_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
isa.addInstruction(sra_imm_Instr)
opCodeExec = cxx_writer.writer_code.Code("""
result = ((int)rs1_op) >> (rs2_op & 0x0000001f);
""")
sra_reg_Instr = trap.Instruction('SRA_reg', True, frequency = 2)
sra_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 0, 0, 1, 1, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('sra r', '%rs1', ' r', '%rs2', ' r', '%rd'))
sra_reg_Instr.setCode(opCodeExec, 'execute')
sra_reg_Instr.setCode(opCodeRegsRegs, 'regs')
sra_reg_Instr.addBehavior(WB_plain, 'wb')
sra_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
sra_reg_Instr.addVariable(('result', 'BIT<32>'))
sra_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
sra_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
isa.addInstruction(sra_reg_Instr)

# Add instruction
opCodeRegsImm = cxx_writer.writer_code.Code("""
rs1_op = rs1;
rs2_op = SignExtend(simm13, 13);
""")
opCodeRegsRegs = cxx_writer.writer_code.Code("""
rs1_op = rs1;
rs2_op = rs2;
""")
opCodeExec = cxx_writer.writer_code.Code("""
result = rs1_op + rs2_op;
""")
add_imm_Instr = trap.Instruction('ADD_imm', True, frequency = 11)
add_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 0, 0, 0, 0, 0]}, ('add r', '%rs1', ' ', '%simm13', ' r', '%rd'))
add_imm_Instr.setCode(opCodeRegsImm, 'regs')
add_imm_Instr.setCode(opCodeExec, 'execute')
add_imm_Instr.addBehavior(WB_plain, 'wb')
add_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
add_imm_Instr.addVariable(('result', 'BIT<32>'))
add_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
add_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
isa.addInstruction(add_imm_Instr)
add_reg_Instr = trap.Instruction('ADD_reg', True, frequency = 9)
add_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 0, 0, 0, 0, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('add r', '%rs1', ' r', '%rs2', ' r', '%rd'))
add_reg_Instr.setCode(opCodeRegsRegs, 'regs')
add_reg_Instr.setCode(opCodeExec, 'execute')
add_reg_Instr.addBehavior(WB_plain, 'wb')
add_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
add_reg_Instr.addVariable(('result', 'BIT<32>'))
add_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
add_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
isa.addInstruction(add_reg_Instr)
addcc_imm_Instr = trap.Instruction('ADDcc_imm', True, frequency = 5)
addcc_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 1, 0, 0, 0, 0]}, ('addcc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
addcc_imm_Instr.setCode(opCodeRegsImm, 'regs')
addcc_imm_Instr.setCode(opCodeExec, 'execute')
addcc_imm_Instr.addBehavior(WB_plain, 'wb')
addcc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
addcc_imm_Instr.addBehavior(ICC_writeAdd, 'execute', False)
addcc_imm_Instr.addVariable(('result', 'BIT<32>'))
addcc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
addcc_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
addcc_imm_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(addcc_imm_Instr)
addcc_reg_Instr = trap.Instruction('ADDcc_reg', True, frequency = 7)
addcc_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 1, 0, 0, 0, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('addcc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
addcc_reg_Instr.setCode(opCodeRegsRegs, 'regs')
addcc_reg_Instr.setCode(opCodeExec, 'execute')
addcc_reg_Instr.addBehavior(WB_plain, 'wb')
addcc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
addcc_reg_Instr.addBehavior(ICC_writeAdd, 'execute', False)
addcc_reg_Instr.addVariable(('result', 'BIT<32>'))
addcc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
addcc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
addcc_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(addcc_reg_Instr)
opCodeExec = cxx_writer.writer_code.Code("""
#ifndef ACC_MODEL
result = rs1_op + rs2_op + PSR[key_ICC_c];
#else
//I read the register of the execute stage since this
//is the one containing the bypass value
result = rs1_op + rs2_op + PSR_execute[key_ICC_c];
#endif
""")
addx_imm_Instr = trap.Instruction('ADDX_imm', True, frequency = 5)
addx_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 0, 1, 0, 0, 0]}, ('addx r', '%rs1', ' ', '%simm13', ' r', '%rd'))
addx_imm_Instr.setCode(opCodeRegsImm, 'regs')
addx_imm_Instr.setCode(opCodeExec, 'execute')
addx_imm_Instr.addBehavior(WB_plain, 'wb')
addx_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
addx_imm_Instr.addVariable(('result', 'BIT<32>'))
addx_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
addx_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
addx_imm_Instr.addSpecialRegister('PSR', 'in', 'execute')
isa.addInstruction(addx_imm_Instr)
addx_reg_Instr = trap.Instruction('ADDX_reg', True, frequency = 6)
addx_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 0, 1, 0, 0, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('addx r', '%rs1', ' r', '%rs2', ' r', '%rd'))
addx_reg_Instr.setCode(opCodeRegsRegs, 'regs')
addx_reg_Instr.setCode(opCodeExec, 'execute')
addx_reg_Instr.addBehavior(WB_plain, 'wb')
addx_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
addx_reg_Instr.addVariable(('result', 'BIT<32>'))
addx_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
addx_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
addx_reg_Instr.addSpecialRegister('PSR', 'in', 'execute')
isa.addInstruction(addx_reg_Instr)
addxcc_imm_Instr = trap.Instruction('ADDXcc_imm', True, frequency = 2)
addxcc_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 1, 1, 0, 0, 0]}, ('addxcc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
addxcc_imm_Instr.setCode(opCodeRegsImm, 'regs')
addxcc_imm_Instr.setCode(opCodeExec, 'execute')
addxcc_imm_Instr.addBehavior(WB_plain, 'wb')
addxcc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
addxcc_imm_Instr.addBehavior(ICC_writeAdd, 'execute', False)
addxcc_imm_Instr.addVariable(('result', 'BIT<32>'))
addxcc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
addxcc_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
addxcc_imm_Instr.addSpecialRegister('PSR', 'inout', 'execute')
isa.addInstruction(addxcc_imm_Instr)
addxcc_reg_Instr = trap.Instruction('ADDXcc_reg', True, frequency = 2)
addxcc_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 1, 1, 0, 0, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('addxcc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
addxcc_reg_Instr.setCode(opCodeRegsRegs, 'regs')
addxcc_reg_Instr.setCode(opCodeExec, 'execute')
addxcc_reg_Instr.addBehavior(WB_plain, 'wb')
addxcc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
addxcc_reg_Instr.addBehavior(ICC_writeAdd, 'execute', False)
addxcc_reg_Instr.addVariable(('result', 'BIT<32>'))
addxcc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
addxcc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
addxcc_reg_Instr.addSpecialRegister('PSR', 'inout', 'execute')
isa.addInstruction(addxcc_reg_Instr)
opCodeExec = cxx_writer.writer_code.Code("""
result = rs1_op + rs2_op;
temp_V = ((unsigned int)((rs1_op & rs2_op & (~result)) | ((~rs1_op) & (~rs2_op) & result))) >> 31;
if(!temp_V && (((rs1_op | rs2_op) & 0x00000003) != 0)){
    temp_V = 1;
}
""")
taddcc_imm_Instr = trap.Instruction('TADDcc_imm', True, frequency = 1)
taddcc_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 0, 0, 0, 0, 0]}, ('taddcc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
taddcc_imm_Instr.setCode(opCodeRegsImm, 'regs')
taddcc_imm_Instr.setCode(opCodeExec, 'execute')
taddcc_imm_Instr.addBehavior(WB_plain, 'wb')
taddcc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
taddcc_imm_Instr.addBehavior(ICC_writeTAdd, 'execute', False)
taddcc_imm_Instr.addVariable(('result', 'BIT<32>'))
taddcc_imm_Instr.addVariable(('temp_V', 'BIT<1>'))
taddcc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
taddcc_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
taddcc_imm_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(taddcc_imm_Instr)
taddcc_reg_Instr = trap.Instruction('TADDcc_reg', True, frequency = 1)
taddcc_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 0, 0, 0, 0, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('taddcc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
taddcc_reg_Instr.setCode(opCodeRegsRegs, 'regs')
taddcc_reg_Instr.setCode(opCodeExec, 'execute')
taddcc_reg_Instr.addBehavior(WB_plain, 'wb')
taddcc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
taddcc_reg_Instr.addBehavior(ICC_writeTAdd, 'execute', False)
taddcc_reg_Instr.addVariable(('result', 'BIT<32>'))
taddcc_reg_Instr.addVariable(('temp_V', 'BIT<1>'))
taddcc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
taddcc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
taddcc_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(taddcc_reg_Instr)
opCodeTrap = cxx_writer.writer_code.Code("""
if(temp_V){
    RaiseException(pcounter, npcounter, TAG_OVERFLOW);
}
""")
taddcctv_imm_Instr = trap.Instruction('TADDccTV_imm', True, frequency = 1)
taddcctv_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 0, 0, 0, 1, 0]}, ('taddcctv r', '%rs1', ' ', '%simm13', ' r', '%rd'))
taddcctv_imm_Instr.setCode(opCodeRegsImm, 'regs')
taddcctv_imm_Instr.setCode(opCodeExec, 'execute')
taddcctv_imm_Instr.setCode(opCodeTrap, 'exception')
taddcctv_imm_Instr.addBehavior(WB_tv, 'wb')
taddcctv_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
taddcctv_imm_Instr.addBehavior(ICC_writeTVAdd, 'execute', False)
taddcctv_imm_Instr.setCode(opCodeReadPC, 'fetch')
taddcctv_imm_Instr.setCode(opCodeReadNPC, 'decode')
taddcctv_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
taddcctv_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
taddcctv_imm_Instr.addVariable(('result', 'BIT<32>'))
taddcctv_imm_Instr.addVariable(('temp_V', 'BIT<1>'))
taddcctv_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
taddcctv_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
taddcctv_imm_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(taddcctv_imm_Instr)
taddcctv_reg_Instr = trap.Instruction('TADDccTV_reg', True, frequency = 1)
taddcctv_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 0, 0, 0, 1, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('taddcctv r', '%rs1', ' r', '%rs2', ' r', '%rd'))
taddcctv_reg_Instr.setCode(opCodeRegsRegs, 'regs')
taddcctv_reg_Instr.setCode(opCodeExec, 'execute')
taddcctv_reg_Instr.setCode(opCodeTrap, 'exception')
taddcctv_reg_Instr.addBehavior(WB_tv, 'wb')
taddcctv_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
taddcctv_reg_Instr.addBehavior(ICC_writeTVAdd, 'execute', False)
taddcctv_reg_Instr.setCode(opCodeReadPC, 'fetch')
taddcctv_reg_Instr.setCode(opCodeReadNPC, 'decode')
taddcctv_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
taddcctv_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
taddcctv_reg_Instr.addVariable(('result', 'BIT<32>'))
taddcctv_reg_Instr.addVariable(('temp_V', 'BIT<1>'))
taddcctv_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
taddcctv_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
taddcctv_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(taddcctv_reg_Instr)

# Subtract
opCodeRegsImm = cxx_writer.writer_code.Code("""
rs1_op = rs1;
rs2_op = SignExtend(simm13, 13);
""")
opCodeRegsRegs = cxx_writer.writer_code.Code("""
rs1_op = rs1;
rs2_op = rs2;
""")
opCodeExec = cxx_writer.writer_code.Code("""
result = rs1_op - rs2_op;
""")
sub_imm_Instr = trap.Instruction('SUB_imm', True, frequency = 4)
sub_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 0, 0, 1, 0, 0]}, ('sub r', '%rs1', ' ', '%simm13', ' r', '%rd'))
sub_imm_Instr.setCode(opCodeRegsImm, 'regs')
sub_imm_Instr.setCode(opCodeExec, 'execute')
sub_imm_Instr.addBehavior(WB_plain, 'wb')
sub_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
sub_imm_Instr.addVariable(('result', 'BIT<32>'))
sub_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
sub_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
isa.addInstruction(sub_imm_Instr)
sub_reg_Instr = trap.Instruction('SUB_reg', True, frequency = 6)
sub_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 0, 0, 1, 0, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('sub r', '%rs1', ' r', '%rs2', ' r', '%rd'))
sub_reg_Instr.setCode(opCodeRegsRegs, 'regs')
sub_reg_Instr.setCode(opCodeExec, 'execute')
sub_reg_Instr.addBehavior(WB_plain, 'wb')
sub_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
sub_reg_Instr.addVariable(('result', 'BIT<32>'))
sub_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
sub_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
isa.addInstruction(sub_reg_Instr)
subcc_imm_Instr = trap.Instruction('SUBcc_imm', True, frequency = 10)
subcc_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 1, 0, 1, 0, 0]}, ('subcc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
subcc_imm_Instr.setCode(opCodeRegsImm, 'regs')
subcc_imm_Instr.setCode(opCodeExec, 'execute')
subcc_imm_Instr.addBehavior(WB_plain, 'wb')
subcc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
subcc_imm_Instr.addBehavior(ICC_writeSub, 'execute', False)
subcc_imm_Instr.addVariable(('result', 'BIT<32>'))
subcc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
subcc_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
subcc_imm_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(subcc_imm_Instr)
subcc_reg_Instr = trap.Instruction('SUBcc_reg', True, frequency = 8)
subcc_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 1, 0, 1, 0, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('subcc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
subcc_reg_Instr.setCode(opCodeRegsRegs, 'regs')
subcc_reg_Instr.setCode(opCodeExec, 'execute')
subcc_reg_Instr.addBehavior(WB_plain, 'wb')
subcc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
subcc_reg_Instr.addBehavior(ICC_writeSub, 'execute', False)
subcc_reg_Instr.addVariable(('result', 'BIT<32>'))
subcc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
subcc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
subcc_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(subcc_reg_Instr)
opCodeExec = cxx_writer.writer_code.Code("""
#ifndef ACC_MODEL
result = rs1_op - rs2_op - PSR[key_ICC_c];
#else
result = rs1_op - rs2_op - PSR_execute[key_ICC_c];
#endif
""")
subx_imm_Instr = trap.Instruction('SUBX_imm', True, frequency = 3)
subx_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 0, 1, 1, 0, 0]}, ('subx r', '%rs1', ' ', '%simm13', ' r', '%rd'))
subx_imm_Instr.setCode(opCodeRegsImm, 'regs')
subx_imm_Instr.setCode(opCodeExec, 'execute')
subx_imm_Instr.addBehavior(WB_plain, 'wb')
subx_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
subx_imm_Instr.addVariable(('result', 'BIT<32>'))
subx_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
subx_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
subx_imm_Instr.addSpecialRegister('PSR', 'in', 'execute')
isa.addInstruction(subx_imm_Instr)
subx_reg_Instr = trap.Instruction('SUBX_reg', True, frequency = 5)
subx_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 0, 1, 1, 0, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('subx r', '%rs1', ' r', '%rs2', ' r', '%rd'))
subx_reg_Instr.setCode(opCodeRegsRegs, 'regs')
subx_reg_Instr.setCode(opCodeExec, 'execute')
subx_reg_Instr.addBehavior(WB_plain, 'wb')
subx_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
subx_reg_Instr.addVariable(('result', 'BIT<32>'))
subx_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
subx_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
subx_reg_Instr.addSpecialRegister('PSR', 'in', 'execute')
isa.addInstruction(subx_reg_Instr)
subxcc_imm_Instr = trap.Instruction('SUBXcc_imm', True, frequency = 2)
subxcc_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 1, 1, 1, 0, 0]}, ('subxcc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
subxcc_imm_Instr.setCode(opCodeRegsImm, 'regs')
subxcc_imm_Instr.setCode(opCodeExec, 'execute')
subxcc_imm_Instr.addBehavior(WB_plain, 'wb')
subxcc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
subxcc_imm_Instr.addBehavior(ICC_writeSub, 'execute', False)
subxcc_imm_Instr.addVariable(('result', 'BIT<32>'))
subxcc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
subxcc_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
subxcc_imm_Instr.addSpecialRegister('PSR', 'inout', 'execute')
isa.addInstruction(subxcc_imm_Instr)
subxcc_reg_Instr = trap.Instruction('SUBXcc_reg', True, frequency = 2)
subxcc_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 1, 1, 1, 0, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('subxcc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
subxcc_reg_Instr.setCode(opCodeRegsRegs, 'regs')
subxcc_reg_Instr.setCode(opCodeExec, 'execute')
subxcc_reg_Instr.addBehavior(WB_plain, 'wb')
subxcc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
subxcc_reg_Instr.addBehavior(ICC_writeSub, 'execute', False)
subxcc_reg_Instr.addVariable(('result', 'BIT<32>'))
subxcc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
subxcc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
subxcc_reg_Instr.addSpecialRegister('PSR', 'inout', 'execute')
isa.addInstruction(subxcc_reg_Instr)
opCodeExec = cxx_writer.writer_code.Code("""
result = rs1_op - rs2_op;
temp_V = ((unsigned int)((rs1_op & (~rs2_op) & (~result)) | ((~rs1_op) & rs2_op & result))) >> 31;
if(!temp_V && (((rs1_op | rs2_op) & 0x00000003) != 0)){
    temp_V = 1;
}
""")
tsubcc_imm_Instr = trap.Instruction('TSUBcc_imm', True, frequency = 1)
tsubcc_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 0, 0, 0, 0, 1]}, ('tsubcc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
tsubcc_imm_Instr.setCode(opCodeRegsImm, 'regs')
tsubcc_imm_Instr.setCode(opCodeExec, 'execute')
tsubcc_imm_Instr.addBehavior(WB_plain, 'wb')
tsubcc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
tsubcc_imm_Instr.addBehavior(ICC_writeTSub, 'execute', False)
tsubcc_imm_Instr.addVariable(('result', 'BIT<32>'))
tsubcc_imm_Instr.addVariable(('temp_V', 'BIT<1>'))
tsubcc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
tsubcc_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
tsubcc_imm_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(tsubcc_imm_Instr)
tsubcc_reg_Instr = trap.Instruction('TSUBcc_reg', True, frequency = 1)
tsubcc_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 0, 0, 0, 0, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('tsubcc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
tsubcc_reg_Instr.setCode(opCodeRegsRegs, 'regs')
tsubcc_reg_Instr.setCode(opCodeExec, 'execute')
tsubcc_reg_Instr.addBehavior(WB_plain, 'wb')
tsubcc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
tsubcc_reg_Instr.addBehavior(ICC_writeTSub, 'execute', False)
tsubcc_reg_Instr.addVariable(('result', 'BIT<32>'))
tsubcc_reg_Instr.addVariable(('temp_V', 'BIT<1>'))
tsubcc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
tsubcc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
tsubcc_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(tsubcc_reg_Instr)
opCodeTrap = cxx_writer.writer_code.Code("""
if(temp_V){
    RaiseException(pcounter, npcounter, TAG_OVERFLOW);
}
""")
tsubcctv_imm_Instr = trap.Instruction('TSUBccTV_imm', True, frequency = 1)
tsubcctv_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 0, 0, 0, 1, 1]}, ('tsubcctv r', '%rs1', ' ', '%simm13', ' r', '%rd'))
tsubcctv_imm_Instr.setCode(opCodeRegsImm, 'regs')
tsubcctv_imm_Instr.setCode(opCodeExec, 'execute')
tsubcctv_imm_Instr.setCode(opCodeTrap, 'exception')
tsubcctv_imm_Instr.addBehavior(WB_tv, 'wb')
tsubcctv_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
tsubcctv_imm_Instr.addBehavior(ICC_writeTVSub, 'execute', False)
tsubcctv_imm_Instr.setCode(opCodeReadPC, 'fetch')
tsubcctv_imm_Instr.setCode(opCodeReadNPC, 'decode')
tsubcctv_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
tsubcctv_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
tsubcctv_imm_Instr.addVariable(('result', 'BIT<32>'))
tsubcctv_imm_Instr.addVariable(('temp_V', 'BIT<1>'))
tsubcctv_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
tsubcctv_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
tsubcctv_imm_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(tsubcctv_imm_Instr)
tsubcctv_reg_Instr = trap.Instruction('TSUBccTV_reg', True, frequency = 1)
tsubcctv_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 0, 0, 0, 1, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('tsubcctv r', '%rs1', ' r', '%rs2', ' r', '%rd'))
tsubcctv_reg_Instr.setCode(opCodeRegsRegs, 'regs')
tsubcctv_reg_Instr.setCode(opCodeExec, 'execute')
tsubcctv_reg_Instr.setCode(opCodeTrap, 'exception')
tsubcctv_reg_Instr.addBehavior(WB_tv, 'wb')
tsubcctv_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
tsubcctv_reg_Instr.addBehavior(ICC_writeTVSub, 'execute', False)
tsubcctv_reg_Instr.setCode(opCodeReadPC, 'fetch')
tsubcctv_reg_Instr.setCode(opCodeReadNPC, 'decode')
tsubcctv_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
tsubcctv_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
tsubcctv_reg_Instr.addVariable(('result', 'BIT<32>'))
tsubcctv_reg_Instr.addVariable(('temp_V', 'BIT<1>'))
tsubcctv_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
tsubcctv_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
tsubcctv_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
isa.addInstruction(tsubcctv_reg_Instr)

# Multiply Step
opCodeRegsImm = cxx_writer.writer_code.Code("""
rs1_op = rs1;
rs2_op = SignExtend(simm13, 13);
""")
opCodeRegsRegs = cxx_writer.writer_code.Code("""
rs1_op = rs1;
rs2_op = rs2;
""")
opCodeExec = cxx_writer.writer_code.Code("""
#ifndef ACC_MODEL
unsigned int yNew = (((unsigned int)Y) >> 1) | (rs1_op << 31);
#else
unsigned int yNew = (((unsigned int)Y_execute) >> 1) | (rs1_op << 31);
#endif
rs1_op = ((PSR[key_ICC_n] ^ PSR[key_ICC_v]) << 31) | (((unsigned int)rs1_op) >> 1);
result = rs1_op;
#ifndef ACC_MODEL
unsigned int yOld = Y;
#else
unsigned int yOld = Y_execute;
#endif
if((yOld & 0x00000001) != 0){
    result += rs2_op;
}
else{
    rs2_op = 0;
}
Y = yNew;
""")
mulscc_imm_Instr = trap.Instruction('MULScc_imm', True, frequency = 2)
mulscc_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 0, 0, 1, 0, 0]}, ('mulscc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
mulscc_imm_Instr.setCode(opCodeRegsImm, 'regs')
mulscc_imm_Instr.setCode(opCodeExec, 'execute')
mulscc_imm_Instr.addBehavior(WB_plain, 'wb')
mulscc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
mulscc_imm_Instr.addBehavior(ICC_writeAdd, 'execute', False)
mulscc_imm_Instr.addVariable(('result', 'BIT<32>'))
mulscc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
mulscc_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
mulscc_imm_Instr.addSpecialRegister('PSR', 'in')
mulscc_imm_Instr.addSpecialRegister('Y', 'inout', 'execute')
isa.addInstruction(mulscc_imm_Instr)
mulscc_reg_Instr = trap.Instruction('MULScc_reg', True, frequency = 2)
mulscc_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 0, 0, 1, 0, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('mulscc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
mulscc_reg_Instr.setCode(opCodeRegsRegs, 'regs')
mulscc_reg_Instr.setCode(opCodeExec, 'execute')
mulscc_reg_Instr.addBehavior(WB_plain, 'wb')
mulscc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
mulscc_reg_Instr.addBehavior(ICC_writeAdd, 'execute', False)
mulscc_reg_Instr.addVariable(('result', 'BIT<32>'))
mulscc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
mulscc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
mulscc_reg_Instr.addSpecialRegister('PSR', 'in')
mulscc_reg_Instr.addSpecialRegister('Y', 'inout', 'execute')
isa.addInstruction(mulscc_reg_Instr)

# Multiply
opCodeRegsImm = cxx_writer.writer_code.Code("""
rs1_op = rs1;
rs2_op = SignExtend(simm13, 13);
""")
opCodeRegsRegs = cxx_writer.writer_code.Code("""
rs1_op = rs1;
rs2_op = rs2;
""")
opCodeExecS = cxx_writer.writer_code.Code("""
long long resultTemp = (long long)(((long long)((int)rs1_op))*((long long)((int)rs2_op)));
Y = ((unsigned long long)resultTemp) >> 32;
result = resultTemp & 0x00000000FFFFFFFF;
#ifdef MULT_SIZE_16
stall(3);
#endif
""")
opCodeExecU = cxx_writer.writer_code.Code("""
unsigned long long resultTemp = (unsigned long long)(((unsigned long long)((unsigned int)rs1_op))*((unsigned long long)((unsigned int)rs2_op)));
Y = resultTemp >> 32;
result = resultTemp & 0x00000000FFFFFFFF;
#ifdef MULT_SIZE_16
stall(3);
#endif
""")
umul_imm_Instr = trap.Instruction('UMUL_imm', True, frequency = 2)
umul_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 0, 1, 0, 1, 0]}, ('umul r', '%rs1', ' ', '%simm13', ' r', '%rd'))
umul_imm_Instr.setCode(opCodeRegsImm, 'regs')
umul_imm_Instr.setCode(opCodeExecU, 'execute')
umul_imm_Instr.addBehavior(WB_plain, 'wb')
umul_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
umul_imm_Instr.addVariable(('result', 'BIT<32>'))
umul_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
umul_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
umul_imm_Instr.addSpecialRegister('Y', 'out', 'execute')
#if pipelinedMult:
    #umul_imm_Instr.setWbDelay('rd', 3)
#else:
    #umul_imm_Instr.setWbDelay('rd', 2)
isa.addInstruction(umul_imm_Instr)
umul_reg_Instr = trap.Instruction('UMUL_reg', True, frequency = 2)
umul_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 0, 1, 0, 1, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('umul r', '%rs1', ' r', '%rs2', ' r', '%rd'))
umul_reg_Instr.setCode(opCodeRegsRegs, 'regs')
umul_reg_Instr.setCode(opCodeExecU, 'execute')
umul_reg_Instr.addBehavior(WB_plain, 'wb')
umul_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
umul_reg_Instr.addVariable(('result', 'BIT<32>'))
umul_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
umul_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
umul_reg_Instr.addSpecialRegister('Y', 'out', 'execute')
#if pipelinedMult:
    #umul_reg_Instr.setWbDelay('rd', 3)
#else:
    #umul_reg_Instr.setWbDelay('rd', 2)
isa.addInstruction(umul_reg_Instr)
smul_imm_Instr = trap.Instruction('SMUL_imm', True, frequency = 3)
smul_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 0, 1, 0, 1, 1]}, ('smul r', '%rs1', ' ', '%simm13', ' r', '%rd'))
smul_imm_Instr.setCode(opCodeRegsImm, 'regs')
smul_imm_Instr.setCode(opCodeExecS, 'execute')
smul_imm_Instr.addBehavior(WB_plain, 'wb')
smul_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
smul_imm_Instr.addVariable(('result', 'BIT<32>'))
smul_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
smul_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
smul_imm_Instr.addSpecialRegister('Y', 'out', 'execute')
#if pipelinedMult:
    #smul_imm_Instr.setWbDelay('rd', 3)
#else:
    #smul_imm_Instr.setWbDelay('rd', 2)
isa.addInstruction(smul_imm_Instr)
smul_reg_Instr = trap.Instruction('SMUL_reg', True, frequency = 4)
smul_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 0, 1, 0, 1, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('smul r', '%rs1', ' r', '%rs2', ' r', '%rd'))
smul_reg_Instr.setCode(opCodeRegsRegs, 'regs')
smul_reg_Instr.setCode(opCodeExecS, 'execute')
smul_reg_Instr.addBehavior(WB_plain, 'wb')
smul_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
smul_reg_Instr.addVariable(('result', 'BIT<32>'))
smul_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
smul_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
smul_reg_Instr.addSpecialRegister('Y', 'out', 'execute')
#if pipelinedMult:
    #smul_reg_Instr.setWbDelay('rd', 3)
#else:
    #smul_reg_Instr.setWbDelay('rd', 2)
isa.addInstruction(smul_reg_Instr)
umulcc_imm_Instr = trap.Instruction('UMULcc_imm', True, frequency = 2)
umulcc_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 1, 1, 0, 1, 0]}, ('umulcc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
umulcc_imm_Instr.setCode(opCodeRegsImm, 'regs')
umulcc_imm_Instr.setCode(opCodeExecU, 'execute')
umulcc_imm_Instr.addBehavior(WB_plain, 'wb')
umulcc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
umulcc_imm_Instr.addBehavior(ICC_writeLogic, 'execute', False)
umulcc_imm_Instr.addVariable(('result', 'BIT<32>'))
umulcc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
umulcc_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
umulcc_imm_Instr.addSpecialRegister('Y', 'out', 'execute')
umulcc_imm_Instr.addSpecialRegister('PSR', 'out', 'execute')
#if pipelinedMult:
    #umulcc_imm_Instr.setWbDelay('rd', 3)
#else:
    #umulcc_imm_Instr.setWbDelay('rd', 2)
isa.addInstruction(umulcc_imm_Instr)
umulcc_reg_Instr = trap.Instruction('UMULcc_reg', True, frequency = 2)
umulcc_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 1, 1, 0, 1, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('umulcc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
umulcc_reg_Instr.setCode(opCodeRegsRegs, 'regs')
umulcc_reg_Instr.setCode(opCodeExecU, 'execute')
umulcc_reg_Instr.addBehavior(WB_plain, 'wb')
umulcc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
umulcc_reg_Instr.addBehavior(ICC_writeLogic, 'execute', False)
umulcc_reg_Instr.addVariable(('result', 'BIT<32>'))
umulcc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
umulcc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
umulcc_reg_Instr.addSpecialRegister('Y', 'out', 'execute')
umulcc_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
#if pipelinedMult:
    #umulcc_reg_Instr.setWbDelay('rd', 3)
#else:
    #umulcc_reg_Instr.setWbDelay('rd', 2)
isa.addInstruction(umulcc_reg_Instr)
smulcc_imm_Instr = trap.Instruction('SMULcc_imm', True, frequency = 2)
smulcc_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 1, 1, 0, 1, 1]}, ('smulcc r', '%rs1', ' ', '%simm13', ' r', '%rd'))
smulcc_imm_Instr.setCode(opCodeRegsImm, 'regs')
smulcc_imm_Instr.setCode(opCodeExecS, 'execute')
smulcc_imm_Instr.addBehavior(WB_plain, 'wb')
smulcc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
smulcc_imm_Instr.addBehavior(ICC_writeLogic, 'execute', False)
smulcc_imm_Instr.addVariable(('result', 'BIT<32>'))
smulcc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
smulcc_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
smulcc_imm_Instr.addSpecialRegister('Y', 'out', 'execute')
smulcc_imm_Instr.addSpecialRegister('PSR', 'out', 'execute')
#if pipelinedMult:
    #smulcc_imm_Instr.setWbDelay('rd', 3)
#else:
    #smulcc_imm_Instr.setWbDelay('rd', 2)
isa.addInstruction(smulcc_imm_Instr)
smulcc_reg_Instr = trap.Instruction('SMULcc_reg', True, frequency = 2)
smulcc_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 1, 1, 0, 1, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('smulcc r', '%rs1', ' r', '%rs2', ' r', '%rd'))
smulcc_reg_Instr.setCode(opCodeRegsRegs, 'regs')
smulcc_reg_Instr.setCode(opCodeExecS, 'execute')
smulcc_reg_Instr.addBehavior(WB_plain, 'wb')
smulcc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
smulcc_reg_Instr.addBehavior(ICC_writeLogic, 'execute', False)
smulcc_reg_Instr.addVariable(('result', 'BIT<32>'))
smulcc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
smulcc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
smulcc_reg_Instr.addSpecialRegister('Y', 'out', 'execute')
smulcc_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
#if pipelinedMult:
    #smulcc_reg_Instr.setWbDelay('rd', 3)
#else:
    #smulcc_reg_Instr.setWbDelay('rd', 2)
isa.addInstruction(smulcc_reg_Instr)

# Multiply Accumulate Instructions
opCodeRegsImm = cxx_writer.writer_code.Code("""
rs1_op = rs1;
rs2_op = SignExtend(simm13, 13);
""")
opCodeRegsRegs = cxx_writer.writer_code.Code("""
rs1_op = rs1;
rs2_op = rs2;
""")
opCodeExecS = cxx_writer.writer_code.Code("""
int resultTemp = ((int)SignExtend(rs1_op & 0x0000ffff, 16))*((int)SignExtend(rs2_op & 0x0000ffff, 16));
long long resultAcc = ((((long long)(Y & 0x000000ff)) << 32) | (int)ASR[18]) + resultTemp;
Y = (resultAcc & 0x000000ff00000000LL) >> 32;
ASR[18] = resultAcc & 0x00000000FFFFFFFFLL;
result = resultAcc & 0x00000000FFFFFFFFLL;
stall(1);
""")
opCodeExecU = cxx_writer.writer_code.Code("""
unsigned int resultTemp = ((unsigned int)rs1_op & 0x0000ffff)*((unsigned int)rs2_op & 0x0000ffff);
unsigned long long resultAcc = ((((unsigned long long)(Y & 0x000000ff)) << 32) | (unsigned int)ASR[18]) + resultTemp;
Y = (resultAcc & 0x000000ff00000000LL) >> 32;
ASR[18] = resultAcc & 0x00000000FFFFFFFFLL;
result = resultAcc & 0x00000000FFFFFFFFLL;
stall(1);
""")
umac_imm_Instr = trap.Instruction('UMAC_imm', True, frequency = 1)
umac_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 1, 1, 1, 1, 0]}, ('umac r', '%rs1', ' ', '%simm13', ' r', '%rd'))
umac_imm_Instr.setCode(opCodeRegsImm, 'regs')
umac_imm_Instr.setCode(opCodeExecU, 'execute')
umac_imm_Instr.addBehavior(WB_plain, 'wb')
umac_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
umac_imm_Instr.addVariable(('result', 'BIT<32>'))
umac_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
umac_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
umac_imm_Instr.addSpecialRegister('Y', 'inout', 'execute')
umac_imm_Instr.addSpecialRegister('ASR[18]', 'inout', 'execute')
isa.addInstruction(umac_imm_Instr)
umac_reg_Instr = trap.Instruction('UMAC_reg', True, frequency = 1)
umac_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 1, 1, 1, 1, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('umac r', '%rs1', ' r', '%rs2', ' r', '%rd'))
umac_reg_Instr.setCode(opCodeRegsRegs, 'regs')
umac_reg_Instr.setCode(opCodeExecU, 'execute')
umac_reg_Instr.addBehavior(WB_plain, 'wb')
umac_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
umac_reg_Instr.addVariable(('result', 'BIT<32>'))
umac_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
umac_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
umac_reg_Instr.addSpecialRegister('Y', 'inout', 'execute')
umac_reg_Instr.addSpecialRegister('ASR[18]', 'inout', 'execute')
isa.addInstruction(umac_reg_Instr)
smac_imm_Instr = trap.Instruction('SMAC_imm', True, frequency = 1)
smac_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 1, 1, 1, 1, 1]}, ('smac r', '%rs1', ' ', '%simm13', ' r', '%rd'))
smac_imm_Instr.setCode(opCodeRegsImm, 'regs')
smac_imm_Instr.setCode(opCodeExecS, 'execute')
smac_imm_Instr.addBehavior(WB_plain, 'wb')
smac_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
smac_imm_Instr.addVariable(('result', 'BIT<32>'))
smac_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
smac_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
smac_imm_Instr.addSpecialRegister('Y', 'inout', 'execute')
smac_imm_Instr.addSpecialRegister('ASR[18]', 'inout', 'execute')
isa.addInstruction(smac_imm_Instr)
smac_reg_Instr = trap.Instruction('SMAC_reg', True, frequency = 1)
smac_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 1, 1, 1, 1, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('smac r', '%rs1', ' r', '%rs2', ' r', '%rd'))
smac_reg_Instr.setCode(opCodeRegsRegs, 'regs')
smac_reg_Instr.setCode(opCodeExecS, 'execute')
smac_reg_Instr.addBehavior(WB_plain, 'wb')
smac_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
smac_reg_Instr.addVariable(('result', 'BIT<32>'))
smac_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
smac_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
smac_reg_Instr.addSpecialRegister('Y', 'inout', 'execute')
smac_reg_Instr.addSpecialRegister('ASR[18]', 'inout', 'execute')
isa.addInstruction(smac_reg_Instr)

# Divide
opCodeRegsImm = cxx_writer.writer_code.Code("""
rs1_op = rs1;
rs2_op = SignExtend(simm13, 13);
""")
opCodeRegsRegs = cxx_writer.writer_code.Code("""
rs1_op = rs1;
rs2_op = rs2;
""")
opCodeExecU = cxx_writer.writer_code.Code("""
exception = rs2_op == 0;
if(!exception){
    #ifndef ACC_MODEL
    unsigned long long res64 = ((unsigned long long)((((unsigned long long)Y) << 32) | (unsigned long long)rs1_op))/(unsigned long long)rs2_op;
    #else
    unsigned long long res64 = ((unsigned long long)((((unsigned long long)Y_execute) << 32) | (unsigned long long)rs1_op))/(unsigned long long)rs2_op;
    #endif
    temp_V = (res64 & 0xFFFFFFFF00000000LL) != 0;
    if(temp_V){
        result = 0xFFFFFFFF;
    }
    else{
        result = (unsigned int)(res64 & 0x00000000FFFFFFFFLL);
    }
}
stall(34);
""")
opCodeExecS = cxx_writer.writer_code.Code("""
exception = rs2_op == 0;
if(!exception){
    #ifndef ACC_MODEL
    long long res64 = ((long long)((((unsigned long long)Y) << 32) | (unsigned long long)rs1_op))/((long long)((int)rs2_op));
    #else
    long long res64 = ((long long)((((unsigned long long)Y_execute) << 32) | (unsigned long long)rs1_op))/((long long)((int)rs2_op));
    #endif
    temp_V = (res64 & 0xFFFFFFFF80000000LL) != 0 && (res64 & 0xFFFFFFFF80000000LL) != 0xFFFFFFFF80000000LL;
    if(temp_V){
        if(res64 > 0){
            result = 0x7FFFFFFF;
        }
        else{
            result = 0x80000000;
        }
    }
    else{
        result = (unsigned int)(res64 & 0x00000000FFFFFFFFLL);
    }
}
stall(34);
""")
opCodeTrap = cxx_writer.writer_code.Code("""
if(exception){
    RaiseException(pcounter, npcounter, DIV_ZERO);
}
""")
udiv_imm_Instr = trap.Instruction('UDIV_imm', True, frequency = 2)
udiv_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 0, 1, 1, 1, 0]}, ('udiv', ' r', '%rs1', ' ', '%simm13', ' r', '%rd'))
udiv_imm_Instr.setCode(opCodeRegsImm, 'regs')
udiv_imm_Instr.setCode(opCodeExecU, 'execute')
udiv_imm_Instr.setCode(opCodeTrap, 'exception')
udiv_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
udiv_imm_Instr.addBehavior(WB_plain, 'wb')
udiv_imm_Instr.setCode(opCodeReadPC, 'fetch')
udiv_imm_Instr.setCode(opCodeReadNPC, 'decode')
udiv_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
udiv_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
udiv_imm_Instr.addVariable(('exception', 'BIT<1>'))
udiv_imm_Instr.addVariable(('temp_V', 'BIT<1>'))
udiv_imm_Instr.addVariable(('result', 'BIT<32>'))
udiv_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
udiv_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
udiv_imm_Instr.addSpecialRegister('Y', 'in', 'execute')
udiv_imm_Instr.setWbDelay('rd', 33)
isa.addInstruction(udiv_imm_Instr)
udiv_reg_Instr = trap.Instruction('UDIV_reg', True, frequency = 2)
udiv_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 0, 1, 1, 1, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('udiv', ' r', '%rs1', ' r', '%rs2', ' r', '%rd'))
udiv_reg_Instr.setCode(opCodeRegsRegs, 'regs')
udiv_reg_Instr.setCode(opCodeExecU, 'execute')
udiv_reg_Instr.setCode(opCodeTrap, 'exception')
udiv_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
udiv_reg_Instr.addBehavior(WB_plain, 'wb')
udiv_reg_Instr.setCode(opCodeReadPC, 'fetch')
udiv_reg_Instr.setCode(opCodeReadNPC, 'decode')
udiv_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
udiv_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
udiv_reg_Instr.addVariable(('exception', 'BIT<1>'))
udiv_reg_Instr.addVariable(('temp_V', 'BIT<1>'))
udiv_reg_Instr.addVariable(('result', 'BIT<32>'))
udiv_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
udiv_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
udiv_reg_Instr.addSpecialRegister('Y', 'in', 'execute')
udiv_reg_Instr.setWbDelay('rd', 33)
isa.addInstruction(udiv_reg_Instr)
sdiv_imm_Instr = trap.Instruction('SDIV_imm', True, frequency = 2)
sdiv_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 0, 1, 1, 1, 1]}, ('sdiv', ' r', '%rs1', ' ', '%simm13', '%rd'))
sdiv_imm_Instr.setCode(opCodeRegsImm, 'regs')
sdiv_imm_Instr.setCode(opCodeExecS, 'execute')
sdiv_imm_Instr.setCode(opCodeTrap, 'exception')
sdiv_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
sdiv_imm_Instr.addBehavior(WB_plain, 'wb')
sdiv_imm_Instr.setCode(opCodeReadPC, 'fetch')
sdiv_imm_Instr.setCode(opCodeReadNPC, 'decode')
sdiv_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
sdiv_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
sdiv_imm_Instr.addVariable(('exception', 'BIT<1>'))
sdiv_imm_Instr.addVariable(('temp_V', 'BIT<1>'))
sdiv_imm_Instr.addVariable(('result', 'BIT<32>'))
sdiv_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
sdiv_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
sdiv_imm_Instr.addSpecialRegister('Y', 'in', 'execute')
sdiv_imm_Instr.setWbDelay('rd', 33)
isa.addInstruction(sdiv_imm_Instr)
sdiv_reg_Instr = trap.Instruction('SDIV_reg', True, frequency = 2)
sdiv_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 0, 1, 1, 1, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('sdiv', ' r', '%rs1', ' r', '%rs2', '%rd'))
sdiv_reg_Instr.setCode(opCodeRegsRegs, 'regs')
sdiv_reg_Instr.setCode(opCodeExecS, 'execute')
sdiv_reg_Instr.setCode(opCodeTrap, 'exception')
sdiv_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
sdiv_reg_Instr.addBehavior(WB_plain, 'wb')
sdiv_reg_Instr.setCode(opCodeReadPC, 'fetch')
sdiv_reg_Instr.setCode(opCodeReadNPC, 'decode')
sdiv_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
sdiv_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
sdiv_reg_Instr.addVariable(('exception', 'BIT<1>'))
sdiv_reg_Instr.addVariable(('temp_V', 'BIT<1>'))
sdiv_reg_Instr.addVariable(('result', 'BIT<32>'))
sdiv_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
sdiv_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
sdiv_reg_Instr.addSpecialRegister('Y', 'in', 'execute')
sdiv_reg_Instr.setWbDelay('rd', 33)
isa.addInstruction(sdiv_reg_Instr)
udivcc_imm_Instr = trap.Instruction('UDIVcc_imm', True, frequency = 1)
udivcc_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 1, 1, 1, 1, 0]}, ('udivcc', ' r', '%rs1', ' ', '%simm13', '%rd'))
udivcc_imm_Instr.setCode(opCodeRegsImm, 'regs')
udivcc_imm_Instr.setCode(opCodeExecU, 'execute')
udivcc_imm_Instr.setCode(opCodeTrap, 'exception')
udivcc_imm_Instr.addBehavior(ICC_writeDiv, 'execute', False)
udivcc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
udivcc_imm_Instr.addBehavior(WB_plain, 'wb')
udivcc_imm_Instr.setCode(opCodeReadPC, 'fetch')
udivcc_imm_Instr.setCode(opCodeReadNPC, 'decode')
udivcc_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
udivcc_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
udivcc_imm_Instr.addVariable(('exception', 'BIT<1>'))
udivcc_imm_Instr.addVariable(('temp_V', 'BIT<1>'))
udivcc_imm_Instr.addVariable(('result', 'BIT<32>'))
udivcc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
udivcc_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
udivcc_imm_Instr.addSpecialRegister('Y', 'in', 'execute')
udivcc_imm_Instr.addSpecialRegister('PSR', 'inout', 'execute')
udivcc_imm_Instr.setWbDelay('rd', 33)
isa.addInstruction(udivcc_imm_Instr)
udivcc_reg_Instr = trap.Instruction('UDIVcc_reg', True, frequency = 1)
udivcc_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 1, 1, 1, 1, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('udivcc', ' r', '%rs1', ' r', '%rs2', '%rd'))
udivcc_reg_Instr.setCode(opCodeRegsRegs, 'regs')
udivcc_reg_Instr.setCode(opCodeExecU, 'execute')
udivcc_reg_Instr.setCode(opCodeTrap, 'exception')
udivcc_reg_Instr.addBehavior(ICC_writeDiv, 'execute', False)
udivcc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
udivcc_reg_Instr.addBehavior(WB_plain, 'wb')
udivcc_reg_Instr.setCode(opCodeReadPC, 'fetch')
udivcc_reg_Instr.setCode(opCodeReadNPC, 'decode')
udivcc_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
udivcc_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
udivcc_reg_Instr.addVariable(('exception', 'BIT<1>'))
udivcc_reg_Instr.addVariable(('temp_V', 'BIT<1>'))
udivcc_reg_Instr.addVariable(('result', 'BIT<32>'))
udivcc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
udivcc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
udivcc_reg_Instr.addSpecialRegister('Y', 'in', 'execute')
udivcc_reg_Instr.addSpecialRegister('PSR', 'inout', 'execute')
udivcc_reg_Instr.setWbDelay('rd', 33)
isa.addInstruction(udivcc_reg_Instr)
sdivcc_imm_Instr = trap.Instruction('SDIVcc_imm', True, frequency = 1)
sdivcc_imm_Instr.setMachineCode(dpi_format2, {'op3': [0, 1, 1, 1, 1, 1]}, ('sdivcc', ' r', '%rs1', ' ', '%simm13', '%rd'))
sdivcc_imm_Instr.setCode(opCodeRegsImm, 'regs')
sdivcc_imm_Instr.setCode(opCodeExecS, 'execute')
sdivcc_imm_Instr.setCode(opCodeTrap, 'exception')
sdivcc_imm_Instr.addBehavior(ICC_writeDiv, 'execute', False)
sdivcc_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
sdivcc_imm_Instr.addBehavior(WB_plain, 'wb')
sdivcc_imm_Instr.setCode(opCodeReadPC, 'fetch')
sdivcc_imm_Instr.setCode(opCodeReadNPC, 'decode')
sdivcc_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
sdivcc_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
sdivcc_imm_Instr.addVariable(('exception', 'BIT<1>'))
sdivcc_imm_Instr.addVariable(('temp_V', 'BIT<1>'))
sdivcc_imm_Instr.addVariable(('result', 'BIT<32>'))
sdivcc_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
sdivcc_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
sdivcc_imm_Instr.addSpecialRegister('Y', 'in', 'execute')
sdivcc_imm_Instr.addSpecialRegister('PSR', 'inout', 'execute')
sdivcc_imm_Instr.setWbDelay('rd', 33)
isa.addInstruction(sdivcc_imm_Instr)
sdivcc_reg_Instr = trap.Instruction('SDIVcc_reg', True, frequency = 1)
sdivcc_reg_Instr.setMachineCode(dpi_format1, {'op3': [0, 1, 1, 1, 1, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('sdivcc', ' r', '%rs1', ' r', '%rs2', '%rd'))
sdivcc_reg_Instr.setCode(opCodeRegsRegs, 'regs')
sdivcc_reg_Instr.setCode(opCodeExecS, 'execute')
sdivcc_reg_Instr.setCode(opCodeTrap, 'exception')
sdivcc_reg_Instr.addBehavior(ICC_writeDiv, 'execute', False)
sdivcc_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
sdivcc_reg_Instr.addBehavior(WB_plain, 'wb')
sdivcc_reg_Instr.setCode(opCodeReadPC, 'fetch')
sdivcc_reg_Instr.setCode(opCodeReadNPC, 'decode')
sdivcc_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
sdivcc_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
sdivcc_reg_Instr.addVariable(('exception', 'BIT<1>'))
sdivcc_reg_Instr.addVariable(('temp_V', 'BIT<1>'))
sdivcc_reg_Instr.addVariable(('result', 'BIT<32>'))
sdivcc_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
sdivcc_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
sdivcc_reg_Instr.addSpecialRegister('Y', 'in', 'execute')
sdivcc_reg_Instr.addSpecialRegister('PSR', 'inout', 'execute')
sdivcc_reg_Instr.setWbDelay('rd', 33)
isa.addInstruction(sdivcc_reg_Instr)

# Save and Restore
opCodeDec = """
okNewWin = DecrementRegWindow();
#ifdef ACC_MODEL
if(!okNewWin){
    flush();
}
else{
    rd.lock();
}
#endif
"""
opCodeDecRegs = cxx_writer.writer_code.Code(ReadNPCDecode + 'result = rs1 + rs2;\n' + opCodeDec)
opCodeDecImm = cxx_writer.writer_code.Code(ReadNPCDecode + 'result = rs1 + SignExtend(simm13, 13);\n' + opCodeDec)

opCodeFlush = cxx_writer.writer_code.Code("""
#ifdef ACC_MODEL
if(!okNewWin){
    flush();
}
#endif
""")
opCodeTrap = cxx_writer.writer_code.Code("""
if(!okNewWin){
    RaiseException(pcounter, npcounter, WINDOW_OVERFLOW);
}
""")
opCodeWb = cxx_writer.writer_code.Code("""
if(okNewWin){
    rd = result;
    #ifdef ACC_MODEL
    unlockQueue[0].push_back(rd.getPipeReg());
    #endif
}
""")
save_imm_Instr = trap.Instruction('SAVE_imm', True, frequency = 6)
save_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 1, 1, 1, 0, 0]}, ('save', ' r', '%rs1', ' ', '%simm13', ' r', '%rd'))
save_imm_Instr.setCode(opCodeDecImm, 'decode')
save_imm_Instr.setCode(opCodeFlush, 'regs')
save_imm_Instr.setCode(opCodeFlush, 'execute')
save_imm_Instr.setCode(opCodeFlush, 'memory')
save_imm_Instr.setCode(opCodeTrap, 'exception')
save_imm_Instr.setCode(opCodeWb, 'wb')
save_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
save_imm_Instr.setCode(opCodeReadPC, 'fetch')
save_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
save_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
save_imm_Instr.addVariable(('okNewWin', 'BIT<1>'))
save_imm_Instr.addVariable(('result', 'BIT<32>'))
save_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
save_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
save_imm_Instr.addVariable(('newCwp', 'BIT<32>'))
save_imm_Instr.removeLockRegRegister('rd')
save_imm_Instr.addSpecialRegister('PSR', 'inout')
isa.addInstruction(save_imm_Instr)
save_reg_Instr = trap.Instruction('SAVE_reg', True, frequency = 2)
save_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 1, 1, 1, 0, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('save', ' r', '%rs1', ' r', '%rs2', ' r', '%rd'))
save_reg_Instr.setCode(opCodeDecRegs, 'decode')
save_reg_Instr.setCode(opCodeFlush, 'regs')
save_reg_Instr.setCode(opCodeFlush, 'execute')
save_reg_Instr.setCode(opCodeFlush, 'memory')
save_reg_Instr.setCode(opCodeTrap, 'exception')
save_reg_Instr.setCode(opCodeWb, 'wb')
save_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
save_reg_Instr.setCode(opCodeReadPC, 'fetch')
save_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
save_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
save_reg_Instr.addVariable(('okNewWin', 'BIT<1>'))
save_reg_Instr.addVariable(('result', 'BIT<32>'))
save_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
save_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
save_reg_Instr.addVariable(('newCwp', 'BIT<32>'))
save_reg_Instr.removeLockRegRegister('rd')
save_reg_Instr.addSpecialRegister('PSR', 'inout')
isa.addInstruction(save_reg_Instr)

opCodeDec = """
okNewWin = IncrementRegWindow();
#ifdef ACC_MODEL
if(!okNewWin){
    flush();
}
else{
    rd.lock();
}
#endif
"""
opCodeDecRegs = cxx_writer.writer_code.Code(ReadNPCDecode + 'result = rs1 + rs2;\n' + opCodeDec)
opCodeDecImm = cxx_writer.writer_code.Code(ReadNPCDecode + 'result = rs1 + SignExtend(simm13, 13);\n' + opCodeDec)

opCodeTrap = cxx_writer.writer_code.Code("""
if(!okNewWin){
    RaiseException(pcounter, npcounter, WINDOW_UNDERFLOW);
}
""")
restore_imm_Instr = trap.Instruction('RESTORE_imm', True, frequency = 2)
restore_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 1, 1, 1, 0, 1]}, ('restore', ' r', '%rs1', ' ', '%simm13', ' r', '%rd'))
restore_imm_Instr.setCode(opCodeDecImm, 'decode')
restore_imm_Instr.setCode(opCodeFlush, 'regs')
restore_imm_Instr.setCode(opCodeFlush, 'execute')
restore_imm_Instr.setCode(opCodeFlush, 'memory')
restore_imm_Instr.setCode(opCodeTrap, 'exception')
restore_imm_Instr.setCode(opCodeWb, 'wb')
restore_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
restore_imm_Instr.setCode(opCodeReadPC, 'fetch')
restore_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
restore_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
restore_imm_Instr.addVariable(('okNewWin', 'BIT<1>'))
restore_imm_Instr.addVariable(('result', 'BIT<32>'))
restore_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
restore_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
restore_imm_Instr.addVariable(('newCwp', 'BIT<32>'))
restore_imm_Instr.removeLockRegRegister('rd')
restore_imm_Instr.addSpecialRegister('PSR', 'inout')
isa.addInstruction(restore_imm_Instr)
restore_reg_Instr = trap.Instruction('RESTORE_reg', True, frequency = 6)
restore_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 1, 1, 1, 0, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('restore', ' r', '%rs1', ' r', '%rs2', ' r', '%rd'))
restore_reg_Instr.setCode(opCodeDecRegs, 'decode')
restore_reg_Instr.setCode(opCodeFlush, 'regs')
restore_reg_Instr.setCode(opCodeFlush, 'execute')
restore_reg_Instr.setCode(opCodeFlush, 'memory')
restore_reg_Instr.setCode(opCodeTrap, 'exception')
restore_reg_Instr.setCode(opCodeWb, 'wb')
restore_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
restore_reg_Instr.setCode(opCodeReadPC, 'fetch')
restore_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
restore_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
restore_reg_Instr.addVariable(('okNewWin', 'BIT<1>'))
restore_reg_Instr.addVariable(('result', 'BIT<32>'))
restore_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
restore_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
restore_reg_Instr.addVariable(('newCwp', 'BIT<32>'))
restore_reg_Instr.removeLockRegRegister('rd')
restore_reg_Instr.addSpecialRegister('PSR', 'inout')
isa.addInstruction(restore_reg_Instr)

# Branch on Integer Condition Codes
opCode = cxx_writer.writer_code.Code(ReadNPCDecode + """
switch(cond){
    case 0x8:{
        // Branch Always
        unsigned int targetPc = pcounter + 4*(SignExtend(disp22, 22));
        #ifdef ACC_MODEL
        PC = targetPc;
        NPC = targetPc + 4;
        if(a == 1){
            flush();
        }
        #else
        if(a == 1){
            PC = targetPc;
            NPC = targetPc + 4;
        }
        else{
            PC = npcounter;
            NPC = targetPc;
        }
        #endif
    break;}
    case 0:{
        // Branch Never
        #ifdef ACC_MODEL
        if(a == 1){
            flush();
        }
        #else
        if(a == 1){
            PC = npcounter + 4;
            NPC = npcounter + 8;
        }
        else{
            PC = npcounter;
            NPC = npcounter + 4;
        }
        #endif
    break;}
    default:{
        #ifndef ACC_MODEL
        bool icc_z = PSR[key_ICC_z];
        bool icc_n = PSR[key_ICC_n];
        bool icc_v = PSR[key_ICC_v];
        bool icc_c = PSR[key_ICC_c];
        #else
        bool icc_z = PSR_execute[key_ICC_z];
        bool icc_n = PSR_execute[key_ICC_n];
        bool icc_v = PSR_execute[key_ICC_v];
        bool icc_c = PSR_execute[key_ICC_c];
        #endif
        // All the other non-special situations
        bool exec = ((cond == 0x9) && !icc_z) ||
                    ((cond == 0x1) && icc_z) ||
                    ((cond == 0xa) && !icc_z && (icc_n == icc_v)) ||
                    ((cond == 0x2) && (icc_z || (icc_n != icc_v))) ||
                    ((cond == 0xb) && (icc_n == icc_v)) ||
                    ((cond == 0x3) && (icc_n != icc_v)) ||
                    ((cond == 0xc) && !icc_c && !icc_z) ||
                    ((cond == 0x4) && (icc_c || icc_z)) ||
                    ((cond == 0xd) && !icc_c) ||
                    ((cond == 0x5) && icc_c) ||
                    ((cond == 0xe) && !icc_n) ||
                    ((cond == 0x6) && icc_n) ||
                    ((cond == 0xf) && !icc_v) ||
                    ((cond == 0x7) && icc_v);
        if(exec){
            unsigned int targetPc = pcounter + 4*(SignExtend(disp22, 22));
            #ifdef ACC_MODEL
            PC = targetPc;
            NPC = targetPc + 4;
            #else
            PC = npcounter;
            NPC = targetPc;
            #endif
        }
        else{
            if(a == 1){
                #ifdef ACC_MODEL
                flush();
                #else
                PC = npcounter + 4;
                NPC = npcounter + 8;
                #endif
            }
            #ifndef ACC_MODEL
            else{
                PC = npcounter;
                NPC = npcounter + 4;
            }
            #endif
        }
    break;}
}
""")
branch_Instr = trap.Instruction('BRANCH', True, frequency = 13)
branch_Instr.setMachineCode(b_sethi_format2, {'op2' : [0, 1, 0]},
('b', ('%cond', {int('1000', 2) : 'a',
int('0000', 2) : 'n', int('1001', 2) : 'ne', int('0001', 2) : 'e', int('1010', 2) : 'g', int('0010', 2) : 'le',
int('1011', 2) : 'ge', int('0011', 2) : 'l', int('1100', 2) : 'gu', int('0100', 2) : 'leu', int('1101', 2) : 'cc',
int('01010', 2) : 'cs', int('1110', 2) : 'pos', int('0110', 2) : 'neg', int('1111', 2) : 'vc', int('0111', 2) : 'vs',}),
('%a', {1: ',a'}), ' ', '%disp22'))
branch_Instr.setCode(opCode, 'decode')
branch_Instr.addBehavior(IncrementPC, 'fetch', pre = False, functionalModel = False)
branch_Instr.setCode(opCodeReadPC, 'fetch')
branch_Instr.addVariable(('pcounter', 'BIT<32>'))
branch_Instr.addVariable(('npcounter', 'BIT<32>'))
branch_Instr.addSpecialRegister('PSR', 'in', 'execute')
isa.addInstruction(branch_Instr)

# Call and Link
opCodeWb = cxx_writer.writer_code.Code("""
REGS[15] = pcounter;
""")
opCode = cxx_writer.writer_code.Code(ReadNPCDecode + """
unsigned int target = pcounter + (disp30 << 2);
#ifdef ACC_MODEL
PC = target;
NPC = target + 4;
#else
PC = npcounter;
NPC = target;
#endif
""")
call_Instr = trap.Instruction('CALL', True, frequency = 8)
call_Instr.setMachineCode(call_format, {}, ('call ', '%disp30'))
call_Instr.setCode(opCode, 'decode')
call_Instr.setCode(opCodeWb, 'wb')
call_Instr.addBehavior(IncrementPC, 'fetch', pre = False, functionalModel = False)
call_Instr.setCode(opCodeReadPC, 'fetch')
call_Instr.addVariable(('pcounter', 'BIT<32>'))
call_Instr.addVariable(('npcounter', 'BIT<32>'))
call_Instr.addSpecialRegister('REGS[15]', 'out')
call_Instr.addVariable(('oldPC', 'BIT<32>'))
isa.addInstruction(call_Instr)

# Jump and Link
opCodeWb = cxx_writer.writer_code.Code("""
if(!trapNotAligned){
    rd = pcounter;
}
""")
actualJumpCode = """if((jumpAddr & 0x00000003) != 0){
    trapNotAligned = true;
}
else{
    trapNotAligned = false;
    #ifdef ACC_MODEL
    PC = jumpAddr;
    NPC = jumpAddr + 4;
    #else
    PC = npcounter;
    NPC = jumpAddr;
    #endif
}
"""
opCodeDecodeImm = cxx_writer.writer_code.Code(ReadNPCDecode + """
unsigned int jumpAddr = rs1 + SignExtend(simm13, 13);
""" + actualJumpCode)
opCodeDecodeRegs = cxx_writer.writer_code.Code(ReadNPCDecode + """
unsigned int jumpAddr = rs1 + rs2;
""" + actualJumpCode)
opCodeTrap = cxx_writer.writer_code.Code("""
if(trapNotAligned){
    RaiseException(pcounter, npcounter, MEM_ADDR_NOT_ALIGNED);
}
""")
opCodeExec = cxx_writer.writer_code.Code("""
stall(2);
""")
jump_imm_Instr = trap.Instruction('JUMP_imm', True, frequency = 7)
jump_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 1, 1, 0, 0, 0]}, ('jmpl', ' r', '%rs1', '+', '%simm13', ' r', '%rd'))
jump_imm_Instr.setCode(opCodeDecodeImm, 'decode')
jump_imm_Instr.setCode(opCodeTrap, 'exception')
jump_imm_Instr.setCode(opCodeWb, 'wb')
jump_imm_Instr.setCode(opCodeExec, 'execute')
jump_imm_Instr.setCode(opCodeReadPC, 'fetch')
jump_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
jump_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
jump_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False, functionalModel = False)
jump_imm_Instr.addVariable(cxx_writer.writer_code.Variable('trapNotAligned', cxx_writer.writer_code.boolType))
isa.addInstruction(jump_imm_Instr)
jump_reg_Instr = trap.Instruction('JUMP_reg', True, frequency = 3)
jump_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 1, 1, 0, 0, 0], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('jmpl', ' r', '%rs1', '+r', '%rs2', ' r', '%rd'))
jump_reg_Instr.setCode(opCodeDecodeRegs, 'decode')
jump_reg_Instr.setCode(opCodeTrap, 'exception')
jump_reg_Instr.setCode(opCodeExec, 'execute')
jump_reg_Instr.setCode(opCodeWb, 'wb')
jump_reg_Instr.setCode(opCodeReadPC, 'fetch')
jump_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
jump_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
jump_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False, functionalModel = False)
jump_reg_Instr.addVariable(cxx_writer.writer_code.Variable('trapNotAligned', cxx_writer.writer_code.boolType))
isa.addInstruction(jump_reg_Instr)

# Return from Trap
# N.B. In the reg read stage it writes the values of the SU and ET PSR
# fields??????? TODO: check the stages where the operations are performed:
# is everything performed in the decode stage?
opCodeAll = """newCwp = ((unsigned int)(PSR[key_CWP] + 1)) % NUM_REG_WIN;
exceptionEnabled = PSR[key_ET];
supervisor = PSR[key_S];
invalidWin = ((0x01 << (newCwp)) & WIM) != 0;
notAligned = (targetAddr & 0x00000003) != 0;
if(!exceptionEnabled && supervisor && !invalidWin && !notAligned){
    #ifdef ACC_MODEL
    PC = targetAddr;
    NPC = targetAddr + 4;
    #else
    PC = npcounter;
    NPC = targetAddr;
    #endif
}
"""
opCodeImm = cxx_writer.writer_code.Code(ReadNPCDecode + 'targetAddr = rs1 + SignExtend(simm13, 13);\n' + opCodeAll)
opCodeRegs = cxx_writer.writer_code.Code(ReadNPCDecode + 'targetAddr = rs1 + rs2;\n' + opCodeAll)
opCodeExec = cxx_writer.writer_code.Code("""
if(exceptionEnabled || !supervisor || invalidWin || notAligned){
    flush();
}
else{
    PSR.immediateWrite((PSR & 0xFFFFFF40) | (newCwp | 0x20 | (PSR[key_PS] << 7)));
    stall(2);
}
""")
TrapCode = """
if(exceptionEnabled){
    if(supervisor){
        RaiseException(pcounter, npcounter, ILLEGAL_INSTR);
    }
    else{
        RaiseException(pcounter, npcounter, PRIVILEDGE_INSTR);
    }
}
else if(!supervisor || invalidWin || notAligned){
    THROW_EXCEPTION("Invalid processor mode during execution of the RETT instruction - supervisor: " << supervisor << " newCwp: " << std::hex << std::showbase << newCwp << " targetAddr: " << std::hex << std::showbase << targetAddr);
}
"""
TrapCode += 'else{\n' + updateAliasCode_exception() + '\n}'
opCodeTrap = cxx_writer.writer_code.Code(TrapCode)
opCodeFlush = cxx_writer.writer_code.Code("""
#ifdef ACC_MODEL
if(exceptionEnabled || !supervisor || invalidWin || notAligned){
    flush();
}
#endif
""")
rett_imm_Instr = trap.Instruction('RETT_imm', True, frequency = 2)
rett_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 1, 1, 0, 0, 1]}, ('rett r', '%rs1', '+', '%simm13'))
rett_imm_Instr.setCode(opCodeImm, 'decode')
rett_imm_Instr.setCode(opCodeFlush, 'regs')
rett_imm_Instr.setCode(opCodeExec, 'execute')
rett_imm_Instr.setCode(opCodeFlush, 'memory')
rett_imm_Instr.setCode(opCodeTrap, 'exception')
rett_imm_Instr.setCode(opCodeFlush, 'wb')
rett_imm_Instr.setCode(opCodeReadPC, 'fetch')
rett_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
rett_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
rett_imm_Instr.addVariable(('rs1_op', 'BIT<32>'))
rett_imm_Instr.addVariable(('rs2_op', 'BIT<32>'))
rett_imm_Instr.addVariable(('targetAddr', 'BIT<32>'))
rett_imm_Instr.addVariable(('newCwp', 'BIT<32>'))
rett_imm_Instr.addVariable(cxx_writer.writer_code.Variable('exceptionEnabled', cxx_writer.writer_code.boolType))
rett_imm_Instr.addVariable(cxx_writer.writer_code.Variable('invalidWin', cxx_writer.writer_code.boolType))
rett_imm_Instr.addVariable(cxx_writer.writer_code.Variable('notAligned', cxx_writer.writer_code.boolType))
rett_imm_Instr.addVariable(cxx_writer.writer_code.Variable('supervisor', cxx_writer.writer_code.boolType))
rett_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
rett_imm_Instr.addSpecialRegister('PSR', 'inout')
isa.addInstruction(rett_imm_Instr)
rett_reg_Instr = trap.Instruction('RETT_reg', True, frequency = 2)
rett_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 1, 1, 0, 0, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('rett r', '%rs1', '+r', '%rs2'))
rett_reg_Instr.setCode(opCodeRegs, 'decode')
rett_reg_Instr.setCode(opCodeFlush, 'regs')
rett_reg_Instr.setCode(opCodeExec, 'execute')
rett_reg_Instr.setCode(opCodeFlush, 'memory')
rett_reg_Instr.setCode(opCodeTrap, 'exception')
rett_reg_Instr.setCode(opCodeFlush, 'wb')
rett_reg_Instr.setCode(opCodeReadPC, 'fetch')
rett_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
rett_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
rett_reg_Instr.addVariable(('rs1_op', 'BIT<32>'))
rett_reg_Instr.addVariable(('rs2_op', 'BIT<32>'))
rett_reg_Instr.addVariable(('targetAddr', 'BIT<32>'))
rett_reg_Instr.addVariable(('newCwp', 'BIT<32>'))
rett_reg_Instr.addVariable(cxx_writer.writer_code.Variable('exceptionEnabled', cxx_writer.writer_code.boolType))
rett_reg_Instr.addVariable(cxx_writer.writer_code.Variable('invalidWin', cxx_writer.writer_code.boolType))
rett_reg_Instr.addVariable(cxx_writer.writer_code.Variable('notAligned', cxx_writer.writer_code.boolType))
rett_reg_Instr.addVariable(cxx_writer.writer_code.Variable('supervisor', cxx_writer.writer_code.boolType))
rett_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
rett_reg_Instr.addSpecialRegister('PSR', 'inout')
isa.addInstruction(rett_reg_Instr)

# Trap on Integer Condition Code; note this instruction also receives the forwarding
# of the PSR, the same as the branch instruction
opCode = cxx_writer.writer_code.Code(ReadNPCDecode + """
#ifndef ACC_MODEL
bool icc_z = PSR[key_ICC_z];
bool icc_n = PSR[key_ICC_n];
bool icc_v = PSR[key_ICC_v];
bool icc_c = PSR[key_ICC_c];
#else
bool icc_z = PSR_execute[key_ICC_z];
bool icc_n = PSR_execute[key_ICC_n];
bool icc_v = PSR_execute[key_ICC_v];
bool icc_c = PSR_execute[key_ICC_c];
#endif

//While TRAP normally stops simulation when the _exit routine is encountered, TSIM stops simulation
//when a TA instruction is encountered (no matter what the argument of TA is)
#ifdef TSIM_COMPATIBILITY
if(cond == 0x8){
    std::cerr << std::endl << "Simulation stopped by a TA instruction" << std::endl << std::endl;
    sc_stop();
    wait(SC_ZERO_TIME);
}
#endif

raiseException = (cond == 0x8) ||
            ((cond == 0x9) && !icc_z) ||
            ((cond == 0x1) && icc_z) ||
            ((cond == 0xa) && !icc_z && (icc_n == icc_v)) ||
            ((cond == 0x2) && (icc_z || (icc_n != icc_v))) ||
            ((cond == 0xb) && (icc_n == icc_v)) ||
            ((cond == 0x3) && (icc_n != icc_v)) ||
            ((cond == 0xc) && !icc_c && !icc_z) ||
            ((cond == 0x4) && (icc_c || icc_z)) ||
            ((cond == 0xd) && !icc_c) ||
            ((cond == 0x5) && icc_c) ||
            ((cond == 0xe) && !icc_n) ||
            ((cond == 0x6) && icc_n) ||
            ((cond == 0xf) && !icc_v) ||
            ((cond == 0x7) && icc_v);
""")
opCodeTrapImm = cxx_writer.writer_code.Code("""
if(raiseException){
    stall(4);
    RaiseException(pcounter, npcounter, TRAP_INSTRUCTION, (rs1 + SignExtend(imm7, 7)) & 0x0000007F);
}
#ifndef ACC_MODEL
else{
    PC = npcounter;
    NPC = npcounter + 4;
}
#endif
""")
opCodeTrapReg = cxx_writer.writer_code.Code("""
if(raiseException){
    stall(4);
    RaiseException(pcounter, npcounter, TRAP_INSTRUCTION, (rs1 + rs2) & 0x0000007F);
}
#ifndef ACC_MODEL
else{
    PC = npcounter;
    NPC = npcounter + 4;
}
#endif
""")
trap_imm_Instr = trap.Instruction('TRAP_imm', True, frequency = 1)
trap_imm_Instr.setMachineCode(ticc_format2, {'op3': [1, 1, 1, 0, 1, 0]},
('t', ('%cond', {int('1000', 2) : 'a',
int('0000', 2) : 'n', int('1001', 2) : 'ne', int('0001', 2) : 'e', int('1010', 2) : 'g', int('0010', 2) : 'le',
int('1011', 2) : 'ge', int('0011', 2) : 'l', int('1100', 2) : 'gu', int('0100', 2) : 'leu', int('1101', 2) : 'cc',
int('01010', 2) : 'cs', int('1110', 2) : 'pos', int('0110', 2) : 'neg', int('1111', 2) : 'vc', int('0111', 2) : 'vs',}),
' r', '%rs1', '+', '%imm7'))
trap_imm_Instr.setCode(opCode, 'decode')
trap_imm_Instr.setCode(opCodeTrapImm, 'exception')
trap_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False, functionalModel = False)
trap_imm_Instr.setCode(opCodeReadPC, 'fetch')
trap_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
trap_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
trap_imm_Instr.addSpecialRegister('PSR', 'in', 'execute')
trap_imm_Instr.addVariable(cxx_writer.writer_code.Variable('raiseException', cxx_writer.writer_code.boolType))
isa.addInstruction(trap_imm_Instr)
trap_reg_Instr = trap.Instruction('TRAP_reg', True, frequency = 1)
trap_reg_Instr.setMachineCode(ticc_format1, {'op3': [1, 1, 1, 0, 1, 0]},
('t', ('%cond', {int('1000', 2) : 'a',
int('0000', 2) : 'n', int('1001', 2) : 'ne', int('0001', 2) : 'e', int('1010', 2) : 'g', int('0010', 2) : 'le',
int('1011', 2) : 'ge', int('0011', 2) : 'l', int('1100', 2) : 'gu', int('0100', 2) : 'leu', int('1101', 2) : 'cc',
int('01010', 2) : 'cs', int('1110', 2) : 'pos', int('0110', 2) : 'neg', int('1111', 2) : 'vc', int('0111', 2) : 'vs',}),
' r', '%rs1', '+r', '%rs2'))
trap_reg_Instr.setCode(opCode, 'decode')
trap_reg_Instr.setCode(opCodeTrapReg, 'exception')
trap_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False, functionalModel = False)
trap_reg_Instr.setCode(opCodeReadPC, 'fetch')
trap_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
trap_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
trap_reg_Instr.addSpecialRegister('PSR', 'in', 'execute')
trap_reg_Instr.addVariable(cxx_writer.writer_code.Variable('raiseException', cxx_writer.writer_code.boolType))
isa.addInstruction(trap_reg_Instr)

# Read State Register
opCodeRegs = cxx_writer.writer_code.Code("""
y_temp = Y;
""")
opCodeWb = cxx_writer.writer_code.Code("""
rd = y_temp;
""")
readY_Instr = trap.Instruction('READy', True, frequency = 1)
readY_Instr.setMachineCode(read_special_format, {'op3': [1, 0, 1, 0, 0, 0], 'asr': [0, 0, 0, 0, 0]},
('rd ', 'y', ' r', '%rd'), subInstr = True)
readY_Instr.setCode(opCodeRegs, 'regs')
readY_Instr.setCode(opCodeWb, 'wb')
readY_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
readY_Instr.addVariable(('y_temp', 'BIT<32>'))
readY_Instr.addSpecialRegister('Y', 'in')
isa.addInstruction(readY_Instr)
opCodeRegs = cxx_writer.writer_code.Code("""
asr_temp = ASR[asr];
""")
opCodeWb = cxx_writer.writer_code.Code("""
rd = asr_temp;
""")
readASR_Instr = trap.Instruction('READasr', True, frequency = 1)
readASR_Instr.setMachineCode(read_special_format, {'op3': [1, 0, 1, 0, 0, 0]}, ('rd asr ', '%asr', ' r', '%rd'))
readASR_Instr.setCode(opCodeRegs, 'regs')
readASR_Instr.setCode(opCodeWb, 'wb')
readASR_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
readASR_Instr.addVariable(('asr_temp', 'BIT<32>'))
isa.addInstruction(readASR_Instr)
opCodeRegs = cxx_writer.writer_code.Code("""
#ifdef ACC_MODEL
psr_temp = PSR_execute;
#else
psr_temp = PSR;
#endif
supervisor = (psr_temp & 0x00000080) != 0;
""")
opCodeTrap = cxx_writer.writer_code.Code("""
if(!supervisor){
    RaiseException(pcounter, npcounter, PRIVILEDGE_INSTR);
}
""")
opCodeWb = cxx_writer.writer_code.Code("""
rd = psr_temp;
""")
readPsr_Instr = trap.Instruction('READpsr', True, frequency = 2)
readPsr_Instr.setMachineCode(read_special_format, {'op3': [1, 0, 1, 0, 0, 1]}, ('rd ', 'psr r', '%rd'))
readPsr_Instr.setCode(opCodeRegs, 'regs')
readPsr_Instr.setCode(opCodeTrap, 'exception')
readPsr_Instr.setCode(opCodeWb, 'wb')
readPsr_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
readPsr_Instr.setCode(opCodeReadPC, 'fetch')
readPsr_Instr.setCode(opCodeReadNPC, 'decode')
readPsr_Instr.addVariable(('pcounter', 'BIT<32>'))
readPsr_Instr.addVariable(('npcounter', 'BIT<32>'))
readPsr_Instr.addVariable(cxx_writer.writer_code.Variable('supervisor', cxx_writer.writer_code.boolType))
readPsr_Instr.addVariable(('psr_temp', 'BIT<32>'))
isa.addInstruction(readPsr_Instr)
opCodeRegs = cxx_writer.writer_code.Code("""
wim_temp = WIM;
supervisor = PSR[key_S];
""")
opCodeTrap = cxx_writer.writer_code.Code("""
if(!supervisor){
    RaiseException(pcounter, npcounter, PRIVILEDGE_INSTR);
}
""")
opCodeWb = cxx_writer.writer_code.Code("""
rd = wim_temp;
""")
readWim_Instr = trap.Instruction('READwim', True, frequency = 1)
readWim_Instr.setMachineCode(read_special_format, {'op3': [1, 0, 1, 0, 1, 0]}, ('rd ', 'wim r', '%rd'))
readWim_Instr.setCode(opCodeRegs, 'regs')
readWim_Instr.setCode(opCodeTrap, 'exception')
readWim_Instr.setCode(opCodeWb, 'wb')
readWim_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
readWim_Instr.setCode(opCodeReadPC, 'fetch')
readWim_Instr.setCode(opCodeReadNPC, 'decode')
readWim_Instr.addVariable(('pcounter', 'BIT<32>'))
readWim_Instr.addVariable(('npcounter', 'BIT<32>'))
readWim_Instr.addVariable(cxx_writer.writer_code.Variable('supervisor', cxx_writer.writer_code.boolType))
readWim_Instr.addVariable(('wim_temp', 'BIT<32>'))
isa.addInstruction(readWim_Instr)
opCodeRegs = cxx_writer.writer_code.Code("""
tbr_temp = TBR;
supervisor = PSR[key_S];
""")
opCodeTrap = cxx_writer.writer_code.Code("""
if(!supervisor){
    RaiseException(pcounter, npcounter, PRIVILEDGE_INSTR);
}
""")
opCodeWb = cxx_writer.writer_code.Code("""
rd = tbr_temp;
""")
readTbr_Instr = trap.Instruction('READtbr', True, frequency = 1)
readTbr_Instr.setMachineCode(read_special_format, {'op3': [1, 0, 1, 0, 1, 1]}, ('rd ', 'tbr r', '%rd'))
readTbr_Instr.setCode(opCodeRegs, 'regs')
readTbr_Instr.setCode(opCodeTrap, 'exception')
readTbr_Instr.setCode(opCodeWb, 'wb')
readTbr_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
readTbr_Instr.setCode(opCodeReadPC, 'fetch')
readTbr_Instr.setCode(opCodeReadNPC, 'decode')
readTbr_Instr.addVariable(('pcounter', 'BIT<32>'))
readTbr_Instr.addVariable(('npcounter', 'BIT<32>'))
readTbr_Instr.addVariable(cxx_writer.writer_code.Variable('supervisor', cxx_writer.writer_code.boolType))
readTbr_Instr.addVariable(('tbr_temp', 'BIT<32>'))
isa.addInstruction(readTbr_Instr)

# Write State Register
opCodeXorR = cxx_writer.writer_code.Code("""
result = rs1 ^ rs2;
""")
opCodeXorI = cxx_writer.writer_code.Code("""
result = rs1 ^ SignExtend(simm13, 13);
""")
opCodeExec = cxx_writer.writer_code.Code("""
Y = result;
""")
writeY_reg_Instr = trap.Instruction('WRITEY_reg', True, frequency = 1)
writeY_reg_Instr.setMachineCode(write_special_format1, {'op3': [1, 1, 0, 0, 0, 0], 'rd': [0, 0, 0, 0, 0]}, ('wr r', '%rs1', ' r', '%rs2', ' y'), subInstr = True)
writeY_reg_Instr.setCode(opCodeXorR, 'regs')
writeY_reg_Instr.setCode(opCodeExec, 'execute')
writeY_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
writeY_reg_Instr.addSpecialRegister('Y', 'out', 'execute')
writeY_reg_Instr.addVariable(('result', 'BIT<32>'))
isa.addInstruction(writeY_reg_Instr)
writeY_imm_Instr = trap.Instruction('WRITEY_imm', True, frequency = 2)
writeY_imm_Instr.setMachineCode(write_special_format2, {'op3': [1, 1, 0, 0, 0, 0], 'rd': [0, 0, 0, 0, 0]}, ('wr r', '%rs1', ' ', '%simm13', ' y'), subInstr = True)
writeY_imm_Instr.setCode(opCodeXorI, 'regs')
writeY_imm_Instr.setCode(opCodeExec, 'execute')
writeY_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
writeY_imm_Instr.addSpecialRegister('Y', 'out', 'execute')
writeY_imm_Instr.addVariable(('result', 'BIT<32>'))
isa.addInstruction(writeY_imm_Instr)
opCodeWb = cxx_writer.writer_code.Code("""
ASR[rd] = result;
""")
writeASR_reg_Instr = trap.Instruction('WRITEasr_reg', True, frequency = 1)
writeASR_reg_Instr.setMachineCode(write_special_format1, {'op3': [1, 1, 0, 0, 0, 0]}, ('wr r', '%rs1', ' r', '%rs2', ' asr', '%rd'))
writeASR_reg_Instr.setCode(opCodeXorR, 'regs')
writeASR_reg_Instr.setCode(opCodeExec, 'execute')
writeASR_reg_Instr.setCode(opCodeWb, 'wb')
writeASR_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
writeASR_reg_Instr.addVariable(('result', 'BIT<32>'))
isa.addInstruction(writeASR_reg_Instr)
writeASR_imm_Instr = trap.Instruction('WRITEasr_imm', True, frequency = 1)
writeASR_imm_Instr.setMachineCode(write_special_format2, {'op3': [1, 1, 0, 0, 0, 0]}, ('wr r', '%rs1', ' ', '%simm13', ' asr', '%rd'))
writeASR_imm_Instr.setCode(opCodeXorI, 'regs')
writeASR_imm_Instr.setCode(opCodeExec, 'execute')
writeASR_imm_Instr.setCode(opCodeWb, 'wb')
writeASR_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
writeASR_imm_Instr.addVariable(('result', 'BIT<32>'))
isa.addInstruction(writeASR_imm_Instr)

# ############################TODO: With respect to exceptions, the program counter appears to be written immediately:
# this means that exceptions has to see the new value of the program counter ####################################
opCodeXorR = cxx_writer.writer_code.Code("""
// Note how we filter writes to EF and EC fields since we do not
// have neither a co-processor nor the FPU
result = ((rs1 ^ rs2) & 0x00FFCFFF) | 0xF3000000;
supervisorException = (PSR[key_S] == 0);
illegalCWP = (result & 0x0000001f) >= NUM_REG_WIN;
""")
opCodeXorI = cxx_writer.writer_code.Code("""
// Note how we filter writes to EF and EC fields since we do not
// have neither a co-processor nor the FPU
result = ((rs1 ^ SignExtend(simm13, 13)) & 0x00FFCFFF) | 0xF3000000;
supervisorException = (PSR[key_S] == 0);
illegalCWP = (result & 0x0000001f) >= NUM_REG_WIN;
""")
opCodeExec = cxx_writer.writer_code.Code("""
if(!(supervisorException || illegalCWP)){
    PSR = result;
}
""")
opCodeTrap = cxx_writer.writer_code.Code("""
if(supervisorException){
    RaiseException(pcounter, npcounter, PRIVILEDGE_INSTR);
}
if(illegalCWP){
    RaiseException(pcounter, npcounter, ILLEGAL_INSTR);
}
""")
writePsr_reg_Instr = trap.Instruction('WRITEpsr_reg', True, frequency = 2)
writePsr_reg_Instr.setMachineCode(write_special_format1, {'op3': [1, 1, 0, 0, 0, 1]}, ('wr r', '%rs1', ' r', '%rs2', ' psr'))
writePsr_reg_Instr.setCode(opCodeXorR, 'regs')
writePsr_reg_Instr.setCode(opCodeExec, 'execute')
writePsr_reg_Instr.setCode(opCodeTrap, 'exception')
writePsr_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
writePsr_reg_Instr.setCode(opCodeReadPC, 'fetch')
writePsr_reg_Instr.setCode(opCodeReadNPC, 'decode')
writePsr_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
writePsr_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
writePsr_reg_Instr.addSpecialRegister('PSR', 'out', 'execute')
writePsr_reg_Instr.addVariable(cxx_writer.writer_code.Variable('supervisorException', cxx_writer.writer_code.boolType))
writePsr_reg_Instr.addVariable(cxx_writer.writer_code.Variable('illegalCWP', cxx_writer.writer_code.boolType))
writePsr_reg_Instr.addVariable(('result', 'BIT<32>'))
isa.addInstruction(writePsr_reg_Instr)
writePsr_imm_Instr = trap.Instruction('WRITEpsr_imm', True, frequency = 2)
writePsr_imm_Instr.setMachineCode(write_special_format2, {'op3': [1, 1, 0, 0, 0, 1]}, ('wr r', '%rs1', ' ', '%simm13', ' psr'))
writePsr_imm_Instr.setCode(opCodeXorI, 'regs')
writePsr_imm_Instr.setCode(opCodeExec, 'execute')
writePsr_imm_Instr.setCode(opCodeTrap, 'exception')
writePsr_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
writePsr_imm_Instr.setCode(opCodeReadPC, 'fetch')
writePsr_imm_Instr.setCode(opCodeReadNPC, 'decode')
writePsr_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
writePsr_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
writePsr_imm_Instr.addSpecialRegister('PSR', 'out', 'execute')
writePsr_imm_Instr.addVariable(cxx_writer.writer_code.Variable('supervisorException', cxx_writer.writer_code.boolType))
writePsr_imm_Instr.addVariable(cxx_writer.writer_code.Variable('illegalCWP', cxx_writer.writer_code.boolType))
writePsr_imm_Instr.addVariable(('result', 'BIT<32>'))
isa.addInstruction(writePsr_imm_Instr)
opCodeXorR = cxx_writer.writer_code.Code("""
result = rs1 ^ rs2;
raiseException = (PSR[key_S] == 0);
""")
opCodeXorI = cxx_writer.writer_code.Code("""
result = rs1 ^ SignExtend(simm13, 13);
raiseException = (PSR[key_S] == 0);
""")
opCodeWb = cxx_writer.writer_code.Code("""
if(!raiseException){
    WIM = result & ((unsigned int)0xFFFFFFFF >> (32 - NUM_REG_WIN));
}
""")
opCodeTrap = cxx_writer.writer_code.Code("""
if(raiseException){
    RaiseException(pcounter, npcounter, PRIVILEDGE_INSTR);
}
""")
writeWim_reg_Instr = trap.Instruction('WRITEwim_reg', True, frequency = 1)
writeWim_reg_Instr.setMachineCode(write_special_format1, {'op3': [1, 1, 0, 0, 1, 0]}, ('wr r', '%rs1', ' r', '%rs2', ' wim'))
writeWim_reg_Instr.setCode(opCodeXorR, 'regs')
writeWim_reg_Instr.setCode(opCodeTrap, 'exception')
writeWim_reg_Instr.setCode(opCodeWb, 'wb')
writeWim_reg_Instr.setCode(opCodeReadPC, 'fetch')
writeWim_reg_Instr.setCode(opCodeReadNPC, 'decode')
writeWim_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
writeWim_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
writeWim_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
writeWim_reg_Instr.addVariable(cxx_writer.writer_code.Variable('raiseException', cxx_writer.writer_code.boolType))
writeWim_reg_Instr.addVariable(('result', 'BIT<32>'))
isa.addInstruction(writeWim_reg_Instr)
writeWim_imm_Instr = trap.Instruction('WRITEwim_imm', True, frequency = 1)
writeWim_imm_Instr.setMachineCode(write_special_format2, {'op3': [1, 1, 0, 0, 1, 0]}, ('wr r', '%rs1', ' ', '%simm13', ' wim'))
writeWim_imm_Instr.setCode(opCodeXorI, 'regs')
writeWim_imm_Instr.setCode(opCodeTrap, 'exception')
writeWim_imm_Instr.setCode(opCodeWb, 'wb')
writeWim_imm_Instr.setCode(opCodeReadPC, 'fetch')
writeWim_imm_Instr.setCode(opCodeReadNPC, 'decode')
writeWim_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
writeWim_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
writeWim_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
writeWim_imm_Instr.addVariable(cxx_writer.writer_code.Variable('raiseException', cxx_writer.writer_code.boolType))
writeWim_imm_Instr.addVariable(('result', 'BIT<32>'))
isa.addInstruction(writeWim_imm_Instr)
opCodeWb = cxx_writer.writer_code.Code("""
if(!raiseException){
    TBR |= (result & 0xFFFFF000);
}
""")
writeTbr_reg_Instr = trap.Instruction('WRITEtbr_reg', True, frequency = 1)
writeTbr_reg_Instr.setMachineCode(write_special_format1, {'op3': [1, 1, 0, 0, 1, 1]}, ('wr r', '%rs1', ' r', '%rs2', ' tbr'))
writeTbr_reg_Instr.setCode(opCodeXorR, 'regs')
writeTbr_reg_Instr.setCode(opCodeTrap, 'exception')
writeTbr_reg_Instr.setCode(opCodeWb, 'wb')
writeTbr_reg_Instr.setCode(opCodeReadPC, 'fetch')
writeTbr_reg_Instr.setCode(opCodeReadNPC, 'decode')
writeTbr_reg_Instr.addVariable(('pcounter', 'BIT<32>'))
writeTbr_reg_Instr.addVariable(('npcounter', 'BIT<32>'))
writeTbr_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
writeTbr_reg_Instr.addVariable(cxx_writer.writer_code.Variable('raiseException', cxx_writer.writer_code.boolType))
writeTbr_reg_Instr.addVariable(('result', 'BIT<32>'))
isa.addInstruction(writeTbr_reg_Instr)
writeTbr_imm_Instr = trap.Instruction('WRITEtbr_imm', True, frequency = 1)
writeTbr_imm_Instr.setMachineCode(write_special_format2, {'op3': [1, 1, 0, 0, 1, 1]}, ('wr r', '%rs1', ' ', '%simm13', ' tbr'))
writeTbr_imm_Instr.setCode(opCodeXorI, 'regs')
writeTbr_imm_Instr.setCode(opCodeTrap, 'exception')
writeTbr_imm_Instr.setCode(opCodeReadPC, 'fetch')
writeTbr_imm_Instr.setCode(opCodeReadNPC, 'decode')
writeTbr_imm_Instr.addVariable(('pcounter', 'BIT<32>'))
writeTbr_imm_Instr.addVariable(('npcounter', 'BIT<32>'))
writeTbr_imm_Instr.setCode(opCodeWb, 'wb')
writeTbr_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
writeTbr_imm_Instr.addVariable(cxx_writer.writer_code.Variable('raiseException', cxx_writer.writer_code.boolType))
writeTbr_imm_Instr.addVariable(('result', 'BIT<32>'))
isa.addInstruction(writeTbr_imm_Instr)

## Store Barrier
opCode = cxx_writer.writer_code.Code("""
""")
stbar_Instr = trap.Instruction('STBAR', True, frequency = 1)
stbar_Instr.setMachineCode(stbar_format, {}, ('stbar'), subInstr = True)
stbar_Instr.setCode(opCode, 'execute')
stbar_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
isa.addInstruction(stbar_Instr)

# Unimplemented Instruction
opCode = cxx_writer.writer_code.Code("""
RaiseException(pcounter, npcounter, ILLEGAL_INSTR);
""")
unimpl_Instr = trap.Instruction('UNIMP', True, frequency = 1)
unimpl_Instr.setMachineCode(b_sethi_format1, {'op2' : [0, 0, 0]}, ('unimp ', '%imm22'))
unimpl_Instr.setCode(opCode, 'exception')
unimpl_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
unimpl_Instr.setCode(opCodeReadPC, 'fetch')
unimpl_Instr.setCode(opCodeReadNPC, 'decode')
unimpl_Instr.addVariable(('pcounter', 'BIT<32>'))
unimpl_Instr.addVariable(('npcounter', 'BIT<32>'))
unimpl_Instr.removeLockRegRegister('rd')
isa.addInstruction(unimpl_Instr)

# Flush Memory
opCode = cxx_writer.writer_code.Code("""
""")
flush_reg_Instr = trap.Instruction('FLUSH_reg', True, frequency = 1)
flush_reg_Instr.setMachineCode(dpi_format1, {'op3': [1, 1, 1, 0, 1, 1], 'asi' : [0, 0, 0, 0, 0, 0, 0, 0]}, ('flush r', '%rs1', '+r', '%rs2'))
flush_reg_Instr.setCode(opCode, 'execute')
flush_reg_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
flush_reg_Instr.removeLockRegRegister('rd')
isa.addInstruction(flush_reg_Instr)
flush_imm_Instr = trap.Instruction('FLUSH_imm', True, frequency = 1)
flush_imm_Instr.setMachineCode(dpi_format2, {'op3': [1, 1, 1, 0, 1, 1]}, ('flush r', '%rs1', '+', '%simm13'))
flush_imm_Instr.setCode(opCode, 'execute')
flush_imm_Instr.addBehavior(IncrementPC, 'fetch', pre = False)
flush_imm_Instr.removeLockRegRegister('rd')
isa.addInstruction(flush_imm_Instr)
