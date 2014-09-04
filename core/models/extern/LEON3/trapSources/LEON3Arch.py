# -*- coding: iso-8859-1 -*-

# Nice ascii art banner for the header of the generated files
banner = r"""
      _/        _/_/_/_/    _/_/    _/      _/   _/_/_/
     _/        _/        _/    _/  _/_/    _/         _/
    _/        _/_/_/    _/    _/  _/  _/  _/     _/_/
   _/        _/        _/    _/  _/    _/_/         _/
  _/_/_/_/  _/_/_/_/    _/_/    _/      _/   _/_/_/
"""

import sys

# For easy of use, I add the possibility of specifying command line parameters to tune
# the processor creation steps without having to modify this script
destFolderName = 'processor'
standalone = False
if len(sys.argv) > 1:
    destFolderName = sys.argv[1]
if len(sys.argv) > 2 and sys.argv[2].lower() == 'standalone':
    standalone = True

# Lets first of all import the necessary files for the
# creation of the processor; note that if the trap modules are
# not in the default search path I have to manually specify the path
try:
    import trap
except ImportError:
    import sys, os
    sys.path.append(os.path.abspath(os.path.join('..', '..')))
    try:
        import trap
    except ImportError:
        print ('Please specify in file LEON3Arch.py the path where the core TRAP files are located')

import cxx_writer

# It is nice to keep the ISA and the architecture separated
# so we use the import trick
import LEON3Isa
import LEON3Tests
from LEONDefs import *
# Code used to move to a new register window
from LEON3Methods import updateAliasCode_exception
from LEON3Methods import updateAliasCode_abi

# Lets now start building the processor
processor = trap.Processor('LEON3', version = '0.3', systemc = True, instructionCache = True, cacheLimit = 256)
processor.setIpRights('gpl', 'Luca Fossati', 'fossati.l@gmail.com', banner)
processor.setBigEndian() # big endian
processor.setWordsize(4, 8) # 4 bytes per word, 8 bits per byte
processor.setISA(LEON3Isa.isa) # lets set the instruction set

# Lets customize the compilation options: actually it is possible only to define pre-processor macros;
# For example the following directive means that if the tsim-comp compilation option is activated,
# the TSIM_COMPATIBILITY macro will be defined
processor.setPreProcMacro('tsim-comp', 'TSIM_COMPATIBILITY')

# Also we need to initialize the processor status to default values when TSIM
# compatibility mode is used, as this is not done by software (i.e. not done by
# the BSP contained in the BCC compiler)
processor.setBeginOperation("""#ifdef TSIM_COMPATIBILITY
PSR.immediateWrite(0xf30000E0L);
WIM.immediateWrite(2);
#endif""")

# Ok, now we move to the description of more complicated processor
# resources

# Here I add a constant to the instruction set so that it can be used from the code implementing
# the various instructions
# Number of defined register windows
LEON3Isa.isa.addConstant(cxx_writer.writer_code.uintType, 'NUM_REG_WIN', numRegWindows)
# Specifies whether the multiplier unit is pipelined or not
if pipelinedMult:
    LEON3Isa.isa.addConstant(cxx_writer.writer_code.boolType, 'PIPELINED_MULT', 'true')
else:
    LEON3Isa.isa.addConstant(cxx_writer.writer_code.boolType, 'PIPELINED_MULT', 'false')
if multiplier_size == 16:
    LEON3Isa.isa.addDefines('#define MULT_SIZE_16')

# There are 8 global register, and a variable number of
# of 16-registers set; this number depends on the number of
# register windows
# global registers
globalRegs = trap.RegisterBank('GLOBAL', 8, 32)
globalRegs.setConst(0, 0)
processor.addRegBank(globalRegs)
# Register sets
windowRegs = trap.RegisterBank('WINREGS', 16*numRegWindows, 32)
processor.addRegBank(windowRegs)
# Program status register
psrBitMask = {'IMPL': (28, 31), 'VER': (24, 27), 'ICC_n': (23, 23), 'ICC_z': (22, 22), 'ICC_v': (21, 21), 'ICC_c': (20, 20), 'EC': (13, 13), 'EF': (12, 12), 'PIL': (8, 11), 'S': (7, 7), 'PS': (6, 6), 'ET': (5, 5), 'CWP': (0, 4)}
psrReg = trap.Register('PSR', 32, psrBitMask)
psrReg.setDefaultValue(0xF3000080)
#psrReg.setDelay(3)
processor.addRegister(psrReg)
# Window Invalid Mask Register
wimBitMask = {}
for i in range(0, 32):
    wimBitMask['WIM_' + str(i)] = (i, i)
wimReg = trap.Register('WIM', 32, wimBitMask)
wimReg.setDefaultValue(0)
#wimReg.setDelay(3)
processor.addRegister(wimReg)
# Trap Base Register
tbrBitMask = {'TBA' : (12, 31), 'TT' : (4, 11)}
tbrReg = trap.Register('TBR', 32, tbrBitMask)
tbrReg.setDefaultValue(0)
processor.addRegister(tbrReg)
# Multiply / Divide Register
yReg = trap.Register('Y', 32)
#yReg.setDelay(3)
processor.addRegister(yReg)
# Program Counter
pcReg = trap.Register('PC', 32)
pcReg.setDefaultValue('ENTRY_POINT')
pcReg.setWbStageOrder(['exception', 'decode', 'fetch'])
processor.addRegister(pcReg)
# Program Counter
npcReg = trap.Register('NPC', 32)
npcReg.setDefaultValue(('ENTRY_POINT', 4))
npcReg.setWbStageOrder(['exception', 'decode', 'fetch'])
processor.addRegister(npcReg)
# Ancillary State Registers
# in the LEON3 processor some of them have a special meaning:
# 24-31 are used for hardware breakpoints
# 17 is the processor configuration register
asrRegs = trap.RegisterBank('ASR', 32, 32)
# here I set the default value for the processor configuration register
# (see page 24 of LEON3 preliminary datasheed)
asrRegs.setDefaultValue(('MPROC_ID', 0x00000300 + (numRegWindows -1)), 17)
#asrRegs.setGlobalDelay(3)
processor.addRegBank(asrRegs)

# Now I set the alias: they can (and will) be used by the instructions
# to access the registers more easily. Note that, in general, it is
# responsibility of the programmer keeping the aliases updated
regs = trap.AliasRegBank('REGS', 32, ('GLOBAL[0-7]', 'WINREGS[0-23]'))
regs.setFixed([0, 1, 2, 3, 4, 5, 6, 7])
regs.setCheckGroup()
processor.addAliasRegBank(regs)
FP = trap.AliasRegister('FP', 'REGS[30]')
FP.setFixed()
processor.addAliasReg(FP)
LR = trap.AliasRegister('LR', 'REGS[31]')
LR.setFixed()
processor.addAliasReg(LR)
SP = trap.AliasRegister('SP', 'REGS[14]')
SP.setFixed()
processor.addAliasReg(SP)
PCR = trap.AliasRegister('PCR', 'ASR[17]')
PCR.setFixed()
processor.addAliasReg(PCR)

# Now I add the registers which I want to see printed in the instruction trace
# COMMENT FOR COMPARISON
#LEON3Isa.isa.addTraceRegister(pcReg)
#LEON3Isa.isa.addTraceRegister(npcReg)
LEON3Isa.isa.addTraceRegister(psrReg)
LEON3Isa.isa.addTraceRegister(regs)
LEON3Isa.isa.addTraceRegister(tbrReg)
LEON3Isa.isa.addTraceRegister(wimReg)

# Register from which the instructions are fetched; note that in the
# functional model there is an offset between the PC and the actual
# fetch address (all of this is to take into account the fact that we do
# not have the pipeline)
processor.setFetchRegister('PC')

# Lets now add details about the processor interconnection (i.e. memory ports,
# interrupt ports, pins, etc.)
if standalone:
    processor.setMemory('dataMem', 10*1024*1024)
else:
    processor.addTLMPort('instrMem', True)
    processor.addTLMPort('dataMem')
    # Here I set the properties of the TLM memory: note that they are only used
    # to instantiate a fake memory in the simulator main file, but they do not
    # affect at all the processor core: i.e. once the processor is plugged in a
    # system-level simulator, they have no effect at all
    processor.setTLMMem(10*1024*1024, 0, True)
#processor.setMemory('dataMem', 10*1024*1024, True, 'PC')

# It PSR[ET] == 0 I do not do anything; else
# I check the interrupt level, if == 15 or > PSR[PIL] I service the interrupt,
# The interrupt level is carried by the value at the interrupt port
# Otherwise it is treated like a normal exception, to I jump to the
# TBR: we can use a routine for this ...
# At the end I have to acknowledge the interrupt, by writing on the ack
# port.
irqPort = trap.Interrupt('IRQ', 32)
irqPort.setOperation("""
pcounter = PC;
#ifndef ACC_MODEL
npcounter = NPC;
#endif
""", 'fetch')
irqPort.setOperation("""
#ifdef ACC_MODEL
npcounter = PC;
#endif
""", 'decode')
irqPort.setOperation("""//Basically, what I have to do when
//an interrupt arrives is very simple: we check that interrupts
//are enabled and that the the processor can take this interrupt
//(valid interrupt level). The we simply raise an exception and
//acknowledge the IRQ on the irqAck port.
//All of this can be simply done by calling the
//RaiseException method
// Note that 38 corresponds to the highest defined exception
// (IMPL_DEP_EXC): this because interrupt 1 has id 37, etc.
RaiseException(pcounter, npcounter, 38 - IRQ);
""", 'exception')
irqPort.setCondition('PSR[key_ET] && (IRQ == 15 || IRQ > PSR[key_PIL])')
irqPort.addVariable(('pcounter', 'BIT<32>'))
irqPort.addVariable(('npcounter', 'BIT<32>'))
# in the IRQ tests I specify first the status of the processor before the
# interrupt, then the status I expect after the execution of the interrupt
irqPort.addTest({'IRQ': 0x1, 'PSR' : 0x00000f20, 'TBR': 0x0, 'PC': 0x0, 'NPC': 0x0}, {'PSR' : 0x00000f20, 'TBR': 0x0, 'PC': 0x0, 'NPC': 0x0})
irqPort.addTest({'IRQ': 0x1, 'PSR' : 0x00000000, 'TBR': 0x0, 'PC': 0x0, 'NPC': 0x0}, {'PSR' : 0x00000000, 'TBR': 0x0, 'PC': 0x0, 'NPC': 0x0})
irqPort.addTest({'IRQ': 0xa, 'PSR' : 0x00000800, 'TBR': 0x0, 'PC': 0x0, 'NPC': 0x0}, {'PSR' : 0x00000800, 'TBR': 0x0, 'PC': 0x0, 'NPC': 0x0})
irqPort.addTest({'IRQ': 0xf, 'PSR' : 0x00000f20, 'TBR': 0x0, 'PC': 0x0, 'NPC': 0x0}, {'PSR' : 0x00000f87, 'TBR': 0x1f0, 'PC': 0x1f0, 'NPC': 0x1f4})
irqPort.addTest({'IRQ': 0xf, 'PSR' : 0x00000fa0, 'TBR': 0x0, 'PC': 0x0, 'NPC': 0x0}, {'PSR' : 0x00000fc7, 'TBR': 0x1f0, 'PC': 0x1f0, 'NPC': 0x1f4})
irqPort.addTest({'IRQ': 0xa, 'PSR' : 0x00000827, 'TBR': 0x80000000, 'PC': 0x0, 'NPC': 0x0}, {'PSR' : 0x00000886, 'TBR': 0x800001a0, 'PC': 0x800001a0, 'NPC': 0x800001a4})
processor.addIrq(irqPort)

# I also need to add the external port which is used to acknowledge
# the interrupt
irqAckPin = trap.Pins('irqAck', 32, inbound = False)
processor.addPin(irqAckPin)

# Now it is time to add the pipeline stages
fetchStage = trap.PipeStage('fetch')
processor.addPipeStage(fetchStage)
decodeStage = trap.PipeStage('decode')
decodeStage.setHazard()
processor.addPipeStage(decodeStage)
regsStage = trap.PipeStage('regs')
processor.addPipeStage(regsStage)
executeStage = trap.PipeStage('execute')
executeStage.setCheckUnknownInstr()
processor.addPipeStage(executeStage)
memoryStage = trap.PipeStage('memory')
processor.addPipeStage(memoryStage)
exceptionStage = trap.PipeStage('exception')
processor.addPipeStage(exceptionStage)
wbStage = trap.PipeStage('wb')
wbStage.setWriteBack()
wbStage.setEndHazard()
processor.addPipeStage(wbStage)

# The ABI is necessary to emulate system calls, personalize the GDB stub and,
# eventually, retarget GCC
abi = trap.ABI('REGS[24]', 'REGS[24-29]', 'PC', 'LR', 'SP', 'FP')
abi.addVarRegsCorrespondence({'REGS[0-31]': (0, 31), 'Y': 64, 'PSR': 65, 'WIM': 66, 'TBR': 67, 'PC': 68, 'NPC': 69})
# ************* TODO ************ Do I need to check for register window over/under -flow even for
# systemcalls ?????
pre_code = """
unsigned int newCwp = ((unsigned int)(PSR[key_CWP] - 1)) % """ + str(numRegWindows) + """;
PSR.immediateWrite((PSR & 0xFFFFFFE0) | newCwp);
"""
pre_code += updateAliasCode_abi()
post_code = """
unsigned int newCwp = ((unsigned int)(PSR[key_CWP] + 1)) % """ + str(numRegWindows) + """;
PSR.immediateWrite((PSR & 0xFFFFFFE0) | newCwp);
"""
post_code += updateAliasCode_abi()
abi.processorID('(ASR[17] & 0xF0000000) >> 28')
abi.setECallPreCode(pre_code)
abi.setECallPostCode(post_code)
abi.returnCall([('PC', 'LR', 8), ('NPC', 'LR', 12)])
abi.addMemory('dataMem')
#abi.setCallInstr([LEON3Isa.call_Instr, None, (LEON3Isa.save_imm_Instr, LEON3Isa.save_reg_Instr)])
abi.setCallInstr([LEON3Isa.call_Instr, None, None])
abi.setReturnCallInstr([(LEON3Isa.restore_imm_Instr, LEON3Isa.restore_reg_Instr, LEON3Isa.jump_imm_Instr, LEON3Isa.jump_reg_Instr), (LEON3Isa.jump_imm_Instr, LEON3Isa.jump_reg_Instr, LEON3Isa.restore_imm_Instr, LEON3Isa.restore_reg_Instr)])
abi.addIgnoreStateReg('ASR[17]')
processor.setABI(abi)

# Finally we can dump the processor on file
#processor.write(folder = 'processor', models = ['funcLT'], dumpDecoderName = 'decoder.dot')
#processor.write(folder = 'processor', models = ['funcLT'], trace = True)
if standalone:
    processor.write(folder = destFolderName, models = ['funcLT'], tests = False)
else:
    processor.write(folder = destFolderName, models = ['funcLT', 'funcAT'], tests = False)
#processor.write(folder = 'processor', models = ['funcAT'], tests = False)
#processor.write(folder = 'processor', models = ['funcLT'], trace = True, tests = False)
#processor.write(folder = 'processor', models = ['funcAT'], trace = False)
#processor.write(folder = 'processor', models = ['funcAT'])
#processor.write(folder = 'processor', models = ['accLT', 'funcLT', 'funcAT'], tests = True)
#processor.write(folder = 'processor', models = ['accLT'], trace = True)
#processor.write(folder = 'processor', models = ['accLT', 'funcAT', 'accAT', 'funcLT'], trace = False)
#processor.write(folder = 'processor', models = ['accLT', 'funcLT'], trace = True, combinedTrace = True)
